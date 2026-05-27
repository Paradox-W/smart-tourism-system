/**
 * @file diary_service.h
 * @brief 日记管理服务 —— CRUD + HashMap精确查找 + 倒排索引全文检索 + Huffman压缩
 *
 * 核心算法：
 *   - HashMap: 以标题为key进行O(1)精确查找
 *   - InvertedIndex: 全文检索（TF-IDF简化版，AND查询）
 *   - Huffman编码: 日记内容无损压缩存储
 *
 * 设计要点：
 *   - InvertedIndex 使用 static 成员变量懒加载，避免每次请求重建
 *   - 压缩结果以 base64 编码存入数据库
 */

#pragma once

#include "json.hpp"
#include "algorithm/hash_table.h"
#include "algorithm/inverted_index.h"
#include "algorithm/huffman.h"
#include "repository/diary_repo.h"
#include <string>
#include <cstring>
#include <utility>

using json = nlohmann::json;

namespace service {

class DiaryService {
private:
    // 静态缓存：倒排索引（懒加载）
    static algorithm::InvertedIndex& get_index() {
        static algorithm::InvertedIndex index;
        static bool initialized = false;

        if (!initialized) {
            rebuild_index(index);
            initialized = true;
        }

        return index;
    }

    // 静态缓存：标题 HashMap（标题 -> 日记ID）
    static algorithm::HashMap<std::string, int>& get_title_map() {
        static algorithm::HashMap<std::string, int> title_map(64);
        static bool initialized = false;

        if (!initialized) {
            rebuild_title_map(title_map);
            initialized = true;
        }

        return title_map;
    }

    /**
     * @brief 从数据库加载所有日记，重建倒排索引
     */
    static void rebuild_index(algorithm::InvertedIndex& index) {
        try {
            json diaries = repository::DiaryRepo::get_all_for_indexing();
            if (!diaries.is_array()) return;

            for (const auto& d : diaries) {
                int id = d.value("id", 0);
                std::string title = d.value("title", "");
                std::string content = d.value("content", "");
                if (id > 0 && (!title.empty() || !content.empty())) {
                    index.add_document(id, title, content);
                }
            }
        } catch (...) {
            // 索引构建失败不影响基本功能
        }
    }

    /**
     * @brief 从数据库加载所有日记标题，构建 HashMap
     */
    static void rebuild_title_map(algorithm::HashMap<std::string, int>& title_map) {
        try {
            json diaries = repository::DiaryRepo::get_all_for_indexing();
            if (!diaries.is_array()) return;

            for (const auto& d : diaries) {
                int id = d.value("id", 0);
                std::string title = d.value("title", "");
                if (id > 0 && !title.empty()) {
                    title_map.insert(title, id);
                }
            }
        } catch (...) {
            // 构建失败不影响基本功能
        }
    }

    /**
     * @brief 强制刷新索引（新增/修改/删除日记后调用）
     */
    static void refresh_index() {
        static bool initialized = false;
        // 通过重建来刷新
        try {
            json diaries = repository::DiaryRepo::get_all_for_indexing();
            auto& index = get_index();
            auto& title_map = get_title_map();

            // 清空并重建
            index = std::move(algorithm::InvertedIndex());
            title_map.clear();

            if (diaries.is_array()) {
                for (const auto& d : diaries) {
                    int id = d.value("id", 0);
                    std::string title = d.value("title", "");
                    std::string content = d.value("content", "");
                    if (id > 0) {
                        if (!title.empty() || !content.empty()) {
                            index.add_document(id, title, content);
                        }
                        if (!title.empty()) {
                            title_map.insert(title, id);
                        }
                    }
                }
            }
        } catch (...) {}
    }

    /**
     * @brief 将字节数组转为 base64 编码字符串
     */
    static std::string bytes_to_base64(const unsigned char* data, int len) {
        static const char charset[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        result.reserve(((len + 2) / 3) * 4);

        for (int i = 0; i < len; i += 3) {
            unsigned int n = static_cast<unsigned int>(data[i]) << 16;
            if (i + 1 < len) n |= static_cast<unsigned int>(data[i + 1]) << 8;
            if (i + 2 < len) n |= static_cast<unsigned int>(data[i + 2]);

            result += charset[(n >> 18) & 0x3F];
            result += charset[(n >> 12) & 0x3F];
            result += (i + 1 < len) ? charset[(n >> 6) & 0x3F] : '=';
            result += (i + 2 < len) ? charset[n & 0x3F] : '=';
        }

        return result;
    }

public:
    /**
     * @brief 获取日记列表
     */
    static json get_diaries(int page = 1, int page_size = 20,
                            const std::string& sort_by = "created_at",
                            const std::string& order = "desc",
                            int destination_id = -1) {
        json result;
        try {
            result = repository::DiaryRepo::get_all(page, page_size, sort_by, order, destination_id);
        } catch (const std::exception& e) {
            result["error"] = std::string("获取日记列表异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 获取日记详情
     */
    static json get_diary(int id) {
        json result;
        try {
            if (id <= 0) {
                result["error"] = "无效的日记ID";
                return result;
            }
            result = repository::DiaryRepo::get_by_id(id);
        } catch (const std::exception& e) {
            result["error"] = std::string("获取日记详情异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 创建日记
     * @param body JSON: {user_id, title, content, destination, destination_id, tags, images, videos}
     */
    static json create_diary(const json& body) {
        json result;
        try {
            int user_id = body.value("user_id", 0);
            std::string title = body.value("title", "");
            std::string content = body.value("content", "");
            std::string dest = body.value("destination", "");
            int dest_id = body.value("destination_id", 0);
            std::string tags = body.value("tags", "");
            std::string images = body.value("images", "");
            std::string videos = body.value("videos", "");

            if (user_id <= 0 || title.empty()) {
                result["error"] = "参数错误: user_id 和 title 不能为空";
                return result;
            }

            int new_id = repository::DiaryRepo::create(
                user_id, title, content, dest, dest_id, tags, images, videos);

            if (new_id > 0) {
                result["success"] = true;
                result["id"] = new_id;
                result["message"] = "日记创建成功";
                // 刷新索引
                refresh_index();
            } else {
                result["error"] = "日记创建失败";
            }
        } catch (const std::exception& e) {
            result["error"] = std::string("创建日记异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 更新日记
     */
    static json update_diary(int id, const json& body) {
        json result;
        try {
            if (id <= 0) {
                result["error"] = "无效的日记ID";
                return result;
            }

            bool success = repository::DiaryRepo::update(id, body);
            if (success) {
                result["success"] = true;
                result["message"] = "日记更新成功";
                // 刷新索引
                refresh_index();
            } else {
                result["error"] = "日记更新失败";
            }
        } catch (const std::exception& e) {
            result["error"] = std::string("更新日记异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 删除日记
     */
    static json delete_diary(int id) {
        json result;
        try {
            if (id <= 0) {
                result["error"] = "无效的日记ID";
                return result;
            }

            bool success = repository::DiaryRepo::remove(id);
            if (success) {
                result["success"] = true;
                result["message"] = "日记删除成功";
                // 刷新索引
                refresh_index();
            } else {
                result["error"] = "日记删除失败";
            }
        } catch (const std::exception& e) {
            result["error"] = std::string("删除日记异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 日记搜索
     *
     * @param keyword 搜索关键词
     * @param mode 搜索模式:
     *   - "exact": HashMap 精确查找（按标题匹配）
     *   - "fulltext": 倒排索引全文检索（TF-IDF）
     * @param limit 最大返回数量
     * @return json 搜索结果
     */
    static json search_diaries(const std::string& keyword,
                               const std::string& mode = "fulltext",
                               int limit = 20) {
        json result;
        try {
            if (keyword.empty()) {
                result["error"] = "搜索关键词不能为空";
                return result;
            }

            if (mode == "exact") {
                // HashMap 精确查找
                auto& title_map = get_title_map();
                const int* diary_id = title_map.find(keyword);

                json items = json::array();
                if (diary_id) {
                    json diary = repository::DiaryRepo::get_by_id(*diary_id);
                    if (diary.contains("id")) {
                        items.push_back(diary);
                    }
                }

                result["data"] = items;
                result["total"] = static_cast<int>(items.size());
                result["mode"] = "exact";

            } else {
                // 倒排索引全文检索
                auto& index = get_index();
                int* doc_ids = new int[limit];
                double* scores = new double[limit];

                int match_count = index.search(keyword, doc_ids, scores, limit);

                json items = json::array();
                for (int i = 0; i < match_count; i++) {
                    json diary = repository::DiaryRepo::get_by_id(doc_ids[i]);
                    if (diary.contains("id")) {
                        diary["relevance_score"] = scores[i];
                        items.push_back(diary);
                    }
                }

                result["data"] = items;
                result["total"] = match_count;
                result["mode"] = "fulltext";

                delete[] doc_ids;
                delete[] scores;
            }
        } catch (const std::exception& e) {
            result["error"] = std::string("日记搜索异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief Huffman 压缩日记内容
     *
     * @param body JSON: {"diary_id": int} 或 {"content": string}
     * @return json 压缩结果 {success, original_size, compressed_size, ratio, base64}
     */
    static json compress_diary(const json& body) {
        json result;
        try {
            std::string content;

            // 支持两种输入方式
            if (body.contains("diary_id") && body["diary_id"].is_number()) {
                int diary_id = body["diary_id"].get<int>();
                if (diary_id <= 0) {
                    result["error"] = "无效的日记ID";
                    return result;
                }
                json diary = repository::DiaryRepo::get_by_id(diary_id);
                if (!diary.contains("id")) {
                    result["error"] = "日记不存在";
                    return result;
                }
                content = diary.value("content", "");
            } else if (body.contains("content") && body["content"].is_string()) {
                content = body["content"].get<std::string>();
            } else {
                result["error"] = "参数错误: 需要提供 diary_id 或 content";
                return result;
            }

            if (content.empty()) {
                result["error"] = "日记内容为空，无需压缩";
                return result;
            }

            int original_size = static_cast<int>(content.size());
            const unsigned char* data =
                reinterpret_cast<const unsigned char*>(content.c_str());

            // 调用 Huffman 压缩
            unsigned char* compressed = nullptr;
            int compressed_size = algorithm::Huffman::compress(data, original_size, &compressed);

            if (compressed_size <= 0 || !compressed) {
                result["error"] = "Huffman 压缩失败";
                return result;
            }

            // 转为 base64 存储
            std::string base64 = bytes_to_base64(compressed, compressed_size);

            // 计算压缩率
            double ratio = algorithm::Huffman::compression_ratio(original_size, compressed_size);

            // 存入数据库（如果指定了 diary_id）
            if (body.contains("diary_id") && body["diary_id"].is_number()) {
                int diary_id = body["diary_id"].get<int>();
                repository::DiaryRepo::save_compressed(diary_id, base64);
            }

            result["success"] = true;
            result["original_size"] = original_size;
            result["compressed_size"] = compressed_size;
            result["compression_ratio"] = ratio;
            result["space_saved_percent"] = (1.0 - ratio) * 100.0;
            result["base64_length"] = static_cast<int>(base64.size());

            delete[] compressed;
        } catch (const std::exception& e) {
            result["error"] = std::string("日记压缩异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 评价日记
     * @param user_id 用户ID
     * @param diary_id 日记ID
     * @param score 评分 (1-5)
     */
    static json rate_diary(int user_id, int diary_id, int score) {
        json result;
        try {
            if (user_id <= 0 || diary_id <= 0) {
                result["error"] = "参数错误: user_id 和 diary_id 必须为正整数";
                return result;
            }
            if (score < 1 || score > 5) {
                result["error"] = "评分必须在 1-5 之间";
                return result;
            }

            bool success = repository::DiaryRepo::rate(user_id, diary_id, score);
            if (success) {
                // 增加热度
                repository::DiaryRepo::increment_popularity(diary_id);
                result["success"] = true;
                result["message"] = "评分成功";
            } else {
                result["error"] = "评分失败";
            }
        } catch (const std::exception& e) {
            result["error"] = std::string("评分异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 强制刷新搜索索引
     * 在数据变更后可手动调用
     */
    static json refresh_search_index() {
        json result;
        try {
            refresh_index();
            result["success"] = true;
            result["message"] = "搜索索引已刷新";
        } catch (const std::exception& e) {
            result["error"] = std::string("刷新索引异常: ") + e.what();
        }
        return result;
    }
};

} // namespace service
