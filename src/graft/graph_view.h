#pragma once

#include "graph.h"
#include "erased_graph.h"
#include <vector>
#include <memory>
#include <variant>


namespace graft
{

    // z buffered graph view jest podobny problem, otóż potrzebuje ono źródła
    // aczoklwiek
    // teoretycznie mogę zrobić erased graph jeszcze
    // ale to mógłbym zrobić interfejs tagujący pod tytułem, że...
    // jeżeli object_template jest object_view
    // to wtedy używam innej funkcji do tworzenia wewnątrz make acycli
    // tak czy siak warto zrobić erased graph!

    // to istotnie jest specjalny przypadek
    // tylko pytanie,
    // template<template<class, template<class> class object_template_arg, template<class> class effective_metaclass_template, class... metaclasses>



    // a co z takimi rzeczami jak undo graph???
    // aczkolwiek tutaj add_association i pozosałe operacje grafowe są nadpisane


    // a oprócz tego jest kwestia tego, że dla dziedziny mogę sobie dziedziczyć po grafie!

    // albo mogę mieć parametry dla grafu? albo po prostu
    template<class source_graph, class base>
    struct graph_view : base
    {
        using base_graph = base;
        using base::base;

        graph_view(base base_arg) : base{std::move(base_arg)} {}

        // teraz jest problem taki, że metody są wołane
        // więc mogę zrobić konstruktor graph_view, który przyjmuje graf
        // tylko rzecz w tym, że ja dzedziczę po grafie
        template<class metaclass>
        auto create(auto&&... args) -> decltype(auto)
        {
            const auto ptr = base::template create<metaclass>(std::forward<decltype(args)>(args)...);
            m_dirty_objects.emplace_back(ptr);
            return ptr;
        }

        template<class A, class B>
        auto add_association(A& a, B& b)
        {
            auto a_ptr = base::template get_storage<typename A::metaclass>().find(a);
            auto b_ptr = base::template get_storage<typename B::metaclass>().find(b);
            m_dirty_objects.emplace_back(a_ptr);
            m_dirty_objects.emplace_back(b_ptr);
            base::add_association(a, b);
        }

        // możliwe, że coś takiego byłoby konieczne, żeby było tylko w samym grafie!
        using dirty_objects_type = decltype
            ([]{
                using object_types = typename base::object_types;

                return []<std::size_t... object_type_indices>(std::index_sequence<object_type_indices...>)
                {
                    return std::type_identity
                        <
                            std::vector
                                <
                                    std::variant
                                        <
                                            std::shared_ptr
                                                <
                                                std::tuple_element_t<object_type_indices, object_types>
                                                >...
                                        >
                                >
                        >{};
                }(std::make_index_sequence<std::tuple_size_v<object_types>>());
            }())::type;

        dirty_objects_type m_dirty_objects; // private
        dirty_objects_type m_destroyed_objects;

        // std::shared_ptr<source_graph> m_source_graph;
         source_graph* m_source_graph;


        graph_view(std::shared_ptr<source_graph> source_graph_ptr) : m_source_graph{std::move(source_graph_ptr)}
        {
        }

        auto push_upstream() -> void
        {
            for (const auto& dirty_object_variant : m_dirty_objects)
            {
                std::visit
                    (
                        [this](const auto& object_ptr)
                        {
                            using dirty_object_metaclass = typename std::remove_cvref_t<decltype(object_ptr)>::element_type::metaclass::source_metaclass;
                            const auto& source_object_ptr = m_source_graph->template create<dirty_object_metaclass>();
                            // using non_associators = tuple_remove_if_t
                            //     <
                            //         typename dirty_object_metaclass::members,
                            //         []<class member> { return some_associator<member>; }
                            //     >;
                            [&]<std::size_t member_index = 0>(this auto&& self)
                            {
                                if constexpr (member_index == std::tuple_size_v<typename dirty_object_metaclass::members>){ return; }
                                else
                                {
                                    using current_member = std::tuple_element_t<member_index, typename dirty_object_metaclass::members>;

                                    if constexpr (not some_associator<current_member>)
                                    {
                                        source_object_ptr->set(current_member{}, object_ptr->get(current_member{}));
                                    }
                                    return self.template operator()<member_index + 1>();
                                }
                            }();
                        },
                        dirty_object_variant
                    );
            }
        }
        /*
         * do kogo powinna należeć ta relacja?
         * no bo rzecz w tym, że zwykłe create może być przeciążone na usunięcie czy coś
         */

    };

}
/*
 *
 */
