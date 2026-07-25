# Camus Admin Dashboard - Deployment Guide

## Quick Start (Free Tier)

### 1. Supabase Setup (Database + Auth)
1. Go to [supabase.com](https://supabase.com) → New Project
2. Wait for provisioning (2 min)
3. Go to **Settings → API** → copy:
   - `Project URL` → `NEXT_PUBLIC_SUPABASE_URL`
   - `anon public` key → `NEXT_PUBLIC_SUPABASE_ANON_KEY`
4. Go to **SQL Editor** → New Query → paste contents of `supabase/schema.sql` → Run

### 2. Cloudflare Workers (API Backend)
1. Go to [dash.cloudflare.com](https://dash.cloudflare.com) → Workers & Pages → Create Application → Create Worker
2. Name: `camus-api`
3. Paste contents of `worker.ts` into editor
4. Settings → Variables → Add:
   - `SUPABASE_URL` = your Supabase URL
   - `SUPABASE_ANON_KEY` = your Supabase anon key
5. Save & Deploy → note the URL (e.g., `https://camus-api.your-subdomain.workers.dev`)

### 3. Vercel (Frontend)
1. Push this folder to GitHub
2. Go to [vercel.com](https://vercel.com) → Add New Project → Import from GitHub
3. Framework: Next.js (auto-detected)
4. Environment Variables:
   ```
   NEXT_PUBLIC_SUPABASE_URL=your-supabase-url
   NEXT_PUBLIC_SUPABASE_ANON_KEY=your-anon-key
   NEXT_PUBLIC_API_URL=https://camus-api.your-subdomain.workers.dev
   ```
5. Deploy → get `https://camus-admin.vercel.app`

### 4. Create First Admin
1. Go to your Vercel URL → `/login`
2. Click "Sign Up" → use your email
3. Go to Supabase Dashboard → Table Editor → `users` → find your row → set `role` = `admin`
4. Refresh → you're in the admin panel

---

## Features

### Dashboard
- Real-time stats (total/active/expired keys, created today)
- Generate keys with custom duration (1 day → lifetime)
- Search/filter keys by status (active/expired/trial)
- Toggle key active/revoked, delete keys

### Free Trial System
- `/dashboard/trials` shows all trial activations
- 7-day trial, one per email + HWID
- Auto-revokes on expiry

### Users Management
- View all users (email, role, trial status, HWID)
- Promote/demote admin
- Delete users

### Audit Log
- Every admin action logged (who, what, when, target)
- Searchable, filterable

### Settings
- Update Supabase/Stripe credentials
- Configure trial duration
- Webhook URLs

---

## Local Development

```bash
cd admin
npm install
cp .env.example .env.local
# Edit .env.local with your keys
npm run dev
# Open http://localhost:3000
```

---

## Database Schema (supabase/schema.sql)

Run in Supabase SQL Editor. Creates:
- `users` - email, HWID, trial_used, role
- `keys` - key_code, user_id, days, expires_at, is_trial, is_revoked
- `hwid_bindings` - HWID → user/key mapping
- `audit_log` - admin actions

RLS policies ensure:
- Admins see/manage everything
- Users see only their own keys

---

## Free Trial Logic

1. User visits site → clicks "Get 7-Day Free Trial"
2. Enters email → gets magic link or OTP
3. Supabase Auth creates user
4. Backend calls `claim_trial(email, hwid)` RPC
5. Checks:
   - `trial_used = false` on user
   - HWID not in `hwid_bindings`
6. Creates key: `CAMUS-TRIAL-XXXX` (7 days, is_trial=true)
7. Binds HWID → trial used = true
8. Returns key to user instantly

---

## Stripe Integration (Optional)

1. Create products in Stripe:
   - Monthly ($15)
   - Quarterly ($35)
   - Lifetime ($100)
2. Add price IDs to buy page
3. Create webhook endpoint: `https://your-api.workers.dev/api/stripe/webhook`
4. Handle `checkout.session.completed`:
   - Get customer email
   - Call `admin_create_key(days=30/90/99999, user_id)`
   - Email key to customer

---

## Security Notes

- All admin routes check `role = 'admin'` in middleware
- API keys only in Cloudflare Worker secrets
- RLS policies on all tables
- Audit log for every sensitive action
- Rate limit on trial endpoint (1 req/min per IP)

---

## Custom Domain (Optional)

1. Vercel → Project Settings → Domains → Add `admin.camuscheat.com`
2. Cloudflare → DNS → CNAME `admin` → `cname.vercel-dns.com`
3. Cloudflare Worker → Custom Domain → `api.camuscheat.com`

---

## File Structure

```
admin/
├── app/
│   ├── layout.tsx          # Root layout with neon theme
│   ├── page.tsx            # Redirect to login/dashboard
│   ├── login/page.tsx      # Login/signup page
│   ├── dashboard/
│   │   ├── layout.tsx      # Sidebar + header
│   │   ├── page.tsx        # Dashboard with stats + key gen
│   │   ├── keys/page.tsx   # Keys management
│   │   ├── users/page.tsx  # User management
│   │   ├── trials/page.tsx # Free trial tracking
│   │   ├── audit/page.tsx  # Audit log
│   │   └── settings/page.tsx
│   └── api/                # API routes (proxy to Worker)
├── lib/
│   ├── supabase.ts         # Supabase client
│   ├── auth.tsx            # Auth context + hooks
│   └── types.ts            # TypeScript types
├── components/             # Reusable UI components
├── worker.ts               # Cloudflare Worker (API)
├── supabase/schema.sql     # Database schema
├── package.json
├── next.config.js
├── tsconfig.json
└── .env.example
```

---

## Cost Breakdown (All Free)

| Service | Free Tier Limits |
|---------|------------------|
| Supabase | 500MB DB, 50k MAU, 1GB bandwidth |
| Cloudflare Workers | 100k requests/day |
| Vercel | 100GB bandwidth, unlimited personal |
| **Total** | **$0/month** |

---

## Production Checklist

- [ ] Supabase project created, schema applied
- [ ] Cloudflare Worker deployed with secrets
- [ ] Vercel project deployed with env vars
- [ ] First admin created and role set
- [ ] Trial endpoint tested (7-day key generated)
- [ ] Paid key generation tested
- [ ] Audit log capturing actions
- [ ] Stripe webhook configured (if using)
- [ ] Custom domains configured (optional)
- [ ] Monitoring/alerts set up

---

## Support

- Check Supabase logs for DB errors
- Cloudflare Worker logs for API errors
- Vercel function logs for frontend errors
- All errors logged to `audit_log` table