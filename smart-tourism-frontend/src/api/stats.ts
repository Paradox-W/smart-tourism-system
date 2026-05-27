import { get } from '@/utils/http'

export interface OverviewStats {
  database_status: string
  scenic_spots: number
  campus_count: number
  scenic_count: number
  nodes: number
  buildings: number
  facilities: number
  facility_types: number
  roads: number
  users: number
  diaries: number
  foods: number
  cuisines: number
  ratings: number
}

export interface RequirementCheck {
  label: string
  actual: number
  target: number
  passed: boolean
}

export interface HotSpotSummary {
  id: number
  name: string
  type: string
  category: string
  popularity: number
  rating: number
}

export interface OverviewData {
  stats: OverviewStats
  requirements: RequirementCheck[]
  hot_spots: HotSpotSummary[]
}

export function getOverviewStats() {
  return get<OverviewData>('/api/stats/overview')
}
