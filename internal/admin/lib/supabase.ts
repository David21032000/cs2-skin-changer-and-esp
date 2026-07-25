import { createClient, SupabaseClient } from '@supabase/supabase-js'

const supabaseUrl = process.env.NEXT_PUBLIC_SUPABASE_URL
const supabaseAnonKey = process.env.NEXT_PUBLIC_SUPABASE_ANON_KEY

function createMockClient(): SupabaseClient {
  const handler = {
    get(_target: any, prop: string) {
      if (prop === 'then') return undefined
      if (prop === 'auth') return {
        getSession: () => Promise.resolve({ data: { session: null }, error: null }),
        onAuthStateChange: () => ({ data: { subscription: { unsubscribe: () => {} } } }),
        signOut: () => Promise.resolve({ error: null }),
        getUser: () => Promise.resolve({ data: { user: null }, error: null }),
      }
      return (...args: any[]) => createMockClient()
    }
  }
  return new Proxy({} as SupabaseClient, handler)
}

const isBuildTime = typeof window === 'undefined' && (!supabaseUrl || !supabaseAnonKey)

export const supabase: SupabaseClient = isBuildTime
  ? createMockClient()
  : createClient(
      supabaseUrl!,
      supabaseAnonKey!
    )