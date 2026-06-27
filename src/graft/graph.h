#pragma once
#include "graft.h"
#include <vector>
#include <memory>
#include <ranges>
#include <ostream>
#include <ranges>

namespace graft
{
    template<class from_metaclass, class to_metaclass>
    constexpr static  bool directed_edge_exists_v = []
        {

            return tuple_contains_if_v<typename from_metaclass::members, []<class from_metaclass_member>
                {
                    if constexpr (not some_associator<from_metaclass_member>) { return false; }
                    else
                    {
                        return from_metaclass_member::value_type::neighbour_metaclass::metaclass_name == to_metaclass::metaclass_name;
                    }
                }>;
        }();
    template<class object_type>
    struct object_storage
    {
        std::vector<std::shared_ptr<object_type>> m_objects;

        auto store(std::shared_ptr<object_type> object_ptr) -> void
        {
            m_objects.emplace_back(std::move(object_ptr));
        }

        auto find(const object_type& object) -> const std::shared_ptr<object_type>&
        {
            auto it = std::ranges::find(m_objects, &object, &std::shared_ptr<object_type>::get);
            if (it == m_objects.end()) { throw std::runtime_error{""}; }
            return *it;
        }

        friend auto operator<<(std::ostream& os, const object_storage& object_storage) -> std::ostream&
        {
            constexpr auto metaclass_name = object_type::metaclass::metaclass_name;
            os << "<" << metaclass_name << ">";
            os << "{";
            if (not object_storage.m_objects.empty())
            {
                os << *object_storage.m_objects.front();
                if (object_storage.m_objects.size() > 1)
                {
                    for (const auto& object_ptr : object_storage.m_objects | std::views::drop(1))
                    {
                        os << ", " << *object_ptr;
                    }
                }
            }
            os << "}";
            return os;
        }
    };
    /*
     * jeżeli mam neighbour object template oraz object_template, czy mogę być tym samym
     */
    template<template<class, template<class> class> class object_template_arg, template<class> class neighbour_effective_metaclass_template, class... metaclasses>
    struct graph
    {

        template<class T>
        using object_for_metaclass = object_template_arg<T, neighbour_effective_metaclass_template>;

        template<class T, template<class> class neif>
        using object_template = object_template_arg<T, neif>;

        using metaclasses_tuple = std::tuple<metaclasses...>;
        template<class T>
        using effective_metaclass_template = neighbour_effective_metaclass_template<T>;

        std::tuple<object_storage<object_template_arg<metaclasses, neighbour_effective_metaclass_template>>...> m_storages;

        using object_types = std::tuple<object_template_arg<metaclasses, neighbour_effective_metaclass_template>...>;

        template<class metaclass>
        auto get_storage() -> object_storage<object_template_arg<metaclass, neighbour_effective_metaclass_template>>&
        {
            return std::get<object_storage<object_template_arg<metaclass, neighbour_effective_metaclass_template>>>(m_storages);
        }

        friend std::hash<graph>;

        friend auto compare_graphs(const auto&, const auto&) -> bool;
        friend auto operator==(const graph& graph1, const graph& graph2) -> bool
        {
            const auto make_tie_of_non_associators = [](const auto& object) -> auto
            {
                using object_metaclass = typename std::remove_cvref_t<decltype(object)>::metaclass;
                using non_associators = tuple_remove_if_t
                <
                    typename object_metaclass::members,
                    []<class T> { return some_associator<T>; }
                >;
                return [&object]<std::size_t... non_associator_indices>(std::index_sequence<non_associator_indices...>)
                {
                    return std::tie(object.get(std::tuple_element_t<non_associator_indices, non_associators>{})...);
                }(std::make_index_sequence<std::tuple_size_v<non_associators>>());
            };

            const bool result = [&graph1, &graph2, &make_tie_of_non_associators]<std::size_t metaclass_index = 0>(this auto&& self)
            {
                if constexpr (metaclass_index == sizeof...(metaclasses)) { return true; }
                else
                {
                    const auto& object_storage_1 = std::get<metaclass_index>(graph1.m_storages);
                    const auto& object_storage_2 = std::get<metaclass_index>(graph2.m_storages);

                    const auto check_storage = [&make_tie_of_non_associators](const auto& object_storage_1, const auto& object_storage_2) -> bool
                    {
                        for (const auto& object_ptr_1 : object_storage_1.m_objects)
                        {
                            const auto& it = std::ranges::find
                            (
                                object_storage_2.m_objects,
                                make_tie_of_non_associators(*object_ptr_1),
                                [&make_tie_of_non_associators](const auto& object_ptr) { return make_tie_of_non_associators(*object_ptr); }
                            );
                            if (it == object_storage_2.m_objects.end()) { return false; }

                            using object_1_metaclass = typename std::remove_cvref_t<decltype(*object_ptr_1)>::metaclass;
                            using associators = tuple_remove_if_t
                            <
                                typename object_1_metaclass::members,
                                []<class T> { return not some_associator<T>; }
                            >;

                            const auto& object_ptr_2 = *it;

                            const bool same_neighbours =
                                [&object_1 = *object_ptr_1, &object_2 = *object_ptr_2, &make_tie_of_non_associators]
                                <std::size_t associator_index =0>(this auto&& self)
                                {
                                    if constexpr (associator_index == std::tuple_size_v<associators>) { return true; }
                                    else
                                    {
                                        using current_associator = std::tuple_element_t<associator_index, associators>;

                                        const bool object_1_neighbours_empty = object_1.get(current_associator{}).empty();
                                        const bool object_2_neighbours_empty = object_2.get(current_associator{}).empty();
                                        if (object_1_neighbours_empty != object_2_neighbours_empty) { return false; }
                                        if
                                        (
                                            make_tie_of_non_associators(object_1) !=
                                            make_tie_of_non_associators(object_2)
                                        )
                                        {
                                            return false;
                                        }

                                        return self.template operator()<associator_index +1>();
                                    }
                                }();
                            if (not same_neighbours) { return false; }
                        }
                        return true;
                    };

                    if (not check_storage(object_storage_1, object_storage_2)) { return false; }
                    if (not check_storage(object_storage_2, object_storage_1)) { return false; }

                    return self.template operator()<metaclass_index + 1>();
                }
            }();

            return result;
        }

        friend auto operator!=(const graph& graph1, const graph& graph2) -> bool { return not (graph1 == graph2); }



        // template<class... p_metaclasses>
        // auto present()
        // {
            // czy ja to mogę zrobić po prostu tym tam, tym tam, tym tam -> flat map
        // }
        //
        friend auto operator<<(std::ostream& os, const graph& graph) -> std::ostream&
        {
            [&os, &graph]<std::size_t storage_index = 0>(this auto&& self)
            {
                if constexpr (storage_index == sizeof...(metaclasses)) { return; }
                else
                {
                    os << std::get<storage_index>(graph.m_storages) << '\n';
                    self.template operator()<storage_index + 1>();
                }
            }();
            return os;
        }


        template<some_metaclass metaclass>
        // requires (std::is_same_v<metaclass, metaclasses> || ...)
        auto create(auto&&... args)
        {
            auto object_ptr = std::make_shared<object_template_arg<metaclass, neighbour_effective_metaclass_template>>(std::forward<decltype(args)>(args)...);

            get_storage<metaclass>().store(object_ptr);

            return object_ptr;
        }

        // template<some_metaclass metaclass>
        // requires (std::is_same_v<metaclass, metaclasses> || ...)
        // auto create_new(auto&&... args)
        // {
        //     auto object_ptr = std::make_shared<object<metaclass>>(std::forward<decltype(args)>(args)...);

        //     get_storage<metaclass>().store(object_ptr);

        //     return object_ptr;
        // }


        template<class A, class B>
        auto exists_association(const A& a, const B& b)
        {
            bool a_to_b_exists = true;
            if constexpr (directed_edge_exists_v<typename A::metaclass, typename B::metaclass>)
            {
                a_to_b_exists = a.exists_link(b);
            }
             bool b_to_a_exists = true;
            if constexpr (directed_edge_exists_v<typename B::metaclass, typename A::metaclass>)
            {
                b_to_a_exists = b.exists_link(a);
            }

            return a_to_b_exists or b_to_a_exists;
        }

        template<class A, class B>
        auto add_association(A& a, B& b)
        {
            auto a_ptr = get_storage<typename A::metaclass>().find(a);
            auto b_ptr = get_storage<typename B::metaclass>().find(b);

            // B jest refką to obiektu, czyli graft::object<Album>&
            // natomiast b_ptr jest już std::shared_ptr<graft::object<Album>>
            // static_assert(std::is_same_v<decltype(b_ptr), std::shared_ptr<graft::object<Album>>>);

            if constexpr (directed_edge_exists_v<typename A::metaclass, typename B::metaclass>)
            {
                a.insert_link(std::move(b_ptr));
            }
            if constexpr (directed_edge_exists_v<typename B::metaclass, typename A::metaclass>)
            {
                b.insert_link(std::move(a_ptr));

            }
        }
    };

    // auto compare_graphs(const auto& graph1, const auto& graph2) -> bool
    // {
    //     const auto make_tie_of_non_associators = [](const auto& object) -> auto
    //     {
    //         using object_metaclass = typename std::remove_cvref_t<decltype(object)>::metaclass;
    //         using non_associators = tuple_remove_if_t
    //         <
    //             typename object_metaclass::members,
    //             []<class T> { return some_associator<T>; }
    //         >;
    //         return [&object]<std::size_t... non_associator_indices>(std::index_sequence<non_associator_indices...>)
    //         {
    //             return std::tie(object.get(std::tuple_element_t<non_associator_indices, non_associators>{})...);
    //         }(std::make_index_sequence<std::tuple_size_v<non_associators>>());
    //     };

    //     const bool result = [&graph1, &graph2, &make_tie_of_non_associators]<std::size_t metaclass_index = 0>(this auto&& self)
    //     {
    //         if constexpr (metaclass_index == sizeof...(metaclasses)) { return true; }
    //         else
    //         {
    //             const auto& object_storage_1 = std::get<metaclass_index>(graph1.m_storages);
    //             const auto& object_storage_2 = std::get<metaclass_index>(graph2.m_storages);

    //             const auto check_storage = [&make_tie_of_non_associators](const auto& object_storage_1, const auto& object_storage_2) -> bool
    //             {
    //                 for (const auto& object_ptr_1 : object_storage_1.m_objects)
    //                 {
    //                     const auto& it = std::ranges::find
    //                     (
    //                         object_storage_2.m_objects,
    //                         make_tie_of_non_associators(*object_ptr_1),
    //                         [&make_tie_of_non_associators](const auto& object_ptr) { return make_tie_of_non_associators(*object_ptr); }
    //                     );
    //                     if (it == object_storage_2.m_objects.end()) { return false; }

    //                     using object_1_metaclass = typename std::remove_cvref_t<decltype(*object_ptr_1)>::metaclass;
    //                     using associators = tuple_remove_if_t
    //                     <
    //                         typename object_1_metaclass::members,
    //                         []<class T> { return not some_associator<T>; }
    //                     >;

    //                     const auto& object_ptr_2 = *it;

    //                     const bool same_neighbours =
    //                         [&object_1 = *object_ptr_1, &object_2 = *object_ptr_2, &make_tie_of_non_associators]
    //                         <std::size_t associator_index =0>(this auto&& self)
    //                         {
    //                             if constexpr (associator_index == std::tuple_size_v<associators>) { return true; }
    //                             else
    //                             {
    //                                 using current_associator = std::tuple_element_t<associator_index, associators>;

    //                                 const bool object_1_neighbours_empty = object_1.get(current_associator{}).empty();
    //                                 const bool object_2_neighbours_empty = object_2.get(current_associator{}).empty();
    //                                 if (object_1_neighbours_empty != object_2_neighbours_empty) { return false; }
    //                                 if
    //                                 (
    //                                     make_tie_of_non_associators(object_1) !=
    //                                     make_tie_of_non_associators(object_2)
    //                                 )
    //                                 {
    //                                     return false;
    //                                 }

    //                                 return self.template operator()<associator_index +1>();
    //                             }
    //                         }();
    //                     if (not same_neighbours) { return false; }
    //                 }
    //                 return true;
    //             };

    //             if (not check_storage(object_storage_1, object_storage_2)) { return false; }
    //             if (not check_storage(object_storage_2, object_storage_1)) { return false; }

    //             return self.template operator()<metaclass_index + 1>();
    //         }
    //     }();

    //     return result;
    // }
}
// template<class... metaclasses>
// struct std::hash<graft::object<metaclasses...>>
// {
//     auto operator()(const graft::object<metaclasses...>& object) const noexcept -> std::size_t
//     {
//         return 0;
//     }
// };
