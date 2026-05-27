#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""第八周周报 - 简洁黑白版"""

from reportlab.lib.pagesizes import A4
from reportlab.lib import colors
from reportlab.lib.units import cm
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER, TA_LEFT, TA_JUSTIFY
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, HRFlowable
)
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
import os

# ── 字体 ──────────────────────────────────────────────────────────
FONT   = "SimSun"
BOLD   = "SimHei"
for path, name in [
    ("C:/Windows/Fonts/simsun.ttc", "SimSun"),
    ("C:/Windows/Fonts/msyh.ttc",   "SimSun"),
]:
    if os.path.exists(path):
        try:
            pdfmetrics.registerFont(TTFont("SimSun", path))
            break
        except Exception:
            pass

for path, name in [
    ("C:/Windows/Fonts/simhei.ttf", "SimHei"),
    ("C:/Windows/Fonts/msyhbd.ttc", "SimHei"),
]:
    if os.path.exists(path):
        try:
            pdfmetrics.registerFont(TTFont("SimHei", path))
            break
        except Exception:
            pass

# ── 样式 ──────────────────────────────────────────────────────────
BLACK = colors.black
GRAY  = colors.HexColor("#555555")
LGRAY = colors.HexColor("#dddddd")

def st(name, fn=FONT, size=10.5, color=BLACK, align=TA_LEFT, **kw):
    return ParagraphStyle(name, fontName=fn, fontSize=size,
                          textColor=color, alignment=align,
                          leading=kw.pop("leading", size * 1.7), **kw)

s_title    = st("title",  fn=BOLD,  size=18, align=TA_CENTER, spaceAfter=4)
s_date     = st("date",   fn=FONT,  size=10, color=GRAY, align=TA_CENTER, spaceAfter=2)
s_h1       = st("h1",     fn=BOLD,  size=12, spaceBefore=12, spaceAfter=4)
s_h2       = st("h2",     fn=BOLD,  size=10.5, spaceBefore=8, spaceAfter=3)
s_body     = st("body",   size=10,  align=TA_JUSTIFY, spaceAfter=2)
s_bullet   = st("bullet", size=10,  spaceAfter=2, leftIndent=12)
s_th       = st("th",     fn=BOLD,  size=10, align=TA_CENTER)
s_td       = st("td",     size=9.5, align=TA_LEFT, leading=15)
s_td_c     = st("tdc",    size=9.5, align=TA_CENTER, leading=15)

# ── 文档 ──────────────────────────────────────────────────────────
OUTPUT = "C:/Users/李佳/Desktop/智能旅游系统/docs/第八周周报.pdf"
doc = SimpleDocTemplate(OUTPUT, pagesize=A4,
    topMargin=2.5*cm, bottomMargin=2.5*cm,
    leftMargin=2.8*cm, rightMargin=2.8*cm)
W = A4[0] - 5.6*cm

story = []

def HR(thick=0.8):
    return HRFlowable(width=W, thickness=thick, color=BLACK, spaceAfter=6)

def h1(text):
    story.append(Spacer(1, 4))
    story.append(Paragraph(text, s_h1))
    story.append(HR())

def h2(text):
    story.append(Paragraph(text, s_h2))

def body(text):
    story.append(Paragraph(text, s_body))

def bullets(items):
    for item in items:
        story.append(Paragraph(f"· {item}", s_bullet))

# ── 标题 ──────────────────────────────────────────────────────────
story.append(Spacer(1, 0.5*cm))
story.append(Paragraph("基于智能体的个性化旅游系统", s_title))
story.append(Paragraph("第八周进度周报（2026.04.21 — 2026.04.26）", s_date))
story.append(Paragraph("数据结构课程设计 · 指导教师：郭岗", s_date))
story.append(Spacer(1, 0.3*cm))
story.append(HR(1.2))

# ── 一、概述 ──────────────────────────────────────────────────────
h1("一、本周工作概述")
body("本周核心任务为前端界面设计与功能代码实现。基于 Vue 3 + Vite 5 + TypeScript + "
     "Element Plus + TailwindCSS 完成了完整前端工程搭建，实现了 5 个功能页面的 UI 设计与"
     "前后端接口联调，系统数据实现端到端流通。")

# ── 二、前端工程搭建 ──────────────────────────────────────────────
h1("二、前端工程搭建")
bullets([
    "使用 Vite 5 + Vue 3 Composition API 初始化项目，配置 TypeScript 严格模式；",
    "引入 Element Plus 组件库（按需加载）+ TailwindCSS 实现样式系统；",
    "集成 Pinia 状态管理，封装全局用户状态与路由守卫；",
    "封装 Axios HTTP 工具层（src/utils/http.ts），统一请求拦截、响应拦截与错误处理；",
    "配置 Vite 反向代理，将 /api/* 请求转发至 C++ 后端 8080 端口；",
    "定义完整 TypeScript 类型（src/types/index.ts），覆盖景点、路线、日记、美食等实体；",
    "划分 5 个 API 模块（spot / route / facility / diary / food），与后端 REST 接口对应；",
    "实现 AppHeader 全局导航组件，支持滚动透明 → 毛玻璃过渡效果。",
])

# ── 三、功能页面实现 ──────────────────────────────────────────────
h1("三、五大功能页面实现")

page_data = [
    [Paragraph("页面", s_th),
     Paragraph("UI 组件", s_th),
     Paragraph("核心功能", s_th)],
    [Paragraph("旅游推荐\nRecommendView", s_td),
     Paragraph("景点卡片网格、搜索栏\n分类标签、详情弹窗", s_td),
     Paragraph("关键词搜索、分类过滤\n热度/评分排序、分页", s_td)],
    [Paragraph("路线规划\nMapView", s_td),
     Paragraph("Canvas 路网画布\n节点信息面板", s_td),
     Paragraph("Dijkstra 最短路 / TSP 多点规划\n路径高亮、拥挤度色阶", s_td)],
    [Paragraph("设施查询\nFacilityView", s_td),
     Paragraph("区域/节点选择器\n类别标签过滤", s_td),
     Paragraph("按 Dijkstra 实际路径距离排序\n就近设施推荐", s_td)],
    [Paragraph("旅游日记\nDiaryView", s_td),
     Paragraph("CRUD 弹窗\n检索模式切换", s_td),
     Paragraph("全文检索 / 精确查找切换\nHuffman 压缩率实时展示", s_td)],
    [Paragraph("美食推荐\nFoodView", s_td),
     Paragraph("Top-K 排行榜\n菜系标签栏", s_td),
     Paragraph("编辑距离模糊搜索\n相似度进度条可视化", s_td)],
]
cw = [W*0.22, W*0.38, W*0.40]
pt = Table(page_data, colWidths=cw)
pt.setStyle(TableStyle([
    ("FONTNAME",      (0,0), (-1,0),  BOLD),
    ("BACKGROUND",    (0,0), (-1,0),  colors.HexColor("#f0f0f0")),
    ("GRID",          (0,0), (-1,-1), 0.5, LGRAY),
    ("VALIGN",        (0,0), (-1,-1), "TOP"),
    ("TOPPADDING",    (0,0), (-1,-1), 6),
    ("BOTTOMPADDING", (0,0), (-1,-1), 6),
    ("LEFTPADDING",   (0,0), (-1,-1), 7),
    ("RIGHTPADDING",  (0,0), (-1,-1), 7),
]))
story.append(pt)

# ── 四、前后端联调 ─────────────────────────────────────────────────
h1("四、前后端接口联调")
bullets([
    "修复响应码判断：前端拦截器将 code !== 0 改为 code !== 200，与后端规范对齐；",
    "修复接口双重嵌套：Route 层将 {data:[...], total} 解包平铺至顶层响应对象；",
    "修复数据库路径：确认 CLion 工作目录为 smart-tourism-backend/，在 data/ 目录"
     "放置包含初始数据的 tourism.db；",
    "完成端到端验证：后端 API 正常响应，前端 RecommendView 成功渲染景点卡片。",
])

# ── 五、代码量统计 ─────────────────────────────────────────────────
h1("五、代码产出统计")

stat_data = [
    [Paragraph("模块", s_th),
     Paragraph("文件数", s_th),
     Paragraph("估计行数", s_th),
     Paragraph("说明", s_th)],
    [Paragraph("工程配置", s_td), Paragraph("6",  s_td_c), Paragraph("~350",  s_td_c), Paragraph("vite.config / tsconfig / package.json 等", s_td)],
    [Paragraph("工具 & 状态层", s_td), Paragraph("4",  s_td_c), Paragraph("~280",  s_td_c), Paragraph("http.ts / router / stores / 类型定义", s_td)],
    [Paragraph("API 接口层", s_td), Paragraph("5",  s_td_c), Paragraph("~220",  s_td_c), Paragraph("spot / route / facility / diary / food", s_td)],
    [Paragraph("页面组件", s_td),   Paragraph("5",  s_td_c), Paragraph("~1600", s_td_c), Paragraph("5 个功能页面 .vue（模板+逻辑+样式）", s_td)],
    [Paragraph("全局样式 & 布局", s_td), Paragraph("3", s_td_c), Paragraph("~160",  s_td_c), Paragraph("AppHeader.vue / index.css / tailwind 配置", s_td)],
    [Paragraph("合计", s_th),       Paragraph("23", s_td_c), Paragraph("~2610", s_td_c), Paragraph("纯前端代码，不含 node_modules", s_td)],
]
sw = [W*0.22, W*0.12, W*0.14, W*0.52]
st2 = Table(stat_data, colWidths=sw)
st2.setStyle(TableStyle([
    ("FONTNAME",      (0,0),  (-1,0),   BOLD),
    ("FONTNAME",      (0,-1), (-1,-1),  BOLD),
    ("BACKGROUND",    (0,0),  (-1,0),   colors.HexColor("#f0f0f0")),
    ("BACKGROUND",    (0,-1), (-1,-1),  colors.HexColor("#f8f8f8")),
    ("GRID",          (0,0),  (-1,-1),  0.5, LGRAY),
    ("VALIGN",        (0,0),  (-1,-1),  "MIDDLE"),
    ("TOPPADDING",    (0,0),  (-1,-1),  6),
    ("BOTTOMPADDING", (0,0),  (-1,-1),  6),
    ("LEFTPADDING",   (0,0),  (-1,-1),  7),
    ("RIGHTPADDING",  (0,0),  (-1,-1),  7),
]))
story.append(st2)

# ── 六、问题与解决 ─────────────────────────────────────────────────
h1("六、遇到的问题与解决方案")

problems = [
    ("前端页面无数据显示",
     "前端拦截器判断 res.code !== 0 为错误，后端返回 code=200，正常响应被丢弃。",
     "将拦截器判断改为 res.code !== 200。"),
    ("接口响应体双重嵌套",
     "Service 层返回 {data:[...], total}，Route 层再用 Response::ok() 包装，形成嵌套结构。",
     "在 Route 层显式解包，data 数组和 total 字段平铺至顶层。"),
    ("后端启动后读不到数据",
     "main.cpp 使用相对路径 data/tourism.db，CLion 工作目录未设置导致文件找不到。",
     "确认工作目录为 smart-tourism-backend/，在 data/ 放置数据库文件。"),
]

prob_data = [
    [Paragraph("问题", s_th),
     Paragraph("现象", s_th),
     Paragraph("解决方案", s_th)]
]
for title, pheno, sol in problems:
    prob_data.append([
        Paragraph(title, s_td),
        Paragraph(pheno, s_td),
        Paragraph(sol, s_td),
    ])
pw = [W*0.22, W*0.40, W*0.38]
prob_t = Table(prob_data, colWidths=pw)
prob_t.setStyle(TableStyle([
    ("FONTNAME",      (0,0), (-1,0),  BOLD),
    ("BACKGROUND",    (0,0), (-1,0),  colors.HexColor("#f0f0f0")),
    ("GRID",          (0,0), (-1,-1), 0.5, LGRAY),
    ("VALIGN",        (0,0), (-1,-1), "TOP"),
    ("TOPPADDING",    (0,0), (-1,-1), 6),
    ("BOTTOMPADDING", (0,0), (-1,-1), 6),
    ("LEFTPADDING",   (0,0), (-1,-1), 7),
    ("RIGHTPADDING",  (0,0), (-1,-1), 7),
]))
story.append(prob_t)

# ── 七、下周计划 ──────────────────────────────────────────────────
h1("七、下周工作计划")
bullets([
    "完善 MapView 路网数据加载与 Canvas 绘制效果，补充表单校验与加载状态动画；",
    "补充景点数据至 200 条以上，完善 Dijkstra 多权值（时间/距离/综合）路线返回格式；",
    "在 FacilityView 添加 Dijkstra 逐步动画演示，在 DiaryView 展示 Huffman 树结构；",
    "开始撰写课程设计报告[系统实现]章节，整理前端代码设计方案。",
])

story.append(Spacer(1, 1.5*cm))
story.append(HR(0.5))
story.append(Paragraph("汇报日期：2026 年 4 月 26 日", st("sign", fn=FONT, size=9.5,
    color=GRAY, align=TA_LEFT)))

doc.build(story)
print(f"PDF 生成成功: {OUTPUT}")
