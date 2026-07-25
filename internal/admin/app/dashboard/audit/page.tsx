'use client'

import { useState, useEffect } from 'react'
import { supabase } from '@/lib/supabase'
import { formatDistanceToNow } from 'date-fns'

export default function AuditPage() {
  const [logs, setLogs] = useState<any[]>([])
  const [loading, setLoading] = useState(true)
  const [search, setSearch] = useState('')
  const [actionFilter, setActionFilter] = useState('all')
  const [currentPage, setCurrentPage] = useState(1)
  const PAGE_SIZE = 25

  useEffect(() => {
    fetchLogs()
  }, [currentPage, search, actionFilter])

  const fetchLogs = async () => {
    setLoading(true)
    let query = supabase
      .from('audit_log')
      .select('*')
      .order('created_at', { ascending: false })
      .range((currentPage - 1) * PAGE_SIZE, currentPage * PAGE_SIZE - 1)

    if (search) {
      query = query.ilike('action', `%${search}%`)
    }
    if (actionFilter !== 'all') {
      query = query.eq('action', actionFilter)
    }

    const { data } = await query
    if (data) setLogs(data)
    setLoading(false)
  }

  const actionColors: Record<string, string> = {
    'create_key': 'var(--accent)',
    'revoke_key': 'var(--pink)',
    'toggle_key': '#635bff',
    'delete_key': 'var(--pink)',
    'create_user': 'var(--accent)',
    'toggle_admin': '#635bff',
    'delete_user': 'var(--pink)',
  }

  return (
    <div className="space-y-6">
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-2xl font-bold">Audit Log</h1>
          <p style={{ color: 'var(--muted)' }}>Track all administrative actions</p>
        </div>
      </div>

      <div className="flex flex-col sm:flex-row gap-4" style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '12px', padding: '1rem' }}>
        <div className="relative flex-1 min-w-[200px]">
          <SearchIcon className="absolute left-3 top-1/2 -translate-y-1/2 w-5 h-5" style={{ color: 'var(--muted)' }} />
          <input type="text" value={search} onChange={e => setSearch(e.target.value)} placeholder="Search actions..." className="w-full pl-10 pr-4 py-2 input" />
        </div>
        <select value={actionFilter} onChange={e => setActionFilter(e.target.value)} className="input w-auto min-w-[180px]" style={{ background: 'var(--bg)', border: '1px solid var(--border)' }}>
          <option value="all">All Actions</option>
          <option value="create_key">Create Key</option>
          <option value="revoke_key">Revoke Key</option>
          <option value="toggle_key">Toggle Key</option>
          <option value="delete_key">Delete Key</option>
          <option value="create_user">Create User</option>
          <option value="toggle_admin">Toggle Admin</option>
          <option value="delete_user">Delete User</option>
        </select>
      </div>

      <div style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '12px', overflow: 'hidden' }}>
        <div className="overflow-x-auto">
          <table className="w-full">
            <thead>
              <tr className="border-b" style={{ borderColor: 'var(--border)' }}>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>ACTION</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>ADMIN</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>TARGET</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>DETAILS</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>TIME</th>
              </tr>
            </thead>
            <tbody>
              {loading ? (
                <tr><td colSpan={5} className="px-4 py-8 text-center" style={{ color: 'var(--muted)' }}>Loading...</td></tr>
              ) : logs.length === 0 ? (
                <tr><td colSpan={5} className="px-4 py-12 text-center" style={{ color: 'var(--muted)' }}>No audit logs</td></tr>
              ) : (
                logs.map(log => (
                  <tr key={log.id} className="border-b hover:bg-[var(--bg-elevated)]" style={{ borderColor: 'var(--border)' }}>
                    <td className="px-4 py-3">
                      <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded text-xs font-medium" style={{ background: `${actionColors[log.action] || 'var(--muted)'}20`, color: actionColors[log.action] || 'var(--muted)' }}>
                        <span className="w-1.5 h-1.5 rounded-full" style={{ background: actionColors[log.action] || 'var(--muted)' }} />
                        {log.action.replace(/_/g, ' ')}
                      </span>
                    </td>
                    <td className="px-4 py-3">
                      <div>
                        <p className="font-mono text-xs" style={{ color: 'var(--fg)' }}>{log.admin_id ? log.admin_id.slice(0, 8) + '…' : 'System'}</p>
                      </div>
                    </td>
                    <td className="px-4 py-3 font-mono text-xs" style={{ color: 'var(--muted)' }}>
                      {log.target_id ? log.target_id.slice(0, 16) + '…' : '—'}
                    </td>
                    <td className="px-4 py-3 text-sm" style={{ color: 'var(--muted)' }}>
                      <pre className="text-xs overflow-hidden" style={{ color: 'var(--muted)' }}>{JSON.stringify(log.metadata || {}, null, 2)}</pre>
                    </td>
                    <td className="px-4 py-3 text-sm" style={{ color: 'var(--muted)' }}>
                      {formatDistanceToNow(new Date(log.created_at), { addSuffix: true })}
                    </td>
                  </tr>
                ))
              )}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  )
}

function SearchIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg> }