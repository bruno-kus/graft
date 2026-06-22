#pragma once
#include "cached_presenter.h"
#include "graph.h"
#include "../util/meta.hpp"
#include <typeindex>
#include <unordered_map>
#include <ostream>
#include <ranges>
#include <sstream>

// struct old {};
namespace graft
{

    template<class source_graph, class... metaclasses>
    struct patch
    {

        template<class from_metaclass, class to_metaclass>
        constexpr static  bool directed_edge_exists_v = []
            {

                // no tak, bo metaklasy się nie zachowują...
                // i to jest grupby problem
                //

                // to zawsze będzie false, ponieważ
                return tuple_contains_if_v<typename from_metaclass::members, []<class from_metaclass_member>
                    {
                        if constexpr (not some_associator<from_metaclass_member>) { return false; }
                        else
                        {
                            return std::is_same_v<typename from_metaclass_member::value_type::neighbour_metaclass, typename to_metaclass::source_metaclass>;
                        }
                    }>;
            }();


        template<class T>
        using source_object_template = source_graph::template object_template<T>;

        // using metaclasses_tuple = std::tuple<metaclasses...>;

        // to jest zwyczajna funkcja, która mapuje klasę nominalną do zadanej, to bardzo dobrze
        template<class nominal_metaclass>
        using nominal_metaclass_to_effective_metaclass = decltype([]
            {
                constexpr static auto blacklisted_index = tuple_find_if_v<std::tuple<metaclasses...>, []<class T>{ return T::metaclass_name == nominal_metaclass::metaclass_name; }>;
                return std::type_identity<metaclasses...[blacklisted_index]>{};
            }())::type;


        // ten ostatni parametr jest trochę dziwny...
        template<class source_metaclass>
        using object_template = mono_cached_presenter<source_object_template, nominal_metaclass_to_effective_metaclass, nominal_metaclass_to_effective_metaclass<source_metaclass>>; // po prostu dać metaklas tu na koniec?


        graph<object_template, metaclasses...> m_graph;

        std::shared_ptr<source_graph> m_source_graph;

        static auto debug_info() -> std::string
        {
            std::stringstream ss;

            [&ss]<std::size_t metaclass_index = 0>(this auto&& self)
            {
                if constexpr (metaclass_index == sizeof...(metaclasses)) { return; }
                else
                {
                    ss << object_template<metaclasses...[metaclass_index]>::debug_info() << '\n';
                    self.template operator()<metaclass_index + 1>();
                }
            }();

            return ss.str();
        }

        friend auto operator<<(std::ostream& os, const patch& patch) -> std::ostream&
        {
            return os << patch.m_graph << '\n';
        }


        template<class A, class B>
        requires (std::is_same_v<typename std::remove_cvref_t<A>::metaclass, metaclasses> || ...)
        auto add_association(A& a, B& b)
        {
            auto a_ptr = m_graph.template get_storage<typename A::metaclass>().find(a);
            auto b_ptr = m_graph.template get_storage<typename B::metaclass>().find(b);

            if constexpr (directed_edge_exists_v<typename A::metaclass, typename B::metaclass>)
            {
                a.insert_link(std::move(b_ptr));
            }
            if constexpr (directed_edge_exists_v<typename B::metaclass, typename A::metaclass>)
            {
                b.insert_link(std::move(a_ptr));
            }
        }
        template<class A, class B>
        auto exists_association( A& a,  B& b)
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
            // nie istenieje AB oraz nie istnieje B -> to bez sensu kompletnie
            // static_assert(not ((not directed_edge_exists_v<typename A::metaclass, typename B::metaclass>) and (not directed_edge_exists_v<typename B::metaclass, typename A::metaclass>)));
            return a_to_b_exists and b_to_a_exists;

        }

        // template<class metaclass>
        // auto create(old, auto&&...args) { return m_graph.template create<metaclass>(std::forward<decltype(args)>(args)...);}
        template<class metaclass>
        auto create(auto&&... args)
        {
            // mogę znaleźć ręcznie przepetrując metaklasy i patrząc, która ma oryginalną taką jak zadana!
            //
            using trimmed_metaclass = decltype
                (
                    []<std::size_t metaclasses_index = 0> (this auto&& self)
                    {
                        if constexpr (metaclasses_index == sizeof...(metaclasses)) { static_assert(false); }
                        else
                        {
                            using current_metaclass = metaclasses...[metaclasses_index];
                            if constexpr (std::is_same_v<typename current_metaclass::source_metaclass, metaclass>)
                            { return std::type_identity<current_metaclass>{}; }
                            else return self.template operator()<metaclasses_index + 1>();
                        }
                    }()
                )::type;
            return m_graph.template create<trimmed_metaclass>(std::forward<decltype(args)>(args)...);
        }
    };
}
