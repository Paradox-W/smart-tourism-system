#ifndef SMART_TOURISM_DB_CONNECTION_H
#define SMART_TOURISM_DB_CONNECTION_H

#include <sqlite3.h>
#include <cstdint>
#include <string>
#include <functional>
#include <mutex>

/**
 * SQLite 数据库连接管理器
 * 采用 RAII 模式管理数据库连接生命周期
 * 支持事务操作和便捷的查询执行方法
 */
class DBConnection {
public:
    /**
     * 构造函数 - 打开数据库连接
     * @param db_path 数据库文件路径
     * @param init_sql_path 初始化SQL文件路径（可选，首次创建时执行）
     */
    explicit DBConnection(const std::string& db_path,
                          const std::string& init_sql_path = "");

    // 禁止拷贝
    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;

    // 允许移动
    DBConnection(DBConnection&& other) noexcept;
    DBConnection& operator=(DBConnection&& other) noexcept;

    // 默认构造（延迟初始化）
    DBConnection() : db_(nullptr), in_transaction_(false) {}

    ~DBConnection();

    // ========== 基础操作 ==========

    /** 获取原始 sqlite3 指针（谨慎使用） */
    sqlite3* get_raw() const { return db_; }

    /** 检查连接是否有效 */
    bool is_open() const { return db_ != nullptr; }

    /** 获取最后错误信息 */
    std::string get_last_error() const;

    /** 执行 SQL 语句（无返回值）: INSERT, UPDATE, DELETE, CREATE 等 */
    bool execute(const std::string& sql);

    /** 执行 SQL 文件（用于数据库初始化） */
    bool execute_file(const std::string& file_path);

    /**
     * 执行查询 SQL，对每一行结果调用回调函数
     * @param sql SQL 查询语句
     * @param callback 回调函数，参数为 (列数, 列值数组, 列名数组)，返回 false 可提前终止
     */
    bool query(const std::string& sql,
               const std::function<bool(int, char**, char**)>& callback);

    // ========== 事务操作 ==========

    /** 开始事务 */
    bool begin_transaction();

    /** 提交事务 */
    bool commit();

    /** 回滚事务 */
    bool rollback();

    // ========== 便捷查询方法 ==========

    /**
     * 执行查询并返回单行单列的值（如 COUNT, SUM）
     * @param sql SQL 查询
     * @param default_val 查询无结果时的默认值
     */
    int query_int(const std::string& sql, int default_val = 0);

    /**
     * 执行查询并返回单行单列的字符串值
     */
    std::string query_string(const std::string& sql,
                             const std::string& default_val = "");

    /** 获取最后插入的行 ID */
    int64_t last_insert_id() const;

    /** 获取上次操作影响的行数 */
    int changes_count() const;

private:
    sqlite3* db_ = nullptr;
    std::string db_path_;
    bool in_transaction_ = false;
    mutable std::recursive_mutex mutex_;

    void close();
};

/**
 * 全局数据库连接单例
 * 使用方法: DBConnection::get() 获取实例
 */
class Database {
public:
    static DBConnection& get(const std::string& db_path = "data/tourism.db",
                             const std::string& init_sql_path = "data/init_db.sql");

    // 禁止拷贝和移动
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

private:
    Database() = default;
    static DBConnection instance_;
};

#endif // SMART_TOURISM_DB_CONNECTION_H
