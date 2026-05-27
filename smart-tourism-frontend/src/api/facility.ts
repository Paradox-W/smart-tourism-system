import { get } from '@/utils/http'
import type { FacilityItem } from '@/types'

/** 查找附近设施 */
export function findNearbyFacilities(params: {
  area_id: number
  node_id: number
  category?: string
  radius?: number
}) {
  return get<FacilityItem[]>('/api/facilities/nearby', params)
}
