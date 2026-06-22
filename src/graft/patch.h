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
                        if constexpr (not associator<from_metaclass_member>) { return false; }
                        else
                        {
                            return std::is_same_v<typename from_metaclass_member::value_type::neighbour_metaclass, typename to_metaclass::source_metaclass>;
                        }
                    }>;
            }();


        template<class T>
        using source_object_template = source_graph::template object_template<T>;

        // using metaclasses_tuple = std::tuple<metaclasses...>;

        // mapped metaclass
        template<class metaclass>
        using domain_metaclass_to_blacklisted = decltype([]
            {
                constexpr static auto blacklisted_index = tuple_find_if_v<std::tuple<metaclasses...>, []<class T>{ return T::metaclass_name == metaclass::metaclass_name; }>;
                return std::type_identity<metaclasses...[blacklisted_index]>{};
            }())::type;


        template<class source_metaclass>
        using object_template = mono_cached_presenter</*source_metaclass,*/ source_object_template, domain_metaclass_to_blacklisted, domain_metaclass_to_blacklisted<source_metaclass>>; // po prostu dać metaklas tu na koniec?


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




    // auto make_patch_helper(auto& patch, auto& object, const auto member)
    // {
    //     auto cached_presenter_ptr = patch.create<std::remove_cvref_t<decltype(object)>>(object);
    //     for (const auto& neighbour_ptr : object.get(member))
    //     {
    //         auto cached_presenter_neighbour_ptr = patch.create</* */>(*neighbour_ptr);
    //         patch.add_association(*cached_presenter_ptr, *cached_presenter_neighbour_ptr);
    //     }
    // }

    // template<class path_members, class... metaclasses>
    // auto make_patch(auto& graph, auto& object) -> auto
    // {
    //     auto patch;
    //     const auto& cached_presenter_ptr = patch.create<std::remove_cvref_t<decltype(object)>>(object);


    //     [&]<std::size_t path_member_index = 0>(this auto&& self, auto& cached_presenter)
    //     {
    //         for (const auto& neighbour_ptr : object.get(member))
    //         {
    //             const auto& cached_presenter_neighbour_ptr = patch.create(neighbour_ptr);
    //             patch.add_association(object, *cached_presenter_neighbour_ptr);
    //             self(*cached_presenter_neighbour_ptr, /* */);
    //         }
    //     }
    //     return patch;
    // }
    //

    struct edge
    {
        std::size_t left_class_index{};
        std::size_t right_class_index{};
        std::size_t left_to_right_associator_index{};
        std::size_t right_to_left_associator_index{};
    };



    template<class... metaclasses>
    consteval auto compute_edge_count() -> std::size_t
    {
        std::size_t edge_count = 0;

        [&edge_count]<std::size_t metaclass_index = 0>(this auto&& traverse_metaclass)
        {
            if constexpr (metaclass_index == sizeof...(metaclasses)) { return; }
            else
            {
                using current_metaclass = metaclasses...[metaclass_index];
                using members = typename current_metaclass::members;
                [&edge_count]<std::size_t member_index = 0>(this auto&& traverse_member)
                {
                    if constexpr (member_index == std::tuple_size_v<members>) { return; }
                    else
                    {
                        using member = std::tuple_element_t<member_index, members>;
                        if constexpr (associator<member>)
                        {
                            using neighbour_metaclass = typename member::value_type::neighbour_metaclass;
                            constexpr static bool metaclasses_contain_neighbour = tuple_contains_v<std::tuple<metaclasses...>, neighbour_metaclass>;

                            constexpr static bool association_canonical =
                                current_metaclass::metaclass_name <
                                neighbour_metaclass::metaclass_name;

                            if (metaclasses_contain_neighbour and association_canonical) { ++edge_count; }
                        }
                        traverse_member.template operator()<member_index + 1>();
                    }
                }();
                traverse_metaclass.template operator()<metaclass_index + 1>();
            }
        }();
        return edge_count;
    }


    template<class... metaclasses>
    consteval auto get_edges() -> std::array<edge, compute_edge_count<metaclasses...>()>
    {
        constexpr static auto edges_size = compute_edge_count<metaclasses...>();
        std::array<edge, edges_size> edges;
        std::size_t edges_back = 0;

        // przydałoby mi się jeszcze liczyć tego tam, tego tam, tego tam indeks absolutny?/
        // mogę po prostu robić pseudo push back, ale też przydałyby mi się dane krawędzie
        [&edges, &edges_back]<std::size_t metaclass_index = 0>(this auto&& traverse_metaclass)
        {
            if constexpr (metaclass_index == sizeof...(metaclasses)) { return; }
            else
            {
                using current_metaclass = metaclasses...[metaclass_index];
                using members = typename current_metaclass::members;
                [&edges, &edges_back]<std::size_t member_index = 0>(this auto&& traverse_member)
                {
                    if constexpr (member_index == std::tuple_size_v<members>) { return; }
                    else
                    {
                        using member = std::tuple_element_t<member_index, members>;
                        if constexpr (associator<member>)
                        {
                            using neighbour_metaclass = typename member::value_type::neighbour_metaclass;
                            constexpr bool metaclasses_contain_neighbour = tuple_contains_v<std::tuple<metaclasses...>, neighbour_metaclass>;

                            constexpr bool association_canonical =
                                current_metaclass::metaclass_name <
                                neighbour_metaclass::metaclass_name;

                            if constexpr (metaclasses_contain_neighbour and association_canonical)
                            {
                                static_assert(tuple_contains_v<std::tuple<metaclasses...>, neighbour_metaclass>);
                                using neighbour_metaclass_members = typename neighbour_metaclass::members;
                                auto v_edge = edge
                                {
                                    .left_class_index = metaclass_index,
                                    .right_class_index = tuple_find_v<std::tuple<metaclasses...>, neighbour_metaclass>,
                                    .left_to_right_associator_index = member_index,
                                    .right_to_left_associator_index = tuple_find_if_v
                                    <
                                        neighbour_metaclass_members,
                                        []<class T> static
                                        {
                                            // return true;
                                            if constexpr (requires { typename T::value_type::neighbour_metaclass; })
                                            {
                                                using neighbour_metaclass = typename T::value_type::neighbour_metaclass;
                                                return std::is_same_v<std::remove_cvref_t<neighbour_metaclass>, std::remove_cvref_t<current_metaclass>>;
                                            } else { return false; }
                                        }
                                    >
                                };
                                edges.at(edges_back) = v_edge;
                                ++edges_back;
                            }
                        }
                        traverse_member.template operator()<member_index + 1>();
                    }
                }();
                traverse_metaclass.template operator()<metaclass_index + 1>();
            }
        }();
        return edges;
    }

    auto shallow_clone(auto& new_graph, auto& object) -> decltype(auto)
    {
        using object_metaclass = typename std::remove_cvref_t<decltype(object)>::metaclass;
        // muszę wszystkie atrybuty pobrać...
        using non_associators = tuple_remove_if_t
        <
            typename object_metaclass::members,
            []<class T> { return associator<T>; }
        >;

        return [&object, &new_graph]<std::size_t... non_associator_indices>(std::index_sequence<non_associator_indices...>)
        {
            return new_graph.template create<object_metaclass>(object.get(std::tuple_element_t<non_associator_indices, non_associators>{})...);
        }(std::make_index_sequence<std::tuple_size_v<non_associators>>());
    }

    // template<class source_graph>
    // struct make_reachable_acyclic_detail
    // {
    //     template<class... metaclasses>
    //     struct patch_type_template : patch<std::remove_cvref_t<source_graph>, metaclasses...> {
    //         template<class T>
    //         using object_template = patch<std::remove_cvref_t<source_graph>, metaclasses...>::template object_template<T>;
    //     };
    // };


    template<class root_metaclass>
    consteval auto compute_member_blacklist_size() -> std::size_t
    {
        std::size_t result = 0;

        struct association
        {
            constexpr association( std::string_view metaclass_name_1, std::string_view metaclass_name_2)
            {
                from_metaclass_name = std::min(metaclass_name_1, metaclass_name_2);
                to_metaclass_name = std::max(metaclass_name_1, metaclass_name_2);
            };

            std::string_view from_metaclass_name;
            std::string_view to_metaclass_name;

            constexpr bool operator==(const association&) const  = default;
        };
        std::vector<association> traversed;


        constexpr static auto is_edge_traversed = [](std::vector<association>& traversed, std::string_view metaclass_name_1, std::string_view metaclass_name_2)
        {
            const auto it = std::ranges::find(traversed, association{metaclass_name_1, metaclass_name_2});
            return (it != traversed.end());
        };

        constexpr static auto add_traversed_edge = [](std::vector<association>& traversed, std::string_view metaclass_name_1, std::string_view metaclass_name_2)
        {
            if (not is_edge_traversed(traversed, metaclass_name_1, metaclass_name_2)) { traversed.emplace_back(association{metaclass_name_1, metaclass_name_2}); }
        };

        [&result, &traversed]<class previous_metaclass, class current_metaclass>(this auto&& visit_metaclass) -> void
        {
            using associators = tuple_remove_if_t
            <
                typename current_metaclass::members,
                []<class T>{ return not associator<T>; }
            >;
            [&visit_metaclass, &result, &traversed]<std::size_t associator_index = 0>(this auto&& traverse_associator)
            {
                if constexpr (associator_index == std::tuple_size_v<associators>) { return; }
                else
                {
                    using current_associator = std::tuple_element_t<associator_index, associators>;
                    using neighbour_metaclass = typename current_associator::value_type::neighbour_metaclass;

                    constexpr bool current_associator_neighbour_metaclass_same_as_previous_metaclass = std::is_same_v<typename current_associator::value_type::neighbour_metaclass, previous_metaclass>;
                    constexpr bool current_associator_arity_many = requires { typename current_associator::value_type::arity_many; };
                    if constexpr (current_associator_neighbour_metaclass_same_as_previous_metaclass and current_associator_arity_many)
                    {
                        result += 1;
                    }

                    if (not is_edge_traversed(traversed, current_metaclass::metaclass_name, neighbour_metaclass::metaclass_name))
                    {
                        add_traversed_edge(traversed, current_metaclass::metaclass_name, neighbour_metaclass::metaclass_name);
                        visit_metaclass.template operator()<current_metaclass, typename current_associator::value_type::neighbour_metaclass>();
                    }

                    // jeżeli nie szedłem już tą krawędzią // TODO
                    return traverse_associator.template operator()<associator_index + 1>();

                }
            }();
        }.template operator()<void, root_metaclass>();
        return result;
    }

    template<class metaclasses_tuple, class root_metaclass>
    consteval auto compute_member_blacklist() -> auto
    {
        constexpr static auto member_blacklist_size = compute_member_blacklist_size<root_metaclass>();


        struct member_location
        {
            std::size_t metaclass_index = 0;
            std::size_t member_index = 0;
        };


        struct association
        {
            constexpr association( std::string_view metaclass_name_1, std::string_view metaclass_name_2)
            {
                from_metaclass_name = std::min(metaclass_name_1, metaclass_name_2);
                to_metaclass_name = std::max(metaclass_name_1, metaclass_name_2);
            };

            std::string_view from_metaclass_name;
            std::string_view to_metaclass_name;

            constexpr bool operator==(const association&) const  = default;
        };
        std::vector<association> traversed;


        constexpr static auto is_edge_traversed = [](std::vector<association>& traversed, std::string_view metaclass_name_1, std::string_view metaclass_name_2)
        {
            const auto it = std::ranges::find(traversed, association{metaclass_name_1, metaclass_name_2});
            const bool found = (it != traversed.end());
            return found;
        };

        constexpr static auto add_traversed_edge = [](std::vector<association>& traversed, std::string_view metaclass_name_1, std::string_view metaclass_name_2)
        {
            if (not is_edge_traversed(traversed, metaclass_name_1, metaclass_name_2)) { traversed.emplace_back(association{metaclass_name_1, metaclass_name_2}); }
        };


        std::array<member_location, member_blacklist_size> result;
        std::size_t result_back = 0;
        [&result, & result_back, &traversed ]<class previous_metaclass, class current_metaclass>(this auto&& visit_metaclass) -> void
        {
            using associators = tuple_remove_if_t
            <
                typename current_metaclass::members,
                []<class T>{ return not associator<T>; }
            >;
            [&visit_metaclass, &result, &result_back, &traversed]<std::size_t associator_index = 0>(this auto&& traverse_associator)
            {
                if constexpr (associator_index == std::tuple_size_v<associators>) { return; }
                else
                {
                    using current_associator = std::tuple_element_t<associator_index, associators>;
                    using neighbour_metaclass = typename current_associator::value_type::neighbour_metaclass;

                    constexpr bool current_associator_neighbour_metaclass_same_as_previous_metaclass = std::is_same_v<typename current_associator::value_type::neighbour_metaclass, previous_metaclass>;
                    constexpr bool current_associator_arity_many = requires { typename current_associator::value_type::arity_many; }; // PODEJRZANE
                    if constexpr (current_associator_neighbour_metaclass_same_as_previous_metaclass and current_associator_arity_many)
                    {
                        result.at(result_back) = member_location{.metaclass_index = tuple_find_v<metaclasses_tuple, current_metaclass>, .member_index=tuple_find_v<typename current_metaclass::members, current_associator> };
                        ++result_back;
                    }

                    if (not is_edge_traversed(traversed, current_metaclass::metaclass_name, neighbour_metaclass::metaclass_name))
                    {
                        add_traversed_edge(traversed, current_metaclass::metaclass_name, neighbour_metaclass::metaclass_name);
                        visit_metaclass.template operator()<current_metaclass, typename current_associator::value_type::neighbour_metaclass>();
                    }
                    return traverse_associator.template operator()<associator_index + 1>();

                };
            }();
        }.template operator()<void, root_metaclass>();
        return result;
    }

    template<class metaclasses_tuple, class root_metaclass>
    using make_reachable_acyclic_patch_adhoc_metaclasses_t = decltype([]
    {
        constexpr static auto member_blacklist = compute_member_blacklist<metaclasses_tuple, root_metaclass>();


        constexpr auto derive_adhoc_metacalss = []<class metaclass>
            {
                using members = typename metaclass::members;
                constexpr auto is_member_blacklisted = []<class member> -> bool
                    {
                        return []<std::size_t member_blacklist_index = 0>(this auto&& self) -> bool
                        {
                            if constexpr (member_blacklist_index == member_blacklist.size()) { return false; }
                            else
                            {
                                constexpr auto current_member_blacklist_element = std::get<member_blacklist_index>(member_blacklist);
                                using current_member_blacklist_element_metaclass = std::tuple_element_t<current_member_blacklist_element.metaclass_index, metaclasses_tuple >;
                                using current_member_blacklist_element_member = std::tuple_element_t<current_member_blacklist_element.member_index, typename current_member_blacklist_element_metaclass::members>;
                                if constexpr (std::is_same_v<metaclass, current_member_blacklist_element_metaclass> and std::is_same_v<member, current_member_blacklist_element_member>)
                                { return true; }
                                else { return self.template operator()<member_blacklist_index + 1>();}
                            }
                        }();
                    };
                using filtered_members = tuple_remove_if_t<members, is_member_blacklisted>;
                return std::type_identity<tuple_specialise_template_t<tuple_prepend_t<filtered_members, metaclass>, ::graft::metaclass>>{};
            };

        return std::type_identity<tuple_transform_t<metaclasses_tuple, derive_adhoc_metacalss>>{};
    }())::type;

    auto make_reachable_acyclic_patch(const auto& source_graph, auto& object_ptr) -> auto
    {
        using root_object_metaclass = typename std::remove_cvref_t<decltype(object_ptr)>::element_type::metaclass;
        using source_graph_type = std::remove_cvref_t<decltype(source_graph)>;
        using patch_type = tuple_specialise_template_t
        <
            tuple_prepend_t
            <
                // typename source_graph_type::metaclasses_tuple, // OK!
                make_reachable_acyclic_patch_adhoc_metaclasses_t<typename source_graph_type::metaclasses_tuple, root_object_metaclass>,
                source_graph_type
            >,
            patch
        >;
        patch_type patch;
        std::unordered_map<void*, std::shared_ptr<void>> source_target_map;

        auto outer_tick = 0;
        auto inner_tick = 0;
        // mogę się zastanowić nad algorytmem, który będzie przyjmować funkcję mapującą czy coś...
        [&source_graph, &patch, &source_target_map, &outer_tick, &inner_tick]<class previous_metaclass>(this auto&& self, const auto& source_object_ptr) ->void
        {
            ++outer_tick;

            using source_object_metaclass = std::remove_cvref_t<decltype(source_object_ptr)>::element_type::metaclass;
            using source_object_type = std::remove_cvref_t<decltype(source_object_ptr)>::element_type; // object<Album>
            using associators = tuple_remove_if_t
            <
                typename source_object_metaclass::members,
                // typename patch_type::template object_template< source_object_metaclass>::metaclass::members,
                []<class T> { return not associator<T>; }
            >;

           [&source_graph, &patch, &source_target_map, &source_object_ptr, &self, &outer_tick, &inner_tick]<std::size_t associator_index = 0>(this auto&& traverse_associators) -> void
           {
               ++inner_tick;

               if constexpr (associator_index == std::tuple_size_v<associators>) { return; }
               else
               {
                   using current_associator = std::tuple_element_t<associator_index, associators>; // artists
                   constexpr bool current_associator_neighbour_metaclass_same_as_previous_metaclass = std::is_same_v<typename current_associator::value_type::neighbour_metaclass, previous_metaclass>;
                   constexpr bool current_associator_arity_many = requires { typename current_associator::value_type::arity_many; };

                   // std::string previous_metaclass_name = "void";
                   // static_assert(std::is_same_v<previous)
                   // if constexpr (requires { previous_metaclass::metaclass_name; }) { previous_metaclass_name = previous_metaclass::metaclass_name; }
                   // if constexpr (not std::is_same_v<previous_metaclass, void>) { previous_metaclass_name = previous_metaclass::metaclass_name; }


                   std::cout << "(" << outer_tick  << "." << inner_tick << ") " << "edge=(" << source_object_metaclass::metaclass_name  << ", " << current_associator::value_type::neighbour_metaclass::metaclass_name << ")" << '\n';
                   if constexpr (current_associator_neighbour_metaclass_same_as_previous_metaclass and current_associator_arity_many)
                   {
                       return traverse_associators.template operator()<associator_index + 1>();
                   }
                   for (const auto& source_neighbour_ptr : source_object_ptr->get(current_associator{})) // artists
                   {
                       using source_neighbour_metaclass = typename std::remove_cvref_t<decltype(source_neighbour_ptr)>::element_type::metaclass;
                       using source_neighbour_type = typename std::remove_cvref_t<decltype(source_neighbour_ptr)>::element_type;
                       if (not source_target_map.contains(source_object_ptr.get()))
                       {
                           const auto& target_object_ptr = patch.template create<source_object_metaclass>(source_object_ptr, std::vector<int>{}); // TODO
                           source_target_map.emplace(source_object_ptr.get(), std::static_pointer_cast<void>(target_object_ptr));
                       }
                       if (not source_target_map.contains(source_neighbour_ptr.get()))
                       {
                           const auto& target_neighbour_ptr = patch.template create<source_neighbour_metaclass>(source_neighbour_ptr, std::vector<int>{}); // TODO
                           source_target_map.emplace(source_neighbour_ptr.get(), std::static_pointer_cast<void>(target_neighbour_ptr));
                       }

                       using target_object_type = patch_type::template object_template<source_object_metaclass>;
                       using target_neighbour_type = patch_type::template object_template<source_neighbour_metaclass>;

                       const auto& target_object_ptr = std::static_pointer_cast<target_object_type>(source_target_map.at(source_object_ptr.get()));
                       const auto& target_neighbour_ptr = std::static_pointer_cast<target_neighbour_type>(source_target_map.at(source_neighbour_ptr.get()));

                       std::cout << "(" << outer_tick  << "." << inner_tick << ") ";
                       std::cout << std::boolalpha;
                       std::cout << "patch.exists_association(*target_object_ptr, *target_neighbour_ptr) -> " << (patch.exists_association(*target_object_ptr, *target_neighbour_ptr)) << '\n';
                      if( patch.exists_association(*target_object_ptr, *target_neighbour_ptr))
                      {
                          std::cout << "*target_object_ptr=" << *target_object_ptr <<"\n*target_neighbour_ptr=" << *target_neighbour_ptr << '\n';
                      }
                       if (not patch.exists_association(*target_object_ptr, *target_neighbour_ptr)) // TODO
                       {
                           patch.add_association(*target_object_ptr, *target_neighbour_ptr); // TODO
                           self.template operator()<source_object_metaclass>(source_neighbour_ptr);
                       }
                   }
                   return traverse_associators.template operator()<associator_index + 1>();
               };
           }();
        }.template operator()<void>(object_ptr);;
        return patch;
    }

    auto clone_reachable_acyclic(auto& graph, auto& object_ptr)
    {
        std::remove_cvref_t<decltype(graph)> new_graph;

        std::unordered_map<void*, std::shared_ptr<void>> old_new_map;
        int iteration = 0;
        [&graph, &new_graph, &old_new_map, &iteration] <class previous_metaclass>(this auto&& clone_reachable_depth_first, const auto& old_object_ptr)
        {
            ++iteration;
            int inner_iter = 0;
            using associators = tuple_remove_if_t
            <
                typename std::remove_cvref_t<decltype(old_object_ptr)>::element_type::metaclass::members,
                []<class T> { return  not associator<T>; }
            >;
            [&old_new_map, &new_graph, &old_object_ptr, &clone_reachable_depth_first, &iteration, &inner_iter]
            <std::size_t associator_index = 0>(this auto&& traverse_associators) -> void
            {
                ++inner_iter;
                if constexpr(associator_index == std::tuple_size_v<associators>) { return; }
                else
                {
                    using current_associator = std::tuple_element_t<associator_index, associators>;

                    using old_object_metaclass = typename std::remove_cvref_t<decltype(old_object_ptr)>::element_type::metaclass;


                    if constexpr (std::is_same_v<typename current_associator::value_type::neighbour_metaclass, previous_metaclass>)
                    {
                        if constexpr (requires { typename current_associator::value_type::arity_many; })
                        {

                            const auto& current_associator_name = current_associator::name;
                            std::cout << "tick(" << iteration << "." << inner_iter << ")" << "SKIPPING: " << current_associator::name << " from " << std::remove_cvref_t<decltype(old_object_ptr)>::element_type::metaclass::metaclass_name << '\n';
                            return traverse_associators.template operator()<associator_index + 1>();
                        }
                    }


                    for (const auto& old_neighbour_ptr : old_object_ptr->get(current_associator{}))
                    {
                        using old_neighbour_metaclass = typename std::remove_cvref_t<decltype(old_neighbour_ptr)>::element_type::metaclass;

                        if (not old_new_map.contains(old_object_ptr.get()))
                        {
                            const auto& new_object_ptr = shallow_clone(new_graph, *old_object_ptr);
                            old_new_map.emplace(old_object_ptr.get(), std::static_pointer_cast<void>(new_object_ptr));
                        }
                        if (not old_new_map.contains(old_neighbour_ptr.get()))
                        {
                            const auto& new_neighbour_ptr = shallow_clone(new_graph, *old_neighbour_ptr);
                            old_new_map.emplace(old_neighbour_ptr.get(), std::static_pointer_cast<void>(new_neighbour_ptr));
                        }
                        const auto& new_object_ptr = std::static_pointer_cast<object<old_object_metaclass>>( old_new_map.at(old_object_ptr.get()) );
                        const auto& new_neighbour_ptr = std::static_pointer_cast<object<old_neighbour_metaclass>>(old_new_map.at(old_neighbour_ptr.get()));


                        if (not new_graph.exists_association(*new_object_ptr, *new_neighbour_ptr))
                        {
                            std::cout << "tick(" << iteration << "." << inner_iter << ")" << "adding assoc (" << old_object_metaclass::metaclass_name << ", " << old_neighbour_metaclass::metaclass_name << ")" << '\n';
                            if (old_object_metaclass::metaclass_name == "Artist" || old_neighbour_metaclass::metaclass_name == "Artist")
                            {
                                std::cout << '\t' << *new_object_ptr << " --- " << *new_neighbour_ptr << '\n';
                            }
                            new_graph.add_association(*new_object_ptr, *new_neighbour_ptr);
                            clone_reachable_depth_first.template operator()<old_object_metaclass>(old_neighbour_ptr);
                        }
                    }
                    traverse_associators.template operator()<associator_index + 1>();
                }
            }();
        }.template operator()</*typename std::remove_cvref_t<decltype(object_ptr)>::element_type::metaclass*/ void>(object_ptr);
        return new_graph;
    }

    auto clone_reachable(auto& graph, auto& object_ptr) -> auto
    {
        // std::remove_cvref_t<decltype(graph)> new_graph;
        // auto new_graph = auto(graph);
        std::remove_cvref_t<decltype(graph)> new_graph;

        std::unordered_map<void*, std::shared_ptr<void>> old_new_map; // tam mam void*, które prowadzą to obiektów, czyli do old_object_ptr.get() !
        // potrzebuję funkcji, która klonuje obiekt

        [&graph, &new_graph, &old_new_map] (this auto&& clone_reachable_depth_first, const auto& old_object_ptr)
        {
            using associators = tuple_remove_if_t
            <
                typename std::remove_cvref_t<decltype(old_object_ptr)>::element_type::metaclass::members,
                []<class T> { return  not associator<T>; }
            >;
            [&old_new_map, &new_graph, &old_object_ptr, &clone_reachable_depth_first]<std::size_t associator_index = 0>(this auto&& traverse_associators) -> void
            {
                if constexpr(associator_index == std::tuple_size_v<associators>) { return; }
                else
                {
                    using current_associator = std::tuple_element_t<associator_index, associators>;

                    // static_assert(std::is_same_v<void, decltype(old_object_ptr->get(current_associator{}))>);
                    // static_assert(std::is_same_v<void, current_associator>);

                    for (const auto& old_neighbour_ptr : old_object_ptr->get(current_associator{}))
                    {
                        // static_assert(std::is_same_v<char, std::remove_cvref_t<decltype(old_neighbour_ptr)>>);
                        using old_object_metaclass = typename std::remove_cvref_t<decltype(old_object_ptr)>::element_type::metaclass;
                        using old_neighbour_metaclass = typename std::remove_cvref_t<decltype(old_neighbour_ptr)>::element_type::metaclass;

                        if (not old_new_map.contains(old_object_ptr.get()))
                        {
                            const auto& new_object_ptr = shallow_clone(new_graph, *old_object_ptr); /*new_graph.template create<old_object_metaclass>(old_object_ptr);*/
                            old_new_map.emplace(old_object_ptr.get(), std::static_pointer_cast<void>(new_object_ptr));
                        }
                        if (not old_new_map.contains(old_neighbour_ptr.get()))
                        {
                            const auto& new_neighbour_ptr = shallow_clone(new_graph, *old_neighbour_ptr);/* new_graph.template create<old_neighbour_metaclass>(old_neighbour_ptr);*/ // i tu chcę pointer do starego!
                            old_new_map.emplace(old_neighbour_ptr.get(), std::static_pointer_cast<void>(new_neighbour_ptr));
                        }
                        const auto& new_object_ptr = std::static_pointer_cast<object<old_object_metaclass>>( old_new_map.at(old_object_ptr.get()) );
                        const auto& new_neighbour_ptr = std::static_pointer_cast<object<old_neighbour_metaclass>>(old_new_map.at(old_neighbour_ptr.get()));

                        if (not new_graph.exists_association(*new_object_ptr, *new_neighbour_ptr))
                        {
                            // std::cout << "adding assoc" << '\n';
                            new_graph.add_association(*new_object_ptr, *new_neighbour_ptr);
                            clone_reachable_depth_first(old_neighbour_ptr);
                        }
                    }
                    traverse_associators.template operator()<associator_index + 1>();
                }
            }();
        }(object_ptr);
        return new_graph;

        // [&old_graph, &new_graph, &old_new_map] clone_reachable_depth_first(&old_object)
        // for each &associator in old_objects.associators
        //     for each &old_neighbour in associator.neigbhours
        //         if not old_new_map.contains(old_object) { old_new_map[old_object] = new_graph.create_shallow_copy(old_object) }
        //         &new_object = old_new_map[old_object]
        //         if not old_new_map_contains(old_neighbour) { old_new_map[old_neighbour] = new_graph.create_shallow_copy(old_neigbhour)}
        //         &new_neighbour = old_new_map[old_neighbour]
        //         if not exists_association(new_object, new_neighbour)
        //             graph.add_association(new_object, new_neighbour)
        //             clone_reachable_depth_first(old_neighbour)
    }





    // // auto make_patch(auto& graph, const object<Album>& album)
    // {
    //     // to będzie inferować numerowanie na albumie!
    //     // będzie dodawać połączenia pomiędzy albumem, a utworem
    //     // będzie tak, że utworzona łata będzie zawierała wyłącznie 1 album oraz wszystkie kawałki
    //     // no tylko, że żeby wiedzieć, które połączenia istnieją pomiędzy albumem, a kawałkiem
    //     // to muszę iść poprzez DiscNumberedTrack!
    //     // no chyba, że mógłbym mieć API, które by mi wybrało elementy i je mógłbym dodać do łaty w osobnych krokach
    //     // SPRÓBUJMY NAJPIERW WERSJI, GDZIE TWORZONA JEST ŁATA W JEDNYM KROKU
    //     //
    //     //
    //     using patch_metaclasses = std::tuple<Album, Disc, DiscNumberedTrack, AlbumNumberedTrack>;
    //     // i cały czas zakładam, że cache'uję wszystko, aczkolwiek i tak nie mogę cache'ować wszystkiego, bo nie mogę cache'ować asocjacji
    //     patch<Album, AlbumNumberedTrack, Disc, DiscNumberedTrack, Track> patch;

    //     const auto& album_cached_presenter_ptr = patch.create<Album>(album);
    //     // [&](this auto&& self)
    // }

}
