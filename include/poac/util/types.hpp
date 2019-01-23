#ifndef POAC_UTIL_TYPES_HPP
#define POAC_UTIL_TYPES_HPP

#include <type_traits>
#include <vector>
#include <stack>


namespace poac::util::types {
    // If the type T is a reference type, provides the member typedef type
    //  which is the type referred to by T with its topmost cv-qualifiers removed.
    // Otherwise type is T with its topmost cv-qualifiers removed.
    // C++20, std::remove_cvref_t<T>
    template<typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    // std::conditional for non-type template
    template <auto Value>
    struct value_holder { static constexpr auto value = Value; };
    template <bool B, auto T, auto F>
    using non_type_conditional = std::conditional<B, value_holder<T>, value_holder<F>>;
    template <bool B, auto T, auto F>
    using non_type_conditional_t = std::conditional_t<B, value_holder<T>, value_holder<F>>;
    template <bool B, auto T, auto F>
    static constexpr auto non_type_conditional_v = non_type_conditional_t<B, T, F>::value;

    // boost::property_tree::ptree : {"key": ["array", "...", ...]}
    //  -> std::vector<T> : ["array", "...", ...]
    template <typename T, typename U, typename K=typename U::key_type>
    std::vector<T> ptree_to_vector(const U& pt, const K& key) { // ptree_to_vector(pt, "key")
        std::vector<T> r;
        for (const auto& item : pt.get_child(key)) {
            r.push_back(item.second.template get_value<T>());
        }
        return r;
    }
    // boost::property_tree::ptree : ["array", "...", ...]
    //  -> std::vector<T> : ["array", "...", ...]
    template <typename T, typename U>
    std::vector<T> ptree_to_vector(const U &pt) {
        std::vector<T> r;
        for (const auto& item : pt) {
            r.push_back(item.second.template get_value<T>());
        }
        return r;
    }

    template <typename T>
    std::stack<T> vector_to_stack(const std::vector<T>& v) {
        return std::stack<T>(std::deque<T>(v.begin(), v.end()));
    }
} // end namespace
#endif // !POAC_UTIL_TYPES_HPP
