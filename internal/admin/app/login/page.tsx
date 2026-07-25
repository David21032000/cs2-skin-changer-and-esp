'use client'

import { useState } from 'react'
import { supabase } from '@/lib/supabase'
import { useRouter } from 'next/navigation'
import { useAuth } from '@/lib/auth'
import Link from 'next/link'

export default function LoginPage() {
  const router = useRouter()
  const { refreshUser } = useAuth()
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState('')
  const [mode, setMode] = useState<'login' | 'signup'>('login')

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')
    setLoading(true)

    try {
      if (mode === 'login') {
        const { error } = await supabase.auth.signInWithPassword({ email, password })
        if (error) throw error
      } else {
        const { error } = await supabase.auth.signUp({ email, password })
        if (error) throw error
        setError('Check your email to confirm your account')
        setMode('login')
        setLoading(false)
        return
      }
      await refreshUser()
      router.push('/dashboard')
    } catch (err: any) {
      setError(err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="min-h-screen flex items-center justify-center p-4" style={{ background: 'var(--bg)' }}>
      <div className="bg-grid" />
      <div className="glow-orb orb-1" />
      <div className="glow-orb orb-2" />
      <div className="w-full max-w-md relative z-10">
        <div className="text-center mb-8">
          <Link href="/" className="inline-flex items-center gap-2 mb-6">
            <span className="font-mono font-bold text-2xl" style={{ color: 'var(--accent)' }}>CAMUS</span>
            <span className="text-xs uppercase tracking-widest" style={{ color: 'var(--muted)' }}>ADMIN</span>
          </Link>
          <h1 className="text-2xl font-bold gradient-text">{mode === 'login' ? 'Welcome Back' : 'Create Account'}</h1>
          <p className="mt-2" style={{ color: 'var(--muted)' }}>{mode === 'login' ? 'Enter your admin credentials' : 'Register new admin account'}</p>
        </div>

        <form onSubmit={handleSubmit} className="space-y-4 premium-card">
          {error && <div className="p-3 rounded-lg text-sm" style={{ background: 'rgba(255,42,109,0.15)', border: '1px solid var(--pink)', color: 'var(--pink)' }}>{error}</div>}

          <div>
            <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Email</label>
            <input type="email" value={email} onChange={e => setEmail(e.target.value)} required className="w-full input" placeholder="admin@camus.com" />
          </div>

          <div>
            <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Password</label>
            <input type="password" value={password} onChange={e => setPassword(e.target.value)} required minLength={8} className="w-full input" placeholder="••••••••" />
          </div>

          <button type="submit" disabled={loading} className="btn-primary w-full py-3 mt-2" style={{ fontSize: '1rem' }}>
            {loading ? 'Please wait...' : mode === 'login' ? 'Sign In' : 'Create Account'}
          </button>

          <p className="text-center text-sm" style={{ color: 'var(--muted)' }}>
            {mode === 'login' ? "Don't have an account? " : 'Already have an account? '}
            <button type="button" onClick={() => { setMode(mode === 'login' ? 'signup' : 'login'); setError('') }} className="font-medium hover:underline" style={{ color: 'var(--accent)' }}>
              {mode === 'login' ? 'Sign Up' : 'Sign In'}
            </button>
          </p>
        </form>

        <div className="mt-6 text-center text-xs" style={{ color: 'var(--muted)' }}>
          <p>Admin access only. Trial users cannot access this panel.</p>
        </div>
      </div>
    </div>
  )
}