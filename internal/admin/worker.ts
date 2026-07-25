// Cloudflare Worker - Camus Admin API
// Deploy to: https://dash.cloudflare.com → Workers & Pages

import { createClient } from '@supabase/supabase-js';

const corsHeaders = {
  'Access-Control-Allow-Origin': '*',
  'Access-Control-Allow-Methods': 'GET, POST, PATCH, DELETE, OPTIONS',
  'Access-Control-Allow-Headers': 'Content-Type, Authorization',
};

export default {
  async fetch(request: Request, env: any) {
    const url = new URL(request.url);
    
    // CORS preflight
    if (request.method === 'OPTIONS') {
      return new Response(null, { headers: corsHeaders });
    }

    const supabase = createClient(env.SUPABASE_URL, env.SUPABASE_ANON_KEY);

    // Admin endpoints
    if (url.pathname.startsWith('/api/admin')) {
      return handleAdmin(request, supabase, env);
    }

    // Public trial endpoint
    if (url.pathname === '/api/trial' && request.method === 'POST') {
      return handleTrial(request, supabase);
    }

    // Key verification (called by cheat)
    if (url.pathname === '/api/verify' && request.method === 'POST') {
      return handleVerify(request, supabase);
    }

    return new Response('Not found', { status: 404, headers: corsHeaders });
  },
};

async function handleAdmin(request: Request, supabase: any, env: any) {
  const url = new URL(request.url);
  const path = url.pathname.replace('/api/admin', '') || '/';

  // Auth check
  const authHeader = request.headers.get('Authorization');
  if (!authHeader?.startsWith('Bearer ')) {
    return json({ error: 'Unauthorized' }, 401);
  }
  
  const token = authHeader.slice(7);
  const { data: { user }, error } = await supabase.auth.getUser(token);
  if (error || !user) return json({ error: 'Invalid token' }, 401);

  const { data: profile } = await supabase.from('users').select('role').eq('id', user.id).maybeSingle();
  if (!profile || profile.role !== 'admin') return json({ error: 'Forbidden' }, 403);

  // Keys
  if (path === '/keys' && request.method === 'GET') {
    const { data, count } = await supabase
      .from('keys')
      .select('*', { count: 'exact' })
      .order('created_at', { ascending: false })
      .limit(50);
    return json({ keys: data, total: count });
  }

  if (path === '/keys' && request.method === 'POST') {
    const body = await request.json();
    const days = body.days || 30;
    const isTrial = body.is_trial || false;
    const notes = body.notes || null;

    // Generate key
    const keyCode = `CAMUS-${generateKeyPart(16)}`;
    const expiresAt = new Date(Date.now() + days * 86400000).toISOString();

    const { data, error } = await supabase.from('keys').insert({
      key_code: keyCode,
      days,
      expires_at: expiresAt,
      is_trial: isTrial,
      notes,
    }).select().single();

    if (error) return json({ error: error.message }, 500);

    await logAudit(supabase, user.id, 'create_key', 'key', data.id, { days, is_trial: isTrial });
    return json({ key: data });
  }

  if (path.match(/^\/keys\/[^/]+$/) && request.method === 'PATCH') {
    const id = path.split('/').pop();
    if (!id) return json({ error: 'Invalid key ID' }, 400);
    const body = await request.json();
    const { data, error } = await supabase.from('keys').update(body).eq('id', id).select().single();
    if (error) return json({ error: error.message }, 500);
    await logAudit(supabase, user.id, body.is_revoked ? 'revoke_key' : 'restore_key', 'key', id, body);
    return json({ key: data });
  }

  if (path.match(/^\/keys\/[^/]+$/) && request.method === 'DELETE') {
    const id = path.split('/').pop();
    if (!id) return json({ error: 'Invalid key ID' }, 400);
    await supabase.from('keys').delete().eq('id', id);
    await logAudit(supabase, user.id, 'delete_key', 'key', id, {});
    return json({ success: true });
  }

  // Users
  if (path === '/users' && request.method === 'GET') {
    const { data } = await supabase.from('users').select('*').order('created_at', { ascending: false });
    return json({ users: data });
  }

  if (path.match(/^\/users\/[^/]+$/) && request.method === 'PATCH') {
    const id = path.split('/').pop();
    if (!id) return json({ error: 'Invalid user ID' }, 400);
    const body = await request.json();
    const { data, error } = await supabase.from('users').update(body).eq('id', id).select().single();
    if (error) return json({ error: error.message }, 500);
    await logAudit(supabase, user.id, body.role === 'admin' ? 'make_admin' : 'remove_admin', 'user', id, body);
    return json({ user: data });
  }

  if (path.match(/^\/users\/[^/]+$/) && request.method === 'DELETE') {
    const id = path.split('/').pop();
    if (!id) return json({ error: 'Invalid user ID' }, 400);
    await supabase.from('users').delete().eq('id', id);
    await logAudit(supabase, user.id, 'delete_user', 'user', id, {});
    return json({ success: true });
  }

  // Trials
  if (path === '/trials' && request.method === 'GET') {
    const { data } = await supabase
      .from('keys')
      .select('*, users!keys_user_id_fkey(email)')
      .eq('is_trial', true)
      .order('created_at', { ascending: false });
    return json({ trials: data });
  }

  if (path.match(/^\/trials\/[^/]+$/) && request.method === 'POST') {
    const id = path.split('/').pop();
    if (!id) return json({ error: 'Invalid trial ID' }, 400);
    const { data, error } = await supabase.from('keys').update({ is_revoked: true }).eq('id', id).select().single();
    if (error) return json({ error: error.message }, 500);
    await logAudit(supabase, user.id, 'revoke_trial', 'key', id, {});
    return json({ key: data });
  }

  // Audit log
  if (path === '/audit' && request.method === 'GET') {
    const { data } = await supabase
      .from('audit_log')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(100);
    return json({ logs: data });
  }

  return json({ error: 'Not found' }, 404);
}

async function handleTrial(request: Request, supabase: any) {
  const body = await request.json();
  const { email, hwid } = body;

  if (!email || !hwid) return json({ error: 'Missing email or hwid' }, 400);

  // Check if user exists
  let user: any;
  const { data: existingUser } = await supabase.from('users').select('*').eq('email', email).maybeSingle();
  user = existingUser;

  if (user) {
    if (user.trial_used) return json({ error: 'Trial already used on this email' }, 400);
    
    // Check HWID
    const { data: hwidBinding } = await supabase.from('hwid_bindings').select('*').eq('hwid', hwid).maybeSingle();
    if (hwidBinding) return json({ error: 'HWID already used for trial' }, 400);

    await supabase.from('users').update({ hwid, trial_used: true }).eq('id', user.id);
  } else {
    // Create new user
    const { data: newUser, error } = await supabase.from('users').insert({
      email,
      hwid,
      trial_used: true,
      role: 'user',
    }).select().single();

    if (error) return json({ error: error.message }, 500);
    user = newUser;
  }

  // Check HWID not bound
  const { data: existingHwid } = await supabase.from('hwid_bindings').select('*').eq('hwid', hwid).maybeSingle();
  if (existingHwid) return json({ error: 'HWID already bound to another account' }, 400);

  // Generate trial key
  const keyCode = `CAMUS-TRIAL-${generateKeyPart(12)}`;
  const expiresAt = new Date(Date.now() + 7 * 86400000).toISOString();

  const { data: key, error } = await supabase.from('keys').insert({
    key_code: keyCode,
    user_id: user.id,
    days: 7,
    expires_at: expiresAt,
    is_trial: true,
    notes: 'Free trial via website',
  }).select().single();

  if (error) return json({ error: error.message }, 500);

  // Bind HWID
  await supabase.from('hwid_bindings').insert({
    hwid,
    user_id: user.id,
    key_id: key.id,
  });

  return json({ key: keyCode, expires: expiresAt });
}

async function handleVerify(request: Request, supabase: any) {
  const body = await request.json();
  const { key, hwid } = body;

  if (!key || !hwid) return json({ status: 'invalid', message: 'Missing key or hwid' }, 400);

  const { data: keyData } = await supabase.from('keys').select('*').eq('key_code', key).maybeSingle();
  if (!keyData) return json({ status: 'invalid', message: 'Key not found' });

  if (keyData.is_revoked) return json({ status: 'revoked', message: 'Key revoked' });

  const now = new Date();
  const expires = new Date(keyData.expires_at);
  if (expires <= now) return json({ status: 'expired', message: 'Key expired' });

  // Check HWID
  if (keyData.hwid && keyData.hwid !== hwid) {
    return json({ status: 'hwid_mismatch', message: 'HWID mismatch' });
  }

  // Bind HWID if not set
  if (!keyData.hwid) {
    await supabase.from('keys').update({ hwid }).eq('id', keyData.id);
    // Also bind in hwid_bindings
    await supabase.from('hwid_bindings').upsert({ hwid, user_id: keyData.user_id, key_id: keyData.id });
  }

  // Update last login
  await supabase.from('keys').update({ last_login: new Date().toISOString() }).eq('id', keyData.id);

  return json({
    status: 'ok',
    expires: keyData.expires_at,
    is_trial: keyData.is_trial,
    days_left: Math.ceil((expires.getTime() - now.getTime()) / 86400000),
  });
}

async function logAudit(supabase: any, adminId: string, action: string, targetType: string, targetId: string, metadata: any) {
  await supabase.from('audit_log').insert({
    admin_id: adminId,
    action,
    target_type: targetType,
    target_id: targetId,
    metadata,
  });
}

function generateKeyPart(length: number) {
  const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
  let result = '';
  for (let i = 0; i < length; i++) {
    result += chars[Math.floor(Math.random() * chars.length)];
  }
  return result;
}

function json(data: any, status = 200) {
  return new Response(JSON.stringify(data), {
    status,
    headers: { ...corsHeaders, 'Content-Type': 'application/json' },
  });
}