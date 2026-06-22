#pragma once

#include "../util/meta.hpp"
#include <ostream>
#include "graft.h"
namespace graft
{
    template<class... sources>
    struct presenter
    {
        using sources_tuple = std::tuple<sources...>;

        using source_ptrs_tuple = tuple_transform_t
        <
            sources_tuple,
            []<class T> { return std::type_identity<std::shared_ptr<T>>{}; }
        >;

        source_ptrs_tuple m_source_ptrs;

        presenter(std::shared_ptr<sources>... source_ptrs)
        : m_source_ptrs{source_ptrs...}
        {
        }

        template<class member>
        requires (requires (sources&& p_sources){ p_sources.get(member{}); } || ...)
        auto get(const member) -> decltype(auto)
        {
            return [this]<std::size_t I = 0>(this auto&& self)
            {
                if constexpr (I == std::tuple_size_v<sources_tuple>) { static_assert(false); }
                else
                {
                    using current_source_ptr = std::tuple_element_t<I, source_ptrs_tuple>;
                    if constexpr
                    (
                        requires (current_source_ptr&& current_source_ptr)
                        {
                            current_source_ptr->get(member{});
                        }
                    )
                    {
                        return std::get<I>(m_source_ptrs)->get(member{});
                    }
                    else { return self.template operator()<I + 1>(); }
                }
            }();
        }

        template<class member>
        auto set(const member, auto value) -> void
        requires (requires (sources&& p_sources){ p_sources.set(member{}, std::move(value)); } || ...)
        {
            [this, value = std::move(value)]<std::size_t I = 0>(this auto&& self)
            {
                if constexpr (I == std::tuple_size_v<sources_tuple>) { static_assert(false); }
                else
                {
                    using current_source_ptr = std::tuple_element_t<I, source_ptrs_tuple>;
                    if constexpr
                    (
                        requires (current_source_ptr&& current_source_ptr)
                        {
                            current_source_ptr->set(member{}, std::move(value));
                        }
                    )
                    {
                        std::get<I>(m_source_ptrs)->set(member{}, std::move(value));
                        return;
                    }
                    else { return self.template operator()<I + 1>(); }
                }
            }();
        }
    };
}
