import { writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const __dirname = dirname(fileURLToPath(import.meta.url))
const outFile = join(__dirname, 'generated_demo_seed.sql')
const backendOutFile = join(__dirname, '..', 'smart-tourism-backend', 'data', 'generated_demo_seed.sql')

const AREA_START_ID = 1001
const AREA_COUNT = 210
const NODES_PER_AREA = 20

const cities = [
  '北京', '上海', '广州', '深圳', '杭州', '成都',
  '西安', '南京', '武汉', '重庆', '苏州', '青岛',
]
const categories = ['自然', '历史', '文化', '现代', '综合']
const facilityTypes = [
  'toilet', 'restaurant', 'shop', 'cafe', 'library',
  'medical', 'parking', 'atm', 'service_center', 'viewpoint',
]
const cuisines = [
  '京菜', '川菜', '粤菜', '鲁菜', '湘菜',
  '快餐', '甜品', '饮品', '小吃', '西餐',
]
const scenicThemes = [
  '湖畔公园', '古城文化区', '山水风景区', '城市绿心', '艺术街区',
  '湿地公园', '历史街巷', '科技展馆', '森林步道', '滨江观光带',
]
const campusThemes = [
  '信息科技大学', '交通大学', '师范学院', '理工大学', '医科大学',
  '财经大学', '外国语大学', '艺术学院', '邮电大学', '农业大学',
]
const buildingNames = [
  '游客中心', '综合楼', '展览馆', '研学中心', '观景平台',
  '服务楼', '文化长廊', '活动中心', '会议楼', '资料馆',
  '北区教学楼', '南区教学楼',
]
const foodNames = [
  '招牌牛肉面', '香辣小火锅', '桂花米糕', '手作咖啡', '酱香烤鸭卷',
  '青椒肉丝饭', '番茄牛腩饭', '鲜虾云吞', '麻酱凉面', '芝士焗饭',
]

function esc(value) {
  return String(value).replaceAll("'", "''")
}

function q(value) {
  return `'${esc(value)}'`
}

function spotName(i, type) {
  const id = String(i).padStart(3, '0')
  const idx = i - 1
  if (type === 'campus') {
    return `${cities[idx % cities.length]}${campusThemes[idx % campusThemes.length]}${id}`
  }
  return `${cities[idx % cities.length]}${scenicThemes[idx % scenicThemes.length]}${id}`
}

function nodeId(areaId, idx) {
  return areaId * 100 + idx
}

const sql = []
sql.push('-- Generated demo data for Smart Tourism System')
sql.push('-- Run from repository root with: sqlite3 data/tourism.db ".read data/generated_demo_seed.sql"')
sql.push('BEGIN TRANSACTION;')
sql.push(`DELETE FROM foods WHERE area_id BETWEEN ${AREA_START_ID} AND ${AREA_START_ID + AREA_COUNT - 1};`)
sql.push(`DELETE FROM roads WHERE area_id BETWEEN ${AREA_START_ID} AND ${AREA_START_ID + AREA_COUNT - 1};`)
sql.push(`DELETE FROM nodes WHERE area_id BETWEEN ${AREA_START_ID} AND ${AREA_START_ID + AREA_COUNT - 1};`)
sql.push(`DELETE FROM scenic_spots WHERE id BETWEEN ${AREA_START_ID} AND ${AREA_START_ID + AREA_COUNT - 1};`)

for (let i = 11; i <= 25; i += 1) {
  sql.push(
    `INSERT OR REPLACE INTO users (id, username, password, nickname, role) VALUES ` +
    `(${i}, 'demo_user_${i}', 'pass123', '演示用户${i}', 'user');`
  )
}

let roadId = 100000
let foodId = 100000

for (let i = 0; i < AREA_COUNT; i += 1) {
  const areaId = AREA_START_ID + i
  const type = i % 5 === 0 ? 'campus' : 'scenic'
  const category = categories[i % categories.length]
  const city = cities[i % cities.length]
  const popularity = 3000 + ((i * 7919) % 47000)
  const rating = (3.6 + ((i * 37) % 14) / 10).toFixed(1)
  const name = spotName(i + 1, type)

  sql.push(
    `INSERT INTO scenic_spots ` +
    `(id, name, type, category, description, popularity, rating, rating_count, city, address, open_time, ticket_price, canvas_width, canvas_height) VALUES ` +
    `(${areaId}, ${q(name)}, ${q(type)}, ${q(category)}, ${q(`${name}用于课程设计规模化演示，包含建筑、设施、道路与美食数据。`)}, ` +
    `${popularity}, ${rating}, ${120 + i}, ${q(city)}, ${q(`${city}市示范路${i + 1}号`)}, '08:00-18:00', ${q(type === 'campus' ? '免费' : `${10 + (i % 9) * 5}元`)}, 1000, 700);`
  )

  for (let n = 1; n <= NODES_PER_AREA; n += 1) {
    let nodeType = 'building'
    let subType = n % 6 === 0 ? 'landmark' : 'teaching'
    if (n <= 2) {
      nodeType = 'entrance'
      subType = 'gate'
    } else if (n >= 15) {
      nodeType = 'facility'
      subType = facilityTypes[(i + n) % facilityTypes.length]
    }

    const x = 70 + ((n - 1) % 5) * 185 + ((i * 13) % 35)
    const y = 80 + Math.floor((n - 1) / 5) * 145 + ((i * 17) % 35)
    const namePrefix = nodeType === 'facility' ? '服务设施' : (nodeType === 'entrance' ? '出入口' : buildingNames[(i + n) % buildingNames.length])
    const nid = nodeId(areaId, n)

    sql.push(
      `INSERT INTO nodes ` +
      `(id, area_id, name, type, sub_type, pos_x, pos_y, description, floor) VALUES ` +
      `(${nid}, ${areaId}, ${q(`${namePrefix}${n}`)}, ${q(nodeType)}, ${q(subType)}, ${x}, ${y}, ${q(`${namePrefix}${n}，所属区域：${name}`)}, 1);`
    )
  }

  for (let n = 1; n < NODES_PER_AREA; n += 1) {
    const distance = 90 + ((i + n) % 9) * 25
    const congestion = (0.15 + ((i + n) % 7) * 0.08).toFixed(2)
    const transport = n % 3
    sql.push(
      `INSERT OR IGNORE INTO roads ` +
      `(id, area_id, from_node, to_node, distance, congestion, ideal_speed, transport, is_bidirectional) VALUES ` +
      `(${roadId++}, ${areaId}, ${nodeId(areaId, n)}, ${nodeId(areaId, n + 1)}, ${distance}, ${congestion}, ${transport === 0 ? 1.4 : transport === 1 ? 4.0 : 7.0}, ${transport}, 1);`
    )
  }

  for (let n = 1; n <= 5; n += 1) {
    const from = n
    const to = n + 10
    const distance = 160 + ((i + n) % 8) * 30
    const congestion = (0.2 + ((i + n) % 6) * 0.07).toFixed(2)
    sql.push(
      `INSERT OR IGNORE INTO roads ` +
      `(id, area_id, from_node, to_node, distance, congestion, ideal_speed, transport, is_bidirectional) VALUES ` +
      `(${roadId++}, ${areaId}, ${nodeId(areaId, from)}, ${nodeId(areaId, to)}, ${distance}, ${congestion}, 4.0, 1, 1);`
    )
  }

  for (let f = 1; f <= 3; f += 1) {
    const cuisine = cuisines[(i + f) % cuisines.length]
    const foodName = foodNames[(i + f) % foodNames.length]
    sql.push(
      `INSERT INTO foods ` +
      `(id, area_id, name, cuisine, restaurant, rating, rating_count, popularity, node_id, price, description) VALUES ` +
      `(${foodId++}, ${areaId}, ${q(`${foodName}${String(i + 1).padStart(3, '0')}`)}, ${q(cuisine)}, ${q(`${name}美食驿站${f}`)}, ` +
      `${(3.8 + ((i + f) % 11) / 10).toFixed(1)}, ${80 + i + f}, ${1200 + i * 11 + f * 97}, ${nodeId(areaId, 15 + f)}, ${18 + ((i + f) % 12) * 6}, ${q(`${cuisine}风味，适合游览途中快速补给。`)});`
    )
  }
}

sql.push('COMMIT;')
sql.push('')

const content = sql.join('\n')
writeFileSync(outFile, content, 'utf8')
writeFileSync(backendOutFile, content, 'utf8')
console.log(`Generated ${outFile}`)
console.log(`Generated ${backendOutFile}`)
console.log(`Areas: ${AREA_COUNT}, nodes: ${AREA_COUNT * NODES_PER_AREA}, roads: ${AREA_COUNT * 24}, foods: ${AREA_COUNT * 3}`)
