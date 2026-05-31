/**
 * @file recommend_service.h
 * @brief 旅游推荐服务 —— Top-K堆排序推荐 + 个性化推荐
 *
 * 核心算法：
 *   - Top-K 堆排序：从所有景点中选出评分/热度最高的K个
 *   - 个性化推荐：根据用户兴趣权重加权分数
 *   - 时间复杂度 O(n * log k)，远优于全排序 O(n * log n)
 */

#pragma once

#include "json.hpp"
#include "algorithm/heap.h"
#include "repository/spot_repo.h"
#include <string>
#include <memory>

using json = nlohmann::json;

namespace service {

class RecommendService {
private:
    // 景点评分结构（用于堆排序）
    struct SpotScore {
        int id;
        double score;
        std::string name;
        double rating;
        int rating_count;
        int popularity;
        std::string type;
        std::string category;
        std::string description;
        std::string city;
        std::string address;
        std::string open_time;
        std::string ticket_price;
        std::string image;
    };

    /**
     * @brief 从景点 JSON 数组中提取 SpotScore 列表
     */
    static std::unique_ptr<SpotScore[]> extract_spots(const json& spots_json, int& count) {
        if (!spots_json.is_array() || spots_json.empty()) {
            count = 0;
            return nullptr;
        }
        count = static_cast<int>(spots_json.size());
        auto spots = std::make_unique<SpotScore[]>(count);
        for (int i = 0; i < count; i++) {
            const auto& s = spots_json[i];
            spots[i].id = s.value("id", 0);
            spots[i].name = s.value("name", "");
            spots[i].rating = s.value("rating", 0.0);
            spots[i].rating_count = s.value("rating_count", 0);
            spots[i].popularity = s.value("popularity", 0);
            spots[i].type = s.value("type", "");
            spots[i].category = s.value("category", "");
            spots[i].description = s.value("description", "");
            spots[i].city = s.value("city", "");
            spots[i].address = s.value("address", "");
            spots[i].open_time = s.value("open_time", "");
            spots[i].ticket_price = s.value("ticket_price", "");
            spots[i].image = s.value("image", "");
            spots[i].score = 0.0; // 稍后根据 sort_by 设置
        }
        return spots;
    }

    /**
     * @brief 根据 sort_by 设置景点分数
     */
    static void set_scores(SpotScore* spots, int count, const std::string& sort_by) {
        for (int i = 0; i < count; i++) {
            if (sort_by == "rating") {
                spots[i].score = spots[i].rating;
            } else if (sort_by == "popularity") {
                spots[i].score = static_cast<double>(spots[i].popularity);
            } else {
                // 默认综合分 = rating * 10 + popularity
                spots[i].score = spots[i].rating * 10.0 + static_cast<double>(spots[i].popularity);
            }
        }
    }

    /**
     * @brief 应用用户兴趣权重
     * @param interests 用户兴趣 JSON 数组 [{category, weight, type}]
     */
    static void apply_interest_weights(SpotScore* spots, int count, const json& interests) {
        if (!interests.is_array() || interests.empty()) return;

        for (int i = 0; i < count; i++) {
            for (const auto& interest : interests) {
                std::string interest_type = interest.value("type", "");
                std::string interest_category = interest.value("category", "");
                double weight = interest.value("weight", 1.0);

                // 类型匹配加分
                if (!interest_type.empty() && spots[i].type == interest_type) {
                    spots[i].score *= weight;
                }
                // 分类匹配加分
                if (!interest_category.empty() && spots[i].category == interest_category) {
                    spots[i].score *= (1.0 + (weight - 1.0) * 0.5);
                }
            }
        }
    }

public:
    /**
     * @brief 获取推荐景点（Top-K）
     *
     * @param sort_by 排序依据: "popularity" | "rating" | "interest"(个性化)
     * @param limit 返回数量
     * @param user_id 用户ID（-1表示不使用个性化）
     * @param type 景点类型过滤
     * @param category 分类过滤
     * @return json 推荐景点列表
     */
    static json get_recommendations(const std::string& sort_by = "popularity",
                                    int limit = 10, int user_id = -1,
                                    const std::string& type = "",
                                    const std::string& category = "") {
        json result;
        try {
            // 从数据库获取所有景点（不限分页，由Top-K算法取前K）
            json spots_json = repository::SpotRepo::get_all(1, 10000, sort_by, "desc", type, category);

            if (!spots_json.is_array()) {
                result["error"] = "获取景点数据失败";
                return result;
            }

            int count = 0;
            auto spots = extract_spots(spots_json, count);
            if (count == 0) {
                result["data"] = json::array();
                result["total"] = 0;
                return result;
            }

            // 设置基础分数
            std::string effective_sort = sort_by;
            set_scores(spots.get(), count, effective_sort);

            // 个性化推荐：加载用户兴趣并加权
            if (user_id > 0 && sort_by == "interest") {
                json interests = repository::SpotRepo::get_user_interests(user_id);
                apply_interest_weights(spots.get(), count, interests);
                effective_sort = "interest"; // 确保按加权分数排序
            }

            // 使用 Top-K 堆排序取前 limit 个
            int k = (limit < count) ? limit : count;
            auto top_k = std::make_unique<SpotScore[]>(k);

            auto get_score = [](const SpotScore& s) -> double { return s.score; };
            int actual_k = algorithm::top_k_by_score(spots.get(), count, k, top_k.get(), get_score, true);

            // 构建返回结果（已经是降序）
            json items = json::array();
            for (int i = 0; i < actual_k; i++) {
                json item;
                item["id"] = top_k[i].id;
                item["name"] = top_k[i].name;
                item["rating"] = top_k[i].rating;
                item["rating_count"] = top_k[i].rating_count;
                item["popularity"] = top_k[i].popularity;
                item["type"] = top_k[i].type;
                item["category"] = top_k[i].category;
                item["description"] = top_k[i].description;
                item["city"] = top_k[i].city;
                item["address"] = top_k[i].address;
                item["open_time"] = top_k[i].open_time;
                item["ticket_price"] = top_k[i].ticket_price;
                item["image"] = top_k[i].image;
                item["recommend_score"] = top_k[i].score;
                items.push_back(item);
            }

            result["data"] = items;
            result["total"] = count;
        } catch (const std::exception& e) {
            result["error"] = std::string("推荐服务异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 搜索景点
     * @param keyword 搜索关键词
     * @param limit 返回数量
     * @return json 搜索结果
     */
    static json search_spots(const std::string& keyword, int limit = 20) {
        json result;
        try {
            if (keyword.empty()) {
                result["error"] = "搜索关键词不能为空";
                return result;
            }
            result = repository::SpotRepo::search(keyword, limit);
        } catch (const std::exception& e) {
            result["error"] = std::string("搜索服务异常: ") + e.what();
        }
        return result;
    }

    /**
     * @brief 获取景点详情
     * @param spot_id 景点ID
     * @return json 景点详情
     */
    static json get_spot_detail(int spot_id) {
        json result;
        try {
            if (spot_id <= 0) {
                result["error"] = "无效的景点ID";
                return result;
            }
            result = repository::SpotRepo::get_by_id(spot_id);
        } catch (const std::exception& e) {
            result["error"] = std::string("获取景点详情异常: ") + e.what();
        }
        return result;
    }
};

} // namespace service
