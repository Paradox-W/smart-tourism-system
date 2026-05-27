/**
 * @file facility_service.h
 * @brief 场所查询服务 —— 基于实际可达路径距离的周边设施查找
 *
 * 核心算法：
 *   - Dijkstra 计算从指定节点到所有节点的最短路径距离
 *   - 按路径距离排序（非直线距离）
 *   - 预过滤：直线距离超过 radius 的设施直接排除
 */

#pragma once

#include "json.hpp"
#include "algorithm/graph.h"
#include "algorithm/dijkstra.h"
#include "algorithm/hash_table.h"
#include "repository/spot_repo.h"
#include <string>
#include <cmath>

using json = nlohmann::json;

namespace service {

class FacilityService {
public:
    /**
     * 查找附近设施
     * @param area_id 景区/校园ID
     * @param node_id 当前所在节点ID
     * @param category 设施类别过滤（sub_type），空字符串表示全部
     * @param radius 预过滤半径（米），基于直线距离
     */
    static json find_nearby(int area_id, int node_id,
                            const std::string& category = "",
                            double radius = 500.0) {
        if (area_id <= 0 || node_id <= 0) {
            return {{"error", "area_id and node_id are required"}};
        }

        // 构建图
        algorithm::Graph g;
        algorithm::IntHashMap id_to_idx(64);

        json nodes = repository::SpotRepo::get_nodes(area_id);
        for (const auto& n : nodes) {
            int db_id = n["id"].get<int>();
            std::string name = n.value("name", "");
            double pos_x = n.value("pos_x", 0.0);
            double pos_y = n.value("pos_y", 0.0);
            std::string type = n.value("type", std::string("junction"));
            int node_type = 0;
            if (type == "building" || type == "entrance") node_type = 1;
            else if (type == "facility") node_type = 2;
            int idx = g.add_node(db_id, pos_x, pos_y, area_id, name, node_type);
            id_to_idx.put(db_id, idx);
        }

        json roads = repository::SpotRepo::get_roads(area_id);
        for (const auto& r : roads) {
            int from_db = r["from_node"].get<int>();
            int to_db = r["to_node"].get<int>();
            double distance = r.value("distance", 0.0);
            double congestion = r.value("congestion", 0.5);
            int transport = r.value("transport", 0);
            int* from_idx = id_to_idx.get(from_db);
            int* to_idx = id_to_idx.get(to_db);
            if (from_idx && to_idx) {
                g.add_edge(*from_idx, *to_idx, distance, congestion, transport);
            }
        }

        // 获取起始节点坐标
        int* start_idx_ptr = id_to_idx.get(node_id);
        if (!start_idx_ptr) {
            return {{"error", "Start node not found in area"}, {"node_id", node_id}};
        }
        int start_idx = *start_idx_ptr;
        const auto& start_node = g.get_node(start_idx);

        // 运行 Dijkstra
        algorithm::DijkstraResult dijk = algorithm::Dijkstra::run(g, start_idx);

        // 收集设施节点
        struct FacilityResult {
            int id;
            std::string name;
            std::string sub_type;
            double path_distance;
            double straight_distance;
        };

        // 简单数组收集
        FacilityResult results[256];
        int result_count = 0;

        for (int i = 0; i < g.node_count() && result_count < 256; i++) {
            const auto& node = g.get_node(i);
            if (node.type != 2) continue;  // 只取 facility 类型

            // 类别过滤
            json node_json;
            for (const auto& n : nodes) {
                if (n["id"].get<int>() == node.id) {
                    node_json = n;
                    break;
                }
            }
            std::string sub_type = node_json.value("sub_type", std::string(""));
            if (!category.empty() && sub_type != category) continue;

            // 路径距离
            if (dijk.dist[i] < 0) continue;  // 不可达

            // 直线距离预过滤
            double dx = node.x - start_node.x;
            double dy = node.y - start_node.y;
            double straight_dist = std::sqrt(dx * dx + dy * dy);
            if (straight_dist > radius) continue;

            results[result_count].id = node.id;
            results[result_count].name = node.name;
            results[result_count].sub_type = sub_type;
            results[result_count].path_distance = dijk.dist[i];
            results[result_count].straight_distance = straight_dist;
            result_count++;
        }

        // 按路径距离排序（选择排序）
        for (int i = 0; i < result_count - 1; i++) {
            int min_idx = i;
            for (int j = i + 1; j < result_count; j++) {
                if (results[j].path_distance < results[min_idx].path_distance) {
                    min_idx = j;
                }
            }
            if (min_idx != i) {
                FacilityResult tmp = results[i];
                results[i] = results[min_idx];
                results[min_idx] = tmp;
            }
        }

        // 构建响应
        json items = json::array();
        for (int i = 0; i < result_count; i++) {
            json item;
            item["id"] = results[i].id;
            item["node_id"] = results[i].id;
            item["name"] = results[i].name;
            item["type"] = "facility";
            item["sub_type"] = results[i].sub_type;
            item["path_distance"] = results[i].path_distance;
            item["distance"] = results[i].path_distance;
            item["straight_distance"] = results[i].straight_distance;
            item["direct_distance"] = results[i].straight_distance;
            item["description"] = "";
            items.push_back(item);
        }

        json response;
        response["items"] = items;
        response["total"] = result_count;
        response["node_id"] = node_id;
        response["area_id"] = area_id;
        if (!category.empty()) response["category"] = category;
        return response;
    }
};

} // namespace service
