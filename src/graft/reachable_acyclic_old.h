#pragma once

namespace graft
{
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
                        if constexpr (some_associator<member>)
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
                        if constexpr (some_associator<member>)
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
            []<class T> { return some_associator<T>; }
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
    //
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
                []<class T> { return not some_associator<T>; }
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
                []<class T> { return  not some_associator<T>; }
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
                []<class T> { return  not some_associator<T>; }
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

    }
}
