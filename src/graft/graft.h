#pragma once

// co jeżeli miałbym constant wrapper, w taki sposób
// żebym mógł rzutować member_t do wartości?
// i wtedy mógłbym po kropce mieć dostęp do wszystkich danych
// wtedy brałbym jako parameter coś w stylu cw<member>
// i rzutowanie do wartości za każdym razem odnosiłoby do stałej?
//
//
// template<class class_info>
//
#include "../util/static-string.hpp"
#include "../util/meta.hpp"
#include <type_traits>
#include <ostream>
#include "many.h"
#include <ranges>
#include <iostream>
using namespace std::literals::string_view_literals;

namespace graft
{


    template<class T>
    struct domain_metaclass { using type = typename T::metaclass; };

    template<class T>
    using domain_metaclass_t = domain_metaclass<T>::type;


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

    enum class visibility { k_public, k_private };

    template<static_string p_name, class p_value_type, visibility p_visibility = visibility::k_public>
    struct member_t
    {
        constexpr static auto name = p_name;
        using value_type = p_value_type;
        constexpr static auto visibility = p_visibility;
    };

    template<static_string name, class value_type, visibility visibility = visibility::k_public>
    constexpr member_t member = member_t<name, value_type, visibility>{};

    template<class type>
    struct is_member : std::false_type {};

    template<static_string name, class value_type, visibility visibility>
    struct is_member<member_t<name, value_type, visibility>> : std::true_type {};

    template<class type>
    concept some_member = is_member<std::remove_cvref_t<type>>::value;

}

// namespace graft::detail
// {
//     template<class type>
//     using value_type_t = typename type::value_type;
// }
//

namespace graft
{
    template<class type>
    concept associator = requires
    {
        typename type::value_type:: associator;
    };
}
namespace graft::detail
{
    template<class member, template<class> class object_template>
    using to_holder_t = decltype([]
    {
        if constexpr (associator<member>)
        {
            return std::type_identity<typename member::value_type::template holder<object_template>> {};
        }
        else
        {
            return std::type_identity< typename member::value_type> {};
        }
    }())::type;
}

// obiekt musi mieć members
// friend auto operator

// opcja 1: object ma listę members bezpośrednio
// powinienem zejść poziom niżej i wymagać typów, które mają metaklasę
namespace graft
{
    template<class type>
    concept some_metaclass = true;

    template<class p_metaclass, template<class> class neighbour_object_template>
    struct object_body
    {
        using metaclass = p_metaclass;

        object_body(auto&&... args)
        {
            // i teraz tu w środku robię static assert na wszystkich! żadnego optional coś tam gówno

            // iteruj aż dojdę do końca, jeżeli napotkam na nullopt to continue

            if constexpr (sizeof...(args) > 0)
            {
                [this]<std::size_t member_index = 0, std::size_t arg_index = 0>(this auto&& self, auto&&... args)
                {
                    constexpr bool member_index_past_last = (member_index == std::tuple_size_v<member_holders_type>);
                    constexpr bool arg_index_past_last = (arg_index == sizeof...(args));

                    if constexpr(member_index_past_last or arg_index_past_last) { return; }
                    else
                    {
                        using current_member = std::tuple_element_t<member_index, typename metaclass::members>;
                        if constexpr (associator<current_member>)
                        {
                            return self.template operator()<member_index + 1, arg_index>(std::forward<decltype(args)>(args)...);
                        }
                        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(args...[arg_index])>, std::nullopt_t>)
                        {
                            return self.template operator()<member_index + 1, arg_index + 1>(std::forward<decltype(args)>(args)...);
                        }
                        else
                        {
                            std::get<member_index>(member_holders) = std::forward<decltype(args...[arg_index])>(args...[arg_index]);
                            return self.template operator()<member_index + 1, arg_index +1>(std::forward<decltype(args)>(args)...);
                        }
                    }
                }(std::forward<decltype(args)>(args)...);
            }
        }


        friend auto operator<<(std::ostream& os, const object_body& object) -> std::ostream&
        {
            os << "{";
            // iteruj po memberach
            [&os, &object]<std::size_t I = 0>(this auto&& self)
            {
                if constexpr (I == std::tuple_size_v<typename metaclass::members>) { return;}
                else
                {
                    using current_member = std::tuple_element_t<I, typename metaclass::members>;
                    if constexpr (associator<current_member>)
                    {
                        os << " " << (current_member::name) << " : " << std::get<I>(object.member_holders).size() <<",";
                        return self.template operator()<I + 1>();
                    }
                    else
                    {
                        // static_assert(std::is_same_v<decltype(object.member_holders), void>);
                        // static_assert(not some_many<current_member>);
                        os << " " << (current_member::name) << " : " << std::get<I>(object.member_holders) << ",";
                        return self.template operator()<I + 1>();
                    }
                }
            }();
            os << "}";
            return os;
        }


        auto exists_link(const auto& supposed_neighbour) const -> bool // mono_cahed_presenter<...>
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
                        if constexpr (std::is_same_v<typename std::remove_cvref_t<decltype(supposed_neighbour)>::metaclass, member_neighbour_metaclass>)
                        {
                            for (const auto& existing_neighbour : std::get<member_index>(member_holders))
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
        }

        template<class neighbour> // class member? class neigobject?
        auto insert_link(std::shared_ptr<neighbour> neighbour_ptr) -> void
        {
            [this, neighbour_ptr = std::move(neighbour_ptr)]<std::size_t member_holder_index = 0>(this auto&& self)
            {
                if constexpr (member_holder_index == std::tuple_size_v<member_holders_type>) { static_assert(false); }
                else
                {
                    if constexpr(associator<std::tuple_element_t<member_holder_index, typename metaclass::members>>)
                    {
                        using associator = typename std::tuple_element_t<member_holder_index, typename metaclass::members>;
                        if constexpr
                        (
                            // std::is_same_v<typename neighbour::metaclass, typename associator::value_type::neighbour_metaclass>
                            std::is_same_v<domain_metaclass_t<neighbour>, typename associator::value_type::neighbour_metaclass>

                        )
                        {
                            // std::cout << "Inserting " << *neighbour_ptr << " as " << neighbour::metaclass::metaclass_name << " into " << metaclass::metaclass_name << "\n";
                            std::get<member_holder_index>(member_holders).insert(std::move(neighbour_ptr));
                            return;
                        }
                        else { return self.template operator()<member_holder_index + 1>(); }
                    }
                    else { return self.template operator()<member_holder_index + 1>(); }
                }
            }();
        }

        template<class member>
        requires (tuple_contains_v<typename metaclass::members, const member>)
        auto get(this auto&& self, member) -> decltype(auto)
        {
            constexpr auto member_index = get_member_index<member>();
            return std::get<member_index>(self.member_holders);
        }

        template<class member>
        requires (tuple_contains_v<typename metaclass::members, const member>) and (not associator<std::remove_cvref_t<member>>)
        auto set(member, auto value) -> void
        {
            constexpr auto member_index = get_member_index<member>();
            std::get<member_index>(member_holders) = std::move(value);
        }

        // private: public:
        using member_holders_type = tuple_transform_t
        <
            typename metaclass::members,
            []<class member> { return std::type_identity<detail::to_holder_t<member, neighbour_object_template>>{}; }
        >;

        member_holders_type member_holders{};

        template<class member>
        consteval static auto get_member_index() -> size_t
        {
            return []<std::size_t I = 0>(this auto&& self) -> std::size_t
            {
                if constexpr (I == std::tuple_size_v<typename metaclass::members>) { static_assert(false); }
                else if constexpr (std::is_same_v<std::remove_cvref_t<member>, std::remove_cvref_t<std::tuple_element_t<I, typename metaclass::members>>>) { return I; }
                else { return self.template operator()<I + 1>(); }
            }();
        }
    };
    template<some_metaclass p_metaclass>
    struct object : object_body<p_metaclass, object>
    {
        using metaclass = p_metaclass;
        using object_body<p_metaclass, object>::object_body;
    };
}

namespace graft
{
    template<class T>
    struct members_trait;
}
// auto fn()
// {
//     using AlbumValue = graft::object<AlbumInfo>;
//     using AlbumValue = graft::object<Album::Info>;


//     graft::ref<graft::object<AlbumInfo>>;
// }
