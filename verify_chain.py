"""
端到端验证脚本：测试前后端完整链路
"""
import sqlite3
import json
import urllib.request
import os

BACKEND_URL = "http://localhost:8080"
DB_PATH = r"C:\Users\李佳\Desktop\智能旅游系统\smart-tourism-backend\cmake-build-debug\bin\data\tourism.db"

def test_step(name, func):
    try:
        result = func()
        print(f"  [OK] {name}: {result}")
        return True
    except Exception as e:
        print(f"  [FAIL] {name}: {e}")
        return False

def main():
    print("=" * 50)
    print("前后端链路验证")
    print("=" * 50)

    # 1. 检查数据库
    print("\n[1] 数据库检查")
    def check_db():
        if not os.path.exists(DB_PATH):
            return f"数据库不存在: {DB_PATH}"
        conn = sqlite3.connect(DB_PATH)
        count = conn.execute("SELECT COUNT(*) FROM scenic_spots").fetchone()[0]
        conn.close()
        return f"scenic_spots: {count} 条"

    test_step("bin目录数据库", check_db)

    # 2. 检查后端服务
    print("\n[2] 后端服务检查")
    def check_backend():
        req = urllib.request.urlopen(f"{BACKEND_URL}/api/health", timeout=3)
        return req.read().decode("utf-8")

    if not test_step("后端运行中", check_backend):
        print("  → 请先在 CLion 里重建并启动后端服务器")
        return

    # 3. 测试 API 返回格式
    print("\n[3] API 返回格式验证")
    def check_api():
        req = urllib.request.urlopen(f"{BACKEND_URL}/api/spots?limit=2", timeout=3)
        raw = req.read().decode("utf-8")
        data = json.loads(raw)
        # 关键检查：data 应该是数组，不是对象
        if isinstance(data.get("data"), list):
            return f"格式正确 - data 是数组，长度={len(data['data'])}"
        else:
            return f"格式错误 - data 类型是 {type(data.get('data'))}"

    test_step("/api/spots 返回格式", check_api)

    # 4. 测试推荐 API
    print("\n[4] 景点推荐 API")
    def check_recommend():
        req = urllib.request.urlopen(f"{BACKEND_URL}/api/spots/recommend?limit=3", timeout=3)
        raw = req.read().decode("utf-8")
        data = json.loads(raw)
        if isinstance(data.get("data"), list) and len(data["data"]) > 0:
            spot = data["data"][0]
            return f"返回 {len(data['data'])} 条，示例: {spot.get('name', 'N/A')}"
        return f"空数据或格式错误: {raw[:100]}"

    test_step("/api/spots/recommend", check_recommend)

    # 5. 检查前端代理
    print("\n[5] Vite 代理检查")
    def check_proxy():
        # Vite dev server 通常在 3000 端口
        req = urllib.request.urlopen("http://localhost:3000", timeout=3)
        return f"前端运行中，状态码: {req.status}"

    if not test_step("前端 Vite dev server (3000)", check_proxy):
        print("  -> 请在 smart-tourism-frontend 目录运行: npm run dev")

    print("\n" + "=" * 50)
    print("如所有步骤显示 ✓，刷新浏览器即可看到数据")
    print("=" * 50)

if __name__ == "__main__":
    main()
