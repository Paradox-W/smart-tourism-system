<template>
  <div class="page-content fade-in">
    <div class="mb-6">
      <h1 class="text-2xl font-bold mb-1" style="color: var(--color-primary)">场所查询</h1>
      <p style="color: var(--color-text-muted)">基于实际可达路径距离（Dijkstra），查找周边设施</p>
    </div>

    <!-- 筛选栏 -->
    <div class="bg-white rounded-2xl shadow-sm p-5 mb-6">
      <div class="flex flex-wrap items-end gap-4">
        <div class="flex-1 min-w-[180px]">
          <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
            所在区域
          </label>
          <select
            v-model="areaId"
            class="w-full px-3 py-2 rounded-lg text-sm border outline-none focus:border-green-500"
            style="border-color: var(--color-primary-lightest)"
            @change="loadNodes()"
          >
            <option value="">选择景区/校园</option>
            <option v-for="s in spotOptions" :key="s.id" :value="s.id">{{ s.name }}</option>
          </select>
        </div>

        <div class="flex-1 min-w-[180px]">
          <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
            当前位置
          </label>
          <select
            v-model="nodeId"
            class="w-full px-3 py-2 rounded-lg text-sm border outline-none focus:border-green-500"
            style="border-color: var(--color-primary-lightest)"
          >
            <option value="">选择建筑/节点</option>
            <option v-for="n in nodeOptions" :key="n.id" :value="n.id">
              {{ n.name }} ({{ n.type }}{{ n.sub_type ? '/' + n.sub_type : '' }})
            </option>
          </select>
        </div>

        <div class="w-36">
          <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
            搜索半径（米）
          </label>
          <input
            v-model.number="radius"
            type="number"
            min="100"
            max="3000"
            step="100"
            class="w-full px-3 py-2 rounded-lg text-sm border outline-none focus:border-green-500"
            style="border-color: var(--color-primary-lightest)"
          />
        </div>

        <button
          :disabled="!canSearch"
          class="px-6 py-2 rounded-xl text-white text-sm font-medium transition-all cursor-pointer disabled:opacity-40 disabled:cursor-not-allowed"
          style="background: var(--color-primary)"
          @click="handleSearch"
        >
          查询
        </button>
      </div>
    </div>

    <!-- 设施类别选择 -->
    <div class="flex gap-2 mb-6 flex-wrap">
      <button
        v-for="cat in categories"
        :key="cat.value"
        class="flex items-center gap-1.5 px-4 py-2 rounded-xl text-sm font-medium transition-all cursor-pointer"
        :class="activeCategory === cat.value ? 'cat-active' : 'cat-default'"
        @click="selectCategory(cat.value)"
      >
        <span>{{ cat.icon }}</span>
        <span>{{ cat.label }}</span>
      </button>
    </div>

    <!-- 算法说明 -->
    <div v-if="canSearch" class="bg-green-50 rounded-xl p-4 mb-6 flex items-start gap-3">
      <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24"
           fill="none" stroke="var(--color-primary)" stroke-width="2"
           stroke-linecap="round" stroke-linejoin="round" class="shrink-0 mt-0.5">
        <circle cx="12" cy="12" r="10"/><path d="M12 16v-4"/><path d="M12 8h.01"/>
      </svg>
      <div class="text-xs leading-relaxed" style="color: var(--color-text-secondary)">
        <strong style="color: var(--color-primary)">算法说明：</strong>
        使用 <strong>Dijkstra 最短路径算法</strong> 计算从当前位置到所有设施节点的实际可达路径距离，
        而非简单的直线距离。结果按路径距离升序排列，确保推荐的是步行可达距离最近的设施。
        直线距离仅用于半径预过滤。
      </div>
    </div>

    <!-- 查询结果 -->
    <div v-if="loading" class="space-y-3">
      <div v-for="i in 5" :key="i" class="bg-white rounded-xl p-4 flex items-center gap-4 shadow-sm">
        <div class="w-12 h-12 rounded-xl skeleton shrink-0"></div>
        <div class="flex-1">
          <div class="skeleton h-5 w-1/2 mb-2 rounded"></div>
          <div class="skeleton h-3 w-full rounded"></div>
        </div>
        <div class="skeleton h-8 w-20 rounded-lg"></div>
      </div>
    </div>

    <div v-else-if="facilities.length > 0">
      <div class="flex items-center justify-between mb-4">
        <h3 class="text-base font-medium" style="color: var(--color-text-primary)">
          查询结果（按路径距离排序）
        </h3>
        <span class="text-xs" style="color: var(--color-text-muted)">
          找到 {{ facilities.length }} 个设施
        </span>
      </div>

      <div class="space-y-3">
        <div
          v-for="(f, idx) in facilities"
          :key="f.node_id || f.id"
          class="bg-white rounded-xl p-4 flex items-center gap-4 shadow-sm hover:shadow-md transition-shadow slide-up"
          :style="{ animationDelay: `${idx * 0.05}s` }"
        >
          <!-- 图标 -->
          <div class="w-12 h-12 rounded-xl flex items-center justify-center text-xl shrink-0"
               style="background: var(--color-primary-bg)">
            {{ getCategoryIcon(f.sub_type) }}
          </div>

          <!-- 信息 -->
          <div class="flex-1 min-w-0">
            <div class="flex items-center gap-2 mb-1">
              <h4 class="text-sm font-semibold truncate" style="color: var(--color-text-primary)">
                {{ f.name }}
              </h4>
              <span class="tag tag-primary text-xs">{{ f.sub_type || f.type }}</span>
            </div>
            <p class="text-xs truncate" style="color: var(--color-text-muted)">
              {{ f.description || '暂无描述' }}
            </p>
            <div class="flex items-center gap-3 mt-1 text-xs" style="color: var(--color-text-muted)">
              <span>直线距离: {{ f.direct_distance?.toFixed(0) || '-' }}m</span>
              <span style="color: var(--color-primary); font-weight: 500">
                路径距离: {{ f.distance?.toFixed(0) || '-' }}m
              </span>
            </div>
          </div>

          <!-- 距离标签 -->
          <div class="shrink-0 text-center">
            <div class="text-lg font-bold" :style="{ color: getDistanceColor(f.distance) }">
              {{ formatDistance(f.distance) }}
            </div>
            <div class="text-xs" style="color: var(--color-text-muted)">步行</div>
          </div>
        </div>
      </div>
    </div>

    <!-- 空状态 -->
    <div v-else-if="searched" class="empty-state">
      <svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 24 24"
           fill="none" stroke="currentColor" stroke-width="1.5"
           stroke-linecap="round" stroke-linejoin="round">
        <circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/>
      </svg>
      <p class="text-sm mt-3">附近未找到符合条件的设施</p>
      <p class="text-xs mt-1" style="color: var(--color-text-muted)">尝试增大搜索半径或更换类别</p>
    </div>

    <!-- 未查询状态 -->
    <div v-else class="empty-state">
      <svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 24 24"
           fill="none" stroke="currentColor" stroke-width="1.5"
           stroke-linecap="round" stroke-linejoin="round">
        <path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/>
        <polyline points="9 22 9 12 15 12 15 22"/>
      </svg>
      <p class="text-sm mt-3">选择位置和类别后查询附近设施</p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { getSpotOptions } from '@/api/spot'
import { findNearbyFacilities } from '@/api/facility'
import type { Spot, MapNode, FacilityItem } from '@/types'

const spotOptions = ref<Spot[]>([])
const nodeOptions = ref<MapNode[]>([])
const areaId = ref<number | string>('')
const nodeId = ref<number | string>('')
const radius = ref(500)
const activeCategory = ref('')
const loading = ref(false)
const searched = ref(false)
const facilities = ref<FacilityItem[]>([])

const categories = [
  { label: '全部', value: '', icon: '📋' },
  { label: '卫生间', value: 'toilet', icon: '🚻' },
  { label: '餐饮', value: 'restaurant', icon: '🍽️' },
  { label: '超市/商店', value: 'shop', icon: '🛒' },
  { label: '咖啡馆', value: 'cafe', icon: '☕' },
  { label: '图书馆', value: 'library', icon: '📚' },
  { label: '医疗点', value: 'medical', icon: '🏥' },
  { label: '停车场', value: 'parking', icon: '🅿️' },
  { label: 'ATM', value: 'atm', icon: '💳' },
  { label: '出入口', value: 'entrance', icon: '🚪' },
  { label: '服务中心', value: 'service_center', icon: 'ℹ️' },
  { label: '观景点', value: 'viewpoint', icon: '🗺️' },
]

const canSearch = computed(() =>
  areaId.value && nodeId.value
)

function getCategoryIcon(subType: string): string {
  const map: Record<string, string> = {
    toilet: '🚻', restaurant: '🍽️', cafe: '☕', shop: '🛒',
    library: '📚', medical: '🏥', parking: '🅿️', atm: '💳',
    entrance: '🚪', landmark: '🏛️', service_center: 'ℹ️', viewpoint: '🗺️',
  }
  for (const [key, icon] of Object.entries(map)) {
    if (subType?.includes(key)) return icon
  }
  return '📍'
}

function getDistanceColor(dist: number): string {
  if (dist <= 100) return '#2E7D32'
  if (dist <= 300) return '#40916C'
  if (dist <= 500) return '#F4A261'
  return '#E63946'
}

function formatDistance(dist: number): string {
  if (dist < 1000) return `${Math.round(dist)}m`
  return `${(dist / 1000).toFixed(1)}km`
}

function pickDemoReadySpot(options: Spot[]): Spot | undefined {
  return options.find(s => s.id >= 1001) || options[0]
}

function pickStartNode(nodes: MapNode[]): MapNode | undefined {
  return nodes.find(n => n.type === 'building') ||
    nodes.find(n => n.type === 'entrance') ||
    nodes[0]
}

function selectCategory(value: string) {
  activeCategory.value = value
  if (canSearch.value) handleSearch()
}

function normalizeFacility(item: any): FacilityItem {
  return {
    ...item,
    node_id: item.node_id ?? item.id,
    type: item.type ?? 'facility',
    distance: item.distance ?? item.path_distance ?? 0,
    direct_distance: item.direct_distance ?? item.straight_distance ?? 0,
    description: item.description ?? '',
    pos_x: item.pos_x ?? 0,
    pos_y: item.pos_y ?? 0,
  }
}

async function loadNodes(autoSearch = false) {
  if (!areaId.value) {
    nodeOptions.value = []
    nodeId.value = ''
    return
  }
  try {
    // 复用 graph API 获取节点列表
    const { get } = await import('@/utils/http')
    const res = await get<{ nodes: MapNode[] }>(`/api/map/graph/${areaId.value}`)
    nodeOptions.value = res.data?.nodes || []
    const startNode = pickStartNode(nodeOptions.value)
    nodeId.value = startNode?.id || ''
    facilities.value = []
    searched.value = false
    if (autoSearch && canSearch.value) {
      await handleSearch()
    }
  } catch (e) {
    console.error('加载节点失败:', e)
  }
}

async function handleSearch() {
  if (!canSearch.value) return
  loading.value = true
  searched.value = true
  try {
    const res = await findNearbyFacilities({
      area_id: Number(areaId.value),
      node_id: Number(nodeId.value),
      category: activeCategory.value || undefined,
      radius: radius.value,
    })
    const data = res.data as any
    const items = Array.isArray(data) ? data : (data?.items || [])
    facilities.value = items.map(normalizeFacility)
  } catch (e) {
    console.error('查询设施失败:', e)
    facilities.value = []
  } finally {
    loading.value = false
  }
}

onMounted(() => {
  getSpotOptions().then(res => {
    spotOptions.value = res.data || []
    const selected = pickDemoReadySpot(spotOptions.value)
    if (selected) {
      areaId.value = selected.id
      loadNodes(true)
    }
  })
})
</script>

<style scoped>
.cat-active {
  background: var(--color-primary);
  color: white;
}
.cat-default {
  background: white;
  color: var(--color-text-secondary);
  border: 1px solid #e5e7eb;
}
.cat-default:hover {
  border-color: var(--color-primary-light);
  color: var(--color-primary);
}
</style>
