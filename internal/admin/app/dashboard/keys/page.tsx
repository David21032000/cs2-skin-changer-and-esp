'use client'

import { useState, useEffect } from 'react'
import { supabase } from '@/lib/supabase'

export default function KeysPage() {
  const [keys, setKeys] = useState<any[]>([])
  const [loading, setLoading] = useState(true)
  const [showModal, setShowModal] = useState(false)
  const [days, setDays] = useState(30)
  const [notes, setNotes] = useState('')
  const [generatedKey, setGeneratedKey] = useState<string | null>(null)
  const [copySuccess, setCopySuccess] = useState(false)
  const [generating, setGenerating] = useState(false)
  const [search, setSearch] = useState('')
  const [filterStatus, setFilterStatus] = useState<'all' | 'active' | 'expired' | 'trial'>('all')
  const [currentPage, setCurrentPage] = useState(1)
  const [totalCount, setTotalCount] = useState(0)
  const PAGE_SIZE = 25

  useEffect(() => {
    fetchKeys()
    fetchCount()
  }, [currentPage, search, filterStatus])

  const fetchKeys = async () => {
    setLoading(true)
    let query = supabase
      .from('keys')
      .select('*', { count: 'exact' })
      .order('created_at', { ascending: false })
      .range((currentPage - 1) * PAGE_SIZE, currentPage * PAGE_SIZE - 1)

    if (search) {
      query = query.ilike('key_code', `%${search}%`)
    }
    if (filterStatus !== 'all') {
      const now = new Date().toISOString()
      if (filterStatus === 'active') query = query.eq('is_revoked', false).gt('expires_at', now)
      if (filterStatus === 'expired') query = query.or(`is_revoked.eq.true,expires_at.lte.${now}`)
      if (filterStatus === 'trial') query = query.eq('is_trial', true)
    }

    const { data, count } = await query
    if (data) setKeys(data)
    if (count !== null) setTotalCount(count)
    setLoading(false)
  }

  const fetchCount = async () => {
    const { count } = await supabase.from('keys').select('*', { count: 'exact', head: true })
    if (count !== null) setTotalCount(count)
  }

  const generateKey = async () => {
    setGenerating(true)
    const { data, error } = await supabase.rpc('admin_create_key', { 
      p_days: days, 
      p_notes: notes || null 
    })
    setGenerating(false)
    if (error) { alert('Error: ' + error.message); return }
    const newKey = data[0]?.key_code
    setGeneratedKey(newKey)
    setShowModal(true)
    fetchKeys()
    fetchCount()
  }

  const toggleKey = async (id: string, active: boolean) => {
    await supabase.from('keys').update({ is_revoked: active }).eq('id', id)
    fetchKeys()
  }

  const deleteKey = async (id: string) => {
    if (!confirm('Delete this key permanently?')) return
    await supabase.from('keys').delete().eq('id', id)
    fetchKeys()
    fetchCount()
  }

  const copyKey = () => {
    if (generatedKey) {
      navigator.clipboard.writeText(generatedKey)
      setCopySuccess(true)
      setTimeout(() => setCopySuccess(false), 2000)
    }
  }

  const getStatus = (key: any) => {
    const now = new Date()
    if (key.is_revoked || new Date(key.expires_at) <= now) return { label: 'Expired', color: 'var(--pink)', icon: <XIcon /> }
    return { label: 'Active', color: 'var(--accent)', icon: <CheckIcon /> }
  }

  return (
    <div>
      <div className="page-header">
        <h1 className="gradient-text">Keys Management</h1>
        <p>Generate, manage, and monitor all keys</p>
      </div>

      <div className="flex flex-wrap gap-4 mb-6 items-center justify-between">
        <button onClick={() => setShowModal(true)} className="btn-primary">
          <PlusIcon className="w-5 h-5 mr-2" />
          Generate Key
        </button>
      </div>

      {/* Filters */}
      <div className="flex flex-wrap gap-4 mb-6" style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '12px', padding: '1rem' }}>
        <div className="relative flex-1 min-w-[200px]">
          <SearchIcon className="absolute left-3 top-1/2 -translate-y-1/2 w-5 h-5" style={{ color: 'var(--muted)' }} />
          <input type="text" value={search} onChange={e => setSearch(e.target.value)} placeholder="Search keys..." className="w-full pl-10 pr-4 py-2 input" />
        </div>
        <select value={filterStatus} onChange={e => setFilterStatus(e.target.value as any)} className="select-sm">
          <option value="all">All Status</option>
          <option value="active">Active</option>
          <option value="expired">Expired</option>
          <option value="trial">Trial</option>
        </select>
      </div>

      {/* Table */}
      <div className="premium-card !p-0">
        <div className="overflow-x-auto">
          <table className="premium-table">
            <thead>
              <tr>
                <th>KEY</th>
                <th>STATUS</th>
                <th>TYPE</th>
                <th>EXPIRES</th>
                <th>HWID</th>
                <th>CREATED</th>
                <th className="text-right pr-4">ACTIONS</th>
              </tr>
            </thead>
            <tbody>
              {loading ? (
                <tr><td colSpan={7}><div className="empty-state">Loading...</div></td></tr>
              ) : keys.length === 0 ? (
                <tr><td colSpan={7}><div className="empty-state">No keys found</div></td></tr>
              ) : (
                keys.map(key => {
                  const status = getStatus(key)
                  return (
                    <tr key={key.id} style={{ border: '1px solid var(--border)' }}>
                      <td className="font-mono text-sm" style={{ color: 'var(--fg)' }}>{key.key_code}</td>
                      <td>
                        <span className={`badge ${status.label === 'Active' ? 'badge-active' : 'badge-expired'}`}>
                          <span className="badge-dot" />
                          {status.label}
                        </span>
                      </td>
                      <td>
                        <span className="px-2.5 py-0.5 rounded-full text-xs font-medium" style={{ background: key.is_trial ? 'rgba(99,91,255,0.2)' : 'rgba(0,255,200,0.2)', color: key.is_trial ? '#635bff' : 'var(--accent)' }}>
                          {key.is_trial ? 'Trial' : 'Paid'}
                        </span>
                      </td>
                      <td className="text-sm" style={{ color: 'var(--muted)' }}>
                        {new Date(key.expires_at).toLocaleDateString()}
                        {new Date(key.expires_at) <= new Date() && <span style={{ color: 'var(--pink)', marginLeft: '0.25rem' }}>(Expired)</span>}
                      </td>
                      <td className="font-mono text-xs" style={{ color: 'var(--muted)' }}>
                        {key.hwid ? key.hwid.slice(0, 16) + '…' : '—'}
                      </td>
                      <td className="text-sm" style={{ color: 'var(--muted)' }}>
                        {new Date(key.created_at).toLocaleDateString()}
                      </td>
                      <td className="text-right pr-4">
                        <div className="flex items-center justify-end gap-2">
                          <button onClick={() => toggleKey(key.id, !key.is_revoked)} className="action-btn" title={key.is_revoked ? 'Activate' : 'Revoke'}>
                            {key.is_revoked ? <PlayIcon className="w-4 h-4" style={{ color: 'var(--accent)' }} /> : <PauseIcon className="w-4 h-4" style={{ color: 'var(--pink)' }} />}
                          </button>
                          <button onClick={() => deleteKey(key.id)} className="action-btn danger" title="Delete">
                            <TrashIcon className="w-4 h-4" style={{ color: 'var(--pink)' }} />
                          </button>
                        </div>
                      </td>
                    </tr>
                  )
                })
              )}
            </tbody>
          </table>
        </div>

        {/* Pagination */}
        {totalCount > PAGE_SIZE && (
          <div className="flex items-center justify-between px-4 py-3 border-t" style={{ borderColor: 'var(--border)' }}>
            <span className="text-sm" style={{ color: 'var(--muted)' }}>
              Showing {(currentPage - 1) * PAGE_SIZE + 1} to {Math.min(currentPage * PAGE_SIZE, totalCount)} of {totalCount}
            </span>
            <div className="flex gap-2">
              <button onClick={() => setCurrentPage(p => p - 1)} disabled={currentPage === 1} className="btn-secondary !px-3 !py-1 text-sm">Previous</button>
              <button onClick={() => setCurrentPage(p => p + 1)} disabled={currentPage * PAGE_SIZE >= totalCount} className="btn-secondary !px-3 !py-1 text-sm">Next</button>
            </div>
          </div>
        )}
      </div>

      {/* Modal */}
      {showModal && (
        <div className="modal-overlay" onClick={() => setShowModal(false)}>
          <div className="modal-content" onClick={e => e.stopPropagation()}>
            <h2 className="text-xl font-bold mb-4 gradient-text-accent">Generate Key</h2>
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Duration</label>
                <select value={days} onChange={e => setDays(Number(e.target.value))} className="select-sm w-full">
                  <option value={1}>1 Day (Trial)</option>
                  <option value={7}>7 Days</option>
                  <option value={30}>30 Days</option>
                  <option value={90}>90 Days</option>
                  <option value={365}>1 Year</option>
                  <option value={99999}>Lifetime</option>
                </select>
              </div>
              <div>
                <label className="block text-sm font-medium mb-1" style={{ color: 'var(--fg)' }}>Notes (optional)</label>
                <input type="text" value={notes} onChange={e => setNotes(e.target.value)} className="input" placeholder="Internal notes..." />
              </div>
            </div>
            <div className="flex gap-3 mt-6">
              <button onClick={generateKey} className="btn-primary flex-1" disabled={generating}>
                {generating ? 'Generating...' : 'Generate'}
              </button>
              <button onClick={() => setShowModal(false)} className="btn-secondary">Cancel</button>
            </div>
            {generatedKey && (
              <div className="mt-4">
                <div className="key-display mb-3">{generatedKey}</div>
                <button onClick={copyKey} className="btn-primary w-full">
                  {copySuccess ? '✓ Copied!' : 'Copy Key'}
                </button>
              </div>
            )}
          </div>
        </div>
      )}
    </div>
  )
}

function getStatus(key: any) {
  const now = new Date()
  if (key.is_revoked || new Date(key.expires_at) <= now) return { label: 'Expired', color: 'var(--pink)', icon: <XIcon /> }
  return { label: 'Active', color: 'var(--accent)', icon: <CheckIcon /> }
}

// Icons
function PlusIcon({ className }: { className?: string }) { return <svg className={className} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg> }
function SearchIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg> }
function CheckIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polyline points="20 6 9 17 4 12"/></svg> }
function XIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg> }
function PauseIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><rect x="6" y="4" width="4" height="16"/><rect x="14" y="4" width="4" height="16"/></svg> }
function PlayIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polygon points="5 3 19 12 5 21 5 3"/></svg> }
function TrashIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg> }
function CopyIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><rect x="9" y="9" width="13" height="13" rx="2" ry="2"/><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/></svg> }