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
        (void)value; // implemented in IQ-5b
    }

    std::optional<T> dequeue() {
        return std::nullopt; // implemented in IQ-6b
    }

    void pop() {
        // implemented in IQ-6b
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
};

} // namespace iq
