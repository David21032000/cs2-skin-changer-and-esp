'use client'

import { useEffect } from 'react'
import { useRouter } from 'next/navigation'
import { supabase } from '@/lib/supabase'

export default function Home() {
  const router = useRouter()

  useEffect(() => {
    supabase.auth.getSession().then(({ data: { session } }) => {
      if (session) router.push('/dashboard')
      else router.push('/login')
    })
  }, [router])

  return (
    <div className="min-h-screen flex items-center justify-center" style={{ background: 'var(--bg)' }}>
      <div className="bg-grid" />
      <div className="glow-orb orb-1" />
      <div className="glow-orb orb-2" />
      <div className="text-center relative z-10">
        <div className="font-mono font-bold text-5xl gradient-text mb-2">CAMUS</div>
        <div className="text-xs uppercase tracking-widest mb-8" style={{ color: 'var(--muted)' }}>ADMIN PANEL</div>
        <div className="w-10 h-10 border-3 rounded-full animate-spin" style={{ border: '3px solid var(--border)', borderTopColor: 'var(--accent)', margin: '0 auto' }} />
      </div>
    </div>
  )
}