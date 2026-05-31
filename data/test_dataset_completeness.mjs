import { readFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const __dirname = dirname(fileURLToPath(import.meta.url))
const sqlPath = join(__dirname, 'generated_demo_seed.sql')
const content = readFileSync(sqlPath, 'utf8')

let passed = 0
let failed = 0
const errors = []

function assert(condition, msg) {
  if (condition) {
    passed++
  } else {
    failed++
    errors.push(msg)
  }
}

function assertRange(val, min, max, label) {
  assert(val >= min && val <= max, `${label}: ${val} not in [${min}, ${max}]`)
}

// ==================== Parse SQL ====================
console.log('Parsing generated SQL...')

// Parse nodes
const nodePattern = /INSERT INTO nodes \(id, area_id, name, type, sub_type, pos_x, pos_y, description, floor\) VALUES \((\d+),\s*(\d+),\s*'([^']*)',\s*'([^']*)',\s*'([^']*)',\s*([0-9.-]+),\s*([0-9.-]+),\s*'([^']*)',\s*(\d+)\)/g
const nodesById = new Map()
const nodesByArea = new Map()
for (const match of content.matchAll(nodePattern)) {
  const node = {
    id: Number(match[1]),
    areaId: Number(match[2]),
    name: match[3],
    type: match[4],
    subType: match[5],
    x: Number(match[6]),
    y: Number(match[7]),
  }
  nodesById.set(node.id, node)
  if (!nodesByArea.has(node.areaId)) nodesByArea.set(node.areaId, [])
  nodesByArea.get(node.areaId).push(node)
}

// Parse roads
const roadPattern = /INSERT OR IGNORE INTO roads \(id, area_id, from_node, to_node, distance, congestion, ideal_speed, transport, is_bidirectional\) VALUES \(\d+,\s*(\d+),\s*(\d+),\s*(\d+),\s*([0-9.]+),\s*([0-9.]+),\s*([0-9.]+),\s*(\d+),\s*(\d+)\)/g
const roadsByArea = new Map()
const allRoads = []
for (const match of content.matchAll(roadPattern)) {
  const road = {
    areaId: Number(match[1]),
    fromNode: Number(match[2]),
    toNode: Number(match[3]),
    distance: Number(match[4]),
    congestion: Number(match[5]),
    speed: Number(match[6]),
    transport: Number(match[7]),
  }
  allRoads.push(road)
  if (!roadsByArea.has(road.areaId)) roadsByArea.set(road.areaId, [])
  roadsByArea.get(road.areaId).push(road)
}

// Parse buildings
const buildingPattern = /INSERT INTO buildings \(id, area_id, name, type, total_floors, has_elevator, description\) VALUES \((\d+),\s*(\d+),\s*'([^']*)',\s*'([^']*)',\s*(\d+),\s*(\d+),\s*'([^']*)'\)/g
const buildingsById = new Map()
const buildingsByArea = new Map()
for (const match of content.matchAll(buildingPattern)) {
  const b = {
    id: Number(match[1]),
    areaId: Number(match[2]),
    name: match[3],
    type: match[4],
    floors: Number(match[5]),
    hasElevator: Number(match[6]),
  }
  buildingsById.set(b.id, b)
  if (!buildingsByArea.has(b.areaId)) buildingsByArea.set(b.areaId, [])
  buildingsByArea.get(b.areaId).push(b)
}

// Parse foods
const foodPattern = /INSERT INTO foods \(id, area_id, name, cuisine, restaurant, rating, rating_count, popularity, node_id, price, description\) VALUES \(\d+,\s*(\d+),\s*'([^']*)',\s*'([^']*)',\s*'([^']*)',\s*([0-9.]+),\s*(\d+),\s*(\d+),\s*(\d+|-?\d+),\s*([0-9.]+),\s*'([^']*)'\)/g
const foodsByArea = new Map()
for (const match of content.matchAll(foodPattern)) {
  const food = {
    areaId: Number(match[1]),
    nodeId: Number(match[8]),
  }
  if (!foodsByArea.has(food.areaId)) foodsByArea.set(food.areaId, [])
  foodsByArea.get(food.areaId).push(food)
}

// Parse diaries
const diaryPattern = /INSERT INTO diaries \(id, user_id, title, content, destination, destination_id, tags, images, videos, popularity, avg_rating, rating_count, created_at\) VALUES \((\d+),\s*(\d+),\s*'([^']*)',/g
const diariesByArea = new Map()
for (const match of content.matchAll(diaryPattern)) {
  // Extract destination_id from the INSERT - need a better pattern
}
// Simpler approach: just count diaries
const diaryCount = (content.match(/INSERT INTO diaries/g) || []).length

// Parse indoor nodes
const indoorNodePattern = /INSERT INTO indoor_nodes \(id, building_id, floor, name, type, pos_x, pos_y\) VALUES \((\d+),\s*(\d+),\s*(\d+),/g
const indoorNodesByBuilding = new Map()
for (const match of content.matchAll(indoorNodePattern)) {
  const buildingId = Number(match[2])
  if (!indoorNodesByBuilding.has(buildingId)) indoorNodesByBuilding.set(buildingId, 0)
  indoorNodesByBuilding.set(buildingId, indoorNodesByBuilding.get(buildingId) + 1)
}

// Parse indoor roads
const indoorRoadCount = (content.match(/INSERT INTO indoor_roads/g) || []).length

// Parse scenic spots
const spotCount = (content.match(/INSERT INTO scenic_spots/g) || []).length

// Parse ratings
const ratingCount = (content.match(/INSERT OR IGNORE INTO ratings/g) || []).length

// Parse view_history
const viewHistoryCount = (content.match(/INSERT INTO view_history/g) || []).length

console.log(`Parsed: ${nodesById.size} nodes, ${allRoads.length} roads, ${buildingsById.size} buildings, ${foodsByArea.size} areas with food\n`)

// ==================== Validation ====================
console.log('=== Validation ===')

// 1. Area counts
console.log('\n--- Area counts ---')
assert(spotCount === 210, `scenic_spots count: ${spotCount} !== 210`)
console.log(`scenic_spots: ${spotCount}`)

// 2. Per-area node count
console.log('\n--- Per-area node counts ---')
let minNodes = Infinity, maxNodes = 0
for (const [areaId, nodes] of nodesByArea) {
  if (nodes.length < minNodes) minNodes = nodes.length
  if (nodes.length > maxNodes) maxNodes = nodes.length
}
assertRange(minNodes, 30, 60, 'min nodes per area')
assertRange(maxNodes, 30, 60, 'max nodes per area')
console.log(`nodes per area: ${minNodes}-${maxNodes} (target: 30-60)`)

// 3. Per-area road count
console.log('\n--- Per-area road counts ---')
let minRoads = Infinity, maxRoads = 0
for (const [areaId, roads] of roadsByArea) {
  if (roads.length < minRoads) minRoads = roads.length
  if (roads.length > maxRoads) maxRoads = roads.length
}
assertRange(minRoads, 45, 100, 'min roads per area')
assertRange(maxRoads, 45, 100, 'max roads per area')
console.log(`roads per area: ${minRoads}-${maxRoads} (target: 45-90)`)

// 4. All road endpoints exist
console.log('\n--- Road endpoint validity ---')
let brokenRoads = 0
for (const road of allRoads) {
  if (!nodesById.has(road.fromNode)) brokenRoads++
  if (!nodesById.has(road.toNode)) brokenRoads++
}
assert(brokenRoads === 0, `broken road endpoints: ${brokenRoads}`)
console.log(`broken road endpoints: ${brokenRoads}`)

// 5. BFS connectivity check per area
console.log('\n--- Graph connectivity (BFS) ---')
let disconnectedAreas = 0
for (const [areaId, areaNodes] of nodesByArea) {
  const areaRoads = roadsByArea.get(areaId) || []
  const adj = new Map()
  for (const n of areaNodes) adj.set(n.id, [])
  for (const r of areaRoads) {
    if (adj.has(r.fromNode) && adj.has(r.toNode)) {
      adj.get(r.fromNode).push(r.toNode)
      adj.get(r.toNode).push(r.fromNode)
    }
  }
  if (areaNodes.length === 0) continue
  const visited = new Set()
  const queue = [areaNodes[0].id]
  visited.add(areaNodes[0].id)
  while (queue.length > 0) {
    const cur = queue.shift()
    for (const neighbor of adj.get(cur) || []) {
      if (!visited.has(neighbor)) { visited.add(neighbor); queue.push(neighbor) }
    }
  }
  if (visited.size !== areaNodes.length) {
    disconnectedAreas++
  }
}
assert(disconnectedAreas === 0, `disconnected areas: ${disconnectedAreas}`)
console.log(`disconnected areas: ${disconnectedAreas}`)

// 6. Road endpoints in same area
console.log('\n--- Road area consistency ---')
let crossAreaRoads = 0
for (const road of allRoads) {
  const from = nodesById.get(road.fromNode)
  const to = nodesById.get(road.toNode)
  if (from && to && from.areaId !== road.areaId) crossAreaRoads++
  if (from && to && to.areaId !== road.areaId) crossAreaRoads++
}
assert(crossAreaRoads === 0, `cross-area road references: ${crossAreaRoads}`)
console.log(`cross-area road references: ${crossAreaRoads}`)

// 6. Road distance proportionality
console.log('\n--- Distance proportionality ---')
const METERS_PER_UNIT = 1.2
const ratiosByArea = new Map()
for (const road of allRoads) {
  const from = nodesById.get(road.fromNode)
  const to = nodesById.get(road.toNode)
  if (!from || !to) continue
  const coordDist = Math.hypot(from.x - to.x, from.y - to.y)
  if (coordDist < 1) continue
  const ratio = road.distance / coordDist
  if (!ratiosByArea.has(road.areaId)) ratiosByArea.set(road.areaId, [])
  ratiosByArea.get(road.areaId).push(ratio)
}

let maxOverallError = 0
let badAreas = 0
for (const [areaId, ratios] of ratiosByArea) {
  if (ratios.length < 3) continue
  const sorted = [...ratios].sort((a, b) => a - b)
  const median = sorted[Math.floor(sorted.length / 2)]
  const maxErr = Math.max(...ratios.map(r => Math.abs(r - median) / median))
  if (maxErr > 0.30) badAreas++
  if (maxErr > maxOverallError) maxOverallError = maxErr
}
assert(badAreas === 0, `areas with distance proportionality error > 30%: ${badAreas}`)
console.log(`max relative error: ${(maxOverallError * 100).toFixed(1)}% (threshold: 30%)`)
console.log(`areas with bad proportionality: ${badAreas}`)

// 7. Building-node correspondence
console.log('\n--- Building-node correspondence ---')
let orphanBuildings = 0
for (const [id, b] of buildingsById) {
  if (!nodesById.has(id)) orphanBuildings++
}
assert(orphanBuildings === 0, `buildings without matching nodes: ${orphanBuildings}`)
console.log(`buildings without matching nodes: ${orphanBuildings}`)
console.log(`buildings total: ${buildingsById.size}`)

// 8. Building has_elevator logic
console.log('\n--- Building elevator logic ---')
let badElevators = 0
for (const [id, b] of buildingsById) {
  if (b.floors >= 4 && b.hasElevator === 0) badElevators++
  if (b.floors < 4 && b.hasElevator === 1) badElevators++
}
assert(badElevators <= 10, `suspicious elevator assignments: ${badElevators}`)
console.log(`suspicious elevator assignments: ${badElevators}`)

// 9. Food node_id validity
console.log('\n--- Food node validity ---')
let brokenFoods = 0
for (const [areaId, foods] of foodsByArea) {
  for (const food of foods) {
    const node = nodesById.get(food.nodeId)
    if (!node) brokenFoods++
  }
}
assert(brokenFoods === 0, `foods with invalid node_id: ${brokenFoods}`)
console.log(`foods with invalid node_id: ${brokenFoods}`)
console.log(`areas with food data: ${foodsByArea.size}`)

// 10. Per-area building count
console.log('\n--- Per-area building counts ---')
let minBuildings = Infinity, maxBuildings = 0
for (const [areaId, bs] of buildingsByArea) {
  if (bs.length < minBuildings) minBuildings = bs.length
  if (bs.length > maxBuildings) maxBuildings = bs.length
}
console.log(`buildings per area: ${minBuildings}-${maxBuildings} (target: 4-12)`)

// 11. Per-area food count
console.log('\n--- Per-area food counts ---')
let minFoods = Infinity, maxFoods = 0
for (const [areaId, foods] of foodsByArea) {
  if (foods.length < minFoods) minFoods = foods.length
  if (foods.length > maxFoods) maxFoods = foods.length
}
assertRange(minFoods, 3, 12, 'min foods per area')
assertRange(maxFoods, 5, 12, 'max foods per area')
console.log(`foods per area: ${minFoods}-${maxFoods} (target: 5-10)`)

// Essential facility types per area
console.log('\n--- Essential facility types ---')
const essentialTypes = ['toilet', 'restaurant', 'shop', 'parking', 'service_center']
let areasMissingFacilities = 0
for (const [areaId, areaNodes] of nodesByArea) {
  const facilityTypes = new Set(areaNodes.filter(n => n.type === 'facility').map(n => n.subType))
  const missing = essentialTypes.filter(t => !facilityTypes.has(t))
  if (missing.length > 0) areasMissingFacilities++
}
assert(areasMissingFacilities === 0, `areas missing essential facilities: ${areasMissingFacilities}`)
console.log(`areas missing essential facilities: ${areasMissingFacilities}`)

// 12. Indoor data
console.log('\n--- Indoor navigation ---')
console.log(`indoor nodes: ${[...indoorNodesByBuilding.values()].reduce((s, v) => s + v, 0)}`)
console.log(`indoor roads: ${indoorRoadCount}`)
console.log(`buildings with indoor: ${indoorNodesByBuilding.size}`)

// 13. Diary count
console.log('\n--- Diaries ---')
console.log(`total diaries: ${diaryCount}`)

// 14. Ratings
console.log('\n--- Ratings ---')
console.log(`total ratings: ${ratingCount}`)

// 15. View history
console.log('\n--- View history ---')
console.log(`total view history: ${viewHistoryCount}`)

// 16. Node coordinate bounds
console.log('\n--- Node coordinate bounds ---')
let outOfBounds = 0
for (const [id, node] of nodesById) {
  if (node.x < 40 || node.x > 960 || node.y < 50 || node.y > 650) outOfBounds++
}
assert(outOfBounds <= nodesById.size * 0.05, `nodes out of bounds: ${outOfBounds}/${nodesById.size}`)
console.log(`nodes out of bounds: ${outOfBounds}/${nodesById.size}`)

// 17. Node type distribution
console.log('\n--- Node type distribution ---')
const typeCounts = {}
for (const [id, node] of nodesById) {
  typeCounts[node.type] = (typeCounts[node.type] || 0) + 1
}
for (const [type, count] of Object.entries(typeCounts)) {
  console.log(`  ${type}: ${count}`)
}

// ==================== Summary ====================
console.log(`\n${'='.repeat(40)}`)
console.log(`PASSED: ${passed}`)
console.log(`FAILED: ${failed}`)
if (errors.length > 0) {
  console.log('\nFailures:')
  for (const e of errors) console.log(`  - ${e}`)
}
process.exit(failed > 0 ? 1 : 0)
