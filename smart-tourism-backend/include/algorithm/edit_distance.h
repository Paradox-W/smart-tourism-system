/**
 * @file edit_distance.h
 * @brief 编辑距离（Levenshtein Distance）—— 用于美食/景点名称的模糊查询
 *
 * 核心算法：动态规划
 *
 * 编辑距离定义：
 *   将字符串 A 转换为字符串 B 所需的最少单字符编辑操作次数。
 *   三种操作：
 *     - 插入（Insert）：在 A 中插入一个字符
 *     - 删除（Delete）：从 A 中删除一个字符
 *     - 替换（Replace）：将 A 中的一个字符替换为另一个
 *
 * 递推关系：
 *   dp[i][j] = min(
 *     dp[i-1][j] + 1,           // 删除 A[i-1]
 *     dp[i][j-1] + 1,           // 插入 B[j-1]
 *     dp[i-1][j-1] + cost       // 替换（A[i-1]==B[j-1]时cost=0，否则cost=1）
 *   )
 *
 * 时间复杂度: O(m * n)，m和n为两个字符串的长度
 * 空间复杂度: O(m * n)，可优化到 O(min(m,n))
 *
 * 扩展：Damerau-Levenshtein（允许相邻字符交换）可在此基础实现
 */

#ifndef EDIT_DISTANCE_H
#define EDIT_DISTANCE_H

#include <cstring>
#include <string>
#include <algorithm>

namespace algorithm {

// ============================================================
// 编辑距离计算
// ============================================================
class EditDistance {
public:
    /**
     * @brief 计算两个字符串的编辑距离（Levenshtein Distance）
     *
     * @param s1 源字符串
     * @param s2 目标字符串
     * @return 编辑距离（非负整数）
     */
    static int compute(const std::string& s1, const std::string& s2) {
        int m = static_cast<int>(s1.size());
        int n = static_cast<int>(s2.size());

        // dp[i][j] = s1[0..i-1] 到 s2[0..j-1] 的编辑距离
        // 使用一维数组优化空间到 O(n)
        int* prev = new int[n + 1];
        int* curr = new int[n + 1];

        // 初始化：空串到 s2[0..j-1] 需要j次插入
        for (int j = 0; j <= n; j++) {
            prev[j] = j;
        }

        for (int i = 1; i <= m; i++) {
            curr[0] = i;  // s1[0..i-1] 到空串需要i次删除

            for (int j = 1; j <= n; j++) {
                int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

                curr[j] = std::min({
                    prev[j] + 1,              // 删除 s1[i-1]
                    curr[j - 1] + 1,          // 插入 s2[j-1]
                    prev[j - 1] + cost        // 替换（或不操作）
                });
            }

            // 交换 prev 和 curr
            int* tmp = prev;
            prev = curr;
            curr = tmp;
        }

        int result = prev[n];
        delete[] prev;
        delete[] curr;
        return result;
    }

    /**
     * @brief 计算编辑距离（完整DP表，用于回溯编辑操作序列）
     *
     * @param s1 源字符串
     * @param s2 目标字符串
     * @param ops 输出操作序列（可选，传nullptr则不记录）
     *        每个操作为 char: 'I'=插入, 'D'=删除, 'R'=替换, '='=不变
     * @return 编辑距离
     */
    static int compute_with_trace(const std::string& s1, const std::string& s2, char* ops) {
        int m = static_cast<int>(s1.size());
        int n = static_cast<int>(s2.size());

        // 使用完整二维DP表
        int** dp = new int*[m + 1];
        for (int i = 0; i <= m; i++) {
            dp[i] = new int[n + 1];
        }

        for (int i = 0; i <= m; i++) dp[i][0] = i;
        for (int j = 0; j <= n; j++) dp[0][j] = j;

        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= n; j++) {
                int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
                dp[i][j] = std::min({
                    dp[i - 1][j] + 1,
                    dp[i][j - 1] + 1,
                    dp[i - 1][j - 1] + cost
                });
            }
        }

        // 回溯操作序列
        if (ops) {
            int i = m, j = n;
            int op_idx = 0;
            while (i > 0 || j > 0) {
                if (i > 0 && j > 0 && s1[i - 1] == s2[j - 1] && dp[i][j] == dp[i - 1][j - 1]) {
                    ops[op_idx++] = '=';
                    i--; j--;
                } else if (j > 0 && dp[i][j] == dp[i][j - 1] + 1) {
                    ops[op_idx++] = 'I';
                    j--;
                } else if (i > 0 && dp[i][j] == dp[i - 1][j] + 1) {
                    ops[op_idx++] = 'D';
                    i--;
                } else {
                    ops[op_idx++] = 'R';
                    i--; j--;
                }
            }
            // 反转操作序列（因为是从后往前回溯的）
            for (int a = 0; a < op_idx / 2; a++) {
                char tmp = ops[a];
                ops[a] = ops[op_idx - 1 - a];
                ops[op_idx - 1 - a] = tmp;
            }
            ops[op_idx] = '\0';
        }

        int result = dp[m][n];

        for (int i = 0; i <= m; i++) delete[] dp[i];
        delete[] dp;

        return result;
    }

    /**
     * @brief 计算相似度（0.0 ~ 1.0）
     *
     * 相似度 = 1 - edit_distance / max(len(s1), len(s2))
     *
     * @param s1 字符串1
     * @param s2 字符串2
     * @return 相似度 [0.0, 1.0]，1.0表示完全相同
     */
    static double similarity(const std::string& s1, const std::string& s2) {
        int m = static_cast<int>(s1.size());
        int n = static_cast<int>(s2.size());
        int max_len = (m > n) ? m : n;
        if (max_len == 0) return 1.0;

        int dist = compute(s1, s2);
        return 1.0 - static_cast<double>(dist) / max_len;
    }

    /**
     * @brief 模糊匹配：在候选列表中找出与目标最相似的项
     *
     * @param target 目标字符串
     * @param candidates 候选字符串数组
     * @param candidate_count 候选数量
     * @param similarities 输出每个候选的相似度
     * @param threshold 最小相似度阈值（低于此值不考虑）
     * @return 最佳匹配的索引（-1表示没有满足阈值的匹配）
     */
    static int fuzzy_match(const std::string& target,
                           const std::string* candidates, int candidate_count,
                           double* similarities = nullptr,
                           double threshold = 0.5) {
        int best_idx = -1;
        double best_sim = threshold;

        for (int i = 0; i < candidate_count; i++) {
            double sim = similarity(target, candidates[i]);
            if (similarities) similarities[i] = sim;
            if (sim > best_sim) {
                best_sim = sim;
                best_idx = i;
            }
        }

        return best_idx;
    }

    /**
     * @brief 模糊匹配 Top-K：找出最相似的K个候选
     *
     * @param target 目标字符串
     * @param candidates 候选字符串数组
     * @param candidate_count 候选数量
     * @param result_indices 输出索引数组
     * @param result_sims 输出相似度数组
     * @param k 返回前K个
     * @param threshold 最小相似度阈值
     * @return 实际匹配数
     */
    static int fuzzy_match_top_k(const std::string& target,
                                 const std::string* candidates, int candidate_count,
                                 int* result_indices, double* result_sims,
                                 int k, double threshold = 0.3) {
        // 先计算所有相似度
        double* sims = new double[candidate_count];
        for (int i = 0; i < candidate_count; i++) {
            sims[i] = similarity(target, candidates[i]);
        }

        // 选择排序选出前K个
        int count = 0;
        for (int i = 0; i < candidate_count && count < k; i++) {
            // 找到从i开始最大的
            int best = i;
            for (int j = i + 1; j < candidate_count; j++) {
                if (sims[j] > sims[best]) best = j;
            }
            if (sims[best] >= threshold) {
                result_indices[count] = best;
                result_sims[count] = sims[best];
                count++;
            }
            // 交换到已排序位置
            double tmp = sims[i]; sims[i] = sims[best]; sims[best] = tmp;
        }

        delete[] sims;
        return count;
    }

    /**
     * @brief 不区分大小写的编辑距离
     */
    static int compute_icase(const std::string& s1, const std::string& s2) {
        std::string lower1 = to_lower(s1);
        std::string lower2 = to_lower(s2);
        return compute(lower1, lower2);
    }

    /**
     * @brief 不区分大小写的相似度
     */
    static double similarity_icase(const std::string& s1, const std::string& s2) {
        std::string lower1 = to_lower(s1);
        std::string lower2 = to_lower(s2);
        return similarity(lower1, lower2);
    }

private:
    static std::string to_lower(const std::string& s) {
        std::string result = s;
        for (size_t i = 0; i < result.size(); i++) {
            result[i] = static_cast<char>(tolower(static_cast<unsigned char>(result[i])));
        }
        return result;
    }
};

} // namespace algorithm

#endif // EDIT_DISTANCE_H
