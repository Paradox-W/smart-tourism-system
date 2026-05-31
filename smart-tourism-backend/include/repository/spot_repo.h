#pragma once
#include "json.hpp"
#include "repository/db_connection.h"
#include <string>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace repository {

/**
 * 景区/校园数据访问层
 * 提供景点查询、搜索、节点/道路获取、用户兴趣获取等功能
 */
class SpotRepo {
public:
    /**
     * 获取所有景点/校园（带分页和排序）
     * @param page 页码（从1开始）
     * @param page_size 每页数量
     * @param sort_by 排序字段: "popularity" | "rating" | "created_at"
     * @param order 排序方向: "desc" | "asc"
     * @param type 类型过滤: ""=全部, "scenic", "campus"
     * @param category 分类过滤: ""=全部
     * @return json数组，每个元素为景点对象
     */
    static json get_all(int page = 1, int page_size = 20,
                        const std::string& sort_by = "popularity",
                        const std::string& order = "desc",
                        const std::string& type = "",
                        const std::string& category = "") {
        json result = json::array();
        auto& db = Database::get();

        // 验证排序字段合法性（防止SQL注入）
        std::string col = (sort_by == "rating" || sort_by == "created_at") ? sort_by : "popularity";
        std::string dir = (order == "asc") ? "ASC" : "DESC";

        std::ostringstream sql;
        sql << "SELECT id, name, type, category, description, popularity, rating, rating_count, "
            << "image, city, address, open_time, ticket_price, canvas_width, canvas_height, "
            << "created_at, updated_at FROM scenic_spots";

        // WHERE条件
        bool has_where = false;
        if (!type.empty()) {
            sql << " WHERE type = '" << escape_sql(type) << "'";
            has_where = true;
        }
        if (!category.empty()) {
            sql << (has_where ? " AND" : " WHERE") << " category = '" << escape_sql(category) << "'";
            has_where = true;
        }

        sql << " ORDER BY " << col << " " << dir;
        int offset = (page - 1) * page_size;
        sql << " LIMIT " << page_size << " OFFSET " << offset;

        db.query(sql.str(), [&result](int cols, char** values, char**) {
            json item;
            item["id"]            = values[0] ? std::stoi(values[0]) : 0;
            item["name"]          = values[1] ? values[1] : "";
            item["type"]          = values[2] ? values[2] : "";
            item["category"]      = values[3] ? values[3] : "";
            item["description"]   = values[4] ? values[4] : "";
            item["popularity"]    = values[5] ? std::stoi(values[5]) : 0;
            item["rating"]        = values[6] ? std::stod(values[6]) : 0.0;
            item["rating_count"]  = values[7] ? std::stoi(values[7]) : 0;
            item["image"]         = values[8] ? values[8] : "";
            item["city"]          = values[9] ? values[9] : "";
            item["address"]       = values[10] ? values[10] : "";
            item["open_time"]     = values[11] ? values[11] : "";
            item["ticket_price"]  = values[12] ? values[12] : "";
            item["canvas_width"]  = values[13] ? std::stoi(values[13]) : 0;
            item["canvas_height"] = values[14] ? std::stoi(values[14]) : 0;
            item["created_at"]    = values[15] ? values[15] : "";
            item["updated_at"]    = values[16] ? values[16] : "";
            result.push_back(item);
            return true;
        });

        return result;
    }

    /**
     * 按ID获取单个景点
     * @param id 景点ID
     * @return 景点对象，未找到时为null
     */
    static json get_by_id(int id) {
        json result;
        auto& db = Database::get();

        std::ostringstream sql;
        sql << "SELECT id, name, type, category, description, popularity, rating, rating_count, "
            << "image, city, address, open_time, ticket_price, canvas_width, canvas_height, "
            << "created_at, updated_at FROM scenic_spots WHERE id = " << id << " LIMIT 1";

        bool found = false;
        db.query(sql.str(), [&result, &found](int cols, char** values, char**) {
            found = true;
            result["id"]            = values[0] ? std::stoi(values[0]) : 0;
            result["name"]          = values[1] ? values[1] : "";
            result["type"]          = values[2] ? values[2] : "";
            result["category"]      = values[3] ? values[3] : "";
            result["description"]   = values[4] ? values[4] : "";
            result["popularity"]    = values[5] ? std::stoi(values[5]) : 0;
            result["rating"]        = values[6] ? std::stod(values[6]) : 0.0;
            result["rating_count"]  = values[7] ? std::stoi(values[7]) : 0;
            result["image"]         = values[8] ? values[8] : "";
            result["city"]          = values[9] ? values[9] : "";
            result["address"]       = values[10] ? values[10] : "";
            result["open_time"]     = values[11] ? values[11] : "";
            result["ticket_price"]  = values[12] ? values[12] : "";
            result["canvas_width"]  = values[13] ? std::stoi(values[13]) : 0;
            result["canvas_height"] = values[14] ? std::stoi(values[14]) : 0;
            result["created_at"]    = values[15] ? values[15] : "";
            result["updated_at"]    = values[16] ? values[16] : "";
            return false; // 只取第一行
        });

        return found ? result : json();
    }

    /**
     * 搜索景点（关键字模糊匹配名称/描述）
     * @param keyword 搜索关键字
     * @param limit 最大返回数量
     * @return 匹配的景点数组
     */
    static json search(const std::string& keyword, int limit = 20) {
        json result = json::array();
        if (keyword.empty()) return result;

        auto& db = Database::get();
        std::string escaped = escape_sql(keyword);

        std::ostringstream sql;
        sql << "SELECT id, name, type, category, description, popularity, rating, rating_count, "
            << "image, city, address, open_time, ticket_price, canvas_width, canvas_height, "
            << "created_at, updated_at FROM scenic_spots "
            << "WHERE name LIKE '%" << escaped << "%' "
            << "OR description LIKE '%" << escaped << "%' "
            << "OR city LIKE '%" << escaped << "%' "
            << "ORDER BY popularity DESC LIMIT " << limit;

        db.query(sql.str(), [&result](int cols, char** values, char**) {
            json item;
            item["id"]            = values[0] ? std::stoi(values[0]) : 0;
            item["name"]          = values[1] ? values[1] : "";
            item["type"]          = values[2] ? values[2] : "";
            item["category"]      = values[3] ? values[3] : "";
            item["description"]   = values[4] ? values[4] : "";
            item["popularity"]    = values[5] ? std::stoi(values[5]) : 0;
            item["rating"]        = values[6] ? std::stod(values[6]) : 0.0;
            item["rating_count"]  = values[7] ? std::stoi(values[7]) : 0;
            item["image"]         = values[8] ? values[8] : "";
            item["city"]          = values[9] ? values[9] : "";
            item["address"]       = values[10] ? values[10] : "";
            item["open_time"]     = values[11] ? values[11] : "";
            item["ticket_price"]  = values[12] ? values[12] : "";
            item["canvas_width"]  = values[13] ? std::stoi(values[13]) : 0;
            item["canvas_height"] = values[14] ? std::stoi(values[14]) : 0;
            item["created_at"]    = values[15] ? values[15] : "";
            item["updated_at"]    = values[16] ? values[16] : "";
            result.push_back(item);
            return true;
        });

        return result;
    }

    /**
     * 获取景点总数
     * @param type 类型过滤: ""=全部
     * @param category 分类过滤: ""=全部
     * @return 符合条件的景点总数
     */
    static int count(const std::string& type = "", const std::string& category = "") {
        auto& db = Database::get();
        std::ostringstream sql;
        sql << "SELECT COUNT(*) FROM scenic_spots";

        bool has_where = false;
        if (!type.empty()) {
            sql << " WHERE type = '" << escape_sql(type) << "'";
            has_where = true;
        }
        if (!category.empty()) {
            sql << (has_where ? " AND" : " WHERE") << " category = '" << escape_sql(category) << "'";
        }

        return db.query_int(sql.str());
    }

    /**
     * 获取某景区的节点列表（用于构建图）
     * @param area_id 景区/校园ID
     * @return 节点数组，每个元素: {id, name, type, sub_type, pos_x, pos_y, floor, description}
     */
    static json get_nodes(int area_id) {
        json result = json::array();
        auto& db = Database::get();

        std::ostringstream sql;
        sql << "SELECT id, name, type, sub_type, pos_x, pos_y, floor, description "
            << "FROM nodes WHERE area_id = " << area_id;

        db.query(sql.str(), [&result](int cols, char** values, char**) {
            json item;
            item["id"]        = values[0] ? std::stoi(values[0]) : 0;
            item["name"]      = values[1] ? values[1] : "";
            item["type"]      = values[2] ? values[2] : "";
            item["sub_type"]  = values[3] ? values[3] : "";
            item["pos_x"]     = values[4] ? std::stod(values[4]) : 0.0;
            item["pos_y"]     = values[5] ? std::stod(values[5]) : 0.0;
            item["floor"]     = values[6] ? std::stoi(values[6]) : 0;
            item["description"] = values[7] ? values[7] : "";
            result.push_back(item);
            return true;
        });

        return result;
    }

    /**
     * 获取某景区的道路列表（用于构建图）
     * @param area_id 景区/校园ID
     * @return 道路数组，每个元素: {id, from_node, to_node, distance, congestion, ideal_speed, transport, is_bidirectional}
     */
    static json get_roads(int area_id) {
        json result = json::array();
        auto& db = Database::get();

        std::ostringstream sql;
        sql << "SELECT id, from_node, to_node, distance, congestion, ideal_speed, transport, is_bidirectional "
            << "FROM roads WHERE area_id = " << area_id;

        db.query(sql.str(), [&result](int cols, char** values, char**) {
            json item;
            item["id"]               = values[0] ? std::stoi(values[0]) : 0;
            item["from_node"]        = values[1] ? std::stoi(values[1]) : 0;
            item["to_node"]          = values[2] ? std::stoi(values[2]) : 0;
            item["distance"]         = values[3] ? std::stod(values[3]) : 0.0;
            item["congestion"]       = values[4] ? std::stod(values[4]) : 0.5;
            item["ideal_speed"]      = values[5] ? std::stod(values[5]) : 0.0;
            item["transport"]        = values[6] ? std::stoi(values[6]) : 0;
            item["is_bidirectional"] = values[7] ? (std::stoi(values[7]) != 0) : true;
            result.push_back(item);
            return true;
        });

        return result;
    }

    /**
     * 获取用户兴趣
     * @param user_id 用户ID
     * @return 兴趣数组，每个元素: {category, weight}
     */
    static json get_user_interests(int user_id) {
        json result = json::array();
        auto& db = Database::get();

        std::ostringstream sql;
        sql << "SELECT category, weight FROM user_interests WHERE user_id = " << user_id;

        db.query(sql.str(), [&result](int cols, char** values, char**) {
            json item;
            item["category"] = values[0] ? values[0] : "";
            item["weight"]   = values[1] ? std::stod(values[1]) : 1.0;
            result.push_back(item);
            return true;
        });

        return result;
    }

    /**
     * 更新景点热度（+1）
     * @param spot_id 景点ID
     */
    static void increment_popularity(int spot_id) {
        auto& db = Database::get();
        std::ostringstream sql;
        sql << "UPDATE scenic_spots SET popularity = popularity + 1 WHERE id = " << spot_id;
        db.execute(sql.str());
    }

private:
    /**
     * 简单SQL字符串转义（防注入）
     * 转义单引号
     */
    static std::string escape_sql(const std::string& input) {
        std::string result;
        result.reserve(input.size() * 2);
        for (char c : input) {
            if (c == '\'') {
                result += "''";
            } else {
                result += c;
            }
        }
        return result;
    }
};

} // namespace repository
