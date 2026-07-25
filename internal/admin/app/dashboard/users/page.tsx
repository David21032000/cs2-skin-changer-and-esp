'use client'

import { useState, useEffect } from 'react'
import { supabase } from '@/lib/supabase'
import { formatDistanceToNow } from 'date-fns'

export default function UsersPage() {
  const [users, setUsers] = useState<any[]>([])
  const [loading, setLoading] = useState(true)
  const [search, setSearch] = useState('')
  const [roleFilter, setRoleFilter] = useState<'all' | 'admin' | 'user'>('all')
  const [currentPage, setCurrentPage] = useState(1)
  const [totalCount, setTotalCount] = useState(0)
  const PAGE_SIZE = 25

  useEffect(() => {
    fetchUsers()
    fetchCount()
  }, [currentPage, search, roleFilter])

  const fetchUsers = async () => {
    setLoading(true)
    let query = supabase
      .from('users')
      .select('*', { count: 'exact' })
      .order('created_at', { ascending: false })
      .range((currentPage - 1) * PAGE_SIZE, currentPage * PAGE_SIZE - 1)

    if (search) {
      query = query.ilike('email', `%${search}%`)
    }
    if (roleFilter !== 'all') {
      query = query.eq('role', roleFilter)
    }

    const { data } = await query
    if (data) setUsers(data)
    setLoading(false)
  }

  const fetchCount = async () => {
    let query = supabase.from('users').select('*', { count: 'exact', head: true })
    if (search) query = query.ilike('email', `%${search}%`)
    if (roleFilter !== 'all') query = query.eq('role', roleFilter)
    const { count } = await query
    if (count !== null) setTotalCount(count)
  }

  const toggleRole = async (id: string, currentRole: string) => {
    const newRole = currentRole === 'admin' ? 'user' : 'admin'
    await supabase.from('users').update({ role: newRole }).eq('id', id)
    fetchUsers()
  }

  const deleteUser = async (id: string) => {
    if (!confirm('Delete this user and all their keys?')) return
    await supabase.from('users').delete().eq('id', id)
    fetchUsers()
    fetchCount()
  }

  const getStatus = (user: any) => {
    const roleClass = user.role === 'admin'
      ? 'bg-[var(--accent-dim)] text-[var(--accent)]'
      : 'bg-[var(--pink)]/20 text-[var(--pink)]'
    const role = user.role === 'admin' ? 'Admin' : 'User'
    const trialClass = user.trial_used
      ? 'bg-[var(--pink)]/20 text-[var(--pink)]'
      : 'bg-[var(--accent-dim)] text-[var(--accent)]'
    const trial = user.trial_used ? 'Used' : 'Available'
    return { role, roleClass, trial, trialClass }
  }

  return (
    <div className="space-y-6">
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
        <div>
          <h1 className="text-2xl font-bold">Users</h1>
          <p style={{ color: 'var(--muted)' }}>Manage admin and trial users</p>
        </div>
      </div>

      <div className="flex flex-wrap gap-4" style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '12px', padding: '1rem' }}>
        <div className="relative flex-1 min-w-[200px]">
          <SearchIcon className="absolute left-3 top-1/2 -translate-y-1/2 w-5 h-5" style={{ color: 'var(--muted)' }} />
          <input type="text" value={search} onChange={e => setSearch(e.target.value)} placeholder="Search users..." className="w-full pl-10 pr-4 py-2 input" />
        </div>
        <select value={roleFilter} onChange={e => setRoleFilter(e.target.value as any)} className="input-sm w-auto">
          <option value="all">All Roles</option>
          <option value="admin">Admins</option>
          <option value="user">Users</option>
        </select>
      </div>

      <div style={{ background: 'var(--card)', border: '1px solid var(--border)', borderRadius: '12px', overflow: 'hidden' }}>
        <div className="overflow-x-auto">
          <table className="w-full">
            <thead>
              <tr className="border-b" style={{ borderColor: 'var(--border)' }}>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>USER</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>ROLE</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>TRIAL</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>HWID</th>
                <th className="text-left px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>CREATED</th>
                <th className="text-right px-4 py-3 text-xs uppercase tracking-wider" style={{ color: 'var(--muted)' }}>ACTIONS</th>
              </tr>
            </thead>
            <tbody>
              {loading ? (
                <tr><td colSpan={6} className="px-4 py-8 text-center" style={{ color: 'var(--muted)' }}>Loading...</td></tr>
              ) : users.length === 0 ? (
                <tr><td colSpan={6} className="px-4 py-12 text-center" style={{ color: 'var(--muted)' }}>No users found</td></tr>
              ) : (
                users.map(user => {
                  const status = getStatus(user)
                  return (
                    <tr key={user.id} className="border-b hover:bg-[var(--bg-elevated)]" style={{ borderColor: 'var(--border)' }}>
                      <td className="px-4 py-3">
                        <p className="font-medium" style={{ color: 'var(--fg)' }}>{user.email}</p>
                      </td>
                      <td className="px-4 py-3">
                        <span className={`inline-flex items-center gap-1.5 px-2.5 py-1 rounded text-xs font-medium ${status.roleClass}`}>
                          {status.role}
                        </span>
                      </td>
                      <td className="px-4 py-3">
                        <span className={`inline-flex items-center gap-1.5 px-2.5 py-1 rounded text-xs font-medium ${status.trialClass}`}>
                          {status.trial}
                        </span>
                      </td>
                      <td className="px-4 py-3 font-mono text-xs" style={{ color: 'var(--muted)' }}>
                        {user.hwid ? user.hwid.slice(0, 16) + '…' : <span style={{ color: 'var(--muted)' }}>—</span>}
                      </td>
                      <td className="px-4 py-3 text-sm" style={{ color: 'var(--muted)' }}>
                        {formatDistanceToNow(new Date(user.created_at), { addSuffix: true })}
                      </td>
                      <td className="px-4 py-3 text-right pr-4">
                        <div className="flex items-center justify-end gap-2">
                          <button onClick={() => toggleRole(user.id, user.role)} className="p-1.5 rounded hover:bg-[var(--bg-elevated)]" style={{ border: '1px solid var(--border)' }} title={user.role === 'admin' ? 'Make User' : 'Make Admin'}>
                            {user.role === 'admin' ? <UserMinusIcon className="w-4 h-4" style={{ color: 'var(--pink)' }} /> : <UserPlusIcon className="w-4 h-4" style={{ color: 'var(--accent)' }} />}
                          </button>
                          <button onClick={() => deleteUser(user.id)} className="p-1.5 rounded hover:bg-[var(--pink)]/10 hover:border-[var(--pink)]" style={{ border: '1px solid var(--border)' }} title="Delete">
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
      </div>
    </div>
  )
}

function SearchIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg> }
function UserPlusIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><path d="M16 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/><circle cx="8.5" cy="7" r="4"/><line x1="20" y1="8" x2="20" y2="14"/><line x1="23" y1="11" x2="17" y2="11"/></svg> }
function UserMinusIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><path d="M16 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/><circle cx="8.5" cy="7" r="4"/><line x1="23" y1="11" x2="17" y2="11"/></svg> }
function TrashIcon({ className, style }: { className?: string; style?: React.CSSProperties }) { return <svg className={className} style={style} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg> }