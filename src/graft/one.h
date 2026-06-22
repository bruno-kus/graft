#pragma once
namespace graft
{
    template <class p_neighbour_metaclass>
    struct one
    {
        using neighbour_metaclass = p_neighbour_metaclass;

        using associator = void;
        template<template<class> class object_template>
        struct holder
        {


            using element_type = object_template<neighbour_metaclass>;
            std::shared_ptr<object_template<neighbour_metaclass>> m_neighbour;

            auto insert(std::shared_ptr<object_template<neighbour_metaclass>> neighbour_ptr) -> void
            {
                m_neighbour = std::move(neighbour_ptr);
            }
            auto begin() const -> const std::shared_ptr<object_template<neighbour_metaclass>>*
            {
                return m_neighbour ? &m_neighbour : ((&m_neighbour) + 1);
            }
            auto end() const -> const std::shared_ptr<object_template<neighbour_metaclass>>*
            {
                return (&m_neighbour) + 1;
            }

            auto begin()  ->  std::shared_ptr<object_template<neighbour_metaclass>>*
            {
                return m_neighbour ? &m_neighbour : ((&m_neighbour) + 1);
            }
            auto end() ->  std::shared_ptr<object_template<neighbour_metaclass>>*
            {
                return (&m_neighbour) + 1;
            }

            auto empty() const -> bool
            {
                return m_neighbour == nullptr;
            }

            auto size() const -> std::size_t
            {
                return (m_neighbour == nullptr ? 0 : 1);
            }
        };
    };

    template<class type>
    struct is_one : std::false_type {};

    template<class type>
    struct is_one<one<type>> : std::true_type {};

    template<class type>
    concept some_one = is_one<type>::value;
}
