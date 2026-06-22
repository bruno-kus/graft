#pragma once

#include "../util/meta.hpp"
#include "presenter.h"
#include "graft.h" // object.h
#include <sstream>
#include "../../thirdparty/StaticTypeInfo/static_type_info.h"
namespace graft
{

    template<class source_metaclass_t, class... p_members>
    struct metaclass

    {
        using source_metaclass = source_metaclass_t;
        constexpr static auto metaclass_name = source_metaclass::metaclass_name;
        using members = std::tuple<p_members...>;

    };

    template<class source_metaclass_t, class... p_members>
    struct domain_metaclass<metaclass<source_metaclass_t, p_members...>> { using type = typename metaclass<source_metaclass_t, p_members...>::source_metaclass; };

    namespace detail
    {

        template<class sources_tuple, std::size_t final_member_index>
        using cached_presenter_final_member_t = decltype([]
        {
            return []<std::size_t I = 0, std::size_t current_source_index = 0, std::size_t current_member_index = 0>(this auto&& self)
            {
                using current_source = std::tuple_element_t<current_source_index, sources_tuple>;
                using current_member = std::tuple_element_t<current_member_index, typename current_source::metaclass::members>;

                if constexpr (I == final_member_index)
                {
                    return std::type_identity<current_member>{};
                }
                else
                {
                    if constexpr (current_member_index + 1 == std::tuple_size_v<typename current_source::metaclass::members>)
                    {
                        return self.template operator()<I + 1, current_source_index + 1, 0>();
                    } else
                    {
                        return self.template operator()<I + 1, current_source_index, current_member_index + 1>();
                    }
                }
            }();
        }())::type;


        template<class sources_tuple>
        using cached_presenter_concat_all_members_t = decltype([]
        {
            // constexpr auto sources_members_size =
            //     []<std::size_t sources_index = 0, std::size_t result = 0>(this auto&& self) -> std::size_t
            //     {
            //         if constexpr (sources_index == std::tuple_size_v<sources_tuple>) { return result; }
            //         else
            //         {
            //             using current_source = std::tuple_element_t<sources_index, sources_tuple>;
            //             constexpr auto current_source_members_size = std::tuple_size_v<typename current_source::members>;
            //             return self.template operator()<sources_index + 1, result + current_source_members_size>();
            //         }
            //     }();

            constexpr auto sources_members_size =
                []<std::size_t... source_indices>(std::index_sequence<source_indices...>)
                {
                    return (std::size_t{0} + ... + std::tuple_size_v<typename std::tuple_element_t<source_indices, sources_tuple>::metaclass::members>);
                }(std::make_index_sequence<std::tuple_size_v<sources_tuple>>());

            return []<std::size_t... final_member_indices>(std::index_sequence<final_member_indices...>)
            {
                return std::type_identity<std::tuple<cached_presenter_final_member_t<sources_tuple, final_member_indices>...>>{};
            }(std::make_index_sequence<sources_members_size>());
        }())::type;
    }


    template<class sources_tuple, class members_tuple = detail::cached_presenter_concat_all_members_t<sources_tuple>>
    struct cached_presenter
    {

        tuple_specialise_template_t<sources_tuple, graft::presenter> m_presenter;
        object<tuple_specialise_template_t<members_tuple, metaclass>> m_cache;

        friend auto operator<<(std::ostream& os, const cached_presenter& cached_presenter) -> std::ostream&
        {
            return os << cached_presenter.m_cache << '\n';
        }

        cached_presenter(auto... source_ptrs) : m_presenter{std::move(source_ptrs)...}
        {
            [this]<std::size_t member_index = 0>(this auto&& self)
            {
                if constexpr (member_index == std::tuple_size_v<members_tuple>) { return; }
                else
                {
                    using current_member = const std::tuple_element_t<member_index, members_tuple>;
                    m_cache.set(current_member{}, m_presenter.get(current_member{}));
                    return self.template operator()<member_index + 1>();
                }
            }();
        }


        auto push_upstream() -> void
        {
            [this]<std::size_t I = 0>(this auto&& self)
            {
                if constexpr (I == std::tuple_size_v<members_tuple>) { return; }
                else
                {
                    using current_member = const std::tuple_element_t<I, members_tuple>;
                    m_presenter.set(current_member{}, m_cache.get(current_member{}));
                    return self.template operator()<I + 1>();
                }
            }();
        }

        template<class member>
        requires (tuple_contains_v<members_tuple, const member>)
        auto set(const member, auto value) -> void
        {
            m_cache.set(member{}, std::move(value));
        }
        template<class member>
        requires (tuple_contains_v<members_tuple, const member>)
        auto get(const member) -> decltype(auto)
        {
            return m_cache.get(member{});
        }
    };


    // nie powinienem mieć czegoś takiego jak maska w ogóle, ponieważ mogę wstawić zwyczajnie inną metaklasę!
    // tylko, że muszę znać metaklasę oryginału, nawet jeżeli ten obiekt ma własną
    // ale też muszę znać obiekt

    template<class source_metaclass_t, template<class> class source_object_template_t>
    struct mono_presenter
    {
        using source_metaclass = source_metaclass_t;

        template<class metaclass>
        using source_object_template = source_object_template_t<metaclass>;

        using source = source_object_template<source_metaclass>;

        std::shared_ptr<source> m_source_ptr;

        mono_presenter(auto source_ptr) : m_source_ptr{std::move(source_ptr)}
        {}
        mono_presenter() {}

        auto get(const auto member) -> decltype(auto)
        {
            return m_source_ptr->get(member);
        }

        auto set(const auto member, auto&& value) -> void
        {
            m_source_ptr->set(member, std::forward<decltype(value)>(value));
        }
    };


    // struct from_ptr{};
    // może łatwiej byłoby podawać source_metaclass oraz source_object_template osobno // TODO
    template</*class source_metaclass_t, */template<class> class source_object_template_t, template<class> class metaclass_to_blacklisted, class metaclass_t>
    struct mono_cached_presenter
    {
        template<class T>
        using source_object_template = source_object_template_t<T>;

        using source_metaclass = typename metaclass_t::source_metaclass;
        using metaclass = metaclass_t;
        using members = typename metaclass::members;

        using source = source_object_template_t<source_metaclass>;

        mono_presenter<source_metaclass, source_object_template_t> m_mono_presenter;

        template<class T>
        using object_body_neighbour_object_template = mono_cached_presenter</*T,*/ source_object_template_t, metaclass_to_blacklisted, metaclass_to_blacklisted<T>>;
        object_body<metaclass, object_body_neighbour_object_template> m_cache;

        static auto debug_info() -> std::string
        {
            std::stringstream ss;
            ss << "mono_cached_presenter" << '\n';
            ss << "metaclass=" << metaclass::metaclass_name << '\n';
            ss << "members=" << '\n';
            [&ss]<std::size_t member_index = 0>(this auto&& self)
            {
                if constexpr( member_index == std::tuple_size_v<typename metaclass::members>) { return; }
                else
                {
                    ss << '\t' << std::tuple_element_t<member_index, typename metaclass::members>::name << '\n';
                    self.template operator()<member_index + 1>();

                }
            }();
            ss << "member_holders=" << '\n';
            [&ss]<std::size_t member_holder_index = 0>(this auto&& self)
            {
                if constexpr (member_holder_index == std::tuple_size_v<typename decltype(m_cache)::member_holders_type> ) { return; }
                else
                {
                    using holder =  std::tuple_element_t<member_holder_index, typename decltype(m_cache)::member_holders_type>;
                    if constexpr (requires { typename holder::element_type; })
                    {
                        // ss << '\n' << std::tuple_size_v<holder
                    } else {
                        ss << '\t' << static_type_info::getTypeName<holder>() << '\n';
                    }
                    self.template operator()<member_holder_index + 1>();
                }
            }();
            return ss.str();


        }


        // TU JEST PROBLEM, ŻE OBIEKT DOMYŚLNIE ZAKŁADA, ŻE SĄSIADUJE Z INNYMI OBIEKTAMI!

        mono_cached_presenter(auto&&... args) : m_cache{std::forward<decltype(args)>(args)...} {}

        mono_cached_presenter(auto source_ptr, std::vector<int>) : m_mono_presenter{std::move(source_ptr)}
        {
            // static_assert(std::tuple_size_v<typename source::metaclass::members> == std::tuple_size_v<typename metaclass_t::members>);
            // static_assert(std::is_same_v<typename source::metaclass::members, typename metaclass_t::members>);

            // znajdź retain all
            [this]<std::size_t member_index = 0>(this auto&& self)
            {
                if constexpr (member_index == std::tuple_size_v<members>) { return; }
                else
                {
                    constexpr auto current_member = std::tuple_element_t<member_index, members>{};
                    if constexpr (not associator<std::remove_cvref_t<decltype(current_member)>>)
                    {

                        m_cache.set(current_member, m_mono_presenter.get(current_member));
                    }
                    return self.template operator()<member_index + 1>();
                }
            }();

            // [this]<std::size_t source_member_index = 0>(this auto&& self)
            // {
            //     if constexpr (source_member_index == std::tuple_size_v<members>) { return; }
            //     else
            //     {
            //         using current_source_member = std::tuple_element_t<member_index_
            //         if constexpr(tuple_contains_v<members, current_mem>)
            //     }
            // }();
        }

        auto push_upstream() -> void
        {
            [this]<std::size_t member_index = 0>(this auto&& self)
            {
                if constexpr (member_index == std::tuple_size_v<members>) { return; }
                else
                {
                    constexpr auto current_member = std::tuple_element_t<member_index, members>{};
                    m_mono_presenter.set(current_member, m_cache.get(current_member));
                    return self.template operator()<member_index + 1>();
                }
            };
        }

        auto set(const auto member, auto&& value) -> void
        requires (associator<std::remove_cvref_t<decltype(member)>>)
        {
            m_cache.set(member, std::forward<decltype(value)>(value));
        }

        auto get(const auto member) -> decltype(auto)
        {
            return m_cache.get(member);
        }

        auto exists_link(const auto& supposed_neighbour) const -> bool
        {
            return [this, &supposed_neighbour]<std::size_t member_index = 0>(this auto&& self)
            {

                if constexpr (member_index == std::tuple_size_v<typename metaclass::members>) /* blacklisted metaclass */ { static_assert(false); }
                else
                {
                    using current_member = std::tuple_element_t<member_index, typename metaclass::members>;

                    if constexpr(associator<current_member>)
                    {
                        using member_neighbour_metaclass = typename current_member::value_type::neighbour_metaclass; // domenowa metaklasa sąsiada
                        if constexpr (std::is_same_v<typename std::remove_cvref_t<decltype(supposed_neighbour)>::metaclass::source_metaclass, member_neighbour_metaclass>)
                        {
                            for (const auto& existing_neighbour : std::get<member_index>(m_cache.member_holders))
                            {
                               if (existing_neighbour.get() == &supposed_neighbour)
                               {
                                   return true;
                               }
                            }
                            return false;
                        } else
                        {
                            return self.template operator()<member_index + 1>();
                        }
                    }
                    else
                    {
                        return self.template operator()<member_index + 1>();
                    }
                }
            }();
            // return m_cache.exists_link(supposed_neighbour);
        }

        template<class neighbour> // class member? class neigobject?
        auto insert_link(std::shared_ptr<neighbour> neighbour_ptr) -> void
        {
            [this, neighbour_ptr = std::move(neighbour_ptr)]<std::size_t member_holder_index = 0>(this auto&& self)
            {
                if constexpr (member_holder_index == std::tuple_size_v<typename decltype(m_cache)::member_holders_type>) { static_assert(false); }
                else
                {
                    if constexpr(associator<std::tuple_element_t<member_holder_index, typename metaclass::members>>)
                    {
                        using associator = typename std::tuple_element_t<member_holder_index, typename metaclass::members>;
                        if constexpr
                        (
                            std::is_same_v<typename neighbour::metaclass::source_metaclass, typename associator::value_type::neighbour_metaclass>
                            // std::is_same_v<domain_metaclass_t<neighbour>, typename associator::value_type::neighbour_metaclass>

                        )
                        {
                            // std::cout << "Inserting " << *neighbour_ptr << " as " << neighbour::metaclass::metaclass_name << " into " << metaclass::metaclass_name << "\n";
                            std::get<member_holder_index>(m_cache.member_holders).insert(std::move(neighbour_ptr));
                            return;
                        }
                        else { return self.template operator()<member_holder_index + 1>(); }
                    }
                    else { return self.template operator()<member_holder_index + 1>(); }
                }
            }();
        }

        friend auto operator<<(std::ostream& os, const mono_cached_presenter& mono_cached_presenter) -> std::ostream&
        {
            return os << mono_cached_presenter.m_cache;
        }

    };
}
