import { post } from '@/utils/http'
import type { RouteRequest, SingleRouteResult, MultiRouteResult } from '@/types'

/** 单目标最短路径规划 */
export function planSingleRoute(data: RouteRequest) {
  return post<SingleRouteResult>('/api/route/single', data)
}

/** 多目标TSP路径规划 */
export function planMultiRoute(data: RouteRequest) {
  return post<MultiRouteResult>('/api/route/multi', data)
}
