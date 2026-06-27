#pragma once

#include "../util/static-string.hpp"

using namespace std::literals::string_view_literals;
namespace graft
{


    // template<class T>
    // struct domain_metaclass { using type = typename T::metaclass; }; // ?

    // template<class T>
    // using domain_metaclass_t = domain_metaclass<T>::type; // ?




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
