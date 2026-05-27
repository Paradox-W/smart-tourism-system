#ifndef SMART_TOURISM_RESPONSE_H
#define SMART_TOURISM_RESPONSE_H

#include <string>
#include "json.hpp"

using json = nlohmann::json;

/**
 * 统一 JSON 响应格式封装
 *
 * 所有 API 返回统一的 JSON 结构:
 * {
 *     "code": 200,        // 状态码
 *     "message": "ok",    // 消息描述
 *     "data": { ... }     // 业务数据
 * }
 */
namespace Response {

/** 构建成功响应 */
inline json ok(const json& data = nullptr, const std::string& msg = "success") {
    json resp;
    resp["code"] = 200;
    resp["message"] = msg;
    resp["data"] = data.is_null() ? json::object() : data;
    return resp;
}

/** 构建错误响应 */
inline json error(int code, const std::string& msg) {
    json resp;
    resp["code"] = code;
    resp["message"] = msg;
    resp["data"] = nullptr;
    return resp;
}

/** 400 Bad Request */
inline json bad_request(const std::string& msg = "Invalid request") {
    return error(400, msg);
}

/** 401 Unauthorized */
inline json unauthorized(const std::string& msg = "Unauthorized") {
    return error(401, msg);
}

/** 404 Not Found */
inline json not_found(const std::string& msg = "Resource not found") {
    return error(404, msg);
}

/** 500 Internal Server Error */
inline json server_error(const std::string& msg = "Internal server error") {
    return error(500, msg);
}

/**
 * 构建分页响应
 * @param items 当前页数据数组
 * @param total 总记录数
 * @param page 当前页码
 * @param page_size 每页数量
 */
inline json paginated(const json& items, int total, int page, int page_size) {
    json data;
    data["items"] = items;
    data["total"] = total;
    data["page"] = page;
    data["page_size"] = page_size;
    data["total_pages"] = (total + page_size - 1) / page_size;
    return ok(data);
}

} // namespace Response

#endif // SMART_TOURISM_RESPONSE_H
