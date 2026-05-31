/**
 * @file food_service.h
 * @brief 美食推荐服务 —— Top-K堆排序推荐 + 编辑距离模糊搜索
 *
 * 核心算法：
 *   - Top-K 堆排序：从所有美食中选出评分最高的K个
 *   - 编辑距离（Levenshtein Distance）：模糊匹配美食名称
 *   - 相似度计算：1 - edit_distance / max(len1, len2)
 *
 * 使用场景：
 *   - "推荐附近美食" -> Top-K 排序推荐
 *   - "我想吃火锅" -> 模糊匹配 "火锅" 相关美食
 *   - "找一家川菜馆" -> 按菜系过滤 + 模糊搜索
 */

#pragma once

#include "json.hpp"
#include "algorithm/heap.h"
#include "algorithm/edit_distance.h"
#include "repository/food_repo.h"
#include <string>
#include <memory>

using json = nlohmann::json;

namespace service {

class FoodService {
private:
    // 美食评分结构（用于堆排序）
    struct FoodScore {
        int id;
        int area_id;
        double score;
        std::string name;
        double rating;
        int rating_count;
        int popularity;
        double price;
        std::string cuisine;
        std::string restaurant;
        int node_id;
        std::string description;
        std::string image;
    };

public:
    /**
     * @brief 获取美食推荐（Top-K）
     *
     * @param area_id 区域ID
     * @param limit 返回数量
     * @param sort_by 排序依据: "rating" | "price_asc" | "price_desc" | "popularity"
     * @param cuisine 菜系过滤（空字符串表示不过滤）
     * @return json 推荐美食列表
     */
    static json get_recommendations(int area_id, int limit = 10,
                                     const std::string& sort_by = "rating",
                                     const std::string& cuisine = "") {
        json result;
        try {
            if (area_id <= 0) {
                result["error"] = "无效的区域ID";
                return result;
            }

            // 从数据库获取该区域所有美食
            json foods_json = repository::FoodRepo::get_all(area_id, 1, 10000, sort_by, cuisine);

            if (!foods_json.is_array() || foods_json.empty()) {
                result["data"] = json::array();
                result["total"] = 0;
                return result;
            }

            int count = static_cast<int>(foods_json.size());

            // 构建 FoodScore 数组
            auto foods = std::make_unique<FoodScore[]>(count);
            for (int i = 0; i < count; i++) {
                const auto& f = foods_json[i];
                foods[i].id = f.value("id", 0);
                foods[i].area_id = f.value("area_id", area_id);
                foods[i].name = f.value("name", "");
                foods[i].rating = f.value("rating", 0.0);
                foods[i].rating_count = f.value("rating_count", 0);
                foods[i].popularity = f.value("popularity", 0);
                foods[i].price = f.value("price", 0.0);
                foods[i].cuisine = f.value("cuisine", "");
                foods[i].restaurant = f.value("restaurant", "");
                foods[i].node_id = f.value("node_id", 0);
                foods[i].description = f.value("description", "");
                foods[i].image = f.value("image", "");

                // 根据 sort_by 设置分数
                if (sort_by == "price_asc") {
                    foods[i].score = -foods[i].price; // 取负值，升序
                } else if (sort_by == "price_desc") {
                    foods[i].score = foods[i].price;
                } else if (sort_by == "popularity") {
                    foods[i].score = f.value("popularity", 0.0);
                } else {
                    // 默认按评分排序
                    foods[i].score = foods[i].rating;
                }
            }

            // 使用 Top-K 堆排序取前 limit 个
            int k = (limit < count) ? limit : count;
            auto top_k = std::make_unique<FoodScore[]>(k);

            auto get_score = [](const FoodScore& f) -> double { return f.score; };

            bool descending = (sort_by != "price_asc"); // price_asc 使用升序
            int actual_k = algorithm::top_k_by_score(foods.get(), count, k, top_k.get(), get_score, descending);

            // 构建返回结果
            json items = json::array();
            for (int i = 0; i < actual_k; i++) {
                json item;
                item["id"] = top_k[i].id;
                item["area_id"] = top_k[i].area_id;
                item["name"] = top_k[i].name;
                item["rating"] = top_k[i].rating;
                item["rating_count"] = top_k[i].rating_count;
                item["popularity"] = top_k[i].popularity;
                item["price"] = top_k[i].price;
                item["cuisine"] = top_k[i].cuisine;
                item["restaurant"] = top_k[i].restaurant;
                item["node_id"] = top_k[i].node_id;
                item["description"] = top_k[i].description;
                item["image"] = top_k[i].image;

                // 获取位置信息（如果有）
                json pos = repository::FoodRepo::get_node_position(top_k[i].node_id);
                if (pos.contains("pos_x")) {
                    item["pos_x"] = pos["pos_x"];
                    item["pos_y"] = pos["pos_y"];
                }

                items.push_back(item);
            }

            // 获取总数
            int total = repository::FoodRepo::count(area_id, cuisine);

            result["data"] = items;
            result["total"] = total;
            result["limit"] = actual_k;
        } catch (const std::exception& e) {
            result["error"] = std::string("美食推荐服务异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 模糊搜索美食
     *
     * 使用编辑距离算法进行模糊匹配，支持拼写错误和近似匹配。
     * 例如搜索"火锅"可以匹配到"重庆老火锅"、"火锅店"等。
     *
     * @param area_id 区域ID
     * @param keyword 搜索关键词
     * @param limit 返回数量
     * @return json 搜索结果（包含相似度分数）
     */
    static json search_foods(int area_id, const std::string& keyword, int limit = 20) {
        json result;
        try {
            if (area_id <= 0) {
                result["error"] = "无效的区域ID";
                return result;
            }

            if (keyword.empty()) {
                result["error"] = "搜索关键词不能为空";
                return result;
            }

            // 优先使用数据库的模糊搜索
            json db_results = repository::FoodRepo::search(area_id, keyword, limit);

            // 如果数据库有结果，直接返回
            if (db_results.is_array() && !db_results.empty()) {
                result["data"] = db_results;
                result["total"] = static_cast<int>(db_results.size());
                result["mode"] = "database";
                return result;
            }

            // 数据库无结果时，使用编辑距离模糊匹配
            json names_json = repository::FoodRepo::get_all_names(area_id);
            if (!names_json.is_array() || names_json.empty()) {
                result["data"] = json::array();
                result["total"] = 0;
                return result;
            }

            int name_count = static_cast<int>(names_json.size());
            auto names = std::make_unique<std::string[]>(name_count);
            for (int i = 0; i < name_count; i++) {
                names[i] = names_json[i].value("name", "");
            }

            // 使用编辑距离 Top-K 模糊匹配
            int k = (limit < name_count) ? limit : name_count;
            auto result_indices = std::make_unique<int[]>(k);
            auto result_sims = std::make_unique<double[]>(k);

            int match_count = algorithm::EditDistance::fuzzy_match_top_k(
                keyword, names.get(), name_count,
                result_indices.get(), result_sims.get(),
                k, 0.3  // 相似度阈值 0.3
            );

            // 构建返回结果
            json items = json::array();
            for (int i = 0; i < match_count; i++) {
                int idx = result_indices[i];
                json item = names_json[idx];
                item["similarity"] = result_sims[i];
                items.push_back(item);
            }

            result["data"] = items;
            result["total"] = match_count;
            result["mode"] = "fuzzy";
        } catch (const std::exception& e) {
            result["error"] = std::string("美食搜索服务异常: ") + e.what();
        }
        return result;
    }
};

} // namespace service
