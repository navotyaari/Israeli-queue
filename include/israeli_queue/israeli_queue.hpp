#pragma once

#include <cstddef>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace iq {

template <typename T, typename KeyExtractor>
class IsraeliQueue {
public:
    using key_type = std::decay_t<std::invoke_result_t<KeyExtractor, const T&>>;

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
};

} // namespace iq
