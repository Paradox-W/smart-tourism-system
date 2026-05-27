-- ============================================================
-- 智能旅游系统 - 数据库初始化脚本
-- 数据库: SQLite3
-- 说明: 创建所有表结构、索引和初始数据
-- ============================================================

-- 如果数据库已存在则先删除（开发阶段用，生产环境注释掉）
-- DROP TABLE IF EXISTS ratings;
-- DROP TABLE IF EXISTS diary_tags;
-- DROP TABLE IF EXISTS foods;
-- DROP TABLE IF EXISTS diaries;
-- DROP TABLE IF EXISTS roads;
-- DROP TABLE IF EXISTS nodes;
-- DROP TABLE IF EXISTS scenic_spots;
-- DROP TABLE IF EXISTS users;

-- ============================================================
-- 1. 用户表
-- ============================================================
CREATE TABLE IF NOT EXISTS users (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    username        TEXT    NOT NULL UNIQUE,
    password        TEXT    NOT NULL,
    nickname        TEXT,
    avatar          TEXT,
    role            TEXT    DEFAULT 'user',  -- 'user' | 'admin'
    created_at      TEXT    NOT NULL DEFAULT (datetime('now', 'localtime'))
);

-- ============================================================
-- 2. 景区/校园表
-- ============================================================
CREATE TABLE IF NOT EXISTS scenic_spots (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    name            TEXT    NOT NULL,
    type            TEXT    NOT NULL,          -- 'scenic'(景区) | 'campus'(校园)
    category        TEXT,                      -- '自然' | '人文' | '历史' | '现代' | '综合'
    description     TEXT,
    popularity      INTEGER DEFAULT 0,         -- 热度（浏览量）
    rating          REAL    DEFAULT 0.0,       -- 平均评分
    rating_count    INTEGER DEFAULT 0,         -- 评分人数
    image           TEXT,                      -- 封面图路径
    city            TEXT,                      -- 所在城市
    address         TEXT,                      -- 详细地址
    open_time       TEXT,                      -- 开放时间
    ticket_price    TEXT,                      -- 门票价格描述
    canvas_width    INTEGER DEFAULT 800,       -- 路网Canvas宽度
    canvas_height   INTEGER DEFAULT 600,       -- 路网Canvas高度
    created_at      TEXT    NOT NULL DEFAULT (datetime('now', 'localtime')),
    updated_at      TEXT    DEFAULT (datetime('now', 'localtime'))
);

-- 景区索引
CREATE INDEX IF NOT EXISTS idx_spots_type ON scenic_spots(type);
CREATE INDEX IF NOT EXISTS idx_spots_category ON scenic_spots(category);
CREATE INDEX IF NOT EXISTS idx_spots_popularity ON scenic_spots(popularity DESC);
CREATE INDEX IF NOT EXISTS idx_spots_rating ON scenic_spots(rating DESC);
CREATE INDEX IF NOT EXISTS idx_spots_city ON scenic_spots(city);

-- ============================================================
-- 3. 节点表（景区/校园内部的建筑、设施、路口等）
-- ============================================================
CREATE TABLE IF NOT EXISTS nodes (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    area_id         INTEGER NOT NULL,          -- 所属景区/校园ID
    name            TEXT    NOT NULL,
    type            TEXT    NOT NULL,          -- 'building' | 'facility' | 'junction' | 'entrance'
    sub_type        TEXT,                      -- 子类型: 'toilet'|'canteen'|'library'|'supermarket'|...
    pos_x           REAL    NOT NULL,          -- Canvas X坐标
    pos_y           REAL    NOT NULL,          -- Canvas Y坐标
    description     TEXT,
    floor           INTEGER,                   -- 楼层（室内导航用，0表示室外）
    building_id     INTEGER,                   -- 所属建筑ID（室内导航用）
    FOREIGN KEY (area_id) REFERENCES scenic_spots(id) ON DELETE CASCADE
);

-- 节点索引
CREATE INDEX IF NOT EXISTS idx_nodes_area ON nodes(area_id);
CREATE INDEX IF NOT EXISTS idx_nodes_type ON nodes(area_id, type);
CREATE INDEX IF NOT EXISTS idx_nodes_sub_type ON nodes(area_id, sub_type);
CREATE INDEX IF NOT EXISTS idx_nodes_building ON nodes(building_id);

-- ============================================================
-- 4. 道路表（节点之间的边）
-- ============================================================
CREATE TABLE IF NOT EXISTS roads (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    area_id         INTEGER NOT NULL,          -- 所属景区/校园ID
    from_node       INTEGER NOT NULL,          -- 起点节点ID
    to_node         INTEGER NOT NULL,          -- 终点节点ID
    distance        REAL    NOT NULL,          -- 距离（米）
    congestion      REAL    DEFAULT 1.0,       -- 拥挤度 (0, 1]，1=畅通
    ideal_speed     REAL    DEFAULT 1.4,       -- 理想速度（米/秒）
                                            -- 步行约1.4, 自行车约4.0, 电瓶车约8.0
    transport       INTEGER DEFAULT 0,        -- 交通工具: 0=步行, 1=自行车, 2=电瓶车
    is_bidirectional INTEGER DEFAULT 1,       -- 是否双向: 1=是, 0=否
    FOREIGN KEY (area_id) REFERENCES scenic_spots(id) ON DELETE CASCADE,
    FOREIGN KEY (from_node) REFERENCES nodes(id) ON DELETE CASCADE,
    FOREIGN KEY (to_node) REFERENCES nodes(id) ON DELETE CASCADE
);

-- 道路索引
CREATE INDEX IF NOT EXISTS idx_roads_area ON roads(area_id);
CREATE INDEX IF NOT EXISTS idx_roads_from ON roads(from_node);
CREATE INDEX IF NOT EXISTS idx_roads_to ON roads(to_node);
CREATE INDEX IF NOT EXISTS idx_roads_transport ON roads(area_id, transport);

-- ============================================================
-- 5. 建筑物扩展表（用于室内导航）
-- ============================================================
CREATE TABLE IF NOT EXISTS buildings (
    id              INTEGER PRIMARY KEY,      -- 与nodes表的id对应
    area_id         INTEGER NOT NULL,
    name            TEXT    NOT NULL,
    type            TEXT    NOT NULL,          -- 'teaching'|'office'|'dorm'|'museum'|'library'|'gym'
    total_floors    INTEGER DEFAULT 1,         -- 总楼层数
    has_elevator    INTEGER DEFAULT 0,         -- 是否有电梯: 1=是, 0=否
    description     TEXT,
    FOREIGN KEY (id) REFERENCES nodes(id) ON DELETE CASCADE,
    FOREIGN KEY (area_id) REFERENCES scenic_spots(id) ON DELETE CASCADE
);

-- ============================================================
-- 6. 旅游日记表
-- ============================================================
CREATE TABLE IF NOT EXISTS diaries (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id         INTEGER NOT NULL,
    title           TEXT    NOT NULL,
    content         TEXT    NOT NULL,          -- 日记正文
    compressed_content BLOB,                   -- Huffman压缩后的内容
    is_compressed   INTEGER DEFAULT 0,         -- 是否已压缩: 1=是, 0=否
    destination     TEXT,                      -- 旅游目的地（景区名）
    destination_id  INTEGER,                   -- 旅游目的地景区ID
    tags            TEXT,                      -- 标签（JSON数组，如 '["美食","风景","人文"]'）
    images          TEXT,                      -- 图片路径列表（JSON数组）
    videos          TEXT,                      -- 视频路径列表（JSON数组）
    popularity      INTEGER DEFAULT 0,         -- 浏览量（热度）
    avg_rating      REAL    DEFAULT 0.0,       -- 平均评分
    rating_count    INTEGER DEFAULT 0,         -- 评分人数
    created_at      TEXT    NOT NULL DEFAULT (datetime('now', 'localtime')),
    updated_at      TEXT    DEFAULT (datetime('now', 'localtime')),
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (destination_id) REFERENCES scenic_spots(id) ON DELETE SET NULL
);

-- 日记索引
CREATE INDEX IF NOT EXISTS idx_diaries_user ON diaries(user_id);
CREATE INDEX IF NOT EXISTS idx_diaries_dest ON diaries(destination_id);
CREATE INDEX IF NOT EXISTS idx_diaries_popularity ON diaries(popularity DESC);
CREATE INDEX IF NOT EXISTS idx_diaries_rating ON diaries(avg_rating DESC);
CREATE INDEX IF NOT EXISTS idx_diaries_created ON diaries(created_at DESC);

-- ============================================================
-- 7. 评分表（日记评分）
-- ============================================================
CREATE TABLE IF NOT EXISTS ratings (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id         INTEGER NOT NULL,
    diary_id        INTEGER NOT NULL,
    score           INTEGER NOT NULL CHECK(score >= 1 AND score <= 5),
    created_at      TEXT    NOT NULL DEFAULT (datetime('now', 'localtime')),
    UNIQUE(user_id, diary_id),                -- 每用户对每篇日记只能评一次
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (diary_id) REFERENCES diaries(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_ratings_diary ON ratings(diary_id);

-- ============================================================
-- 8. 美食表
-- ============================================================
CREATE TABLE IF NOT EXISTS foods (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    area_id         INTEGER NOT NULL,          -- 所属景区/校园ID
    name            TEXT    NOT NULL,          -- 美食名称
    cuisine         TEXT    NOT NULL,          -- 菜系: '川菜'|'粤菜'|'鲁菜'|'西餐'|'快餐'|...
    restaurant      TEXT    NOT NULL,          -- 饭店/食堂/窗口名称
    rating          REAL    DEFAULT 0.0,       -- 评分
    rating_count    INTEGER DEFAULT 0,
    popularity      INTEGER DEFAULT 0,         -- 热度
    node_id         INTEGER,                   -- 所在位置节点ID（用于计算距离）
    price           REAL,                      -- 人均价格（元）
    description     TEXT,                      -- 简介
    image           TEXT,                      -- 图片路径
    FOREIGN KEY (area_id) REFERENCES scenic_spots(id) ON DELETE CASCADE,
    FOREIGN KEY (node_id) REFERENCES nodes(id) ON DELETE SET NULL
);

-- 美食索引
CREATE INDEX IF NOT EXISTS idx_foods_area ON foods(area_id);
CREATE INDEX IF NOT EXISTS idx_foods_cuisine ON foods(area_id, cuisine);
CREATE INDEX IF NOT EXISTS idx_foods_rating ON foods(rating DESC);
CREATE INDEX IF NOT EXISTS idx_foods_popularity ON foods(popularity DESC);

-- ============================================================
-- 9. 用户兴趣表（用于个性化推荐）
-- ============================================================
CREATE TABLE IF NOT EXISTS user_interests (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id         INTEGER NOT NULL,
    category        TEXT    NOT NULL,          -- 兴趣类别: '自然'|'人文'|'历史'|'美食'|'户外'|...
    weight          REAL    DEFAULT 1.0,       -- 兴趣权重（0-1）
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(user_id, category)
);

-- ============================================================
-- 10. 用户浏览历史表（用于推荐算法）
-- ============================================================
CREATE TABLE IF NOT EXISTS view_history (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id         INTEGER NOT NULL,
    spot_id         INTEGER NOT NULL,          -- 浏览的景区ID
    view_time       TEXT    NOT NULL DEFAULT (datetime('now', 'localtime')),
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (spot_id) REFERENCES scenic_spots(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_view_history_user ON view_history(user_id);
CREATE INDEX IF NOT EXISTS idx_view_history_time ON view_history(view_time DESC);

-- ============================================================
-- 11. 室内楼层节点表（室内导航用）
-- ============================================================
CREATE TABLE IF NOT EXISTS indoor_nodes (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    building_id     INTEGER NOT NULL,          -- 所属建筑ID
    floor           INTEGER NOT NULL,          -- 楼层
    name            TEXT    NOT NULL,          -- 节点名称: '大门'|'电梯'|'楼梯'|'卫生间'|'101教室'
    type            TEXT    NOT NULL,          -- 'entrance'|'elevator'|'stairs'|'room'|'facility'
    pos_x           REAL    NOT NULL,
    pos_y           REAL    NOT NULL,
    FOREIGN KEY (building_id) REFERENCES buildings(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_indoor_building ON indoor_nodes(building_id);
CREATE INDEX IF NOT EXISTS idx_indoor_floor ON indoor_nodes(building_id, floor);

-- ============================================================
-- 12. 室内道路表（室内导航用）
-- ============================================================
CREATE TABLE IF NOT EXISTS indoor_roads (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    building_id     INTEGER NOT NULL,
    from_node       INTEGER NOT NULL,
    to_node         INTEGER NOT NULL,
    distance        REAL    NOT NULL,
    FOREIGN KEY (building_id) REFERENCES buildings(id) ON DELETE CASCADE,
    FOREIGN KEY (from_node) REFERENCES indoor_nodes(id) ON DELETE CASCADE,
    FOREIGN KEY (to_node) REFERENCES indoor_nodes(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_indoor_roads_building ON indoor_roads(building_id);

-- ============================================================
-- 初始数据：用户（至少10人）
-- ============================================================
INSERT INTO users (username, password, nickname, role) VALUES
    ('zhangsan', '123456', '张三', 'user'),
    ('lisi',     '123456', '李四', 'user'),
    ('wangwu',   '123456', '王五', 'user'),
    ('zhaoliu',  '123456', '赵六', 'user'),
    ('sunqi',    '123456', '孙七', 'user'),
    ('zhouba',   '123456', '周八', 'user'),
    ('wujiu',    '123456', '吴九', 'user'),
    ('zhengshi', '123456', '郑十', 'user'),
    ('admin',    'admin123', '管理员', 'admin'),
    ('testuser', '123456', '测试用户', 'user');

-- ============================================================
-- 初始数据：景区/校园（示例数据，正式版本需补充至200+）
-- ============================================================
INSERT INTO scenic_spots (name, type, category, description, popularity, rating, city, address, open_time, ticket_price) VALUES
    ('故宫博物院',       'scenic', '历史', '中国明清两代的皇家宫殿，世界文化遗产', 98000, 4.9, '北京', '北京市东城区景山前街4号', '08:30-17:00', '60元'),
    ('颐和园',           'scenic', '历史', '中国清朝时期皇家园林，世界文化遗产', 85000, 4.8, '北京', '北京市海淀区新建宫门路19号', '06:30-18:00', '30元'),
    ('八达岭长城',       'scenic', '历史', '万里长城的重要组成部分', 92000, 4.8, '北京', '北京市延庆区G6京藏高速58号出口', '06:30-19:00', '40元'),
    ('天坛公园',         'scenic', '历史', '明清两代帝王祭天祈谷的场所', 65000, 4.7, '北京', '北京市东城区天坛东路甲1号', '06:00-22:00', '15元'),
    ('圆明园遗址公园',   'scenic', '历史', '清代大型皇家园林遗址', 52000, 4.6, '北京', '北京市海淀区清华西路28号', '07:00-19:00', '10元'),
    ('北海公园',         'scenic', '人文', '中国现存最古老、最完整的皇家园林之一', 45000, 4.5, '北京', '北京市西城区文津街1号', '06:30-20:00', '10元'),
    ('鸟巢（国家体育场）','scenic','现代', '2008年北京奥运会主场馆', 55000, 4.4, '北京', '北京市朝阳区国家体育场南路1号', '09:00-21:00', '50元'),
    ('水立方',           'scenic', '现代', '2008年北京奥运会游泳场馆', 48000, 4.3, '北京', '北京市朝阳区天辰东路11号', '09:00-21:00', '30元'),
    ('香山公园',         'scenic', '自然', '以红叶著称的皇家园林', 60000, 4.6, '北京', '北京市海淀区买卖街40号', '06:00-18:30', '10元'),
    ('北京动物园',       'scenic', '综合', '中国开放最早、饲养动物种类最多的动物园', 58000, 4.5, '北京', '北京市西城区西直门外大街137号', '07:30-18:00', '15元'),

    ('北京邮电大学西土城校区', 'campus', '现代', '北京邮电大学主校区', 12000, 4.5, '北京', '北京市海淀区西土城路10号', '全天', '免费'),
    ('北京大学',           'campus', '人文', '中国最高学府之一', 35000, 4.7, '北京', '北京市海淀区颐和园路5号', '全天', '免费'),
    ('清华大学',           'campus', '综合', '中国顶尖理工科大学', 38000, 4.8, '北京', '北京市海淀区双清路30号', '全天', '免费'),
    ('北京师范大学',       'campus', '人文', '中国师范类最高学府', 15000, 4.4, '北京', '北京市海淀区新街口外大街19号', '全天', '免费'),
    ('北京理工大学',       'campus', '现代', '以工科见长的985高校', 13000, 4.3, '北京', '北京市海淀区中关村南大街5号', '全天', '免费');

-- ============================================================
-- 初始数据：用户兴趣
-- ============================================================
INSERT INTO user_interests (user_id, category, weight) VALUES
    (1, '历史', 0.9), (1, '人文', 0.7), (1, '自然', 0.3),
    (2, '自然', 0.8), (2, '户外', 0.9), (2, '美食', 0.5),
    (3, '美食', 0.9), (3, '现代', 0.6), (3, '人文', 0.4),
    (4, '人文', 0.8), (4, '历史', 0.7), (4, '美食', 0.6),
    (5, '综合', 0.7), (5, '户外', 0.8), (5, '自然', 0.5);
