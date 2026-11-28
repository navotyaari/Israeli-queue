#pragma once

#include <cstddef>
#include <limits>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace iq {

template <typename T, typename KeyExtractor>
class IsraeliQueue {
public:
    using key_type = std::decay_t<std::invoke_result_t<KeyExtractor, const T&>>;

    static constexpr std::size_t unlimited = 0;

    explicit IsraeliQueue(KeyExtractor extractor, std::size_t max_group_size = unlimited)
        : group_head_(nullptr)
        , group_tail_(nullptr)
        , total_size_(0)
        , key_extractor_(std::move(extractor))
        , max_group_size_(max_group_size) {}

    ~IsraeliQueue() {
        GroupNode* group = group_head_;
        while (group) {
            ElementNode* elem = group->elem_head;
            while (elem) {
                ElementNode* next_elem = elem->next;
                delete elem;
                elem = next_elem;
            }
            GroupNode* next_group = group->next_group;
            delete group;
            group = next_group;
        }
    }

    IsraeliQueue(IsraeliQueue&& other) noexcept
        : group_head_(other.group_head_)
        , group_tail_(other.group_tail_)
        , group_index_(std::move(other.group_index_))
        , total_size_(other.total_size_)
        , key_extractor_(std::move(other.key_extractor_))
        , max_group_size_(other.max_group_size_) {
        other.group_head_ = nullptr;
        other.group_tail_ = nullptr;
        other.total_size_ = 0;
    }

    IsraeliQueue& operator=(IsraeliQueue&& other) noexcept {
        if (this == &other) return *this;
        this->~IsraeliQueue();
        new (this) IsraeliQueue(std::move(other));
        return *this;
    }

    IsraeliQueue(const IsraeliQueue&)            = delete;
    IsraeliQueue& operator=(const IsraeliQueue&) = delete;

    void enqueue(T value) {
        key_type key = key_extractor_(static_cast<const T&>(value));

        auto it = group_index_.find(key);
        if (it != group_index_.end() && !group_is_full(it->second)) {
            GroupNode* group    = it->second;
            ElementNode* node   = new ElementNode(std::move(value));
            group->elem_tail->next = node;
            group->elem_tail       = node;
            ++group->size;
        } else {
            ElementNode* node      = new ElementNode(std::move(value));
            GroupNode*   group     = new GroupNode(std::move(key), node);
            GroupNode*   prev_tail = group_tail_;
            if (group_tail_) {
                group_tail_->next_group = group;
            } else {
                group_head_ = group;
            }
            group_tail_ = group;
            try {
                group_index_[group->key] = group;
            } catch (...) {
                if (prev_tail) {
                    prev_tail->next_group = nullptr;
                } else {
                    group_head_ = nullptr;
                }
                group_tail_ = prev_tail;
                delete group->elem_head;
                delete group;
                throw;
            }
        }
        ++total_size_;
    }

    std::optional<T> dequeue() {
        if (!group_head_) return std::nullopt;
        T value = std::move(group_head_->elem_head->value);
        remove_front_element();
        return value;
    }

    void pop() {
        remove_front_element();
    }

    const T& front() const {
        return group_head_->elem_head->value;
    }

    bool empty() const noexcept {
        return group_head_ == nullptr;
    }

    std::size_t size() const noexcept {
        return total_size_;
    }

private:
    struct ElementNode {
        T value;
        ElementNode* next;

        explicit ElementNode(T&& val)
            : value(std::move(val)), next(nullptr) {}
    };

    struct GroupNode {
        key_type     key;
        ElementNode* elem_head;
        ElementNode* elem_tail;
        std::size_t  size;
        GroupNode*   next_group;

        GroupNode(key_type k, ElementNode* first_elem)
            : key(std::move(k))
            , elem_head(first_elem)
            , elem_tail(first_elem)
            , size(1)
            , next_group(nullptr) {}
    };

    GroupNode*                               group_head_;
    GroupNode*                               group_tail_;
    std::unordered_map<key_type, GroupNode*> group_index_;
    std::size_t                              total_size_;
    KeyExtractor                             key_extractor_;
    std::size_t                              max_group_size_;

    bool group_is_full(const GroupNode* group) const noexcept {
        return max_group_size_ != unlimited && group->size >= max_group_size_;
    }

    void remove_front_element() {
        GroupNode*   group    = group_head_;
        ElementNode* old_elem = group->elem_head;
        group->elem_head = old_elem->next;
        --group->size;
        delete old_elem;
        --total_size_;

        if (!group->elem_head) {
            // Only evict from the index if this group is still the current one for its key.
            // A full group may have already been replaced in the index by its overflow successor.
            auto it = group_index_.find(group->key);
            if (it != group_index_.end() && it->second == group) {
                group_index_.erase(it);
            }
            group_head_ = group->next_group;
            if (!group_head_) group_tail_ = nullptr;
            delete group;
        }
    }
};

} // namespace iq
