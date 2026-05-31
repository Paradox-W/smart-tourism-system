#include "repository/db_connection.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <mutex>

// ============================================================
// DBConnection 实现
// ============================================================

DBConnection::DBConnection(const std::string& db_path,
                           const std::string& init_sql_path)
    : db_path_(db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Failed to open database: " << db_path << std::endl;
        std::cerr << "[DB] Error: " << sqlite3_errmsg(db_) << std::endl;
        db_ = nullptr;
        return;
    }

    // 启用外键约束
    execute("PRAGMA foreign_keys = ON;");
    // 启用 WAL 模式（提升并发性能）
    execute("PRAGMA journal_mode = WAL;");

    std::cout << "[DB] Database opened: " << db_path << std::endl;

    // 如果提供了初始化SQL且数据库为空，执行初始化
    if (!init_sql_path.empty()) {
        int table_count = query_int(
            "SELECT COUNT(*) FROM sqlite_master WHERE type='table';");
        if (table_count == 0) {
            std::cout << "[DB] Empty database, running init SQL: "
                      << init_sql_path << std::endl;
            execute_file(init_sql_path);
        }
    }
}

DBConnection::DBConnection(DBConnection&& other) noexcept
    : db_(other.db_), db_path_(other.db_path_),
      in_transaction_(other.in_transaction_) {
    other.db_ = nullptr;
    other.in_transaction_ = false;
}

DBConnection& DBConnection::operator=(DBConnection&& other) noexcept {
    if (this != &other) {
        close();
        db_ = other.db_;
        db_path_ = other.db_path_;
        in_transaction_ = other.in_transaction_;
        other.db_ = nullptr;
        other.in_transaction_ = false;
    }
    return *this;
}

DBConnection::~DBConnection() {
    close();
}

void DBConnection::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (db_) {
        if (in_transaction_) {
            sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        }
        sqlite3_close(db_);
        db_ = nullptr;
        std::cout << "[DB] Database closed." << std::endl;
    }
}

std::string DBConnection::get_last_error() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (db_) {
        return sqlite3_errmsg(db_);
    }
    return "Database not opened";
}

bool DBConnection::execute(const std::string& sql) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!db_) return false;

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] SQL Error: " << err_msg << std::endl;
        std::cerr << "[DB] SQL: " << sql << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

bool DBConnection::query(const std::string& sql,
                         const std::function<bool(int, char**, char**)>& callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!db_) return false;

    sqlite3_stmt* stmt = nullptr;
    const char* tail = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, &tail);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Prepare Error: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // RAII 守卫：确保 sqlite3_finalize 在任何退出路径（含异常）都被调用
    struct StmtGuard {
        sqlite3_stmt* s;
        ~StmtGuard() { if (s) sqlite3_finalize(s); }
    } guard{stmt};

    int columns = sqlite3_column_count(stmt);
    bool result = true;

    int step_rc = SQLITE_ROW;  // 初始化为 SQLITE_ROW，确保循环至少执行一次
    while ((step_rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // 构建列值和列名数组
        auto** values = new char*[columns];
        auto** names = new char*[columns];
        for (int i = 0; i < columns; i++) {
            const char* val = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, i));
            values[i] = val ? strdup(val) : nullptr;
            names[i] = strdup(sqlite3_column_name(stmt, i));
        }

        if (!callback(columns, values, names)) {
            result = false; // 用户要求提前终止
        }

        for (int i = 0; i < columns; i++) {
            free(values[i]);
            free(names[i]);
        }
        delete[] values;
        delete[] names;
    }

    // step_rc 此时是 SQLITE_DONE（正常）或 SQLITE_ROW（不应该，因为循环条件）
    // 若是其他错误码（SQLITE_BUSY, SQLITE_MISUSE 等），报告错误
    if (step_rc != SQLITE_DONE) {
        std::cerr << "[DB] Query Error: " << sqlite3_errmsg(db_) << std::endl;
        result = false;
    }

    return result;
}

bool DBConnection::execute_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "[DB] Cannot open SQL file: " << file_path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sql = buffer.str();
    file.close();

    return execute(sql);
}

bool DBConnection::begin_transaction() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (in_transaction_) return false;
    if (execute("BEGIN TRANSACTION;")) {
        in_transaction_ = true;
        return true;
    }
    return false;
}

bool DBConnection::commit() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!in_transaction_) return false;
    if (execute("COMMIT;")) {
        in_transaction_ = false;
        return true;
    }
    in_transaction_ = false;
    return false;
}

bool DBConnection::rollback() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!in_transaction_) return false;
    if (execute("ROLLBACK;")) {
        in_transaction_ = false;
        return true;
    }
    in_transaction_ = false;
    return false;
}

int DBConnection::query_int(const std::string& sql, int default_val) {
    int result = default_val;
    query(sql, [&result](int cols, char** vals, char** names) {
        if (vals[0] != nullptr) {
            result = std::stoi(vals[0]);
        }
        return false; // 只取第一行
    });
    return result;
}

std::string DBConnection::query_string(const std::string& sql,
                                        const std::string& default_val) {
    std::string result = default_val;
    query(sql, [&result](int cols, char** vals, char** names) {
        if (vals[0] != nullptr) {
            result = vals[0];
        }
        return false;
    });
    return result;
}

int64_t DBConnection::last_insert_id() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (db_) {
        return sqlite3_last_insert_rowid(db_);
    }
    return -1;
}

int DBConnection::changes_count() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (db_) {
        return sqlite3_changes(db_);
    }
    return 0;
}

// ============================================================
// Database 单例实现
// ============================================================

DBConnection Database::instance_;

DBConnection& Database::get(const std::string& db_path,
                            const std::string& init_sql_path) {
    // 线程安全的单例初始化（C++11 call_once）
    static std::once_flag init_flag;
    std::call_once(init_flag, [&]() {
        instance_ = DBConnection(db_path, init_sql_path);
    });
    return instance_;
}
