<template>
  <div class="page-content fade-in">
    <div class="flex gap-5" style="height: calc(100vh - 112px)">
      <!-- 左侧控制面板 -->
      <div class="w-72 shrink-0 bg-white rounded-2xl shadow-sm flex flex-col overflow-hidden">
        <div class="p-4 border-b" style="border-color: var(--color-primary-lightest)">
          <h2 class="text-base font-bold flex items-center gap-2" style="color: var(--color-primary)">
            <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24"
                 fill="none" stroke="currentColor" stroke-width="2"
                 stroke-linecap="round" stroke-linejoin="round">
              <circle cx="12" cy="12" r="10"/>
              <polygon points="16.24 7.76 14.12 14.12 7.76 16.24 9.88 9.88 16.24 7.76"/>
            </svg>
            路线规划
          </h2>
        </div>

        <div class="flex-1 overflow-y-auto p-4 flex flex-col gap-4">
          <!-- 选择区域 -->
          <div>
            <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
              选择景区/校园
            </label>
            <select
              v-model="selectedAreaId"
              class="w-full px-3 py-2 rounded-lg text-sm border outline-none transition-colors focus:border-green-500"
              style="border-color: var(--color-primary-lightest)"
              @change="loadGraphData"
            >
              <option value="">请选择...</option>
              <option v-for="s in spotOptions" :key="s.id" :value="s.id">
                {{ s.name }}
              </option>
            </select>
          </div>

          <!-- 策略选择 -->
          <div>
            <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
              规划策略
            </label>
            <div class="grid grid-cols-3 gap-1.5">
              <button
                v-for="s in strategies"
                :key="s.value"
                class="px-2 py-2 rounded-lg text-xs font-medium text-center transition-all cursor-pointer"
                :class="strategy === s.value ? 'strategy-active' : 'strategy-default'"
                @click="strategy = s.value"
              >
                {{ s.label }}
              </button>
            </div>
          </div>

          <!-- 选中的节点信息 -->
          <div v-if="startNode" class="bg-green-50 rounded-xl p-3">
            <div class="text-xs" style="color: var(--color-primary)">起点</div>
            <div class="text-sm font-medium truncate">{{ startNode.name }}</div>
          </div>
          <div v-if="endNode" class="bg-blue-50 rounded-xl p-3">
            <div class="text-xs" style="color: #1565C0">终点</div>
            <div class="text-sm font-medium truncate">{{ endNode.name }}</div>
          </div>

          <!-- 途经点 -->
          <div v-if="waypoints.length > 0">
            <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
              途经点 ({{ waypoints.length }})
            </label>
            <div class="flex flex-col gap-1">
              <div v-for="(wp, i) in waypoints" :key="wp.id"
                   class="flex items-center gap-2 bg-gray-50 rounded-lg px-3 py-1.5 text-xs">
                <span class="font-medium" style="color: var(--color-primary)">{{ i + 1 }}</span>
                <span class="flex-1 truncate">{{ wp.name }}</span>
                <button class="text-red-400 hover:text-red-600 cursor-pointer" @click="removeWaypoint(i)">×</button>
              </div>
            </div>
          </div>

          <!-- 操作按钮 -->
          <div class="flex flex-col gap-2">
            <button
              :disabled="!canPlan"
              class="w-full py-2.5 rounded-xl text-white text-sm font-medium transition-all cursor-pointer disabled:opacity-40 disabled:cursor-not-allowed"
              :style="{ background: canPlan ? 'var(--color-primary)' : '#ccc' }"
              @click="planRoute"
            >
              {{ waypoints.length > 0 ? 'TSP 多目标规划' : '规划最短路径' }}
            </button>
            <button
              v-if="routeResult"
              class="w-full py-2 rounded-xl text-sm font-medium border transition-all cursor-pointer"
              style="border-color: var(--color-primary-lightest); color: var(--color-text-secondary)"
              @click="clearRoute"
            >
              清除路径
            </button>
            <div class="flex gap-2">
              <button
                class="flex-1 py-2 rounded-xl text-xs font-medium border transition-all cursor-pointer"
                style="border-color: var(--color-primary-lightest); color: var(--color-text-secondary)"
                @click="clearAll"
              >
                重置
              </button>
              <button
                class="flex-1 py-2 rounded-xl text-xs font-medium border transition-all cursor-pointer"
                style="border-color: var(--color-primary-lightest); color: var(--color-text-secondary)"
                @click="addCurrentAsWaypoint"
              >
                添加途经点
              </button>
            </div>
          </div>
        </div>

        <!-- 路径结果 -->
        <div v-if="routeResult" class="border-t p-4" style="border-color: var(--color-primary-lightest)">
          <div class="text-xs font-medium mb-2" style="color: var(--color-primary)">规划结果</div>
          <div class="grid grid-cols-2 gap-2 mb-2">
            <div class="bg-green-50 rounded-lg p-2 text-center">
              <div class="text-xs" style="color: var(--color-text-muted)">距离</div>
              <div class="text-base font-bold" style="color: var(--color-primary)">
                {{ routeDistance }}m
              </div>
            </div>
            <div class="bg-blue-50 rounded-lg p-2 text-center">
              <div class="text-xs" style="color: var(--color-text-muted)">时间</div>
              <div class="text-base font-bold" style="color: #1565C0">
                {{ routeTime }}min
              </div>
            </div>
          </div>
          <div class="text-xs leading-relaxed" style="color: var(--color-text-secondary)">
            <div class="font-medium mb-1">途经节点：</div>
            <span v-for="(name, i) in routeNodeNames" :key="i">
              {{ name }}<span v-if="i < routeNodeNames.length - 1"> → </span>
            </span>
          </div>
        </div>
      </div>

      <!-- 中央地图区域 -->
      <div class="flex-1 rounded-2xl overflow-hidden shadow-sm relative">
        <div class="map-canvas-wrapper w-full h-full">
          <canvas
            ref="canvasRef"
            class="w-full h-full cursor-crosshair"
            @click="handleCanvasClick"
            @mousemove="handleCanvasMouseMove"
          />
        </div>
        <!-- 地图提示 -->
        <div v-if="!selectedAreaId" class="absolute inset-0 flex items-center justify-center">
          <div class="text-center" style="color: var(--color-text-muted)">
            <svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 24 24"
                 fill="none" stroke="currentColor" stroke-width="1.5"
                 stroke-linecap="round" stroke-linejoin="round" class="mx-auto mb-3 opacity-40">
              <path d="M14.106 5.553a2 2 0 0 0 1.788 0l3.659-1.83A1 1 0 0 1 21 4.619v12.764a1 1 0 0 1-.553.894l-4.553 2.277a2 2 0 0 1-1.788 0l-4.212-2.106a2 2 0 0 0-1.788 0l-3.659 1.83A1 1 0 0 1 3 19.381V6.618a1 1 0 0 1 .553-.894l4.553-2.277a2 2 0 0 1 1.788 0z"/>
              <path d="M15 5.764v15"/><path d="M9 3.236v15"/>
            </svg>
            <p class="text-sm font-medium">请先在左侧选择景区或校园</p>
            <p class="text-xs mt-1">选择后将显示路网图</p>
          </div>
        </div>
        <!-- 操作提示 -->
        <div v-if="selectedAreaId && !routeResult" class="absolute bottom-4 left-4 bg-white/90 backdrop-blur-sm rounded-xl px-4 py-2 shadow-sm text-xs"
             style="color: var(--color-text-secondary)">
          💡 左键点击选择起点 → 终点，右键添加途经点
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted, watch, nextTick } from 'vue'
import { getGraphData, getSpotOptions } from '@/api/spot'
import { planSingleRoute, planMultiRoute } from '@/api/route'
import type { Spot, MapNode, Road, SingleRouteResult, MultiRouteResult } from '@/types'

const canvasRef = ref<HTMLCanvasElement | null>(null)
let ctx: CanvasRenderingContext2D | null = null

// 数据
const spotOptions = ref<Spot[]>([])
const selectedAreaId = ref<number | string>('')
const graphNodes = ref<MapNode[]>([])
const graphRoads = ref<Road[]>([])

// 选中的节点
const startNode = ref<MapNode | null>(null)
const endNode = ref<MapNode | null>(null)
const waypoints = ref<MapNode[]>([])
const selectMode = ref<'start' | 'end' | 'waypoint'>('start')
const hoveredNode = ref<MapNode | null>(null)

// 规划
const strategy = ref<'distance' | 'time' | 'mixed'>('distance')
const routeResult = ref<SingleRouteResult | MultiRouteResult | null>(null)
const loading = ref(false)

const strategies = [
  { label: '最短距离', value: 'distance' as const },
  { label: '最短时间', value: 'time' as const },
  { label: '混合策略', value: 'mixed' as const },
]

const canPlan = computed(() =>
  selectedAreaId.value && startNode.value && endNode.value
)

const routeNodeNames = computed(() => {
  if (!routeResult.value) return []
  const r = routeResult.value
  if (r.path_names?.length) return r.path_names
  if (r.node_names?.length) return r.node_names
  if ('segments' in r) {
    return r.path.map((id: number) => {
      const n = graphNodes.value.find(node => node.id === id)
      return n?.name || `#${id}`
    })
  }
  return r.path.map((id: number) => {
    const n = graphNodes.value.find(node => node.id === id)
    return n?.name || `#${id}`
  })
})

const routeDistance = computed(() => {
  if (!routeResult.value) return 0
  const r = routeResult.value
  return Math.round(r.total_distance ?? r.distance ?? 0)
})

const routeTime = computed(() => {
  if (!routeResult.value) return 0
  const r = routeResult.value
  return Math.round(r.total_time ?? r.time ?? 0)
})

// Canvas 渲染参数
const CANVAS_PADDING = 40
let scale = 1
let offsetX = 0
let offsetY = 0

function calcCanvasTransform() {
  if (!graphNodes.value.length || !canvasRef.value) return
  const canvas = canvasRef.value
  const rect = canvas.parentElement!.getBoundingClientRect()
  canvas.width = rect.width * window.devicePixelRatio
  canvas.height = rect.height * window.devicePixelRatio
  canvas.style.width = rect.width + 'px'
  canvas.style.height = rect.height + 'px'

  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity
  for (const n of graphNodes.value) {
    minX = Math.min(minX, n.pos_x); minY = Math.min(minY, n.pos_y)
    maxX = Math.max(maxX, n.pos_x); maxY = Math.max(maxY, n.pos_y)
  }

  const rangeX = maxX - minX || 1
  const rangeY = maxY - minY || 1
  const drawW = canvas.width - CANVAS_PADDING * 2
  const drawH = canvas.height - CANVAS_PADDING * 2
  scale = Math.min(drawW / rangeX, drawH / rangeY)
  offsetX = (canvas.width - rangeX * scale) / 2 - minX * scale
  offsetY = (canvas.height - rangeY * scale) / 2 - minY * scale
}

function toCanvas(x: number, y: number): [number, number] {
  return [x * scale + offsetX, y * scale + offsetY]
}

function drawGraph() {
  if (!ctx || !canvasRef.value) return
  const canvas = canvasRef.value
  ctx!.clearRect(0, 0, canvas.width, canvas.height)
  ctx!.scale(window.devicePixelRatio, window.devicePixelRatio)

  // 获取路径集合
  const routePathIds = new Set<number>()
  if (routeResult.value) {
    if ('segments' in routeResult.value) {
      routeResult.value.path.forEach((id: number) => routePathIds.add(id))
    } else {
      routeResult.value.path.forEach((id: number) => routePathIds.add(id))
    }
  }

  // 绘制道路
  for (const road of graphRoads.value) {
    const fromN = graphNodes.value.find(n => n.id === road.from_node)
    const toN = graphNodes.value.find(n => n.id === road.to_node)
    if (!fromN || !toN) continue

    const [x1, y1] = toCanvas(fromN.pos_x, fromN.pos_y)
    const [x2, y2] = toCanvas(toN.pos_x, toN.pos_y)

    const isOnRoute = routePathIds.has(road.from_node) && routePathIds.has(road.to_node)

    ctx!.beginPath()
    ctx!.moveTo(x1 / window.devicePixelRatio, y1 / window.devicePixelRatio)
    ctx!.lineTo(x2 / window.devicePixelRatio, y2 / window.devicePixelRatio)

    if (isOnRoute) {
      ctx!.strokeStyle = '#E63946'
      ctx!.lineWidth = 4
      ctx!.shadowColor = '#E63946'
      ctx!.shadowBlur = 6
    } else {
      // 拥挤度颜色: 0=绿 0.5=黄 1=红
      const c = road.congestion
      const r = Math.round(200 * c + 80)
      const g = Math.round(200 * (1 - c) + 80)
      ctx!.strokeStyle = `rgba(${r}, ${g}, 100, 0.6)`
      ctx!.lineWidth = 2
      ctx!.shadowBlur = 0
    }
    ctx!.stroke()
    ctx!.shadowBlur = 0

    // 方向箭头
    if (road.is_bidirectional !== 1) {
      const mx = (x1 + x2) / 2
      const my = (y1 + y2) / 2
      const angle = Math.atan2((y2 - y1), (x2 - x1))
      ctx!.beginPath()
      ctx!.fillStyle = ctx!.strokeStyle as string
      const ax = mx / window.devicePixelRatio
      const ay = my / window.devicePixelRatio
      ctx!.moveTo(ax + 6 * Math.cos(angle), ay + 6 * Math.sin(angle))
      ctx!.lineTo(ax + 6 * Math.cos(angle + 2.5), ay + 6 * Math.sin(angle + 2.5))
      ctx!.lineTo(ax + 6 * Math.cos(angle - 2.5), ay + 6 * Math.sin(angle - 2.5))
      ctx!.fill()
    }
  }

  // 绘制节点
  for (const node of graphNodes.value) {
    const [cx, cy] = toCanvas(node.pos_x, node.pos_y)
    const x = cx / window.devicePixelRatio
    const y = cy / window.devicePixelRatio
    const isRoute = routePathIds.has(node.id)
    const isStart = startNode.value?.id === node.id
    const isEnd = endNode.value?.id === node.id
    const isWaypoint = waypoints.value.some(w => w.id === node.id)
    const isHovered = hoveredNode.value?.id === node.id

    let radius = 6
    let fillColor = '#8B9E93'
    let strokeColor = '#fff'

    if (isStart) { radius = 10; fillColor = '#2D6A4F'; }
    else if (isEnd) { radius = 10; fillColor = '#E63946'; }
    else if (isWaypoint) { radius = 8; fillColor = '#F4A261'; }
    else if (isRoute) { radius = 7; fillColor = '#2A9D8F'; }
    else if (node.type === 'facility') { radius = 5; fillColor = '#457B9D'; }
    else if (node.type === 'entrance') { radius = 8; fillColor = '#40916C'; }

    if (isHovered) radius += 2

    // 光晕
    if (isStart || isEnd || isHovered) {
      ctx!.beginPath()
      ctx!.arc(x, y, radius + 4, 0, Math.PI * 2)
      ctx!.fillStyle = fillColor + '30'
      ctx!.fill()
    }

    ctx!.beginPath()
    ctx!.arc(x, y, radius, 0, Math.PI * 2)
    ctx!.fillStyle = fillColor
    ctx!.fill()
    ctx!.strokeStyle = strokeColor
    ctx!.lineWidth = 2
    ctx!.stroke()

    // 节点名称
    if (isStart || isEnd || isWaypoint || isHovered) {
      ctx!.font = 'bold 11px "Noto Sans SC", sans-serif'
      ctx!.textAlign = 'center'
      ctx!.fillStyle = '#1B2A21'
      ctx!.shadowColor = '#fff'
      ctx!.shadowBlur = 4
      ctx!.fillText(node.name, x, y - radius - 6)
      ctx!.shadowBlur = 0
    }
  }

  ctx!.setTransform(1, 0, 0, 1, 0, 0)
}

function findNodeAtPosition(canvasX: number, canvasY: number): MapNode | null {
  const hitRadius = 15
  for (const node of graphNodes.value) {
    const [cx, cy] = toCanvas(node.pos_x, node.pos_y)
    const nx = cx / window.devicePixelRatio
    const ny = cy / window.devicePixelRatio
    const dist = Math.sqrt((canvasX - nx) ** 2 + (canvasY - ny) ** 2)
    if (dist < hitRadius) return node
  }
  return null
}

function handleCanvasClick(e: MouseEvent) {
  if (!canvasRef.value) return
  const rect = canvasRef.value.getBoundingClientRect()
  const x = e.clientX - rect.left
  const y = e.clientY - rect.top
  const node = findNodeAtPosition(x, y)
  if (!node) return

  if (!startNode.value) {
    startNode.value = node
    selectMode.value = 'end'
  } else if (!endNode.value) {
    if (node.id !== startNode.value.id) {
      endNode.value = node
      selectMode.value = 'start'
    }
  } else if (e.shiftKey || e.button === 2) {
    // Shift+click 添加途经点
    if (node.id !== startNode.value.id && node.id !== endNode.value.id &&
        !waypoints.value.some(w => w.id === node.id)) {
      waypoints.value.push(node)
    }
  }
  drawGraph()
}

function handleCanvasMouseMove(e: MouseEvent) {
  if (!canvasRef.value) return
  const rect = canvasRef.value.getBoundingClientRect()
  const x = e.clientX - rect.left
  const y = e.clientY - rect.top
  const node = findNodeAtPosition(x, y)
  if (node !== hoveredNode.value) {
    hoveredNode.value = node
    canvasRef.value.style.cursor = node ? 'pointer' : 'crosshair'
    drawGraph()
  }
}

function removeWaypoint(index: number) {
  waypoints.value.splice(index, 1)
  drawGraph()
}

function addCurrentAsWaypoint() {
  if (hoveredNode.value && selectedAreaId.value) {
    if (!waypoints.value.some(w => w.id === hoveredNode.value!.id)) {
      waypoints.value.push(hoveredNode.value)
    }
  }
}

async function planRoute() {
  if (!canPlan.value || !selectedAreaId.value) return
  loading.value = true
  try {
    const areaId = Number(selectedAreaId.value)
    if (waypoints.value.length > 0) {
      // TSP多目标
      const res = await planMultiRoute({
        area_id: areaId,
        from_node: startNode.value!.id,
        to_node: endNode.value!.id,
        waypoints: waypoints.value.map(w => w.id),
        strategy: strategy.value,
      })
      routeResult.value = res.data
    } else {
      // 单目标
      const res = await planSingleRoute({
        area_id: areaId,
        from_node: startNode.value!.id,
        to_node: endNode.value!.id,
        strategy: strategy.value,
      })
      routeResult.value = res.data
    }
    drawGraph()
  } catch (e) {
    console.error('路线规划失败:', e)
  } finally {
    loading.value = false
  }
}

function clearRoute() {
  routeResult.value = null
  drawGraph()
}

function clearAll() {
  startNode.value = null
  endNode.value = null
  waypoints.value = []
  routeResult.value = null
  selectMode.value = 'start'
  drawGraph()
}

async function loadGraphData() {
  if (!selectedAreaId.value) {
    graphNodes.value = []
    graphRoads.value = []
    return
  }
  try {
    const res = await getGraphData(Number(selectedAreaId.value))
    graphNodes.value = res.data?.nodes || []
    graphRoads.value = res.data?.roads || res.data?.edges || []
    clearAll()
    await nextTick()
    calcCanvasTransform()
    drawGraph()
  } catch (e) {
    console.error('加载路网失败:', e)
  }
}

async function loadSpotOptions() {
  try {
    const res = await getSpotOptions()
    spotOptions.value = res.data || []
  } catch (e) {
    console.error('加载景区列表失败:', e)
  }
}

function handleResize() {
  if (graphNodes.value.length) {
    calcCanvasTransform()
    drawGraph()
  }
}

onMounted(() => {
  loadSpotOptions()
  window.addEventListener('resize', handleResize)
})

onUnmounted(() => {
  window.removeEventListener('resize', handleResize)
})
</script>

<style scoped>
.strategy-active {
  background: var(--color-primary);
  color: white;
}
.strategy-default {
  background: white;
  color: var(--color-text-secondary);
  border: 1px solid var(--color-primary-lightest);
}
.strategy-default:hover {
  border-color: var(--color-primary-light);
  color: var(--color-primary);
}
</style>
