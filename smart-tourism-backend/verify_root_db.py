import sqlite3, os
db = r"C:\Users\李佳\Desktop\智能旅游系统\smart-tourism-backend\data\tourism.db"
if not os.path.exists(db):
    print(f"Database NOT found at: {db}")
else:
    conn = sqlite3.connect(db)
    tables = [t[0] for t in conn.execute("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name").fetchall()]
    print(f"Tables ({len(tables)}): {tables}")
    try:
        count = conn.execute("SELECT COUNT(*) FROM scenic_spots").fetchone()[0]
        print(f"scenic_spots: {count} rows")
    except Exception as e:
        print(f"scenic_spots query error: {e}")
    conn.close()
