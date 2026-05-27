import { get } from '@/utils/http'
import type { Spot, PageResult, GraphData } from '@/types'

/** 获取所有景区/校园简要列表（用于下拉选择） */
export function getSpotOptions(params?: { type?: string }) {
  return get<Spot[]>('/api/spots/options', params)
}

/** 景点推荐 */
export function getRecommendations(params?: {
  sort_by?: 'popularity' | 'rating' | 'interest'
  limit?: number
  user_id?: number
  type?: string
  category?: string
}) {
  return get<Spot[]>('/api/spots/recommend', params)
}

/** 搜索景点 */
export function searchSpots(params: {
  keyword: string
  limit?: number
}) {
  return get<Spot[]>('/api/spots/search', params)
}

/** 获取景点列表（分页） */
export function getSpotList(params?: {
  page?: number
  page_size?: number
  sort_by?: string
  order?: string
  type?: string
  category?: string
}) {
  return get<PageResult<Spot>>('/api/spots', params)
}

/** 获取景点详情 */
export function getSpotDetail(id: number) {
  return get<Spot>(`/api/spots/${id}`)
}

/** 获取路网数据 */
export function getGraphData(areaId: number) {
  return get<GraphData>(`/api/map/graph/${areaId}`)
}
