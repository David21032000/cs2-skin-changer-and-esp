'use client'

import React, { useState, useEffect } from 'react'
import { supabase } from '@/lib/supabase'
import { formatDistanceToNow } from 'date-fns'

export default function TrialsPage() {
  const [trials, setTrials] = useState<any[]>([])
  const [loading, setLoading] = useState(true)
  const [search, setSearch] = useState('')

  useEffect(() => {
    fetchTrials()
  }, [])

  const fetchTrials = async () => {
    setLoading(true)
    let query = supabase
      .from('keys')
      .select(`
        *,
        users!keys_user_id_fkey (email)
      `)
      .eq('is_trial', true)
      .order('created_at', { ascending: false })

    if (search) {
      query = query.or(`key_code.ilike.%${search}%,users.email.ilike.%${search}%`)
    }

    const { data } = await query
    if (data) setTrials(data)
    setLoading(false)
  }

  const revokeTrial = async (id: string) => {
    if (!confirm('Revoke this trial key?')) return
    await supabase.from('keys').update({ is_revoked: true }).eq('id', id)
    fetchTrials()
  }

  const getStatus = (key: any) => {
    const now = new Date()
    if (key.is_revoked || new Date(key.expires_at) <= now) return { label: 'Expired', color: 'var(--pink)', icon: <XIcon /> }
    return { label: 'Active', color: 'var(--accent)', icon: <CheckIcon /> }
  }

  return (
    <div className="space-y-6">
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-2xl font-bold">Free Trials</h1>
          <p style={{ color: 'var(--muted)' }}>Track 7-day free trial activations</p>
        </div>
      </div>

      <div className="flex gap-4" style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '12px', padding: '1rem' }}>
        <div className="relative flex-1 min-w-[200px]">
          <SearchIcon className="absolute left-3 top-1/2 -translate-y-1/2 w-5 h-5" style={{ color: 'var(--muted)' }} />
          <input type="text" value={search} onChange={e => setSearch(e.target.value)} placeholder="Search trial keys..." className="w-full pl-10 pr-4 py-2 input" />
        </div>
      </div>

      <div style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '12px', overflow: 'hidden' }}>
        <div className="overflow-x-auto">
          <table className="w-full">
            <thead>
              <tr className="border-b" style={{ borderColor: 'var(--border)' }}>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>TRIAL KEY</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>USER</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>HWID</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>STATUS</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>EXPIRES</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>CREATED</th>
                <th className="text-right px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>ACTION</th>
              </tr>
            </thead>
            <tbody>
              {loading ? (
                <tr><td colSpan={7} className="px-4 py-8 text-center" style={{ color: 'var(--muted)' }}>Loading...</td></tr>
              ) : trials.length === 0 ? (
                <tr><td colSpan={7} className="px-4 py-12 text-center" style={{ color: 'var(--muted)' }}>No trial keys yet</td></tr>
              ) : (
                trials.map(key => {
                  const status = getStatus(key) as unknown as { label: string; icon: React.ComponentType<{ className?: string }>; class: string }
                  return (
                    <tr key={key.id} className="border-b hover:bg-[var(--bg-elevated)]" style={{ borderColor: 'var(--border)' }}>
                      <td className="px-4 py-3 font-mono text-sm" style={{ color: 'var(--fg)' }}>{key.key_code}</td>
                      <td className="px-4 py-3">
                        <p className="text-sm" style={{ color: 'var(--fg)' }}>{key.users?.email}</p>
                      </td>
                      <td className="px-4 py-3 font-mono text-xs" style={{ color: 'var(--muted)' }}>
                        {key.hwid ? key.hwid.slice(0, 16) + '…' : <span style={{ color: 'var(--muted)' }}>—</span>}
                      </td>
                      <td className="px-4 py-3">
                        <span className={`inline-flex items-center gap-1.5 px-2.5 py-1 rounded text-xs font-medium ${status.class}`}>
                          <status.icon className="w-3 h-3" />
                          {status.label}
                        </span>
                      </td>
                      <td className="px-4 py-3 text-sm" style={{ color: 'var(--muted)' }}>
                        {formatDistanceToNow(new Date(key.expires_at), { addSuffix: true })}
                      </td>
                      <td className="px-4 py-3 text-sm" style={{ color: 'var(--muted)' }}>
                        {formatDistanceToNow(new Date(key.created_at), { addSuffix: true })}
                      </td>
                      <td className="px-4 py-3 text-right pr-4">
                        {!key.is_revoked && new Date(key.expires_at) > new Date() && (
                          <button onClick={() => revokeTrial(key.id)} className="btn-danger" style={{ padding: '0.375rem 0.75rem', fontSize: '0.875rem' }}>
                            Revoke
                          </button>
                        )}
                      </td>
                    </tr>
                  )
                })
              )}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  )
}

function getStatus(key: any): { label: string; icon: React.ComponentType<{ className?: string }>; class: string } {
  const now = new Date()
  if (key.is_revoked || new Date(key.expires_at) <= now) return { label: 'Expired', icon: XIcon, class: 'bg-[var(--pink)]/20 text-[var(--pink)]' }
  return { label: 'Active', icon: CheckIcon, class: 'bg-[var(--accent-dim)] text-[var(--accent)]' }
}

function SearchIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg> }
function CheckIcon({ className }: { className?: string }) { return <svg className={className} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polyline points="20 6 9 17 4 12"/></svg> }
function XIcon({ className }: { className?: string }) { return <svg className={className} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg> }