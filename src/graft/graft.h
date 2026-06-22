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
// #include "many.h"
#include <ranges>
#include <iostream>

#include "associator/some_associator.h"
using namespace std::literals::string_view_literals;



// namespace graft::detail
// {
//     template<class type>
//     using value_type_t = typename type::value_type;
// }
//

namespace graft::detail
{
    template<class member, template<class> class object_template>
    using to_holder_t = decltype([]
    {
        if constexpr (some_associator<member>)
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
    // template<some_metaclass p_metaclass>
    // struct object : member_buffer<p_metaclass, object>
    // {
    //     using metaclass = p_metaclass;
    //     using member_buffer<p_metaclass, object>::member_buffer;
    // };
}

// namespace graft
// {
//     template<class T>
//     struct members_trait;
// }
// auto fn()
// {
//     using AlbumValue = graft::object<AlbumInfo>;
//     using AlbumValue = graft::object<Album::Info>;


//     graft::ref<graft::object<AlbumInfo>>;
// }
