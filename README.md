# 智能旅游系统

基于智能体的个性化旅游系统课程设计项目，包含 C++ 后端、SQLite 数据库和 Vue 3 前端。系统围绕旅游推荐、路线规划、场所查询、旅游日记、美食推荐和数据概览等模块实现基础演示闭环。

## 功能模块

- 数据概览：展示景区/校园、节点、道路、设施、用户、日记、美食等数据规模，并检查课程数据规模要求。
- 旅游推荐：按热度、评分、综合分数进行 Top-K 推荐，支持名称和关键字查询。
- 路线规划：基于图结构和 Dijkstra 算法进行单点路径规划，并支持多点路线规划。
- 场所查询：基于实际路径距离查找附近设施，支持设施类别过滤。
- 日记管理：支持日记浏览、撰写、编辑、删除、评分和压缩。
- 美食推荐：按评分、热度、价格等维度推荐美食，支持菜系过滤和模糊搜索。

## 技术栈

- 后端：C++17、cpp-httplib、SQLite3、nlohmann/json、CMake
- 前端：Vue 3、TypeScript、Vite、Vue Router、Element Plus、Axios
- 数据：SQLite，配套 Node.js 演示数据生成脚本

## 目录结构

```text
.
├── data/                         # 根目录演示数据脚本和生成文件
├── docs/                         # 项目文档
├── smart-tourism-backend/        # C++ 后端
│   ├── include/
│   ├── src/
│   ├── data/
│   └── CMakeLists.txt
└── smart-tourism-frontend/       # Vue 前端
    ├── src/
    ├── package.json
    └── vite.config.ts
```

## 快速运行

### 1. 生成并导入演示数据

在项目根目录执行：

```powershell
node data\generate_demo_data.mjs
smart-tourism-backend\cmake-build-debug\bin\SmartTourismSystem.exe --seed-demo
```

生成数据规模约为：

- 景区/校园：225
- 节点：4250
- 道路：5102
- 美食：645
- 用户：25

### 2. 启动后端

```powershell
smart-tourism-backend\cmake-build-debug\bin\SmartTourismSystem.exe --port 8080
```

健康检查：

```text
http://localhost:8080/api/health
```

### 3. 启动前端

```powershell
cd smart-tourism-frontend
npm install
npm run dev
```

浏览器访问：

```text
http://localhost:3000/#/overview
```

常用页面：

```text
http://localhost:3000/#/recommend
http://localhost:3000/#/map
http://localhost:3000/#/facility
http://localhost:3000/#/diary
http://localhost:3000/#/food
```

## 构建验证

前端构建：

```powershell
cd smart-tourism-frontend
npm run build
```

后端构建：

```powershell
cmake --build smart-tourism-backend\cmake-build-debug --target SmartTourismSystem
```

## GitHub 提交建议

首次提交：

```powershell
git add .
git commit -m "Initial smart tourism system"
git remote add origin https://github.com/你的用户名/smart-tourism-system.git
git push -u origin main
```

后续提交：

```powershell
git add .
git commit -m "Update smart tourism features"
git push
```

## 说明

运行时数据库、构建目录、前端依赖和生成 SQL 文件已通过 `.gitignore` 忽略。需要演示数据时，重新执行 `node data\generate_demo_data.mjs` 和 `--seed-demo` 即可。
