import sqlite3
conn = sqlite3.connect(r'C:\Users\李佳\Desktop\智能旅游系统\smart-tourism-backend\cmake-build-debug\bin\data\tourism.db')
tables = conn.execute("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name").fetchall()
print("Tables:", [t[0] for t in tables])
count = conn.execute("SELECT COUNT(*) FROM scenic_spots").fetchone()[0]
print(f"scenic_spots: {count} rows")
conn.close()
