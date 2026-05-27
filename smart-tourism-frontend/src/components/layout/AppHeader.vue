<template>
  <header
    class="fixed top-0 left-0 right-0 z-50 h-16 flex items-center px-6 border-b backdrop-blur-md"
    :class="isScrolled ? 'bg-white/95 shadow-sm' : 'bg-white/80'"
  >
    <router-link to="/" class="flex items-center gap-2.5 mr-8 shrink-0 no-underline">
      <div
        class="w-9 h-9 rounded-xl flex items-center justify-center text-white text-sm font-bold"
        style="background: linear-gradient(135deg, #2D6A4F, #52B788)"
      >
        <MapPinned :size="20" />
      </div>
      <span class="text-lg font-bold" style="color: var(--color-primary)">智能旅游</span>
    </router-link>

    <nav class="hidden md:flex items-center gap-1 flex-1">
      <router-link
        v-for="item in navItems"
        :key="item.path"
        :to="item.path"
        class="nav-item flex items-center gap-2 px-3 py-2 rounded-lg text-sm font-medium transition-all duration-200 no-underline"
        :class="isActive(item.path) ? 'nav-active' : 'nav-inactive'"
      >
        <component :is="item.icon" :size="18" />
        <span>{{ item.label }}</span>
      </router-link>
    </nav>

    <div class="ml-auto flex items-center gap-3">
      <el-dropdown trigger="click">
        <div class="flex items-center gap-2 cursor-pointer px-3 py-1.5 rounded-lg hover:bg-gray-100 transition-colors">
          <div
            class="w-8 h-8 rounded-full flex items-center justify-center text-white text-xs font-medium"
            style="background: linear-gradient(135deg, #40916C, #2A9D8F)"
          >
            游
          </div>
          <span class="text-sm hidden sm:inline" style="color: var(--color-text-secondary)">游客</span>
        </div>
        <template #dropdown>
          <el-dropdown-menu>
            <el-dropdown-item>个人中心</el-dropdown-item>
            <el-dropdown-item divided>退出登录</el-dropdown-item>
          </el-dropdown-menu>
        </template>
      </el-dropdown>
    </div>
  </header>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { useRoute } from 'vue-router'
import { BarChart3, Coffee, Compass, Edit, Map, MapPinned, Search } from 'lucide-vue-next'

const route = useRoute()
const isScrolled = ref(false)

const navItems = [
  { path: '/overview', label: '数据概览', icon: BarChart3 },
  { path: '/recommend', label: '旅游推荐', icon: Compass },
  { path: '/map', label: '路线规划', icon: Map },
  { path: '/facility', label: '场所查询', icon: Search },
  { path: '/diary', label: '旅游日记', icon: Edit },
  { path: '/food', label: '美食推荐', icon: Coffee },
]

function isActive(path: string): boolean {
  return route.path === path
}

function handleScroll() {
  isScrolled.value = window.scrollY > 10
}

onMounted(() => window.addEventListener('scroll', handleScroll))
onUnmounted(() => window.removeEventListener('scroll', handleScroll))
</script>

<style scoped>
.nav-active {
  color: var(--color-primary);
  background: var(--color-primary-bg);
}
.nav-inactive {
  color: var(--color-text-secondary);
}
.nav-inactive:hover {
  color: var(--color-primary);
  background: var(--color-primary-bg);
}
</style>
