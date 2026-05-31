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

          <!-- 节点下拉选择 -->
          <div v-if="graphNodes.length > 0">
            <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
              节点选择（下拉精确选择）
            </label>
            <select
              v-model="dropdownNodeId"
              class="w-full px-3 py-2 rounded-lg text-sm border outline-none transition-colors focus:border-green-500"
              style="border-color: var(--color-primary-lightest)"
              @change="onDropdownSelect"
            >
              <option value="">请选择节点...</option>
              <option v-for="n in graphNodes" :key="n.id" :value="n.id">
                {{ n.name }} ({{ nodeTypeLabel(n.type) }})
              </option>
            </select>
          </div>

          <!-- 选中的节点信息 -->
          <div v-if="startNode" class="bg-green-50 rounded-xl p-3">
            <div class="text-xs" style="color: var(--color-primary)">起点</div>
            <div class="text-sm font-medium truncate">{{ startNode.name }}</div>
          </div>
          <div v-if="endNode" class="bg-red-50 rounded-xl p-3">
            <div class="text-xs" style="color: #E63946">终点</div>
            <div class="text-sm font-medium truncate">{{ endNode.name }}</div>
          </div>

          <!-- 途经点（始终显示） -->
          <div>
            <label class="block text-xs font-medium mb-1.5" style="color: var(--color-text-secondary)">
              途经点 {{ waypoints.length > 0 ? `(${waypoints.length})` : '' }}
            </label>
            <div v-if="waypoints.length > 0" class="flex flex-col gap-1">
              <div v-for="(wp, i) in waypoints" :key="wp.id"
                   class="flex items-center gap-2 bg-gray-50 rounded-lg px-3 py-1.5 text-xs">
                <span class="font-medium" style="color: var(--color-primary)">{{ i + 1 }}</span>
                <span class="flex-1 truncate">{{ wp.name }}</span>
                <button class="text-red-400 hover:text-red-600 cursor-pointer" @click="removeWaypoint(i)">×</button>
              </div>
            </div>
            <div v-else class="text-xs py-1" style="color: var(--color-text-muted)">
              选择起点和终点后，点击地图节点添加途经点
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
                @click="undoLastWaypoint"
                :disabled="waypoints.length === 0"
              >
                撤销途经点
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
                {{ routeDistanceLabel }}
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
            <div v-if="routeSegments.length > 0" class="mt-3">
              <div class="font-medium mb-1">分段距离：</div>
              <div
                v-for="(seg, i) in routeSegments"
                :key="`${seg.fromId}-${seg.toId}-${i}`"
                class="flex items-center justify-between gap-2 py-0.5"
              >
                <span class="truncate">{{ seg.fromName }} → {{ seg.toName }}</span>
                <span class="shrink-0 font-medium" style="color: var(--color-primary)">
                  {{ formatDistance(seg.distance) }}
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- 中央地图区域 -->
      <div class="flex-1 rounded-2xl overflow-hidden shadow-sm relative">
        <div class="map-canvas-wrapper w-full h-full" style="position: relative;">
          <canvas
            ref="canvasRef"
            class="w-full h-full cursor-crosshair"
            @click="handleCanvasClick"
            @mousemove="handleCanvasMouseMove"
            @contextmenu.prevent
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
            <p class="text-xs mt-1">选择后将显示景区平面地图</p>
          </div>
        </div>
        <!-- 动态操作提示 -->
        <div v-if="selectedAreaId" class="absolute bottom-4 left-4 bg-white/90 backdrop-blur-sm rounded-xl px-4 py-2 shadow-sm text-xs"
             style="color: var(--color-text-secondary)">
          {{ hintText }}
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted, nextTick } from 'vue'
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

// 下拉选择
const dropdownNodeId = ref<number | string>('')

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

const hintText = computed(() => {
  if (!selectedAreaId.value) return ''
  if (routeResult.value) return '路径已规划，点击节点可重新选择'
  switch (selectMode.value) {
    case 'start': return '请点击选择起点'
    case 'end': return '请点击选择终点'
    case 'waypoint': return '左键点击添加途经点，或点击"规划最短路径"开始'
    default: return ''
  }
})

const routeNodeNames = computed(() => {
  if (!routeResult.value) return []
  const r = routeResult.value
  if (r.path_names?.length) return r.path_names
  if (r.node_names?.length) return r.node_names
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

const routeDistanceLabel = computed(() => formatDistance(routeDistance.value))

const routeTime = computed(() => {
  if (!routeResult.value) return 0
  const r = routeResult.value
  return Math.round((r.total_time ?? r.time ?? 0) / 60)
})

const routeSegments = computed(() => {
  if (!routeResult.value?.path?.length) return []
  const path = routeResult.value.path
  const segments: Array<{
    fromId: number
    toId: number
    fromName: string
    toName: string
    distance: number
  }> = []

  for (let i = 0; i < path.length - 1; i++) {
    const fromId = path[i]
    const toId = path[i + 1]
    const fromNode = graphNodes.value.find(n => n.id === fromId)
    const toNode = graphNodes.value.find(n => n.id === toId)
    const road = findRoadBetween(fromId, toId)
    if (!fromNode || !toNode || !road) continue

    segments.push({
      fromId,
      toId,
      fromName: fromNode.name,
      toName: toNode.name,
      distance: road.distance,
    })
  }

  return segments
})

// ==================== Canvas 渲染参数 ====================
const CANVAS_PADDING = 40
let scale = 1
let offsetX = 0
let offsetY = 0
let metersPerUnit = 1

// 确定性伪随机（用于装饰元素位置）
function seededRandom(seed: number): number {
  const x = Math.sin(seed * 127.1 + 311.7) * 43758.5453
  return x - Math.floor(x)
}

function resizeCanvas() {
  const canvas = canvasRef.value
  if (!canvas) return
  const parent = canvas.parentElement
  if (!parent) return
  const rect = parent.getBoundingClientRect()
  const w = Math.floor(rect.width)
  const h = Math.floor(rect.height)
  if (w < 1 || h < 1) return
  const dpr = window.devicePixelRatio || 1
  canvas.width = w * dpr
  canvas.height = h * dpr
  canvas.style.width = w + 'px'
  canvas.style.height = h + 'px'
  if (ctx) {
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
  }
}

function calcTransform() {
  if (!graphNodes.value.length) return
  const canvas = canvasRef.value
  if (!canvas) return
  const dpr = window.devicePixelRatio || 1
  const cssW = canvas.width / dpr
  const cssH = canvas.height / dpr

  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity
  for (const n of graphNodes.value) {
    if (n.pos_x < minX) minX = n.pos_x
    if (n.pos_y < minY) minY = n.pos_y
    if (n.pos_x > maxX) maxX = n.pos_x
    if (n.pos_y > maxY) maxY = n.pos_y
  }
  const rangeX = maxX - minX || 1
  const rangeY = maxY - minY || 1
  const drawW = cssW - CANVAS_PADDING * 2
  const drawH = cssH - CANVAS_PADDING * 2
  scale = Math.min(drawW / rangeX, drawH / rangeY)
  offsetX = (cssW - rangeX * scale) / 2 - minX * scale
  offsetY = (cssH - rangeY * scale) / 2 - minY * scale
  metersPerUnit = estimateMetersPerUnit()
}

function toCanvasPos(x: number, y: number): [number, number] {
  return [x * scale + offsetX, y * scale + offsetY]
}

function formatDistance(distance: number): string {
  if (!Number.isFinite(distance) || distance <= 0) return '0m'
  if (distance >= 1000) return `${(distance / 1000).toFixed(1)}km`
  return `${Math.round(distance)}m`
}

function findRoadBetween(fromId: number, toId: number): Road | undefined {
  return graphRoads.value.find(r =>
    (r.from_node === fromId && r.to_node === toId) ||
    (r.is_bidirectional && r.from_node === toId && r.to_node === fromId)
  )
}

function estimateMetersPerUnit(): number {
  if (!graphNodes.value.length || !graphRoads.value.length) return 1

  const nodeMap = new Map<number, MapNode>()
  for (const node of graphNodes.value) nodeMap.set(node.id, node)

  const ratios: number[] = []
  for (const road of graphRoads.value) {
    const fromN = nodeMap.get(road.from_node)
    const toN = nodeMap.get(road.to_node)
    if (!fromN || !toN) continue
    const coordLength = Math.hypot(fromN.pos_x - toN.pos_x, fromN.pos_y - toN.pos_y)
    if (coordLength > 0 && road.distance > 0) {
      ratios.push(road.distance / coordLength)
    }
  }

  if (!ratios.length) return 1
  ratios.sort((a, b) => a - b)
  return ratios[Math.floor(ratios.length / 2)] || 1
}

// ==================== 建筑类型颜色映射 ====================
const buildingColors: Record<string, { fill: string; roof: string }> = {
  teaching:   { fill: '#7B9ACC', roof: '#5A7BAA' },
  library:    { fill: '#C4A882', roof: '#A68B64' },
  sports:     { fill: '#8BC49E', roof: '#6BA87E' },
  canteen:    { fill: '#D4A76A', roof: '#B88B4E' },
  dormitory:  { fill: '#B8A0C8', roof: '#9A82AA' },
  research:   { fill: '#88B8C4', roof: '#6A9AA6' },
  admin:      { fill: '#C49888', roof: '#A67A6A' },
  landmark:   { fill: '#E8C87A', roof: '#D4A85A' },
  hospital:   { fill: '#C48888', roof: '#A66A6A' },
  museum:     { fill: '#A8B888', roof: '#8A9A6A' },
  utility:    { fill: '#A0A8B0', roof: '#828A92' },
  shop:       { fill: '#B8C4A0', roof: '#9AA682' },
}

function getBuildingColor(subType: string | undefined) {
  return buildingColors[subType || ''] || { fill: '#A8B8A0', roof: '#8A9A82' }
}

// 节点类型中文标签
function nodeTypeLabel(type: string): string {
  const labels: Record<string, string> = {
    entrance: '入口',
    building: '建筑',
    facility: '设施',
    junction: '路口',
  }
  return labels[type] || type
}

// ==================== 绘制函数 ====================
function draw() {
  const canvas = canvasRef.value
  if (!ctx || !canvas) return

  const dpr = window.devicePixelRatio || 1
  const cssW = canvas.width / dpr
  const cssH = canvas.height / dpr

  // 清空画布
  ctx.save()
  ctx.setTransform(1, 0, 0, 1, 0, 0)
  ctx.clearRect(0, 0, canvas.width, canvas.height)
  ctx.restore()

  // === 1. 背景层：草地 ===
  drawGrassBackground(cssW, cssH)

  // === 2. 装饰层：树木、花坛 ===
  drawDecorations(cssW, cssH)

  // === 3. 道路层 ===
  drawRoads()

  // === 4. 建筑/节点层 ===
  drawNodes()

  // === 5. 装饰UI：罗盘、图例、比例尺 ===
  drawCompass(cssW, cssH)
  drawLegend(cssW, cssH)
  drawScaleBar(cssW, cssH)
}

// --- 草地背景 ---
function drawGrassBackground(w: number, h: number) {
  if (!ctx) return
  // 径向渐变模拟草坪
  const grad = ctx.createRadialGradient(w / 2, h / 2, 0, w / 2, h / 2, Math.max(w, h) * 0.7)
  grad.addColorStop(0, '#C8E6B0')
  grad.addColorStop(0.6, '#B5D99A')
  grad.addColorStop(1, '#9CC87E')
  ctx.fillStyle = grad
  ctx.fillRect(0, 0, w, h)

  // 草地纹理：细微的点状纹理
  ctx.fillStyle = 'rgba(100, 160, 60, 0.08)'
  for (let i = 0; i < 300; i++) {
    const rx = seededRandom(i * 7.3) * w
    const ry = seededRandom(i * 13.7 + 50) * h
    ctx.beginPath()
    ctx.arc(rx, ry, 1 + seededRandom(i * 3.1) * 2, 0, Math.PI * 2)
    ctx.fill()
  }
}

// --- 装饰元素 ---
function drawDecorations(w: number, h: number) {
  if (!ctx) return
  const nodePositions = new Set<string>()
  for (const n of graphNodes.value) {
    const [cx, cy] = toCanvasPos(n.pos_x, n.pos_y)
    nodePositions.add(`${Math.round(cx / 60)}_${Math.round(cy / 60)}`)
  }

  // 装饰树木
  const treeCount = Math.min(80, graphNodes.value.length * 4)
  for (let i = 0; i < treeCount; i++) {
    const tx = seededRandom(i * 17.3 + 100) * w
    const ty = seededRandom(i * 23.7 + 200) * h
    const gridKey = `${Math.round(tx / 60)}_${Math.round(ty / 60)}`
    if (nodePositions.has(gridKey)) continue

    const treeSize = 6 + seededRandom(i * 7.1) * 10
    // 树影
    ctx.beginPath()
    ctx.arc(tx + 2, ty + 3, treeSize, 0, Math.PI * 2)
    ctx.fillStyle = 'rgba(40, 80, 20, 0.15)'
    ctx.fill()
    // 树冠
    const shade = seededRandom(i * 3.3)
    if (shade < 0.3) {
      ctx.fillStyle = '#3D7A3A'
    } else if (shade < 0.6) {
      ctx.fillStyle = '#4A8C3F'
    } else {
      ctx.fillStyle = '#55944A'
    }
    ctx.beginPath()
    ctx.arc(tx, ty, treeSize, 0, Math.PI * 2)
    ctx.fill()
    // 树冠高光
    ctx.beginPath()
    ctx.arc(tx - treeSize * 0.2, ty - treeSize * 0.2, treeSize * 0.5, 0, Math.PI * 2)
    ctx.fillStyle = 'rgba(120, 200, 80, 0.3)'
    ctx.fill()
  }

  // 装饰花坛
  for (let i = 0; i < 20; i++) {
    const fx = seededRandom(i * 31.3 + 500) * w
    const fy = seededRandom(i * 37.7 + 600) * h
    const gridKey = `${Math.round(fx / 60)}_${Math.round(fy / 60)}`
    if (nodePositions.has(gridKey)) continue

    const flowerColors = ['#E85D75', '#F0A030', '#C87AE8', '#E8C040', '#FF7B7B']
    for (let j = 0; j < 5; j++) {
      const angle = (j / 5) * Math.PI * 2 + seededRandom(i) * 0.5
      const dist = 3 + seededRandom(i * 10 + j) * 3
      ctx.beginPath()
      ctx.arc(fx + Math.cos(angle) * dist, fy + Math.sin(angle) * dist, 2.5, 0, Math.PI * 2)
      ctx.fillStyle = flowerColors[j % flowerColors.length]
      ctx.fill()
    }
    // 花芯
    ctx.beginPath()
    ctx.arc(fx, fy, 2, 0, Math.PI * 2)
    ctx.fillStyle = '#FFD700'
    ctx.fill()
  }
}

// --- 道路绘制 ---
function drawRoads() {
  if (!ctx) return

  const routeEdges = new Set<string>()
  if (routeResult.value?.path?.length) {
    const path = routeResult.value.path
    for (let i = 0; i < path.length - 1; i++) {
      routeEdges.add(`${path[i]}-${path[i + 1]}`)
    }
  }

  // 先构建节点位置查找 map
  const nodeMap = new Map<number, MapNode>()
  for (const n of graphNodes.value) nodeMap.set(n.id, n)

  for (const road of graphRoads.value) {
    const fromN = nodeMap.get(road.from_node)
    const toN = nodeMap.get(road.to_node)
    if (!fromN || !toN) continue

    const [x1, y1] = toCanvasPos(fromN.pos_x, fromN.pos_y)
    const [x2, y2] = toCanvasPos(toN.pos_x, toN.pos_y)
    const path = makeRoadPath(road, x1, y1, x2, y2)
    const isForwardRoute = routeEdges.has(`${road.from_node}-${road.to_node}`)
    const isReverseRoute = routeEdges.has(`${road.to_node}-${road.from_node}`)
    const isOnRoute = isForwardRoute || isReverseRoute

    if (isOnRoute) {
      // 路径高亮：发光效果
      ctx.save()
      ctx.beginPath()
      traceRoadPath(path)
      ctx.strokeStyle = '#2EC4B6'
      ctx.lineWidth = 8
      ctx.shadowColor = '#2EC4B6'
      ctx.shadowBlur = 12
      ctx.lineCap = 'round'
      ctx.stroke()
      ctx.shadowBlur = 0
      // 内部亮线
      ctx.beginPath()
      traceRoadPath(path)
      ctx.strokeStyle = '#8EF0E4'
      ctx.lineWidth = 3
      ctx.stroke()
      ctx.restore()

      drawRouteArrow(path, isReverseRoute)
      drawRoadDistanceLabel(path, road.distance)
    } else {
      // 普通道路
      const roadWidth = road.transport === 2 ? 8 : road.transport === 1 ? 6 : 4

      // 道路阴影
      ctx.beginPath()
      traceRoadPath(makeRoadPath(road, x1 + 1.5, y1 + 1.5, x2 + 1.5, y2 + 1.5))
      ctx.strokeStyle = 'rgba(0, 0, 0, 0.1)'
      ctx.lineWidth = roadWidth + 2
      ctx.lineCap = 'round'
      ctx.stroke()

      // 道路面
      ctx.beginPath()
      traceRoadPath(path)
      const c = road.congestion
      if (c > 0.6) {
        ctx.strokeStyle = '#E8E0D4' // 畅通：浅米色
      } else if (c > 0.3) {
        ctx.strokeStyle = '#DDD4C6' // 中等
      } else {
        ctx.strokeStyle = '#D0C4B0' // 拥堵：深色
      }
      ctx.lineWidth = roadWidth
      ctx.stroke()

      // 主干道中心虚线
      if (road.transport === 2) {
        ctx.beginPath()
        traceRoadPath(path)
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.6)'
        ctx.lineWidth = 1.5
        ctx.setLineDash([6, 8])
        ctx.stroke()
        ctx.setLineDash([])
      }

      // 人行步道细线
      if (road.transport === 0) {
        ctx.beginPath()
        traceRoadPath(path)
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.4)'
        ctx.lineWidth = 1
        ctx.setLineDash([2, 5])
        ctx.stroke()
        ctx.setLineDash([])
      }
    }
  }
}

type RoadPath = {
  x1: number
  y1: number
  x2: number
  y2: number
  cx: number
  cy: number
  curved: boolean
}

function makeRoadPath(road: Road, x1: number, y1: number, x2: number, y2: number): RoadPath {
  const dx = x2 - x1
  const dy = y2 - y1
  const len = Math.hypot(dx, dy)
  if (len < 54) {
    return { x1, y1, x2, y2, cx: (x1 + x2) / 2, cy: (y1 + y2) / 2, curved: false }
  }

  const seed = seededRandom(road.id * 17.7)
  const sign = seed > 0.5 ? 1 : -1
  const bend = Math.min(24, len * 0.11) * (0.35 + seed * 0.65) * sign
  const nx = -dy / len
  const ny = dx / len

  return {
    x1,
    y1,
    x2,
    y2,
    cx: (x1 + x2) / 2 + nx * bend,
    cy: (y1 + y2) / 2 + ny * bend,
    curved: Math.abs(bend) > 1,
  }
}

function traceRoadPath(path: RoadPath) {
  if (!ctx) return
  ctx.moveTo(path.x1, path.y1)
  if (path.curved) {
    ctx.quadraticCurveTo(path.cx, path.cy, path.x2, path.y2)
  } else {
    ctx.lineTo(path.x2, path.y2)
  }
}

function pointOnRoadPath(path: RoadPath, t: number) {
  if (!path.curved) {
    return {
      x: path.x1 + (path.x2 - path.x1) * t,
      y: path.y1 + (path.y2 - path.y1) * t,
    }
  }

  const mt = 1 - t
  return {
    x: mt * mt * path.x1 + 2 * mt * t * path.cx + t * t * path.x2,
    y: mt * mt * path.y1 + 2 * mt * t * path.cy + t * t * path.y2,
  }
}

function roadTangentAngle(path: RoadPath, t: number, reverse: boolean) {
  let dx: number
  let dy: number
  if (path.curved) {
    dx = 2 * (1 - t) * (path.cx - path.x1) + 2 * t * (path.x2 - path.cx)
    dy = 2 * (1 - t) * (path.cy - path.y1) + 2 * t * (path.y2 - path.cy)
  } else {
    dx = path.x2 - path.x1
    dy = path.y2 - path.y1
  }

  return Math.atan2(dy, dx) + (reverse ? Math.PI : 0)
}

function drawRouteArrow(path: RoadPath, reverse: boolean) {
  if (!ctx) return
  const p = pointOnRoadPath(path, 0.52)
  const angle = roadTangentAngle(path, 0.52, reverse)
  ctx.save()
  ctx.fillStyle = '#2EC4B6'
  ctx.translate(p.x, p.y)
  ctx.rotate(angle)
  ctx.beginPath()
  ctx.moveTo(8, 0)
  ctx.lineTo(-4, -5)
  ctx.lineTo(-4, 5)
  ctx.closePath()
  ctx.fill()
  ctx.restore()
}

function drawRoadDistanceLabel(path: RoadPath, distance: number) {
  if (!ctx) return
  const p = pointOnRoadPath(path, 0.5)
  const angle = roadTangentAngle(path, 0.5, false)
  drawDistanceBadge(p.x - Math.sin(angle) * 17, p.y + Math.cos(angle) * 17, formatDistance(distance))
}

// --- 节点/建筑绘制 ---
function drawNodes() {
  if (!ctx) return

  for (const node of graphNodes.value) {
    const [cx, cy] = toCanvasPos(node.pos_x, node.pos_y)
    const isRoute = routeResult.value?.path?.includes(node.id) ?? false
    const isStart = startNode.value?.id === node.id
    const isEnd = endNode.value?.id === node.id
    const isWaypoint = waypoints.value.some(w => w.id === node.id)
    const isHovered = hoveredNode.value?.id === node.id

    // 选中/特殊状态的节点用大圆标记
    if (isStart || isEnd || isWaypoint || isRoute || isHovered) {
      drawSelectedNode(cx, cy, node, isStart, isEnd, isWaypoint, isRoute, isHovered)
      continue
    }

    // 普通节点按类型绘制建筑图标
    switch (node.type) {
      case 'entrance':
        drawEntranceBuilding(cx, cy, node)
        break
      case 'building':
        drawBuildingIcon(cx, cy, node)
        break
      case 'facility':
        drawFacilityIcon(cx, cy, node)
        break
      case 'junction':
        drawJunctionDot(cx, cy)
        break
      default:
        drawJunctionDot(cx, cy)
    }

    // 普通节点标签
    ctx.font = '9px "Noto Sans SC", sans-serif'
    ctx.textAlign = 'center'
    ctx.fillStyle = 'rgba(40, 60, 40, 0.7)'
    ctx.shadowColor = 'rgba(255, 255, 255, 0.8)'
    ctx.shadowBlur = 3
    ctx.fillText(node.name, cx, cy + 22)
    ctx.shadowBlur = 0
  }
}

// 入口建筑
function drawEntranceBuilding(cx: number, cy: number, _node: MapNode) {
  if (!ctx) return
  const w = 20, h = 16
  // 阴影
  ctx.fillStyle = 'rgba(0, 0, 0, 0.12)'
  ctx.fillRect(cx - w / 2 + 2, cy - h / 2 + 2, w, h)
  // 墙体
  ctx.fillStyle = '#4A9E6A'
  ctx.fillRect(cx - w / 2, cy - h / 2, w, h)
  // 屋顶
  ctx.fillStyle = '#3A8050'
  ctx.beginPath()
  ctx.moveTo(cx - w / 2 - 2, cy - h / 2)
  ctx.lineTo(cx, cy - h / 2 - 8)
  ctx.lineTo(cx + w / 2 + 2, cy - h / 2)
  ctx.closePath()
  ctx.fill()
  // 门拱
  ctx.fillStyle = '#2D5A3A'
  ctx.beginPath()
  ctx.arc(cx, cy + 2, 5, Math.PI, 0)
  ctx.fill()
  // 描边
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.5)'
  ctx.lineWidth = 1
  ctx.strokeRect(cx - w / 2, cy - h / 2, w, h)
}

// 普通建筑
function drawBuildingIcon(cx: number, cy: number, node: MapNode) {
  if (!ctx) return
  const colors = getBuildingColor(node.sub_type)
  const w = 18, h = 14
  // 阴影
  ctx.fillStyle = 'rgba(0, 0, 0, 0.12)'
  ctx.fillRect(cx - w / 2 + 2, cy - h / 2 + 2, w, h)
  // 墙体
  ctx.fillStyle = colors.fill
  ctx.fillRect(cx - w / 2, cy - h / 2, w, h)
  // 三角屋顶
  ctx.fillStyle = colors.roof
  ctx.beginPath()
  ctx.moveTo(cx - w / 2 - 1, cy - h / 2)
  ctx.lineTo(cx, cy - h / 2 - 7)
  ctx.lineTo(cx + w / 2 + 1, cy - h / 2)
  ctx.closePath()
  ctx.fill()
  // 窗户
  ctx.fillStyle = 'rgba(255, 255, 255, 0.5)'
  ctx.fillRect(cx - 5, cy - 2, 4, 4)
  ctx.fillRect(cx + 1, cy - 2, 4, 4)
  // 描边
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.4)'
  ctx.lineWidth = 0.8
  ctx.strokeRect(cx - w / 2, cy - h / 2, w, h)
}

// 设施图标
function drawFacilityIcon(cx: number, cy: number, _node: MapNode) {
  if (!ctx) return
  const size = 8
  // 阴影
  ctx.fillStyle = 'rgba(0, 0, 0, 0.1)'
  ctx.beginPath()
  ctx.moveTo(cx + 2, cy - size + 2)
  ctx.lineTo(cx + size + 2, cy + 2)
  ctx.lineTo(cx + 2, cy + size + 2)
  ctx.lineTo(cx - size + 2, cy + 2)
  ctx.closePath()
  ctx.fill()
  // 菱形
  ctx.fillStyle = '#457B9D'
  ctx.beginPath()
  ctx.moveTo(cx, cy - size)
  ctx.lineTo(cx + size, cy)
  ctx.lineTo(cx, cy + size)
  ctx.lineTo(cx - size, cy)
  ctx.closePath()
  ctx.fill()
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.5)'
  ctx.lineWidth = 1
  ctx.stroke()
}

// 路口小点
function drawJunctionDot(cx: number, cy: number) {
  if (!ctx) return
  ctx.beginPath()
  ctx.arc(cx, cy, 3, 0, Math.PI * 2)
  ctx.fillStyle = '#A0A898'
  ctx.fill()
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.5)'
  ctx.lineWidth = 1
  ctx.stroke()
}

// 选中/特殊状态节点
function drawSelectedNode(
  cx: number, cy: number, node: MapNode,
  isStart: boolean, isEnd: boolean, isWaypoint: boolean,
  isRoute: boolean, isHovered: boolean
) {
  if (!ctx) return
  let radius = 10
  let fillColor = '#2A9D8F'
  let label = ''

  if (isStart) { radius = 14; fillColor = '#2D6A4F'; label = '起' }
  else if (isEnd) { radius = 14; fillColor = '#E63946'; label = '终' }
  else if (isWaypoint) {
    radius = 11
    fillColor = '#F4A261'
    const idx = waypoints.value.findIndex(w => w.id === node.id)
    label = String(idx + 1)
  }
  else if (isRoute) { radius = 8; fillColor = '#2A9D8F' }
  if (isHovered) radius += 3

  // 光晕
  ctx.beginPath()
  ctx.arc(cx, cy, radius + 6, 0, Math.PI * 2)
  ctx.fillStyle = fillColor + '30'
  ctx.fill()

  // 阴影
  ctx.beginPath()
  ctx.arc(cx + 1.5, cy + 1.5, radius, 0, Math.PI * 2)
  ctx.fillStyle = 'rgba(0, 0, 0, 0.15)'
  ctx.fill()

  // 主体
  ctx.beginPath()
  ctx.arc(cx, cy, radius, 0, Math.PI * 2)
  ctx.fillStyle = fillColor
  ctx.fill()
  ctx.strokeStyle = '#fff'
  ctx.lineWidth = 2.5
  ctx.stroke()

  // 标签（起/终/序号）
  if (label) {
    ctx.font = 'bold 10px "Noto Sans SC", sans-serif'
    ctx.textAlign = 'center'
    ctx.textBaseline = 'middle'
    ctx.fillStyle = '#fff'
    ctx.fillText(label, cx, cy)
  }

  // 名称
  ctx.font = (isStart || isEnd) ? 'bold 12px "Noto Sans SC", sans-serif' : '11px "Noto Sans SC", sans-serif'
  ctx.textAlign = 'center'
  ctx.textBaseline = 'top'
  ctx.fillStyle = '#1B2A21'
  ctx.shadowColor = 'rgba(255, 255, 255, 0.9)'
  ctx.shadowBlur = 4
  ctx.fillText(node.name, cx, cy + radius + 4)
  ctx.shadowBlur = 0
  ctx.textBaseline = 'alphabetic'
}

// --- 罗盘 ---
function drawCompass(w: number, _h: number) {
  if (!ctx) return
  const cx = w - 40, cy = 40
  const r = 18

  // 背景圆
  ctx.beginPath()
  ctx.arc(cx, cy, r + 4, 0, Math.PI * 2)
  ctx.fillStyle = 'rgba(255, 255, 255, 0.85)'
  ctx.fill()
  ctx.strokeStyle = 'rgba(0, 0, 0, 0.15)'
  ctx.lineWidth = 1
  ctx.stroke()

  // 北箭头（红色）
  ctx.beginPath()
  ctx.moveTo(cx, cy - r)
  ctx.lineTo(cx - 5, cy)
  ctx.lineTo(cx + 5, cy)
  ctx.closePath()
  ctx.fillStyle = '#E63946'
  ctx.fill()

  // 南箭头（灰色）
  ctx.beginPath()
  ctx.moveTo(cx, cy + r)
  ctx.lineTo(cx - 5, cy)
  ctx.lineTo(cx + 5, cy)
  ctx.closePath()
  ctx.fillStyle = '#C0C0C0'
  ctx.fill()

  // N 标记
  ctx.font = 'bold 9px sans-serif'
  ctx.textAlign = 'center'
  ctx.fillStyle = '#E63946'
  ctx.fillText('N', cx, cy - r - 6)
}

// --- 图例 ---
function drawLegend(w: number, h: number) {
  if (!ctx) return
  const lx = w - 130, ly = h - 120
  const lw = 120, lh = 108

  // 背景
  ctx.fillStyle = 'rgba(255, 255, 255, 0.85)'
  ctx.strokeStyle = 'rgba(0, 0, 0, 0.1)'
  ctx.lineWidth = 1
  roundRect(ctx, lx, ly, lw, lh, 6)
  ctx.fill()
  ctx.stroke()

  ctx.font = 'bold 10px "Noto Sans SC", sans-serif'
  ctx.textAlign = 'left'
  ctx.fillStyle = '#333'
  ctx.fillText('图例', lx + 10, ly + 16)

  const items = [
    { color: '#4A9E6A', label: '入口' },
    { color: '#E8C87A', label: '景点/建筑' },
    { color: '#457B9D', label: '设施' },
    { color: '#2D6A4F', label: '起点' },
    { color: '#E63946', label: '终点' },
    { color: '#F4A261', label: '途经点' },
    { color: '#2EC4B6', label: '规划路径' },
  ]

  ctx.font = '9px "Noto Sans SC", sans-serif'
  const c = ctx
  items.forEach((item, i) => {
    const iy = ly + 30 + i * 11
    c.fillStyle = item.color
    c.beginPath()
    c.arc(lx + 16, iy, 4, 0, Math.PI * 2)
    c.fill()
    c.fillStyle = '#555'
    c.fillText(item.label, lx + 26, iy + 3)
  })
}

// --- 比例尺 ---
function drawScaleBar(_w: number, h: number) {
  if (!ctx) return
  const bx = 16, by = h - 68
  const maxBarLen = 110

  const maxMeters = (maxBarLen / Math.max(scale, 0.001)) * metersPerUnit
  const realDist = pickNiceDistance(maxMeters)
  const barLen = Math.max(36, (realDist / Math.max(metersPerUnit, 0.001)) * scale)
  const distLabel = formatDistance(realDist)

  ctx.fillStyle = 'rgba(255, 255, 255, 0.85)'
  roundRect(ctx, bx - 4, by - 14, barLen + 50, 24, 4)
  ctx.fill()

  ctx.fillStyle = '#333'
  ctx.fillRect(bx, by, barLen, 3)
  ctx.fillRect(bx, by - 4, 2, 7)
  ctx.fillRect(bx + barLen - 2, by - 4, 2, 7)

  ctx.font = '9px sans-serif'
  ctx.textAlign = 'left'
  ctx.fillStyle = '#555'
  ctx.fillText(distLabel, bx + barLen + 6, by + 3)
}

function pickNiceDistance(maxMeters: number): number {
  if (!Number.isFinite(maxMeters) || maxMeters <= 0) return 50
  const exponent = Math.pow(10, Math.floor(Math.log10(maxMeters)))
  const candidates = [1, 2, 5, 10].map(m => m * exponent)
  return candidates.find(value => value >= maxMeters * 0.45 && value <= maxMeters) || exponent
}

function drawDistanceBadge(x: number, y: number, label: string) {
  if (!ctx) return
  ctx.save()
  ctx.font = 'bold 10px "Noto Sans SC", sans-serif'
  const width = ctx.measureText(label).width + 14
  const height = 18
  roundRect(ctx, x - width / 2, y - height / 2, width, height, 7)
  ctx.fillStyle = 'rgba(255, 255, 255, 0.9)'
  ctx.fill()
  ctx.strokeStyle = 'rgba(46, 196, 182, 0.5)'
  ctx.lineWidth = 1
  ctx.stroke()
  ctx.textAlign = 'center'
  ctx.textBaseline = 'middle'
  ctx.fillStyle = '#1F6F67'
  ctx.fillText(label, x, y + 0.5)
  ctx.restore()
}

// 圆角矩形辅助
function roundRect(c: CanvasRenderingContext2D, x: number, y: number, w: number, h: number, r: number) {
  c.beginPath()
  c.moveTo(x + r, y)
  c.lineTo(x + w - r, y)
  c.quadraticCurveTo(x + w, y, x + w, y + r)
  c.lineTo(x + w, y + h - r)
  c.quadraticCurveTo(x + w, y + h, x + w - r, y + h)
  c.lineTo(x + r, y + h)
  c.quadraticCurveTo(x, y + h, x, y + h - r)
  c.lineTo(x, y + r)
  c.quadraticCurveTo(x, y, x + r, y)
  c.closePath()
}

// ==================== 交互 ====================
function findNodeAtPosition(clientX: number, clientY: number): MapNode | null {
  if (!canvasRef.value) return null
  const rect = canvasRef.value.getBoundingClientRect()
  const mx = clientX - rect.left
  const my = clientY - rect.top
  const hitRadius = 18
  for (const node of graphNodes.value) {
    const [cx, cy] = toCanvasPos(node.pos_x, node.pos_y)
    const dist = Math.sqrt((mx - cx) ** 2 + (my - cy) ** 2)
    if (dist < hitRadius) return node
  }
  return null
}

function handleCanvasClick(e: MouseEvent) {
  if (!canvasRef.value) return
  const node = findNodeAtPosition(e.clientX, e.clientY)
  if (!node) return

  if (!startNode.value) {
    startNode.value = node
    selectMode.value = 'end'
  } else if (!endNode.value) {
    if (node.id !== startNode.value.id) {
      endNode.value = node
      selectMode.value = 'waypoint'
    }
  } else {
    // 终点已选 → 左键直接添加途径点
    if (node.id !== startNode.value.id &&
        node.id !== endNode.value.id &&
        !waypoints.value.some(w => w.id === node.id)) {
      waypoints.value.push(node)
    }
  }
  draw()
}

function handleCanvasMouseMove(e: MouseEvent) {
  const node = findNodeAtPosition(e.clientX, e.clientY)
  if (node !== hoveredNode.value) {
    hoveredNode.value = node
    if (canvasRef.value) {
      canvasRef.value.style.cursor = node ? 'pointer' : 'crosshair'
    }
    draw()
  }
}

function removeWaypoint(index: number) {
  waypoints.value.splice(index, 1)
  draw()
}

function undoLastWaypoint() {
  if (waypoints.value.length > 0) {
    waypoints.value.pop()
    draw()
  }
}

// 下拉框选择
function onDropdownSelect() {
  if (!dropdownNodeId.value) return
  const node = graphNodes.value.find(n => n.id === Number(dropdownNodeId.value))
  if (!node) return

  if (!startNode.value) {
    startNode.value = node
    selectMode.value = 'end'
  } else if (!endNode.value) {
    if (node.id !== startNode.value.id) {
      endNode.value = node
      selectMode.value = 'waypoint'
    }
  } else {
    if (node.id !== startNode.value.id &&
        node.id !== endNode.value.id &&
        !waypoints.value.some(w => w.id === node.id)) {
      waypoints.value.push(node)
    }
  }

  dropdownNodeId.value = ''
  draw()
}

async function planRoute() {
  if (!canPlan.value || !selectedAreaId.value) return
  loading.value = true
  try {
    const areaId = Number(selectedAreaId.value)
    if (waypoints.value.length > 0) {
      const res = await planMultiRoute({
        area_id: areaId,
        from_node: startNode.value!.id,
        to_node: endNode.value!.id,
        waypoints: waypoints.value.map(w => w.id),
        strategy: strategy.value,
      })
      routeResult.value = res.data
    } else {
      const res = await planSingleRoute({
        area_id: areaId,
        from_node: startNode.value!.id,
        to_node: endNode.value!.id,
        strategy: strategy.value,
      })
      routeResult.value = res.data
    }
    draw()
  } catch (e) {
    console.error('路线规划失败:', e)
  } finally {
    loading.value = false
  }
}

function clearRoute() {
  routeResult.value = null
  draw()
}

function clearAll() {
  startNode.value = null
  endNode.value = null
  waypoints.value = []
  routeResult.value = null
  selectMode.value = 'start'
  draw()
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

    // 重置选择状态
    startNode.value = null
    endNode.value = null
    waypoints.value = []
    routeResult.value = null
    selectMode.value = 'start'
    hoveredNode.value = null

    await nextTick()

    if (canvasRef.value) {
      if (!ctx) {
        ctx = canvasRef.value.getContext('2d')
      }
      if (ctx) {
        resizeCanvas()
        calcTransform()
        draw()
      }
    }
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
    resizeCanvas()
    calcTransform()
    draw()
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
