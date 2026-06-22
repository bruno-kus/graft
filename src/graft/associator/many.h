#pragma once
#include <vector>
#include <memory>
namespace graft
{
    template <class some_type>
    struct many
    {
        using neighbour_metaclass = some_type;

        using associator = void;
        using arity_many = void;

        template<template<class> class object_template>
        struct holder
        {
            using element_type = object_template<some_type>;
            std::vector<std::shared_ptr<object_template<some_type>>> m_neighbours;

            auto insert(std::shared_ptr<object_template<some_type>> neighbour_ptr) -> void
            {
                m_neighbours.emplace_back(std::move(neighbour_ptr));
            }

            auto begin() const -> auto
            {
                return m_neighbours.begin();
            }
            auto end() const -> auto
            {
                return m_neighbours.end();
            }

            auto empty() const -> bool
            {
                return m_neighbours.empty();
            }
            auto size() const -> bool
            {
                return m_neighbours.size();
            }
        };
    };

    template<class some_type>
    struct is_many : std::false_type {};

    template<class some_type>
    struct is_many<many<some_type>> : std::true_type {};

    template<class some_type>
    concept some_many = is_many<some_type>::value;
}
