/**
 * ============================================================
 * 智能旅游系统 - 主程序入口
 * Smart Tourism System - Main Entry Point
 * ============================================================
 *
 * 技术栈:
 *   - C++17
 *   - cpp-httplib (HTTP服务, header-only)
 *   - nlohmann/json (JSON处理, header-only)
 *   - SQLite3 (数据库)
 *
 * 用法:
 *   ./SmartTourismSystem [port]           默认端口 8080
 *   ./SmartTourismSystem --init-db        仅初始化数据库后退出
 *   ./SmartTourismSystem --test           运行算法单元测试
 *
 * API文档:
 *   启动后访问 http://localhost:8080/api/health 检查服务状态
 */

#include "server/http_server.h"
#include "repository/db_connection.h"
#include <iostream>
#include <string>
#include <csignal>

// 算法单元测试（条件编译）
#ifdef RUN_ALGORITHM_TESTS
// 将测试函数声明为 extern，实现在 test_algorithms.cpp 中
extern void run_all_tests();
#endif

// 全局服务器指针（用于信号处理）
static HttpServer* g_server = nullptr;

/** 信号处理函数 - 优雅关闭服务器 */
void signal_handler(int signum) {
    std::cout << "\n[Server] Received signal " << signum
              << ", shutting down..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

/** 打印启动 Banner */
void print_banner(int port) {
    std::cout << R"(
╔══════════════════════════════════════════════════════╗
║                                                      ║
║       智能旅游系统 Smart Tourism System v1.0           ║
║                                                      ║
║       C++17 + cpp-httplib + SQLite3 + Vue 3          ║
║                                                      ║
╚══════════════════════════════════════════════════════╝
)" << std::endl;
    std::cout << "[Server] Starting on http://localhost:" << port << std::endl;
    std::cout << "[Server] API Health: http://localhost:" << port << "/api/health" << std::endl;
    std::cout << "[Server] Press Ctrl+C to stop." << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    int port = 8080;
    bool init_db_only = false;
    bool run_tests = false;
    bool seed_demo_only = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--init-db") {
            init_db_only = true;
        } else if (arg == "--seed-demo") {
            seed_demo_only = true;
        } else if (arg == "--test") {
            run_tests = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0]
                      << " [--port PORT] [--init-db] [--seed-demo] [--test]" << std::endl;
            std::cout << "  --port PORT    Set server port (default: 8080)"
                      << std::endl;
            std::cout << "  --init-db      Initialize database and exit"
                      << std::endl;
            std::cout << "  --seed-demo    Import generated demo data and exit"
                      << std::endl;
            std::cout << "  --test         Run algorithm unit tests"
                      << std::endl;
            return 0;
        }
    }

    // 运行算法单元测试
    if (run_tests) {
#ifdef RUN_ALGORITHM_TESTS
        run_all_tests();
        return 0;
#else
        std::cerr << "[ERROR] Tests not compiled. Rebuild with -DRUN_ALGORITHM_TESTS=ON"
                  << std::endl;
        return 1;
#endif
    }

    // 仅初始化数据库模式
    if (init_db_only) {
        std::cout << "[DB] Initializing database..." << std::endl;
        DBConnection db("data/tourism.db", "data/init_db.sql");
        if (db.is_open()) {
            std::cout << "[DB] Database initialized successfully!" << std::endl;
            return 0;
        } else {
            std::cerr << "[DB] Failed to initialize database!" << std::endl;
            return 1;
        }
    }

    // 初始化数据库连接
    if (seed_demo_only) {
        std::cout << "[DB] Importing generated demo data..." << std::endl;
        DBConnection db("data/tourism.db", "data/init_db.sql");
        if (!db.is_open()) {
            std::cerr << "[DB] Failed to open database!" << std::endl;
            return 1;
        }
        if (!db.execute_file("data/generated_demo_seed.sql")) {
            std::cerr << "[DB] Failed to import data/generated_demo_seed.sql" << std::endl;
            return 1;
        }
        std::cout << "[DB] Demo data imported successfully." << std::endl;
        std::cout << "[DB] scenic_spots=" << db.query_int("SELECT COUNT(*) FROM scenic_spots")
                  << ", nodes=" << db.query_int("SELECT COUNT(*) FROM nodes")
                  << ", roads=" << db.query_int("SELECT COUNT(*) FROM roads")
                  << ", foods=" << db.query_int("SELECT COUNT(*) FROM foods")
                  << std::endl;
        return 0;
    }

    std::cout << "[DB] Connecting to database..." << std::endl;
    DBConnection db("data/tourism.db", "data/init_db.sql");
    if (!db.is_open()) {
        std::cerr << "[FATAL] Failed to connect to database!" << std::endl;
        return 1;
    }

    // 创建并配置 HTTP 服务器
    HttpServer server;
    server.register_routes();

    // 设置前端静态文件目录（生产模式）
    // 开发模式下由 Vite dev server 提供前端服务
    // server.set_static_dir("../smart-tourism-frontend/dist");

    g_server = &server;

    // 注册信号处理
    std::signal(SIGINT, signal_handler);
#ifdef _WIN32
    std::signal(SIGTERM, signal_handler);
#else
    std::signal(SIGPIPE, SIG_IGN);
#endif

    // 打印启动信息
    print_banner(port);

    // 启动服务器
    server.run("0.0.0.0", port);

    std::cout << "[Server] Server stopped." << std::endl;
    return 0;
}
