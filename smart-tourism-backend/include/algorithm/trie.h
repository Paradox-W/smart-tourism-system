/**
 * @file trie.h
 * @brief Trie 树（前缀树）—— 用于日记全文检索和自动补全
 *
 * 核心数据结构：多叉树，每个节点代表一个字符
 *
 * 应用场景：
 *   1. 全文检索：将日记文本分词后插入Trie，支持前缀匹配搜索
 *   2. 自动补全：输入前缀时快速查找所有匹配的词
 *   3. 词频统计：节点记录出现次数
 *
 * 时间复杂度：
 *   - 插入: O(m)，m为字符串长度
 *   - 查找: O(m)
 *   - 前缀搜索: O(m + k)，k为匹配结果数
 *
 * 空间复杂度: O(ALPHABET_SIZE * 总字符数)
 * 优化：本实现使用动态子节点（非固定数组），节省空间
 */

#ifndef TRIE_H
#define TRIE_H

#include <cstring>
#include <string>
#include "hash_table.h"

namespace algorithm {

// ============================================================
// Trie 节点
// ============================================================
struct TrieNode {
    char ch;                    // 当前字符
    bool is_end;                // 是否为词尾
    int count;                  // 词频（该词出现次数）
    HashMap<char, TrieNode*> children;  // 子节点映射
    int doc_count;              // 包含该前缀的文档数（用于倒排索引）

    TrieNode(char c = '\0') : ch(c), is_end(false), count(0), doc_count(0), children(8) {}

    ~TrieNode() {
        // 递归删除子节点
        char keys[256];
        int n = children.get_all_keys(keys);
        for (int i = 0; i < n; i++) {
            TrieNode* child = children.find(keys[i]) ? *children.find(keys[i]) : nullptr;
            if (child) delete child;
        }
    }
};

// ============================================================
// Trie 树
// ============================================================
class Trie {
private:
    TrieNode* root_;

    /// 递归搜索所有以 node 为根、以 prefix 开头的词
    /// results: 输出结果数组
    /// max_results: 最大结果数
    /// @return 实际结果数
    int collect_all(TrieNode* node, std::string prefix,
                    std::string* results, int max_results) const {
        if (!node || max_results <= 0) return 0;

        int count = 0;

        if (node->is_end && count < max_results) {
            results[count++] = prefix;
        }

        // 遍历子节点
        char keys[256];
        int n = node->children.get_all_keys(keys);
        // 简单排序以保证输出有序（按字母序）
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                if (keys[i] > keys[j]) {
                    char tmp = keys[i]; keys[i] = keys[j]; keys[j] = tmp;
                }
            }
        }

        for (int i = 0; i < n && count < max_results; i++) {
            TrieNode* const* child_ptr = node->children.find(keys[i]);
            if (child_ptr) {
                count += collect_all(*child_ptr, prefix + keys[i],
                                     results + count, max_results - count);
            }
        }

        return count;
    }

    /// 递归搜索所有以 node 为根、词频 >= min_freq 的词
    int collect_frequent(TrieNode* node, std::string prefix,
                         std::string* results, int* freqs, int max_results) const {
        if (!node || max_results <= 0) return 0;

        int count = 0;

        if (node->is_end && node->count > 0 && count < max_results) {
            results[count] = prefix;
            freqs[count] = node->count;
            count++;
        }

        char keys[256];
        int n = node->children.get_all_keys(keys);
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                if (keys[i] > keys[j]) {
                    char tmp = keys[i]; keys[i] = keys[j]; keys[j] = tmp;
                }
            }
        }

        for (int i = 0; i < n && count < max_results; i++) {
            TrieNode* const* child_ptr = node->children.find(keys[i]);
            if (child_ptr) {
                count += collect_frequent(*child_ptr, prefix + keys[i],
                                          results + count, freqs + count, max_results - count);
            }
        }

        return count;
    }

public:
    Trie() : root_(new TrieNode()) {}
    ~Trie() { delete root_; }

    /// 插入一个词
    void insert(const std::string& word) {
        if (word.empty()) return;

        // 转小写（统一大小写）
        TrieNode* cur = root_;
        for (size_t i = 0; i < word.size(); i++) {
            char c = static_cast<char>(tolower(static_cast<unsigned char>(word[i])));
            if (c < 'a' || c > 'z') continue;  // 跳过非字母字符

            TrieNode* const* child = cur->children.find(c);
            if (!child) {
                TrieNode* new_node = new TrieNode(c);
                cur->children.insert(c, new_node);
                child = cur->children.find(c);
            }
            cur = *child;
        }
        cur->is_end = true;
        cur->count++;
    }

    /// 插入词并标记所属文档（用于倒排索引）
    void insert_with_doc(const std::string& word, int doc_id) {
        (void)doc_id;  // 文档ID记录在倒排索引中，Trie只做前缀匹配
        insert(word);
    }

    /// 精确查找一个词是否存在
    bool search(const std::string& word) const {
        TrieNode* cur = root_;
        for (size_t i = 0; i < word.size(); i++) {
            char c = static_cast<char>(tolower(static_cast<unsigned char>(word[i])));
            if (c < 'a' || c > 'z') continue;
            TrieNode* const* child = cur->children.find(c);
            if (!child) return false;
            cur = *child;
        }
        return cur->is_end;
    }

    /// 查找词频
    int get_count(const std::string& word) const {
        TrieNode* cur = root_;
        for (size_t i = 0; i < word.size(); i++) {
            char c = static_cast<char>(tolower(static_cast<unsigned char>(word[i])));
            if (c < 'a' || c > 'z') continue;
            TrieNode* const* child = cur->children.find(c);
            if (!child) return 0;
            cur = *child;
        }
        return cur->is_end ? cur->count : 0;
    }

    /// 前缀匹配：查找所有以 prefix 开头的词
    /// @param prefix 前缀
    /// @param results 输出数组（调用者分配）
    /// @param max_results 最大结果数
    /// @return 匹配的词数
    int prefix_search(const std::string& prefix, std::string* results, int max_results) const {
        TrieNode* cur = root_;
        for (size_t i = 0; i < prefix.size(); i++) {
            char c = static_cast<char>(tolower(static_cast<unsigned char>(prefix[i])));
            if (c < 'a' || c > 'z') continue;
            TrieNode* const* child = cur->children.find(c);
            if (!child) return 0;
            cur = *child;
        }
        return collect_all(cur, prefix, results, max_results);
    }

    /// 前缀匹配（带词频），用于按热度排序的自动补全
    int prefix_search_with_freq(const std::string& prefix,
                                std::string* results, int* freqs, int max_results) const {
        TrieNode* cur = root_;
        for (size_t i = 0; i < prefix.size(); i++) {
            char c = static_cast<char>(tolower(static_cast<unsigned char>(prefix[i])));
            if (c < 'a' || c > 'z') continue;
            TrieNode* const* child = cur->children.find(c);
            if (!child) return 0;
            cur = *child;
        }
        return collect_frequent(cur, prefix, results, freqs, max_results);
    }

    /// 判断是否存在以 prefix 开头的词
    bool starts_with(const std::string& prefix) const {
        TrieNode* cur = root_;
        for (size_t i = 0; i < prefix.size(); i++) {
            char c = static_cast<char>(tolower(static_cast<unsigned char>(prefix[i])));
            if (c < 'a' || c > 'z') continue;
            TrieNode* const* child = cur->children.find(c);
            if (!child) return false;
            cur = *child;
        }
        return true;
    }

    /// 获取总词数
    int word_count() const {
        int count = 0;
        word_count_dfs(root_, count);
        return count;
    }

private:
    void word_count_dfs(TrieNode* node, int& count) const {
        if (node->is_end) count++;
        char keys[256];
        int n = node->children.get_all_keys(keys);
        for (int i = 0; i < n; i++) {
            TrieNode* const* child = node->children.find(keys[i]);
            if (child) word_count_dfs(*child, count);
        }
    }
};

} // namespace algorithm

#endif // TRIE_H
