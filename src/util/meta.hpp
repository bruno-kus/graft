#pragma once
#include <array>



// template<std::string_view&... string_views>
// consteval std::string_view string_view_concat()
// {
//     constexpr static auto concatenation_size = (std::size_t{0} + ... + string_views.size()) + 1;
//     constexpr static auto std::array<char, concatenation_size> string_buffer = []
//         {
//             std::array<char, concatenation_size> result;
//             result.back() = '\n';
//             std::size_t result_back = 0;
//             [&result, &result_back]<std::size_t string_view_index = 0>(this auto&& self)
//             {
//                 if constexpr (string_view_index == sizeof...(string_views)) { return; }
//                 else
//                 {
//                     const auto& current_string_view = string_views...[string_view_index];
//                     for (int i = 0; i < current_string_view.size(); ++i)
//                     {
//                         result.at(result_back + i) = current_string_view.at(i);
//                     }
//                 }
//             }();
//             return result;
//         }();
//     return std::string_view{string_buffer};
// }

template<class tuple, class T>
using tuple_append_t = decltype([]
{
    return []<std::size_t... Is>(std::index_sequence<Is...>)
    {
        return std::type_identity<std::tuple<std::tuple_element_t<Is, tuple>..., T>>{};
    }(std::make_index_sequence<std::tuple_size_v<tuple>>());
}
())::type;

template<class tuple, class T>
using tuple_prepend_t = decltype([]
{
    return []<std::size_t... Is>(std::index_sequence<Is...>)
    {
        return std::type_identity<std::tuple<T, std::tuple_element_t<Is, tuple>...>>{};
    }(std::make_index_sequence<std::tuple_size_v<tuple>>());
}
())::type;

template<class tuple, auto lambda>
constexpr std::size_t tuple_count_if_v =
        []<std::size_t I = 0>(this auto&& self, std::size_t count = 0)
        {
            if constexpr (I == std::tuple_size_v<tuple>) { return count; }
            else
            {
                if constexpr (lambda.template operator()<std::tuple_element_t<I, tuple>>()) { ++count; }
                return self.template operator()<I + 1>(count);
            }
        }();

template<class tuple, auto lambda>
using tuple_remove_if_t = decltype([]
{
    constexpr static auto result_size = tuple_count_if_v<tuple, []<class T> { return not lambda.template operator()<T>(); }>;
    constexpr std::array<std::size_t, result_size> result_indices = []
    {
        std::array<std::size_t, result_size> result_indices{};
        std::size_t result_indices_back = 0;
        [&result_indices, &result_indices_back]<std::size_t I = 0>(this auto&& self)
        {
            if constexpr (I == std::tuple_size_v<tuple>) { return; }
            else
            {
                if constexpr (not lambda.template operator()<std::tuple_element_t<I, tuple>>())
                {
                    result_indices.at(result_indices_back) = I;
                    ++result_indices_back;
                }
                return self.template operator()<I + 1>();
            }
        }();
        return result_indices;
    }();

    return [&result_indices]<std::size_t... tuple_indices>(std::index_sequence<tuple_indices...>)
    {
        return std::type_identity<std::tuple<std::tuple_element_t<result_indices.at(tuple_indices), tuple>...>>{};
    }(std::make_index_sequence<std::tuple_size_v<decltype(result_indices)>>());
}())::type;

template<class tuple, class T>
constexpr bool tuple_contains_v =
    []<std::size_t I = 0>(this auto&& self)
    {
        if constexpr (I == std::tuple_size_v<tuple>) { return false; }
        else if constexpr (std::is_same_v<T, std::tuple_element_t<I, tuple>>) { return true; }
        else return self.template operator()<I + 1>();
    }();

template<class tuple, auto lambda>
constexpr bool tuple_contains_if_v =
    []<std::size_t I = 0>(this auto&& self)
    {
        if constexpr (I == std::tuple_size_v<tuple>) { return false; }
        else if constexpr (lambda.template operator()<std::tuple_element_t<I, tuple>>()) { return true; }
        else return self.template operator()<I + 1>();
    }();

template<class tuple, class T>
constexpr std::size_t tuple_find_v =
    []<std::size_t I = 0>(this auto&& self)
    {
        if constexpr (I == std::tuple_size_v<tuple>) { static_assert(false); }
        else if constexpr (std::is_same_v<T, std::tuple_element_t<I, tuple>>) { return I; }
        else return self.template operator()<I + 1>();
    }();


template<class tuple, auto lambda>
constexpr std::size_t tuple_find_if_v =
    []<std::size_t I = 0>(this auto&& self)
    {
        if constexpr (I == std::tuple_size_v<tuple>) { static_assert(false); }
        else if constexpr (lambda.template operator()<std::tuple_element_t<I, tuple>>()) { return I; }
        else return self.template operator()<I + 1>();
    }();

template<class tuple, template<class...> class template_>
using tuple_specialise_template_t = decltype([]
{
    return []<std::size_t... Is>(std::index_sequence<Is...>)
    {
        return std::type_identity<template_<std::tuple_element_t<Is, tuple>...>>{};
    }(std::make_index_sequence<std::tuple_size_v<tuple>>());
}())::type;




template<class tuple, auto lambda>
using tuple_transform_t = decltype([]
{
  constexpr auto transform_type_t = []<size_t index>()static
  {
    using current_element_type = std::tuple_element_t<index, tuple>;
    using transformed = decltype(lambda.template operator()<current_element_type>())::type;
    return std::type_identity<transformed>{};
  };

  constexpr auto result_t = []<size_t... indexes>(std::index_sequence<indexes...>)static
  {
    using transformed = std::tuple<typename decltype(transform_type_t.template operator()<indexes>())::type...>;
    return std::type_identity<transformed>{};
  };

  return result_t(std::make_index_sequence<std::tuple_size_v<tuple>>());
}())::type;
