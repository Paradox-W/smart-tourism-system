/**
 * @file inverted_index.h
 * @brief 倒排索引 —— 用于日记全文检索加速
 *
 * 核心数据结构：词 -> 文档列表 的映射
 *
 * 与 Trie 的配合使用：
 *   - Trie 负责前缀匹配和自动补全
 *   - 倒排索引负责"哪些文档包含这个词"的快速查找
 *
 * 工作流程：
 *   1. 建立索引：对每篇日记分词，逐词插入倒排索引
 *   2. 查询：将查询词分词，查找包含所有查询词的文档交集
 *   3. 排序：按相关度（TF-IDF 简化版）排序返回
 *
 * 时间复杂度：
 *   - 建立索引: O(总词数)
 *   - 单词查询: O(1) + O(文档数)
 *   - 多词查询: O(查询词数 * 文档数)
 *   - 排序: O(n * log n)
 */

#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include "hash_table.h"
#include <cstring>
#include <string>

namespace algorithm {

// ============================================================
// 倒排记录项：某文档中某词的出现信息
// ============================================================
struct Posting {
    int doc_id;         // 文档ID（日记ID）
    int term_freq;      // 词频（该词在该文档中出现次数）
    int first_pos;      // 首次出现位置（用于短语查询和摘要高亮）

    Posting() : doc_id(-1), term_freq(0), first_pos(-1) {}
    Posting(int id, int freq, int pos) : doc_id(id), term_freq(freq), first_pos(pos) {}
};

// ============================================================
// 倒排列表：一个词对应的所有文档记录
// ============================================================
struct PostingList {
    int* doc_ids;       // 文档ID数组
    int* term_freqs;    // 对应词频数组
    int length;         // 记录数

    PostingList() : doc_ids(nullptr), term_freqs(nullptr), length(0) {}
    ~PostingList() {
        delete[] doc_ids;
        delete[] term_freqs;
    }

    PostingList(const PostingList&) = delete;
    PostingList& operator=(const PostingList&) = delete;

    PostingList(PostingList&& other) noexcept
        : doc_ids(other.doc_ids), term_freqs(other.term_freqs), length(other.length) {
        other.doc_ids = nullptr;
        other.term_freqs = nullptr;
        other.length = 0;
    }

    PostingList& operator=(PostingList&& other) noexcept {
        if (this != &other) {
            delete[] doc_ids;
            delete[] term_freqs;
            doc_ids = other.doc_ids;
            term_freqs = other.term_freqs;
            length = other.length;
            other.doc_ids = nullptr;
            other.term_freqs = nullptr;
            other.length = 0;
        }
        return *this;
    }

    /// 添加一条记录
    void add(int doc_id, int term_freq, int first_pos = 0) {
        // 简单实现：重新分配数组
        int* new_ids = new int[length + 1];
        int* new_freqs = new int[length + 1];
        for (int i = 0; i < length; i++) {
            new_ids[i] = doc_ids[i];
            new_freqs[i] = term_freqs[i];
        }
        delete[] doc_ids;
        delete[] term_freqs;
        doc_ids = new_ids;
        term_freqs = new_freqs;
        doc_ids[length] = doc_id;
        term_freqs[length] = term_freq;
        length++;
    }

    /// 检查是否包含某文档
    bool contains(int doc_id) const {
        for (int i = 0; i < length; i++) {
            if (doc_ids[i] == doc_id) return true;
        }
        return false;
    }

    /// 获取某文档的词频
    int get_freq(int doc_id) const {
        for (int i = 0; i < length; i++) {
            if (doc_ids[i] == doc_id) return term_freqs[i];
        }
        return 0;
    }
};

// ============================================================
// 倒排索引
// ============================================================
class InvertedIndex {
private:
    HashMap<std::string, PostingList> index_;   // 词 -> 倒排列表
    int total_docs_;                             // 总文档数
    int* doc_lengths_;                           // 每篇文档的词数（用于TF-IDF归一化）
    int max_doc_id_;

    /// 简单分词：按空格和标点分割，转小写
    /// @param text 输入文本
    /// @param words 输出词数组
    /// @param max_words 最大词数
    /// @return 实际词数
    int tokenize(const std::string& text, std::string* words, int max_words) const {
        int count = 0;
        std::string current;

        for (size_t i = 0; i < text.size() && count < max_words; i++) {
            char c = text[i];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                current += static_cast<char>(tolower(static_cast<unsigned char>(c)));
            } else {
                if (!current.empty() && count < max_words) {
                    words[count++] = current;
                    current.clear();
                }
                // 处理中文：简单按字符切分（实际项目中应使用jieba等分词器）
                if (c >= 0 && static_cast<unsigned char>(c) < 0x80) continue;
                // UTF-8中文：3字节一个字符
                if (i + 2 < text.size()) {
                    std::string cn_char = text.substr(i, 3);
                    if (!cn_char.empty()) {
                        words[count++] = cn_char;
                        i += 2;  // 跳过后两个字节（for循环还会i++）
                    }
                }
            }
        }

        if (!current.empty() && count < max_words) {
            words[count++] = current;
        }

        return count;
    }

public:
    InvertedIndex() : total_docs_(0), doc_lengths_(nullptr), max_doc_id_(0) {
        doc_lengths_ = new int[1024];
        memset(doc_lengths_, 0, sizeof(int) * 1024);
    }

    ~InvertedIndex() {
        delete[] doc_lengths_;
    }

    InvertedIndex& operator=(InvertedIndex&& other) noexcept {
        if (this != &other) {
            delete[] doc_lengths_;
            index_ = static_cast<decltype(index_)&&>(other.index_);
            total_docs_ = other.total_docs_;
            doc_lengths_ = other.doc_lengths_;
            max_doc_id_ = other.max_doc_id_;
            other.doc_lengths_ = nullptr;
            other.total_docs_ = 0;
            other.max_doc_id_ = 0;
        }
        return *this;
    }

    /// 重置索引
    void clear() {
        total_docs_ = 0;
        max_doc_id_ = 0;
        delete[] doc_lengths_;
        doc_lengths_ = new int[1024];
        memset(doc_lengths_, 0, sizeof(int) * 1024);
    }

    /// 索引一篇文档
    /// @param doc_id 文档ID
    /// @param title 标题（权重更高）
    /// @param content 正文
    void add_document(int doc_id, const std::string& title, const std::string& content) {
        // 确保数组够大
        while (doc_id >= max_doc_id_ || doc_id >= 1024) {
            int new_size = (max_doc_id_ == 0) ? 1024 : max_doc_id_ * 2;
            int* new_lengths = new int[new_size];
            memcpy(new_lengths, doc_lengths_, sizeof(int) * (max_doc_id_ > 0 ? max_doc_id_ : 1024));
            delete[] doc_lengths_;
            doc_lengths_ = new_lengths;
            max_doc_id_ = new_size;
        }

        // 合并标题和正文（标题重复3次以提高权重）
        std::string full_text = title + " " + title + " " + title + " " + content;

        // 分词
        const int MAX_WORDS = 10000;
        std::string* words = new std::string[MAX_WORDS];
        int word_count = tokenize(full_text, words, MAX_WORDS);

        doc_lengths_[doc_id] = word_count;

        // 统计词频
        HashMap<std::string, int> word_freq(64);
        for (int i = 0; i < word_count; i++) {
            int* f = word_freq.find(words[i]);
            if (f) {
                (*f)++;
            } else {
                word_freq.insert(words[i], 1);
            }
        }

        // 插入倒排索引
        std::string all_keys[10000];
        int key_count = word_freq.get_all_keys(all_keys);
        for (int i = 0; i < key_count; i++) {
            PostingList* pl = index_.find(all_keys[i]);
            if (!pl) {
                PostingList new_pl;
                index_.insert(all_keys[i], static_cast<PostingList&&>(new_pl));
                pl = index_.find(all_keys[i]);
            }

            int freq = *word_freq.find(all_keys[i]);

            // 查找首次出现位置
            int first_pos = 0;
            for (int j = 0; j < word_count; j++) {
                if (words[j] == all_keys[i]) { first_pos = j; break; }
            }

            pl->add(doc_id, freq, first_pos);
        }

        delete[] words;
        total_docs_++;
    }

    /// 查询：返回包含所有查询词的文档（AND查询）
    /// @param query 查询字符串
    /// @param result_doc_ids 输出文档ID数组
    /// @param result_scores 输出相关度分数数组
    /// @param max_results 最大结果数
    /// @return 匹配的文档数
    int search(const std::string& query,
               int* result_doc_ids, double* result_scores, int max_results) const {
        // 分词查询
        const int MAX_Q_WORDS = 100;
        std::string q_words[MAX_Q_WORDS];
        int q_count = tokenize(query, q_words, MAX_Q_WORDS);

        if (q_count == 0) return 0;

        // 获取第一个词的倒排列表作为初始结果集
        const PostingList* first_pl = index_.find(q_words[0]);
        if (!first_pl) return 0;

        // 候选文档集
        int* candidates = new int[first_pl->length];
        int cand_count = first_pl->length;
        memcpy(candidates, first_pl->doc_ids, sizeof(int) * cand_count);

        // 对后续查询词做交集过滤
        for (int q = 1; q < q_count; q++) {
            const PostingList* pl = index_.find(q_words[q]);
            if (!pl) { delete[] candidates; return 0; }  // 缺少某个词，无结果

            // 过滤：候选文档必须在当前词的倒排列表中
            int new_count = 0;
            for (int i = 0; i < cand_count; i++) {
                if (pl->contains(candidates[i])) {
                    candidates[new_count++] = candidates[i];
                }
            }
            cand_count = new_count;
            if (cand_count == 0) { delete[] candidates; return 0; }
        }

        // 计算相关度分数（简化TF-IDF）
        double* scores = new double[cand_count];
        for (int i = 0; i < cand_count; i++) {
            scores[i] = 0;
            int doc_id = candidates[i];
            for (int q = 0; q < q_count; q++) {
                const PostingList* pl = index_.find(q_words[q]);
                if (pl) {
                    int tf = pl->get_freq(doc_id);
                    int df = pl->length;
                    // TF-IDF 简化：tf * log(N / df)
                    double idf = (total_docs_ > 0 && df > 0)
                                 ? std::log(static_cast<double>(total_docs_) / df) : 0;
                    scores[i] += tf * idf;
                }
            }
        }

        // 按分数降序排序（简单选择排序）
        int result_count = (cand_count < max_results) ? cand_count : max_results;
        for (int i = 0; i < result_count; i++) {
            int best = i;
            for (int j = i + 1; j < cand_count; j++) {
                if (scores[j] > scores[best]) best = j;
            }
            if (best != i) {
                int tmp_id = candidates[i]; candidates[i] = candidates[best]; candidates[best] = tmp_id;
                double tmp_s = scores[i]; scores[i] = scores[best]; scores[best] = tmp_s;
            }
        }

        for (int i = 0; i < result_count; i++) {
            result_doc_ids[i] = candidates[i];
            result_scores[i] = scores[i];
        }

        delete[] candidates;
        delete[] scores;

        return result_count;
    }

    /// 获取总文档数
    int get_total_docs() const { return total_docs_; }
};

} // namespace algorithm

#endif // INVERTED_INDEX_H
