import { get, post, put, del } from '@/utils/http'
import type { Diary, CreateDiaryRequest, DiarySearchResult, PageResult } from '@/types'

/** 获取日记列表 */
export function getDiaryList(params?: {
  page?: number
  page_size?: number
  sort_by?: string
  order?: string
  destination_id?: number
}) {
  return get<PageResult<Diary>>('/api/diaries', params)
}

/** 获取日记详情 */
export function getDiaryDetail(id: number) {
  return get<Diary>(`/api/diaries/${id}`)
}

/** 创建日记 */
export function createDiary(data: CreateDiaryRequest) {
  return post<{ diary_id: number }>('/api/diaries', data)
}

/** 更新日记 */
export function updateDiary(id: number, data: Partial<Diary>) {
  return put<boolean>(`/api/diaries/${id}`, data)
}

/** 删除日记 */
export function deleteDiary(id: number) {
  return del<boolean>(`/api/diaries/${id}`)
}

/** 搜索日记 */
export function searchDiaries(params: {
  keyword: string
  mode?: 'exact' | 'fulltext'
  limit?: number
}) {
  return get<DiarySearchResult[]>('/api/diaries/search', params)
}

/** Huffman压缩日记 */
export function compressDiary(params: { diary_id?: number; content?: string }) {
  return post<{
    diary_id: number
    original_size: number
    compressed_size: number
    compression_ratio: number
  }>('/api/diaries/compress', params)
}

/** 评分日记 */
export function rateDiary(data: {
  user_id: number
  diary_id: number
  score: number
}) {
  return post<boolean>(`/api/diaries/${data.diary_id}/rate`, {
    user_id: data.user_id,
    score: data.score,
  })
}
