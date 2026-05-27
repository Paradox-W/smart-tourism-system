import sqlite3
conn = sqlite3.connect(r'C:\Users\李佳\Desktop\智能旅游系统\data\tourism.db')

# 检查各表数据量
tables = ['users', 'scenic_spots', 'nodes', 'roads', 'foods', 'diaries', 'ratings']
for t in tables:
    try:
        count = conn.execute(f"SELECT COUNT(*) FROM {t}").fetchone()[0]
        print(f"{t}: {count} rows")
    except Exception as e:
        print(f"{t}: ERROR - {e}")

# 直接查看 scenic_spots 数据
print("\n=== scenic_spots 数据 ===")
rows = conn.execute("SELECT id, name, type, popularity, rating FROM scenic_spots").fetchall()
for r in rows:
    print(f"  {r}")

conn.close()
