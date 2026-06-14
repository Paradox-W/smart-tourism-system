<template>
  <div class="page-content fade-in">
    <!-- 头部 -->
    <div class="flex items-center justify-between mb-6">
      <div>
        <h1 class="text-2xl font-bold mb-1" style="color: var(--color-primary)">旅游日记</h1>
        <p style="color: var(--color-text-muted)">记录旅途中的精彩瞬间，支持全文检索和 Huffman 压缩</p>
      </div>
      <button
        class="flex items-center gap-2 px-5 py-2.5 rounded-xl text-white text-sm font-medium cursor-pointer transition-all hover:shadow-md"
        style="background: linear-gradient(135deg, #2D6A4F, #40916C)"
        @click="openCreateDialog"
      >
        <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24"
             fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M12 5v14"/><path d="M5 12h14"/>
        </svg>
        写日记
      </button>
    </div>

    <!-- 搜索和排序栏 -->
    <div class="bg-white rounded-2xl shadow-sm p-4 mb-6">
      <div class="flex items-center gap-4 flex-wrap">
        <div class="flex-1 min-w-[200px] flex items-center bg-gray-50 rounded-xl px-4 py-2">
          <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24"
               fill="none" stroke="var(--color-text-muted)" stroke-width="2"
               stroke-linecap="round" stroke-linejoin="round" class="shrink-0">
            <circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/>
          </svg>
          <input
            v-model="searchKeyword"
            type="text"
            placeholder="搜索日记标题或内容..."
            class="flex-1 bg-transparent text-sm outline-none ml-2"
            @keyup.enter="handleSearch"
          />
        </div>

        <div class="flex items-center gap-1">
          <button
            v-for="s in sortOptions"
            :key="s.value"
            class="px-3 py-1.5 rounded-lg text-xs font-medium transition-all cursor-pointer"
            :class="sortBy === s.value ? 'sort-active' : 'sort-default'"
            @click="sortBy = s.value; loadDiaries()"
          >
            {{ s.label }}
          </button>
        </div>

        <div class="flex items-center gap-2">
          <label class="text-xs" style="color: var(--color-text-muted)">搜索模式:</label>
          <el-radio-group v-model="searchMode" size="small">
            <el-radio-button value="fulltext">全文检索</el-radio-button>
            <el-radio-button value="exact">精确查找</el-radio-button>
          </el-radio-group>
        </div>

        <button
          class="px-4 py-2 rounded-xl text-sm font-medium cursor-pointer transition-all"
          style="background: var(--color-primary); color: white"
          @click="handleSearch"
        >
          搜索
        </button>
      </div>
    </div>

    <!-- 搜索结果提示 -->
    <div v-if="searchMode && searchKeyword && searchResults.length > 0" class="mb-4">
      <div class="text-sm mb-2" style="color: var(--color-text-secondary)">
        找到 {{ searchResults.length }} 条匹配结果：
      </div>
      <div class="flex flex-wrap gap-2">
        <div
          v-for="r in searchResults"
          :key="r.diary_id"
          class="bg-white rounded-xl px-4 py-2 shadow-sm cursor-pointer hover:shadow-md transition-shadow flex items-center gap-2"
          @click="showDiaryDetail(r.diary_id)"
        >
          <span class="text-sm font-medium" style="color: var(--color-text-primary)">{{ r.title }}</span>
          <span class="text-xs px-2 py-0.5 rounded-full"
                style="background: var(--color-primary-bg); color: var(--color-primary)">
            相关度: {{ (r.score * 100).toFixed(0) }}%
          </span>
          <span v-if="r.snippet" class="text-xs truncate max-w-[200px]" style="color: var(--color-text-muted)">
            ...{{ r.snippet }}...
          </span>
        </div>
      </div>
    </div>

    <!-- 加载中 -->
    <div v-if="loading" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-5">
      <div v-for="i in 6" :key="i" class="card overflow-hidden">
        <div class="skeleton h-36" style="background: linear-gradient(135deg, #e8f5e9, #f1f8e9)"></div>
        <div class="p-4">
          <div class="skeleton h-5 w-3/4 mb-2 rounded"></div>
          <div class="skeleton h-3 w-full mb-1 rounded"></div>
          <div class="skeleton h-3 w-2/3 rounded"></div>
        </div>
      </div>
    </div>

    <!-- 日记卡片 -->
    <div v-else-if="diaries.length > 0" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-5">
      <div
        v-for="(d, idx) in diaries"
        :key="d.id"
        class="card overflow-hidden cursor-pointer group"
        @click="showDiaryDetail(d.id)"
      >
        <div class="h-36 relative overflow-hidden">
          <img
            v-if="d.images?.length"
            :src="d.images[0]"
            :alt="d.title"
            class="w-full h-full object-cover transition-transform duration-500 group-hover:scale-110"
          />
          <div v-else class="w-full h-full flex items-center justify-center"
               style="background: linear-gradient(135deg, #e8f5e9, #f1f8e9)">
            <span class="text-4xl opacity-40">📝</span>
          </div>
          <div v-if="d.is_compressed" class="absolute top-2 right-2 bg-purple-500/90 text-white text-xs px-2 py-0.5 rounded-lg">
            已压缩
          </div>
          <div class="absolute bottom-2 right-2 bg-black/50 text-white text-xs px-2 py-0.5 rounded-lg backdrop-blur-sm">
            {{ d.destination || '未标注' }}
          </div>
        </div>
        <div class="p-4">
          <div class="flex items-center gap-2 mb-2">
            <div class="w-6 h-6 rounded-full flex items-center justify-center text-white text-xs"
                 style="background: var(--color-primary-lighter)">
              {{ (d.username || '旅')[0] }}
            </div>
            <span class="text-xs" style="color: var(--color-text-muted)">{{ d.username || '游客' }}</span>
            <span class="text-xs ml-auto" style="color: var(--color-text-muted)">{{ formatDate(d.created_at) }}</span>
          </div>
          <h3 class="font-semibold text-sm mb-1.5 truncate" style="color: var(--color-text-primary)">{{ d.title }}</h3>
          <p class="text-xs mb-3" style="color: var(--color-text-muted); display: -webkit-box; -webkit-line-clamp: 2; -webkit-box-orient: vertical; overflow: hidden;">
            {{ d.content?.slice(0, 100) || '暂无内容' }}
          </p>
          <div class="flex items-center justify-between">
            <div class="flex items-center gap-2">
              <span class="flex items-center gap-0.5 text-xs" style="color: #FFC107">
                ★ {{ d.avg_rating?.toFixed(1) || '-' }}
              </span>
              <span class="text-xs" style="color: var(--color-text-muted)">({{ d.rating_count || 0 }})</span>
            </div>
            <div class="flex items-center gap-1 text-xs" style="color: var(--color-text-muted)">
              <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24"
                   fill="none" stroke="currentColor" stroke-width="2">
                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/>
                <circle cx="12" cy="12" r="3"/>
              </svg>
              {{ d.popularity || 0 }}
            </div>
          </div>
          <!-- 标签 -->
          <div v-if="d.tags?.length" class="flex gap-1 mt-2 flex-wrap">
            <span v-for="tag in d.tags.slice(0, 3)" :key="tag"
                  class="text-xs px-2 py-0.5 rounded-full"
                  style="background: var(--color-primary-bg); color: var(--color-primary)">
              {{ tag }}
            </span>
          </div>
        </div>
      </div>
    </div>

    <!-- 空状态 -->
    <div v-else-if="!loading" class="empty-state">
      <svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 24 24"
           fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
        <path d="M14.5 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V7.5L14.5 2z"/>
        <polyline points="14 2 14 8 20 8"/>
      </svg>
      <p class="text-sm mt-3">还没有日记，快去写一篇吧</p>
    </div>

    <!-- 分页 -->
    <div v-if="total > pageSize" class="flex justify-center mt-8">
      <el-pagination
        v-model:current-page="currentPage"
        :page-size="pageSize"
        :total="total"
        layout="prev, pager, next"
        background
        @current-change="loadDiaries"
      />
    </div>

    <!-- 创建/编辑日记弹窗 -->
    <el-dialog
      v-model="createDialogVisible"
      :title="editingDiary ? '编辑日记' : '写日记'"
      width="680px"
      destroy-on-close
    >
      <div class="space-y-4">
        <div>
          <label class="block text-xs font-medium mb-1" style="color: var(--color-text-secondary)">标题</label>
          <input v-model="diaryForm.title" type="text" placeholder="给你的旅途起个标题..."
                 class="w-full px-3 py-2 rounded-lg text-sm border outline-none focus:border-green-500"
                 style="border-color: var(--color-primary-lightest)" />
        </div>
        <div>
          <label class="block text-xs font-medium mb-1" style="color: var(--color-text-secondary)">目的地</label>
          <input v-model="diaryForm.destination" type="text" placeholder="去过哪里？"
                 class="w-full px-3 py-2 rounded-lg text-sm border outline-none focus:border-green-500"
                 style="border-color: var(--color-primary-lightest)" />
        </div>
        <div>
          <label class="block text-xs font-medium mb-1" style="color: var(--color-text-secondary)">内容</label>
          <textarea v-model="diaryForm.content" rows="8" placeholder="记录旅途中的故事..."
                    class="w-full px-3 py-2 rounded-lg text-sm border outline-none resize-none focus:border-green-500"
                    style="border-color: var(--color-primary-lightest)"></textarea>
          <div class="text-xs mt-1 text-right" style="color: var(--color-text-muted)">
            {{ diaryForm.content.length }} 字
          </div>
        </div>
        <div>
          <label class="block text-xs font-medium mb-1" style="color: var(--color-text-secondary)">标签（逗号分隔）</label>
          <input v-model="diaryForm.tags" type="text" placeholder="美食, 自然, 文化..."
                 class="w-full px-3 py-2 rounded-lg text-sm border outline-none focus:border-green-500"
                 style="border-color: var(--color-primary-lightest)" />
        </div>
      </div>
      <template #footer>
        <div class="flex gap-3">
          <button class="flex-1 py-2 rounded-xl text-sm border cursor-pointer transition-colors"
                  style="border-color: var(--color-primary-lightest); color: var(--color-text-secondary)"
                  @click="createDialogVisible = false">
            取消
          </button>
          <button class="flex-1 py-2 rounded-xl text-sm text-white font-medium cursor-pointer transition-colors"
                  style="background: var(--color-primary)"
                  :disabled="savingDiary"
                  :class="{ 'opacity-60 cursor-not-allowed': savingDiary }"
                  @click="handleSaveDiary">
            {{ savingDiary ? '发布中...' : (editingDiary ? '保存修改' : '发布日记') }}
          </button>
        </div>
      </template>
    </el-dialog>

    <!-- 日记详情弹窗 -->
    <el-dialog
      v-model="detailDialogVisible"
      :title="currentDiary?.title"
      width="680px"
      destroy-on-close
    >
      <div v-if="currentDiary">
        <div class="flex items-center gap-3 mb-4">
          <div class="w-8 h-8 rounded-full flex items-center justify-center text-white text-sm"
               style="background: var(--color-primary-lighter)">
            {{ (currentDiary.username || '旅')[0] }}
          </div>
          <div>
            <div class="text-sm font-medium">{{ currentDiary.username || '游客' }}</div>
            <div class="text-xs" style="color: var(--color-text-muted)">{{ formatDate(currentDiary.created_at) }}</div>
          </div>
          <div class="ml-auto flex items-center gap-2">
            <!-- 评分 -->
            <el-rate
              v-model="userRating"
              :texts="['很差', '较差', '一般', '不错', '很棒']"
              show-text
              size="small"
              @change="handleRate"
            />
          </div>
        </div>

        <!-- 内容 -->
        <div class="text-sm leading-relaxed mb-4 whitespace-pre-wrap" style="color: var(--color-text-secondary)">
          {{ currentDiary.content || '暂无内容' }}
        </div>

        <!-- 图片 -->
        <div v-if="currentDiary.images?.length" class="flex gap-2 overflow-x-auto mb-4">
          <img v-for="(img, i) in currentDiary.images" :key="i" :src="img"
               class="w-32 h-24 rounded-lg object-cover shrink-0" />
        </div>

        <!-- 标签 -->
        <div v-if="currentDiary.tags?.length" class="flex gap-1 mb-4 flex-wrap">
          <span v-for="tag in currentDiary.tags" :key="tag"
                class="text-xs px-2 py-0.5 rounded-full"
                style="background: var(--color-primary-bg); color: var(--color-primary)">
            {{ tag }}
          </span>
        </div>

        <!-- 操作按钮 -->
        <div class="flex gap-2 pt-4 border-t" style="border-color: var(--color-primary-lightest)">
          <button
            v-if="currentDiary.user_id === auth.user?.id || auth.isAdmin"
            class="flex items-center gap-1.5 px-4 py-2 rounded-xl text-xs font-medium cursor-pointer transition-all"
            style="background: #F3E5F5; color: #7B1FA2"
            @click="handleCompress"
          >
            <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24"
                 fill="none" stroke="currentColor" stroke-width="2">
              <path d="m18 16 4-4-4-4"/><path d="m6 8-4 4 4 4"/>
              <path d="m14.5 4-5 16"/>
            </svg>
            Huffman 压缩
          </button>
          <button
            v-if="currentDiary?.is_compressed"
            class="flex items-center gap-1.5 px-4 py-2 rounded-xl text-xs font-medium cursor-pointer transition-all"
            style="background: #E8F5E9; color: #2E7D32"
            @click="handleDecompress"
          >
            <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24"
                 fill="none" stroke="currentColor" stroke-width="2">
              <path d="m18 16 4-4-4-4"/><path d="m6 8-4 4 4 4"/>
              <path d="m14.5 4-5 16"/>
            </svg>
            Huffman 解压
          </button>
          <button
            v-if="currentDiary.user_id === auth.user?.id || auth.isAdmin"
            class="flex items-center gap-1.5 px-4 py-2 rounded-xl text-xs font-medium cursor-pointer transition-all"
            style="background: var(--color-primary-bg); color: var(--color-primary)"
            @click="openEditDialog(currentDiary)"
          >
            编辑
          </button>
          <button
            class="flex items-center gap-1.5 px-4 py-2 rounded-xl text-xs font-medium cursor-pointer transition-all"
            style="background: #FFEBEE; color: #C62828"
            @click="handleDelete(currentDiary.id)"
          >
            删除
          </button>
          <div v-if="compressResult" class="ml-auto flex items-center gap-3 text-xs">
            <span style="color: var(--color-text-muted)">压缩前: {{ compressResult.original_size }}B</span>
            <span style="color: var(--color-primary)">压缩后: {{ compressResult.compressed_size }}B</span>
            <span class="font-bold" style="color: #7B1FA2">
              压缩率: {{ compressResult.compression_ratio }}%
            </span>
          </div>
        </div>
      </div>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted } from 'vue'
import { ElMessageBox, ElMessage } from 'element-plus'
import {
  getDiaryList, getDiaryDetail, createDiary, updateDiary, deleteDiary,
  searchDiaries, compressDiary, decompressDiary, rateDiary,
} from '@/api/diary'
import type { Diary, DiarySearchResult } from '@/types'
import { useAuthStore } from '@/stores/auth'

const auth = useAuthStore()
const diaries = ref<Diary[]>([])
const loading = ref(false)
const searchKeyword = ref('')
const sortBy = ref('created_at')
const searchMode = ref<'fulltext' | 'exact'>('fulltext')
const currentPage = ref(1)
const pageSize = ref(9)
const total = ref(0)

// 搜索结果
const searchResults = ref<DiarySearchResult[]>([])

// 弹窗
const createDialogVisible = ref(false)
const detailDialogVisible = ref(false)
const editingDiary = ref<Diary | null>(null)
const currentDiary = ref<Diary | null>(null)
const userRating = ref(0)
const savingDiary = ref(false)

// 压缩
const compressResult = ref<{ original_size: number; compressed_size: number; compression_ratio: number } | null>(null)

const sortOptions = [
  { label: '最新', value: 'created_at' },
  { label: '热门', value: 'popularity' },
  { label: '好评', value: 'rating' },
]

const diaryForm = reactive({
  title: '',
  content: '',
  destination: '',
  tags: '',
})

function parseTags(value: unknown): string[] {
  if (Array.isArray(value)) return value
  if (typeof value !== 'string' || !value.trim()) return []
  try {
    const parsed = JSON.parse(value)
    if (Array.isArray(parsed)) return parsed.map(String)
  } catch {
    // Fall back to comma-separated tags.
  }
  return value.split(/[,，]/).map(t => t.trim()).filter(Boolean)
}

function normalizeDiary(item: any): Diary {
  return {
    ...item,
    tags: parseTags(item.tags),
    images: Array.isArray(item.images) ? item.images : [],
    videos: Array.isArray(item.videos) ? item.videos : [],
    is_compressed: Boolean(item.is_compressed),
  }
}

function formatDate(dateStr: string): string {
  if (!dateStr) return ''
  const d = new Date(dateStr)
  return d.toLocaleDateString('zh-CN', { month: 'short', day: 'numeric' })
}

async function loadDiaries() {
  loading.value = true
  try {
    const res = await getDiaryList({
      page: currentPage.value,
      page_size: pageSize.value,
      sort_by: sortBy.value,
      order: 'desc',
    })
    const data = res.data as any
    const items = data?.items || data || []
    diaries.value = items.map(normalizeDiary)
    total.value = data?.total || diaries.value.length
  } catch (e) {
    console.error('加载日记失败:', e)
  } finally {
    loading.value = false
  }
}

async function handleSearch() {
  if (!searchKeyword.value.trim()) {
    searchResults.value = []
    loadDiaries()
    return
  }
  try {
    const res = await searchDiaries({
      keyword: searchKeyword.value,
      mode: searchMode.value,
      limit: 20,
    })
    const data = res.data as any
    const items = Array.isArray(data) ? data : (data?.data || [])
    searchResults.value = items.map((item: any) => ({
      ...item,
      diary_id: item.diary_id ?? item.id,
      score: item.score ?? item.relevance_score ?? 1,
      tags: Array.isArray(item.tags) ? item.tags : parseTags(item.tags),
      images: Array.isArray(item.images) ? item.images : [],
      videos: Array.isArray(item.videos) ? item.videos : [],
    }))
  } catch (e) {
    console.error('搜索日记失败:', e)
  }
}

function openCreateDialog() {
  editingDiary.value = null
  diaryForm.title = ''
  diaryForm.content = ''
  diaryForm.destination = ''
  diaryForm.tags = ''
  createDialogVisible.value = true
}

function openEditDialog(diary: Diary) {
  detailDialogVisible.value = false
  editingDiary.value = diary
  diaryForm.title = diary.title
  diaryForm.content = diary.content
  diaryForm.destination = diary.destination
  diaryForm.tags = Array.isArray(diary.tags) ? diary.tags.join(', ') : ''
  createDialogVisible.value = true
}

async function handleSaveDiary() {
  if (savingDiary.value) return
  if (!diaryForm.title.trim()) {
    ElMessage.warning('请输入日记标题')
    return
  }
  if (!diaryForm.content.trim()) {
    ElMessage.warning('请输入日记内容')
    return
  }
  try {
    savingDiary.value = true
    const tags = diaryForm.tags.split(/[,，]/).map(t => t.trim()).filter(Boolean)
    if (editingDiary.value) {
      await updateDiary(editingDiary.value.id, {
        title: diaryForm.title,
        content: diaryForm.content,
        destination: diaryForm.destination,
        tags,
      })
      ElMessage.success('日记已更新')
    } else {
      await createDiary({
        user_id: auth.user!.id,
        title: diaryForm.title,
        content: diaryForm.content,
        destination: diaryForm.destination,
        tags: JSON.stringify(tags),
      })
      ElMessage.success('日记已发布')
    }
    createDialogVisible.value = false
    loadDiaries()
  } catch (e) {
    console.error('保存日记失败:', e)
  } finally {
    savingDiary.value = false
  }
}

async function showDiaryDetail(id: number) {
  try {
    const res = await getDiaryDetail(id)
    currentDiary.value = normalizeDiary(res.data)
    userRating.value = currentDiary.value?.avg_rating || 0
    compressResult.value = null
    detailDialogVisible.value = true
  } catch (e) {
    console.error('加载日记详情失败:', e)
  }
}

async function handleDelete(id: number) {
  try {
    await ElMessageBox.confirm('确定要删除这篇日记吗？', '删除确认', {
      confirmButtonText: '删除',
      cancelButtonText: '取消',
      type: 'warning',
    })
    await deleteDiary(id)
    ElMessage.success('日记已删除')
    detailDialogVisible.value = false
    loadDiaries()
  } catch (e) {
    // 用户取消
  }
}

async function handleRate() {
  if (!currentDiary.value || !userRating.value) return
  try {
    await rateDiary({
      user_id: auth.user!.id,
      diary_id: currentDiary.value.id,
      score: userRating.value,
    })
    ElMessage.success('评分成功')
    loadDiaries()
  } catch (e) {
    console.error('评分失败:', e)
  }
}

async function handleCompress() {
  if (!currentDiary.value) return
  try {
    const res = await compressDiary({ diary_id: currentDiary.value.id })
    compressResult.value = res.data
    ElMessage.success(`压缩完成！压缩率: ${res.data.compression_ratio}%`)
  } catch (e) {
    console.error('压缩失败:', e)
  }
}

async function handleDecompress() {
  if (!currentDiary.value) return
  try {
    const res = await decompressDiary({ diary_id: currentDiary.value.id })
    if (res.data.content && currentDiary.value) {
      currentDiary.value.content = res.data.content
    }
    ElMessage.success(`解压完成！原始大小: ${res.data.original_size}B`)
  } catch (e) {
    console.error('解压失败:', e)
  }
}

onMounted(() => {
  loadDiaries()
})
</script>

<style scoped>
.sort-active {
  background: var(--color-primary-bg);
  color: var(--color-primary);
}
.sort-default {
  color: var(--color-text-muted);
}
.sort-default:hover {
  color: var(--color-primary);
}
</style>
