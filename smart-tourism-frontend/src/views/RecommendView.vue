<template>
  <div class="page-content fade-in">
    <!-- Hero 搜索区域 -->
    <div class="mb-8">
      <div class="text-center mb-6">
        <h1 class="text-3xl font-bold mb-2" style="color: var(--color-primary)">
          发现你的下一站旅行
        </h1>
        <p class="text-base" style="color: var(--color-text-muted)">
          基于智能推荐算法，为你精选热门景点和校园
        </p>
      </div>

      <div class="flex justify-center mb-4">
        <div class="flex items-center w-full max-w-xl bg-white rounded-2xl shadow-md overflow-hidden"
             style="border: 2px solid var(--color-primary-lightest)">
          <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24"
               fill="none" stroke="var(--color-text-muted)" stroke-width="2"
               stroke-linecap="round" stroke-linejoin="round" class="ml-4 shrink-0">
            <circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/>
          </svg>
          <input
            v-model="searchKeyword"
            type="text"
            placeholder="搜索景点名称、城市、类别..."
            class="flex-1 px-3 py-3.5 text-sm outline-none bg-transparent"
            @keyup.enter="handleSearch"
          />
          <button
            class="px-6 py-3.5 text-white text-sm font-medium transition-colors cursor-pointer"
            style="background: var(--color-primary)"
            @click="handleSearch"
          >
            搜索
          </button>
        </div>
      </div>

      <!-- 快捷标签 -->
      <div class="flex justify-center gap-2 flex-wrap">
        <button
          v-for="tag in quickTags"
          :key="tag.value"
          class="px-4 py-1.5 rounded-full text-xs font-medium transition-all cursor-pointer"
          :class="activeCategory === tag.value ? 'tag-active' : 'tag-default'"
          @click="selectTag(tag.value)"
        >
          {{ tag.label }}
        </button>
      </div>
    </div>

    <!-- 排序栏 -->
    <div class="flex items-center justify-between mb-4">
      <h2 class="text-lg font-bold flex items-center gap-2" style="color: var(--color-text-primary)">
        <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24"
             fill="none" stroke="var(--color-primary)" stroke-width="2"
             stroke-linecap="round" stroke-linejoin="round">
          <polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"/>
        </svg>
        <span v-if="isSearchMode">搜索结果</span>
        <span v-else>热门推荐</span>
        <span class="text-xs font-normal" style="color: var(--color-text-muted)">
          共 {{ spots.length }} 个
        </span>
      </h2>
      <div class="flex items-center gap-1 bg-white rounded-lg p-0.5 shadow-sm">
        <button
          v-for="s in sortOptions"
          :key="s.value"
          class="px-3 py-1.5 rounded-md text-xs font-medium transition-all cursor-pointer"
          :class="sortBy === s.value ? 'sort-active' : 'sort-default'"
          @click="sortBy = s.value; loadSpots()"
        >
          {{ s.label }}
        </button>
      </div>
    </div>

    <!-- 卡片网格 -->
    <div v-if="!loading" class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-5">
      <div
        v-for="(spot, idx) in spots"
        :key="spot.id"
        class="card overflow-hidden cursor-pointer group"
        @click="showDetail(spot)"
      >
        <!-- 图片 -->
        <div class="h-44 relative overflow-hidden">
          <img
            :src="spot.image_url || `https://placehold.co/400x260/${placeholderColors[idx % placeholderColors.length]}/ffffff?text=${encodeURIComponent(spot.name.slice(0, 4))}`"
            :alt="spot.name"
            class="w-full h-full object-cover transition-transform duration-500 group-hover:scale-110"
            loading="lazy"
          />
          <div class="absolute top-3 left-3 flex gap-1.5">
            <span class="tag tag-primary">{{ spot.category || spot.type }}</span>
            <span v-if="spot.ticket_price === 0" class="tag" style="background: #E8F5E9; color: #2E7D32">免费</span>
          </div>
          <div class="absolute bottom-3 right-3 bg-black/50 text-white text-xs px-2 py-1 rounded-lg backdrop-blur-sm">
            {{ spot.city }}
          </div>
        </div>
        <!-- 信息 -->
        <div class="p-4">
          <h3 class="font-semibold text-base mb-1 truncate" style="color: var(--color-text-primary)">
            {{ spot.name }}
          </h3>
          <p class="text-xs mb-3 line-clamp-2" style="color: var(--color-text-muted); display: -webkit-box; -webkit-line-clamp: 2; -webkit-box-orient: vertical; overflow: hidden;">
            {{ spot.description || '暂无描述' }}
          </p>
          <div class="flex items-center justify-between">
            <div class="flex items-center gap-1">
              <!-- 评分星星 -->
              <div class="flex items-center gap-0.5" style="color: #FFC107">
                <svg v-for="i in 5" :key="i" xmlns="http://www.w3.org/2000/svg" width="14" height="14"
                     viewBox="0 0 24 24" :fill="i <= Math.round(spot.rating) ? '#FFC107' : '#E0E0E0'"
                     stroke="none">
                  <polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"/>
                </svg>
              </div>
              <span class="text-xs font-medium" style="color: var(--color-text-secondary)">
                {{ spot.rating?.toFixed(1) }}
              </span>
              <span class="text-xs" style="color: var(--color-text-muted)">({{ spot.rating_count }})</span>
            </div>
            <span class="text-xs font-medium" style="color: var(--color-primary)">
              热度 {{ spot.popularity }}
            </span>
          </div>
        </div>
      </div>
    </div>

    <!-- 加载骨架 -->
    <div v-else class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-5">
      <div v-for="i in 8" :key="i" class="card overflow-hidden">
        <div class="skeleton h-44" style="background: linear-gradient(135deg, #e8f5e9, #f1f8e9)"></div>
        <div class="p-4">
          <div class="skeleton h-5 w-3/4 mb-2 rounded"></div>
          <div class="skeleton h-3 w-full mb-1.5 rounded"></div>
          <div class="skeleton h-3 w-2/3 rounded"></div>
        </div>
      </div>
    </div>

    <!-- 空状态 -->
    <div v-if="!loading && spots.length === 0" class="empty-state">
      <svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 24 24"
           fill="none" stroke="currentColor" stroke-width="1.5"
           stroke-linecap="round" stroke-linejoin="round">
        <circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/>
      </svg>
      <p class="text-sm mt-3">未找到相关景点，换个关键词试试</p>
    </div>

    <!-- 分页 -->
    <div v-if="total > pageSize" class="flex justify-center mt-8">
      <el-pagination
        v-model:current-page="currentPage"
        :page-size="pageSize"
        :total="total"
        layout="prev, pager, next"
        background
        @current-change="loadSpots"
      />
    </div>

    <!-- 详情弹窗 -->
    <el-dialog
      v-model="detailVisible"
      :title="selectedSpot?.name"
      width="600px"
      destroy-on-close
    >
      <div v-if="selectedSpot">
        <div class="mb-4 rounded-xl overflow-hidden">
          <img
            :src="selectedSpot.image_url || `https://placehold.co/600x300/${placeholderColors[0]}/ffffff?text=${encodeURIComponent(selectedSpot.name)}`"
            :alt="selectedSpot.name"
            class="w-full h-56 object-cover"
          />
        </div>
        <div class="grid grid-cols-2 gap-3 mb-4">
          <div class="bg-gray-50 rounded-lg p-3">
            <div class="text-xs" style="color: var(--color-text-muted)">评分</div>
            <div class="text-lg font-bold" style="color: #FFC107">{{ selectedSpot.rating?.toFixed(1) }}</div>
          </div>
          <div class="bg-gray-50 rounded-lg p-3">
            <div class="text-xs" style="color: var(--color-text-muted)">热度</div>
            <div class="text-lg font-bold" style="color: var(--color-primary)">{{ selectedSpot.popularity }}</div>
          </div>
          <div class="bg-gray-50 rounded-lg p-3">
            <div class="text-xs" style="color: var(--color-text-muted)">门票</div>
            <div class="text-lg font-bold">{{ selectedSpot.ticket_price === 0 ? '免费' : `¥${selectedSpot.ticket_price}` }}</div>
          </div>
          <div class="bg-gray-50 rounded-lg p-3">
            <div class="text-xs" style="color: var(--color-text-muted)">开放时间</div>
            <div class="text-sm font-medium">{{ selectedSpot.open_time || '全天' }}</div>
          </div>
        </div>
        <p class="text-sm leading-relaxed" style="color: var(--color-text-secondary)">
          {{ selectedSpot.description || '暂无详细描述' }}
        </p>
        <div class="mt-3 text-xs" style="color: var(--color-text-muted)">
          📍 {{ selectedSpot.address }}，{{ selectedSpot.city }}
        </div>
      </div>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { getRecommendations, searchSpots, getSpotList } from '@/api/spot'
import type { Spot } from '@/types'

const spots = ref<Spot[]>([])
const loading = ref(false)
const searchKeyword = ref('')
const activeCategory = ref('')
const sortBy = ref('popularity')
const currentPage = ref(1)
const pageSize = ref(12)
const total = ref(0)
const isSearchMode = ref(false)
const detailVisible = ref(false)
const selectedSpot = ref<Spot | null>(null)

const quickTags = [
  { label: '全部', value: '' },
  { label: '自然风光', value: '自然' },
  { label: '人文历史', value: '历史' },
  { label: '文化体验', value: '文化' },
  { label: '现代地标', value: '现代' },
  { label: '综合休闲', value: '综合' },
  { label: '大学校园', value: 'campus' },
]

const sortOptions = [
  { label: '最热', value: 'popularity' },
  { label: '好评', value: 'rating' },
  { label: '综合', value: 'interest' },
]

const placeholderColors = [
  '2D6A4F', '40916C', '2A9D8F', '457B9D',
  '264653', '6B705C', '588157', '3A5A40',
]

async function loadSpots() {
  loading.value = true
  try {
    if (isSearchMode.value && searchKeyword.value) {
      const res = await searchSpots({ keyword: searchKeyword.value, limit: 50 })
      spots.value = res.data || []
      total.value = spots.value.length
    } else {
      const selectedType = activeCategory.value === 'campus' ? 'campus' : ''
      const selectedCategory = activeCategory.value === 'campus' ? '' : activeCategory.value
      const res = await getSpotList({
        page: currentPage.value,
        page_size: pageSize.value,
        sort_by: sortBy.value,
        order: 'desc',
        type: selectedType,
        category: selectedCategory,
      })
      const data = res.data as any
      spots.value = data?.items || data || []
      total.value = data?.total || (res as any).total || spots.value.length
    }
  } catch (e) {
    console.error('加载景点失败:', e)
    spots.value = []
  } finally {
    loading.value = false
  }
}

function handleSearch() {
  if (searchKeyword.value.trim()) {
    isSearchMode.value = true
  } else {
    isSearchMode.value = false
  }
  currentPage.value = 1
  loadSpots()
}

function selectTag(value: string) {
  activeCategory.value = value
  isSearchMode.value = false
  searchKeyword.value = ''
  currentPage.value = 1
  loadSpots()
}

function showDetail(spot: Spot) {
  selectedSpot.value = spot
  detailVisible.value = true
}

onMounted(() => {
  loadSpots()
})
</script>

<style scoped>
.tag-active {
  background: var(--color-primary);
  color: white;
}
.tag-default {
  background: white;
  color: var(--color-text-secondary);
  border: 1px solid var(--color-primary-lightest);
}
.tag-default:hover {
  border-color: var(--color-primary-light);
  color: var(--color-primary);
}
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
