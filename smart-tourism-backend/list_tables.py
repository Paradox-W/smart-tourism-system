import sqlite3
conn = sqlite3.connect(r'C:\Users\李佳\Desktop\智能旅游系统\data\tourism.db')
tables = conn.execute("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name").fetchall()
for t in tables:
    print(t[0])
conn.close()
