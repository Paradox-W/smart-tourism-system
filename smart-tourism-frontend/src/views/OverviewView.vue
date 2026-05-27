<template>
  <div class="page-content fade-in">
    <div class="mb-6 flex flex-wrap items-end justify-between gap-3">
      <div>
        <h1 class="text-2xl font-bold mb-1" style="color: var(--color-primary)">数据概览</h1>
        <p class="text-sm" style="color: var(--color-text-muted)">
          检查后端连接、数据库规模和课程设计硬性数据指标。
        </p>
      </div>
      <button
        class="px-4 py-2 rounded-lg text-sm font-medium text-white transition-colors"
        style="background: var(--color-primary)"
        @click="loadOverview"
      >
        刷新
      </button>
    </div>

    <div v-if="loading" class="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-4 gap-4">
      <div v-for="i in 8" :key="i" class="bg-white rounded-xl p-5 shadow-sm">
        <div class="skeleton h-4 w-20 mb-4 rounded"></div>
        <div class="skeleton h-8 w-28 rounded"></div>
      </div>
    </div>

    <div v-else-if="error" class="bg-red-50 border border-red-100 rounded-xl p-5">
      <div class="font-semibold mb-1" style="color: #b91c1c">后端连接失败</div>
      <p class="text-sm" style="color: #7f1d1d">{{ error }}</p>
      <p class="text-xs mt-3" style="color: var(--color-text-muted)">
        请先启动后端：smart-tourism-backend\cmake-build-debug\bin\SmartTourismSystem.exe --port 8080
      </p>
    </div>

    <template v-else-if="overview">
      <div class="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-4 gap-4 mb-6">
        <div
          v-for="card in statCards"
          :key="card.label"
          class="bg-white rounded-xl p-5 shadow-sm border border-gray-100"
        >
          <div class="text-xs mb-2" style="color: var(--color-text-muted)">{{ card.label }}</div>
          <div class="text-3xl font-bold" style="color: var(--color-text-primary)">
            {{ card.value }}
          </div>
          <div v-if="card.hint" class="text-xs mt-2" style="color: var(--color-text-muted)">
            {{ card.hint }}
          </div>
        </div>
      </div>

      <div class="grid grid-cols-1 xl:grid-cols-3 gap-5">
        <section class="xl:col-span-2 bg-white rounded-xl p-5 shadow-sm border border-gray-100">
          <div class="flex items-center justify-between mb-4">
            <h2 class="font-semibold" style="color: var(--color-text-primary)">硬性指标检查</h2>
            <span
              class="text-xs px-2 py-1 rounded-full"
              :style="allPassed
                ? 'background:#dcfce7;color:#166534'
                : 'background:#fef3c7;color:#92400e'"
            >
              {{ allPassed ? '全部达标' : '仍需补齐' }}
            </span>
          </div>
          <div class="space-y-3">
            <div
              v-for="item in overview.requirements"
              :key="item.label"
              class="flex items-center gap-3 rounded-lg border border-gray-100 p-3"
            >
              <div
                class="w-8 h-8 rounded-full flex items-center justify-center text-sm font-bold"
                :style="item.passed
                  ? 'background:#dcfce7;color:#166534'
                  : 'background:#fee2e2;color:#991b1b'"
              >
                {{ item.passed ? '✓' : '!' }}
              </div>
              <div class="flex-1 min-w-0">
                <div class="text-sm font-medium truncate" style="color: var(--color-text-primary)">
                  {{ item.label }}
                </div>
                <div class="h-2 bg-gray-100 rounded-full mt-2 overflow-hidden">
                  <div
                    class="h-full rounded-full"
                    :style="{
                      width: `${Math.min(100, Math.round((item.actual / item.target) * 100))}%`,
                      background: item.passed ? 'var(--color-primary)' : '#f59e0b',
                    }"
                  />
                </div>
              </div>
              <div class="text-right">
                <div class="text-sm font-semibold" style="color: var(--color-text-primary)">
                  {{ item.actual }}
                </div>
                <div class="text-xs" style="color: var(--color-text-muted)">目标 {{ item.target }}</div>
              </div>
            </div>
          </div>
        </section>

        <section class="bg-white rounded-xl p-5 shadow-sm border border-gray-100">
          <h2 class="font-semibold mb-4" style="color: var(--color-text-primary)">热门景区</h2>
          <div class="space-y-3">
            <div
              v-for="(spot, index) in overview.hot_spots"
              :key="spot.id"
              class="flex items-center gap-3"
            >
              <div
                class="w-7 h-7 rounded-lg flex items-center justify-center text-xs font-bold"
                :style="index < 3
                  ? 'background:var(--color-primary);color:white'
                  : 'background:var(--color-primary-bg);color:var(--color-primary)'"
              >
                {{ index + 1 }}
              </div>
              <div class="flex-1 min-w-0">
                <div class="text-sm font-medium truncate" style="color: var(--color-text-primary)">
                  {{ spot.name }}
                </div>
                <div class="text-xs" style="color: var(--color-text-muted)">
                  {{ spot.type }} · {{ spot.category }} · ★ {{ Number(spot.rating).toFixed(1) }}
                </div>
              </div>
              <div class="text-xs font-medium" style="color: var(--color-primary)">
                {{ spot.popularity }}
              </div>
            </div>
          </div>
        </section>
      </div>
    </template>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { getOverviewStats, type OverviewData } from '@/api/stats'

const loading = ref(false)
const error = ref('')
const overview = ref<OverviewData | null>(null)

const statCards = computed(() => {
  const s = overview.value?.stats
  if (!s) return []
  return [
    { label: '数据库状态', value: s.database_status, hint: '后端 SQLite 连接' },
    { label: '景区/校园', value: s.scenic_spots, hint: `${s.scenic_count} 景区 / ${s.campus_count} 校园` },
    { label: '节点总数', value: s.nodes, hint: `${s.buildings} 建筑 / ${s.facilities} 设施` },
    { label: '道路边数', value: s.roads, hint: '路线规划图边数量' },
    { label: '设施类型', value: s.facility_types, hint: '厕所、商店、餐饮等' },
    { label: '用户数量', value: s.users, hint: '满足课程规模要求' },
    { label: '日记数量', value: s.diaries, hint: `${s.ratings} 条评分` },
    { label: '美食数量', value: s.foods, hint: `${s.cuisines} 种菜系` },
  ]
})

const allPassed = computed(() =>
  overview.value?.requirements.every(item => item.passed) ?? false
)

async function loadOverview() {
  loading.value = true
  error.value = ''
  try {
    const res = await getOverviewStats()
    overview.value = res.data
  } catch (e: any) {
    error.value = e?.message || '无法访问 /api/stats/overview'
  } finally {
    loading.value = false
  }
}

onMounted(loadOverview)
</script>
