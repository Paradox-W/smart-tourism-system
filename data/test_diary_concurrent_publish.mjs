const API_BASE = process.env.API_BASE || 'http://localhost:8080'
const CONCURRENCY = Number(process.env.CONCURRENCY || 30)

async function getJson(path, options) {
  const res = await fetch(`${API_BASE}${path}`, options)
  const text = await res.text()
  let body = null
  try {
    body = JSON.parse(text)
  } catch {
    body = { raw: text }
  }
  return { status: res.status, ok: res.ok, body }
}

async function assertServerHealthy(label) {
  const health = await getJson('/api/health')
  if (!health.ok || health.body.code !== 200) {
    throw new Error(`${label}: backend health check failed: ${JSON.stringify(health)}`)
  }
}

await assertServerHealthy('before concurrent publish')

const stamp = Date.now()
const payloads = Array.from({ length: CONCURRENCY }, (_, i) => ({
  user_id: 1,
  title: `P0-concurrent-publish-${stamp}-${i}`,
  content: 'Regression test for rapid repeated diary publish clicks.',
  destination: '天坛公园',
  tags: JSON.stringify(['P0', 'concurrent']),
}))

const results = await Promise.allSettled(payloads.map(payload =>
  getJson('/api/diaries', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload),
  })
))

await new Promise(resolve => setTimeout(resolve, 1000))
await assertServerHealthy('after concurrent publish')

const rejected = results.filter(r => r.status === 'rejected')
const failedResponses = results
  .filter(r => r.status === 'fulfilled')
  .map(r => r.value)
  .filter(r => !r.ok || r.body.code !== 200)

if (rejected.length || failedResponses.length) {
  throw new Error(`concurrent publish had failed requests: rejected=${rejected.length}, failed=${failedResponses.length}`)
}

console.log(`Concurrent diary publish OK: ${CONCURRENCY}/${CONCURRENCY} requests succeeded and backend stayed healthy.`)
