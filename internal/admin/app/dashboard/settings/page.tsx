'use client'

import { useState, useEffect } from 'react'
import { supabase } from '@/lib/supabase'
import { useAuth } from '@/lib/auth'

export default function SettingsPage() {
  const { user, refreshUser } = useAuth()
  const [apiUrl, setApiUrl] = useState('')
  const [webhookSecret, setWebhookSecret] = useState('')
  const [stripeKey, setStripeKey] = useState('')
  const [stripePriceMonthly, setStripePriceMonthly] = useState('')
  const [stripePriceQuarterly, setStripePriceQuarterly] = useState('')
  const [stripePriceLifetime, setStripePriceLifetime] = useState('')
  const [loading, setLoading] = useState(false)
  const [saved, setSaved] = useState(false)

  useEffect(() => {
    // Load from localStorage (in production, use Supabase settings table)
    setApiUrl(localStorage.getItem('API_URL') || 'https://your-worker.your-subdomain.workers.dev')
    setWebhookSecret(localStorage.getItem('STRIPE_WEBHOOK_SECRET') || '')
    setStripeKey(localStorage.getItem('STRIPE_SECRET_KEY') || '')
    setStripePriceMonthly(localStorage.getItem('STRIPE_PRICE_MONTHLY') || '')
    setStripePriceQuarterly(localStorage.getItem('STRIPE_PRICE_QUARTERLY') || '')
    setStripePriceLifetime(localStorage.getItem('STRIPE_PRICE_LIFETIME') || '')
  }, [])

  const handleSave = async (key: string, value: string) => {
    setLoading(true)
    localStorage.setItem(key, value)
    setSaved(true)
    setLoading(false)
    setTimeout(() => setSaved(false), 2000)
  }

  return (
    <div className="max-w-3xl space-y-8">
      <div>
        <h1 className="text-2xl font-bold">Settings</h1>
        <p style={{ color: 'var(--muted)' }}>Configure API, payments, and security</p>
      </div>

      <div className="space-y-6" style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '16px', padding: '2rem' }}>
        <h2 className="text-lg font-bold mb-4" style={{ color: 'var(--fg)' }}>API Configuration</h2>
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Cloudflare Worker API URL</label>
            <input type="url" value={apiUrl} onChange={e => setApiUrl(e.target.value)} placeholder="https://your-worker.your-subdomain.workers.dev" className="w-full input" />
            <p className="text-xs mt-1" style={{ color: 'var(--muted)' }}>The cheat will call <code>{apiUrl}/api/verify</code> for authentication</p>
          </div>
          <button onClick={() => handleSave('API_URL', apiUrl)} disabled={loading} className="btn-primary">
            {loading ? 'Saving...' : 'Save API URL'}
          </button>
        </div>

        <hr style={{ borderColor: 'var(--border)', margin: '2rem 0' }} />

        <h2 className="text-lg font-bold mb-4" style={{ color: 'var(--fg)' }}>Stripe Payment Integration</h2>
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Stripe Secret Key</label>
            <input type="password" value={stripeKey} onChange={e => setStripeKey(e.target.value)} placeholder="sk_live_..." className="w-full input" />
          </div>
          <div>
            <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Stripe Webhook Secret</label>
            <input type="password" value={webhookSecret} onChange={e => setWebhookSecret(e.target.value)} placeholder="whsec_..." className="w-full input" />
            <p className="text-xs mt-1" style={{ color: 'var(--muted)' }}>From Stripe Dashboard → Webhooks → Signing secret</p>
          </div>
          <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
            <div>
              <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Monthly Price ID</label>
              <input type="text" value={stripePriceMonthly} onChange={e => setStripePriceMonthly(e.target.value)} placeholder="price_..." className="w-full input" />
            </div>
            <div>
              <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Quarterly Price ID</label>
              <input type="text" value={stripePriceQuarterly} onChange={e => setStripePriceQuarterly(e.target.value)} placeholder="price_..." className="w-full input" />
            </div>
            <div>
              <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Lifetime Price ID</label>
              <input type="text" value={stripePriceLifetime} onChange={e => setStripePriceLifetime(e.target.value)} placeholder="price_..." className="w-full input" />
            </div>
          </div>
          <button onClick={() => { handleSave('STRIPE_SECRET_KEY', stripeKey); handleSave('STRIPE_WEBHOOK_SECRET', webhookSecret); handleSave('STRIPE_PRICE_MONTHLY', stripePriceMonthly); handleSave('STRIPE_PRICE_QUARTERLY', stripePriceQuarterly); handleSave('STRIPE_PRICE_LIFETIME', stripePriceLifetime); }} disabled={loading} className="btn-primary">
            {loading ? 'Saving...' : 'Save Stripe Settings'}
          </button>
        </div>

        <hr style={{ borderColor: 'var(--border)', margin: '2rem 0' }} />

        <h2 className="text-lg font-bold mb-4" style={{ color: 'var(--fg)' }}>Security</h2>
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <div>
              <p className="font-medium" style={{ color: 'var(--fg)' }}>Admin Role</p>
              <p className="text-sm" style={{ color: 'var(--muted)' }}>Your account has full admin access</p>
            </div>
            <span className="px-3 py-1 rounded-full text-xs font-medium" style={{ background: 'var(--accent-dim)', color: 'var(--accent)' }}>Admin</span>
          </div>
        </div>
      </div>

      {saved && (
        <div className="fixed bottom-4 right-4 z-50 animate-fade-in" style={{ background: 'var(--card)', border: '1px solid var(--accent)', borderRadius: '12px', padding: '1rem 1.5rem', boxShadow: '0 12px 32px rgba(0,0,0,0.3)' }}>
          <div className="flex items-center gap-2" style={{ color: 'var(--accent)' }}>
            <CheckIcon className="w-5 h-5" />
            <span className="font-medium">Settings saved successfully</span>
          </div>
        </div>
      )}
    </div>
  )
}

// Icons
function CheckIcon({ className }: { className?: string }) { return <svg className={className} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polyline points="20 6 9 17 4 12"/></svg> }