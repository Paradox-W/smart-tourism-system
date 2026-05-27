import sqlite3
conn = sqlite3.connect(r'C:\Users\李佳\Desktop\智能旅游系统\data\tourism.db')
tables = ['nodes', 'roads', 'scenic_spots', 'foods', 'diaries', 'ratings', 'user_interests', 'view_history']
for t in tables:
    cols = conn.execute(f"PRAGMA table_info({t})").fetchall()
    print(f"\n=== {t} ===")
    for c in cols:
        print(f"  {c[1]} ({c[2]})")
conn.close()
