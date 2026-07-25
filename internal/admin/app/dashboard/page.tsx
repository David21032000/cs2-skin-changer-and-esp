"use client";

import Link from 'next/link'
import { useAuth } from '@/lib/auth'
import { supabase } from '@/lib/supabase'
import { useState, useEffect } from 'react'

export default function Dashboard() {
  const { user, session, loading } = useAuth()
  const [stats, setStats] = useState({ total: 0, active: 0, expired: 0, today: 0 })
  const [keys, setKeys] = useState<any[]>([])
  const [showModal, setShowModal] = useState(false)
  const [newKeyDays, setNewKeyDays] = useState(30)
  const [generatedKey, setGeneratedKey] = useState<string | null>(null)
  const [copySuccess, setCopySuccess] = useState(false)

  useEffect(() => {
    if (!loading && !session) return
    fetchStats()
    fetchKeys()
  }, [session, loading])

  const fetchStats = async () => {
    const { data } = await supabase
      .from('keys')
      .select('id, active, expires_at, created_at')
    
    if (data) {
      const now = new Date()
      const today = new Date(now.setHours(0,0,0,0))
      setStats({
        total: data.length,
        active: data.filter(k => k.active && new Date(k.expires_at) > new Date()).length,
        expired: data.filter(k => !k.active || new Date(k.expires_at) <= new Date()).length,
        today: data.filter(k => new Date(k.created_at) >= today).length,
      })
    }
  }

  const fetchKeys = async () => {
    const { data } = await supabase
      .from('keys')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(50)
    if (data) setKeys(data)
  }

  const generateKey = async () => {
    const { data, error } = await supabase.rpc('generate_key', { days: newKeyDays })
    if (error) {
      alert('Error: ' + error.message)
      return
    }
    setGeneratedKey(data)
    setShowModal(true)
    fetchStats()
    fetchKeys()
  }

  const toggleKey = async (id: string, active: boolean) => {
    await supabase.from('keys').update({ active: !active }).eq('id', id)
    fetchStats()
    fetchKeys()
  }

  const deleteKey = async (id: string) => {
    if (!confirm('Delete this key?')) return
    await supabase.from('keys').delete().eq('id', id)
    fetchStats()
    fetchKeys()
  }

  const copyKey = () => {
    if (generatedKey) {
      navigator.clipboard.writeText(generatedKey)
      setCopySuccess(true)
      setTimeout(() => setCopySuccess(false), 2000)
    }
  }

  if (loading) return (
    <div className="min-h-screen flex items-center justify-center" style={{ background: 'var(--bg)' }}>
      <div className="w-10 h-10 border-3 rounded-full animate-spin" style={{ border: '3px solid var(--border)', borderTopColor: 'var(--accent)' }} />
    </div>
  )

  return (
    <div>
      <div className="page-header">
        <h1 className="gradient-text">Dashboard</h1>
        <p>Overview of your cheat management system</p>
      </div>

      {/* Stats */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-4 mb-8">
        <StatCard label="Total Keys" value={stats.total} color="var(--accent)" icon={<KeyIcon />} />
        <StatCard label="Active" value={stats.active} color="var(--accent)" icon={<CheckIcon />} />
        <StatCard label="Expired" value={stats.expired} color="var(--pink)" icon={<XIcon />} />
        <StatCard label="Created Today" value={stats.today} color="#635bff" icon={<PlusIcon />} />
      </div>

      {/* Actions */}
      <div className="action-bar">
        <button onClick={() => setShowModal(true)} className="btn-primary">
          <PlusIcon className="w-5 h-5 mr-2" />
          Generate New Key
        </button>
        <div className="flex items-center gap-2 text-sm" style={{ color: 'var(--muted)' }}>
          <span>Days:</span>
          <select value={newKeyDays} onChange={e => setNewKeyDays(Number(e.target.value))} className="select-sm">
            <option value={1}>1 Day (Trial)</option>
            <option value={7}>7 Days</option>
            <option value={30}>30 Days</option>
            <option value={90}>90 Days</option>
            <option value={365}>1 Year</option>
            <option value={99999}>Lifetime</option>
          </select>
        </div>
      </div>

      {/* Keys Table */}
      <div className="premium-card !p-0 !overflow-visible">
        <table className="premium-table">
          <thead>
            <tr>
              <th>KEY</th>
              <th>STATUS</th>
              <th>EXPIRES</th>
              <th>HWID</th>
              <th>CREATED</th>
              <th className="text-right pr-4">ACTIONS</th>
            </tr>
          </thead>
          <tbody>
            {keys.map(key => (
              <tr key={key.id} style={{ border: '1px solid var(--border)' }}>
                <td className="font-mono text-sm" style={{ color: 'var(--fg)' }}>
                  {key.key_code}
                </td>
                <td>
                  <span className={`badge ${key.active && new Date(key.expires_at) > new Date() ? 'badge-active' : 'badge-expired'}`}>
                    <span className="badge-dot" />
                    {key.active && new Date(key.expires_at) > new Date() ? 'Active' : 'Expired'}
                  </span>
                </td>
                <td className="text-sm" style={{ color: 'var(--muted)' }}>
                  {new Date(key.expires_at).toLocaleDateString()}
                </td>
                <td className="font-mono text-xs" style={{ color: 'var(--muted)' }}>
                  {key.hwid ? key.hwid.slice(0, 16) + '…' : '—'}
                </td>
                <td className="text-sm" style={{ color: 'var(--muted)' }}>
                  {new Date(key.created_at).toLocaleDateString()}
                </td>
                <td className="text-right pr-4">
                  <div className="flex items-center justify-end gap-2">
                    <button onClick={() => toggleKey(key.id, key.active)} className="action-btn" title={key.active ? 'Deactivate' : 'Activate'}>
                      {key.active ? <PauseIcon className="w-4 h-4" style={{ color: 'var(--pink)' }} /> : <PlayIcon className="w-4 h-4" style={{ color: 'var(--accent)' }} />}
                    </button>
                    <button onClick={() => deleteKey(key.id)} className="action-btn danger" title="Delete">
                      <TrashIcon className="w-4 h-4" style={{ color: 'var(--pink)' }} />
                    </button>
                  </div>
                </td>
              </tr>
            ))}
            {keys.length === 0 && (
              <tr>
                <td colSpan={6}>
                  <div className="empty-state">
                    <KeyIcon className="w-12 h-12" />
                    <p>No keys yet. Generate your first key!</p>
                  </div>
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>

      {/* Modal */}
      {showModal && (
        <div className="modal-overlay" onClick={() => setShowModal(false)}>
          <div className="modal-content" onClick={e => e.stopPropagation()}>
            <h2 className="text-xl font-bold mb-4 gradient-text-accent">Key Generated!</h2>
            {generatedKey ? (
              <>
                <div className="key-display mb-4">{generatedKey}</div>
                <div className="flex gap-3">
                  <button onClick={copyKey} className="btn-primary flex-1">
                    {copySuccess ? '✓ Copied!' : 'Copy Key'}
                  </button>
                  <button onClick={() => { setShowModal(false); setGeneratedKey(null) }} className="btn-secondary">
                    Done
                  </button>
                </div>
              </>
            ) : (
              <div className="key-display mb-4" style={{ color: 'var(--muted)' }}>Generating...</div>
            )}
          </div>
        </div>
      )}
    </div>
  )
}

const StatCard = ({ label, value, color = 'var(--accent)', icon }: { label: string; value: number; color?: string; icon: React.ReactNode }) => (
  <div className="stat-card">
    <div className="flex items-center justify-between mb-3">
      <span className="text-xs uppercase tracking-wider font-medium" style={{ color: 'var(--muted)' }}>{label}</span>
      <div className="stat-icon" style={{ background: `${color}15`, color }}>
        {icon}
      </div>
    </div>
    <div className="text-3xl font-bold font-mono gradient-text-accent">{value}</div>
  </div>
)

// Icons
const KeyIcon = ({ className, style }: { className?: string; style?: React.CSSProperties }) => <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><rect x="2" y="4" width="20" height="16" rx="2"/><path d="M6 12h12"/><path d="M12 4v16"/></svg>
const CheckIcon = ({ className, style }: { className?: string; style?: React.CSSProperties }) => <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polyline points="20 6 9 17 4 12"/></svg>
const XIcon = ({ className, style }: { className?: string; style?: React.CSSProperties }) => <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
const PlusIcon = ({ className, style }: { className?: string; style?: React.CSSProperties }) => <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg>
const PauseIcon = ({ className, style }: { className?: string; style?: React.CSSProperties }) => <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><rect x="6" y="4" width="4" height="16"/><rect x="14" y="4" width="4" height="16"/></svg>
const PlayIcon = ({ className, style }: { className?: string; style?: React.CSSProperties }) => <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polygon points="5 3 19 12 5 21 5 3"/></svg>
const TrashIcon = ({ className, style }: { className?: string; style?: React.CSSProperties }) => <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg>