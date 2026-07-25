-- Supabase Schema for Camus Admin
-- Run in Supabase SQL Editor

-- Enable UUID extension
create extension if not exists "uuid-ossp";

-- Users table (linked to Supabase Auth)
create table if not exists users (
  id uuid primary key default uuid_generate_v4(),
  email text unique not null,
  hwid text unique,
  trial_used boolean default false,
  role text default 'user' check (role in ('user', 'admin')),
  created_at timestamptz default now(),
  updated_at timestamptz default now()
);

-- Keys table
create table if not exists keys (
  id uuid primary key default uuid_generate_v4(),
  user_id uuid references users(id) on delete set null,
  key_code text unique not null,
  hwid text,
  days int not null default 7,
  expires_at timestamptz,
  is_trial boolean default false,
  is_revoked boolean default false,
  notes text,
  created_at timestamptz default now(),
  updated_at timestamptz default now()
);

-- HWID bindings (for trial enforcement)
create table if not exists hwid_bindings (
  id uuid primary key default uuid_generate_v4(),
  hwid text unique not null,
  user_id uuid references users(id) on delete cascade,
  key_id uuid references keys(id) on delete set null,
  created_at timestamptz default now()
);

-- Audit log
create table if not exists audit_log (
  id uuid primary key default uuid_generate_v4(),
  admin_id uuid references users(id) on delete set null,
  action text not null,
  target_type text,
  target_id uuid,
  metadata jsonb,
  created_at timestamptz default now()
);

-- Indexes
create index if not exists idx_keys_user_id on keys(user_id);
create index if not exists idx_keys_code on keys(key_code);
create index if not exists idx_keys_expires on keys(expires_at);
create index if not exists idx_hwid_bindings_hwid on hwid_bindings(hwid);
create index if not exists idx_audit_log_admin on audit_log(admin_id);

-- RLS Policies
alter table users enable row level security;
alter table keys enable row level security;
alter table hwid_bindings enable row level security;
alter table audit_log enable row level security;

-- Users: admins see all, users see own
create policy "Admins manage all users" on users
  for all using (
    exists (select 1 from users where id = auth.uid() and role = 'admin')
  );

create policy "Users view own profile" on users
  for select using (auth.uid() = id);

-- Keys: admins manage all, users view own
create policy "Admins manage all keys" on keys
  for all using (
    exists (select 1 from users where id = auth.uid() and role = 'admin')
  );

create policy "Users view own keys" on keys
  for select using (auth.uid() = user_id);

-- HWID bindings: admins manage all
create policy "Admins manage HWID bindings" on hwid_bindings
  for all using (
    exists (select 1 from users where id = auth.uid() and role = 'admin')
  );

-- Audit log: admins only
create policy "Admins view audit log" on audit_log
  for select using (
    exists (select 1 from users where id = auth.uid() and role = 'admin')
  );

-- Function to update updated_at
create or replace function update_updated_at_column()
returns trigger language plpgsql as $$
begin
  new.updated_at = now();
  return new;
end $$;

create trigger update_users_updated_at before update on users
  for each row execute function update_updated_at_column();

create trigger update_keys_updated_at before update on keys
  for each row execute function update_updated_at_column();

-- Helper function: generate key
create or replace function generate_key_code()
returns text language plpgsql as $$
declare
  prefix text := 'CAMUS';
  chars text := 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
  suffix text := '';
  i int;
begin
  for i in 1..16 loop
    suffix := suffix || substr(chars, floor(random() * length(chars) + 1)::int, 1);
  end loop;
  return prefix || '-' || suffix;
end $$;

-- Function: create key (admin)
create or replace function admin_create_key(
  p_days int default 30,
  p_is_trial boolean default false,
  p_user_id uuid default null,
  p_notes text default null
)
returns table(key_code text, expires_at timestamptz) language plpgsql as $$
declare
  v_key text;
  v_expires timestamptz;
begin
  v_key := generate_key_code();
  v_expires := now() + (p_days || ' days')::interval;
  
  insert into keys (key_code, days, expires_at, is_trial, user_id, notes)
  values (v_key, p_days, v_expires, p_is_trial, p_user_id, p_notes)
  returning key_code, expires_at;
  
  return query select v_key, v_expires;
end $$;

-- Function: claim trial (user)
create or replace function claim_trial(p_email text, p_hwid text)
returns table(key_code text, expires_at timestamptz, error text) language plpgsql as $$
declare
  v_user_id uuid;
  v_key text;
  v_expires timestamptz;
begin
  -- Check/create user
  select id into v_user_id from users where email = p_email;
  
  if v_user_id is null then
    insert into users (email, hwid, trial_used, role)
    values (p_email, p_hwid, true, 'user')
    returning id into v_user_id;
  else
    if (select trial_used from users where id = v_user_id) then
      return query select null, null, 'Trial already used on this email';
    end if;
    
    -- Check HWID
    if exists (select 1 from hwid_bindings where hwid = p_hwid) then
      return query select null, null, 'HWID already used for trial';
    end if;
    
    update users set hwid = p_hwid, trial_used = true where id = v_user_id;
  end if;
  
  -- Check HWID not used
  if exists (select 1 from hwid_bindings where hwid = p_hwid) then
    return query select null, null, 'HWID already bound';
  end if;
  
  -- Create trial key
  v_key := generate_key_code();
  v_expires := now() + interval '7 days';
  
  insert into keys (key_code, user_id, days, expires_at, is_trial, notes)
  values (v_key, v_user_id, 7, v_expires, true, 'Free trial via website')
  returning key_code, expires_at into v_key, v_expires;
  
  -- Bind HWID
  insert into hwid_bindings (hwid, user_id, key_id)
  values (p_hwid, v_user_id, (select id from keys where key_code = v_key));
  
  return query select v_key, v_expires, null;
end $$;

-- Grant execute to authenticated
grant execute on function admin_create_key to authenticated;
grant execute on function claim_trial to authenticated;