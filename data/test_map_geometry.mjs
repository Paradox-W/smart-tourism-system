import { readFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'
import assert from 'node:assert/strict'

const __dirname = dirname(fileURLToPath(import.meta.url))
const sqlPath = join(__dirname, 'generated_demo_seed.sql')
const content = readFileSync(sqlPath, 'utf8')

const nodePattern = /INSERT INTO nodes \(id, area_id, name, type, sub_type, pos_x, pos_y, description, floor\) VALUES \((\d+),\s*(\d+),\s*'[^']*',\s*'[^']*',\s*'[^']*',\s*([0-9.-]+),\s*([0-9.-]+),/g
const roadPattern = /INSERT OR IGNORE INTO roads \(id, area_id, from_node, to_node, distance, congestion, ideal_speed, transport, is_bidirectional\) VALUES \(\d+,\s*(\d+),\s*(\d+),\s*(\d+),\s*([0-9.]+),/g

const nodesById = new Map()
for (const match of content.matchAll(nodePattern)) {
  nodesById.set(Number(match[1]), {
    areaId: Number(match[2]),
    x: Number(match[3]),
    y: Number(match[4]),
  })
}

const ratiosByArea = new Map()
let totalRoads = 0
for (const match of content.matchAll(roadPattern)) {
  const areaId = Number(match[1])
  const from = nodesById.get(Number(match[2]))
  const to = nodesById.get(Number(match[3]))
  const distance = Number(match[4])
  assert(from, `missing from node ${match[2]}`)
  assert(to, `missing to node ${match[3]}`)

  const coordLength = Math.hypot(from.x - to.x, from.y - to.y)
  assert(coordLength > 0, `road ${match[2]}-${match[3]} has zero coordinate length`)

  const ratios = ratiosByArea.get(areaId) ?? []
  ratios.push(distance / coordLength)
  ratiosByArea.set(areaId, ratios)
  totalRoads++
}

const totalNodes = nodesById.size
console.log(`Generated demo data: ${totalNodes} nodes, ${totalRoads} roads, ${ratiosByArea.size} areas`)
assert(totalNodes > 8000, `expected >8000 nodes, got ${totalNodes}`)
assert(totalRoads > 10000, `expected >10000 roads, got ${totalRoads}`)

let badAreas = 0
for (const [areaId, ratios] of ratiosByArea) {
  const sorted = [...ratios].sort((a, b) => a - b)
  const median = sorted[Math.floor(sorted.length / 2)]
  const maxRelativeError = Math.max(...ratios.map(r => Math.abs(r - median) / median))
  if (maxRelativeError > 0.3) {
    badAreas++
    console.warn(`  area ${areaId}: max relative error ${(maxRelativeError * 100).toFixed(1)}% (${ratios.length} roads)`)
  }
}

assert(badAreas === 0, `${badAreas} areas have distance proportionality error > 30%`)
console.log(`Geometry proportionality OK for ${ratiosByArea.size} generated areas.`)
