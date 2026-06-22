#pragma once

#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>
#include "associator/some_associator.h"
#include "../util/meta.hpp"
#include "../util/static-string.hpp"
#include <ranges>
#include "member.h"
#include "metaclass.h"

namespace graft
{



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
                []<class T>{ return not some_associator<T>; }
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
                []<class T>{ return not some_associator<T>; }
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






    // tylko, że ja tu muszę taki currying zastosować, ponieważ szablon, który podaje musi znać wszystkie metaklasy, jakie są...
    //

    // template<class nominal_metaclass>
    // using reachable_acyclic_effective_metaclass = decltype
    //     ([]{
    //         //
    //         using effective_metaclasses = make_reachable_acyclic_patch_adhoc_metaclasses_t<

    //     }())::type;

    template<template<class> class object_template_arg, class metaclasses_tuple, class root_metaclass>
    struct reachable_acyclic_object_template
    {
        template<class nominal_metaclass>
        using object_template = decltype
            ([]{
                using effective_metaclasses = make_reachable_acyclic_patch_adhoc_metaclasses_t
                    <
                        metaclasses_tuple,
                        root_metaclass
                    >;
                constexpr auto effective_metaclass_index = tuple_find_if_v
                    <
                        metaclasses_tuple,
                        []<class effective_metaclass> { return effective_metaclass::metaclass_name == nominal_metaclass::metaclass_name; }
                    >;
                return std::type_identity<object_template_arg<std::tuple_element_t<effective_metaclass_index, effective_metaclasses>>>{};
            }())::type;
    };


    // template<template<class> class object_template, class metaclasses_tuple, class root_metaclass>
    // using reachable_acyclic_object_template = reachable_acyclic_effective_metaclass<object_template, metaclasses_tuple, root_metaclass>;


    template<template<template<class> class, class...> class graph_template, template<class> class object_template>
    auto make_reachable_acyclic_copy(const auto& source_graph, auto& root_object_ptr)
    {
        using source_graph_type = std::remove_cvref_t<decltype(source_graph)>;
        using root_object_metaclass = typename std::remove_cvref_t<decltype(root_object_ptr)>::element_type::metaclass;

        using adhoc = make_reachable_acyclic_patch_adhoc_metaclasses_t<typename source_graph_type::metaclasses_tuple, root_object_metaclass>;

        using target_graph_type = decltype
            ([]<std::size_t... Is>(std::index_sequence<Is...>){
                return std::type_identity
                    <
                        graph_template
                        <
                            reachable_acyclic_object_template
                                <
                                    object_template,
                                    typename source_graph_type::metaclasses_tuple,
                                    root_object_metaclass // effective czy nominal? // todo
                                >::template object_template,
                            std::tuple_element_t<Is, adhoc>...
                        >
                    >{};
            }(std::make_index_sequence<std::tuple_size_v<adhoc>>()))::type;


        target_graph_type target_graph;

        std::unordered_map<void*, std::shared_ptr<void>> source_target_map;

        [&source_graph, &target_graph, &source_target_map]<class previous_metaclass>(this auto&& self, const auto& source_object_ptr) ->void
        {
            using source_object_metaclass = std::remove_cvref_t<decltype(source_object_ptr)>::element_type::metaclass;
            using source_object_type = std::remove_cvref_t<decltype(source_object_ptr)>::element_type; // object<Album>
            using associators = tuple_remove_if_t
            <
                typename source_object_metaclass::members,
                []<class T> { return not some_associator<T>; }
            >;
            [&source_graph, &target_graph, &source_target_map, &source_object_ptr, &self]<std::size_t associator_index = 0>(this auto&& traverse_associators) -> void
            {

                if constexpr (associator_index == std::tuple_size_v<associators>) { return; }
                else
                {
                    using current_associator = std::tuple_element_t<associator_index, associators>; // artists
                    constexpr bool current_associator_neighbour_metaclass_same_as_previous_metaclass = std::is_same_v<typename current_associator::value_type::neighbour_metaclass, previous_metaclass>;
                    constexpr bool current_associator_arity_many = requires { typename current_associator::value_type::arity_many; };

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
                            const auto& target_object_ptr = target_graph.template create<source_object_metaclass>(); // TODO // KURWA WAŻNE, ALE NA POTEM
                            source_target_map.emplace(source_object_ptr.get(), std::static_pointer_cast<void>(target_object_ptr));
                        }
                        if (not source_target_map.contains(source_neighbour_ptr.get()))
                        {
                            const auto& target_neighbour_ptr = target_graph.template create<source_neighbour_metaclass>(); // TODO // KURWA WAŻNE, ALE NA POTEM
                            source_target_map.emplace(source_neighbour_ptr.get(), std::static_pointer_cast<void>(target_neighbour_ptr));
                        }

                        using target_object_type = target_graph_type::template object_template<source_object_metaclass>;
                        using target_neighbour_type = target_graph_type::template object_template<source_neighbour_metaclass>;

                        const auto& target_object_ptr = std::static_pointer_cast<target_object_type>(source_target_map.at(source_object_ptr.get()));
                        const auto& target_neighbour_ptr = std::static_pointer_cast<target_neighbour_type>(source_target_map.at(source_neighbour_ptr.get()));

                        if (not target_graph.exists_association(*target_object_ptr, *target_neighbour_ptr)) // TODO
                        {
                            target_graph.add_association(*target_object_ptr, *target_neighbour_ptr); // TODO
                            self.template operator()<source_object_metaclass>(source_neighbour_ptr);
                        }
                    }
                    return traverse_associators.template operator()<associator_index + 1>();
                };
            }();
        }.template operator()<void>(root_object_ptr);
        return target_graph;
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
