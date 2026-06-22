#include <iostream>
#include "src/util/static-string.hpp"
#include <tuple>
#include <vector>
// util
template<class tuple, auto lambda>
using tuple_transform = decltype([]
{
  constexpr auto transform_type_t = []<size_t index>()static
  {
    using current_element_type = std::tuple_element_t<index, tuple>;
    using transformed = decltype(lambda.template operator()<current_element_type>())::type;
    return std::type_identity<transformed>{};
  };

  constexpr auto result_t = []<size_t... indexes>(std::index_sequence<indexes...>)static
  {
    using transformed = std::tuple<typename decltype(transform_type_t.template operator()<indexes>())::type...>;
    return std::type_identity<transformed>{};
  };

  return result_t(std::make_index_sequence<std::tuple_size_v<tuple>>());
}())::type;

template<class tuple, auto lambda>
// using tuple_for_each

// /util

namespace graft
{
    enum class access { PUBLIC, PRIVATE };

    template<static_string a_name, class a_value_type, access a_access_specifier= access::PUBLIC>
    struct member
    {
        constexpr static auto name = a_name;
        using value_type = a_value_type;
        constexpr static auto access_specifier = a_access_specifier;
    };


    template<class some_type>
    struct is_member : std::false_type {};
    template<static_string name, class value_type, access access_specifier>
    struct is_member<member<name, value_type, access_specifier>> : std::true_type {};
    template<class some_type>
    concept some_member = is_member<std::remove_cvref_t<some_type>>::value;
}

namespace graft
{
    struct associator_t {};
}
namespace graft
{
    template <class some_type>
    struct one
    {
        using associator = associator_t;
        // using neighbour_type = some_type;
        template<template<class> class object_template>
        struct holder
        {
            object_template<some_type>* m_neighbour;
        };
    };

    template<class some_type>
    struct is_one : std::false_type {};

    template<class some_type>
    struct is_one<one<some_type>> : std::true_type {};

    template<class some_type>
    concept some_one = is_one<some_type>::value;
}

namespace graft
{

    template <class some_type>
    struct many
    {
        using associator = associator_t;
        // using neighbour_type = some_type;

        template<template<class> class object_template>
        struct holder
        {
            std::vector<object_template<some_type>*> m_neighbours;
        };
    };

    template<class some_type>
    struct is_many : std::false_type {};

    template<class some_type>
    struct is_many<many<some_type>> : std::true_type {};

    template<class some_type>
    concept some_many = is_many<some_type>::value;
}


namespace Album { struct Spec; }
namespace Artist
{
    struct Spec
    {
        using type =  std::tuple
        <
            graft::member<"albums", graft::many<Album::Spec>>,
            graft::member<"name", std::string>
        >;
    };
};

namespace Album
{
    constexpr static auto releaseYear = graft::member<"releaseYear", std::size_t>{};
    constexpr static auto title = graft::member<"title", std::string>{};
    constexpr static auto artists = graft::member<"artists", graft::many<Artist::Spec>>{};

    struct Spec
    // : std::type_identity
    // <
    //     std::tuple
    //     <
    //         decltype(release_year),
    //         decltype(title),
    //         decltype(artists)
    //     >
    // >
    { using type = std::tuple
    <
        decltype(releaseYear),
        decltype(title),
        decltype(artists)
    >; };

};

using x = Album::Spec::type;

namespace graft
{
    template<class type>
    concept associator = requires
    {
         typename type::associator;
    };
}
namespace graft::detail
{
    // template<class


    template<some_member member, template<class> class object_template>
    using to_holder_t =  decltype([]
    {
        if constexpr (some_associator<member>)
        {
            return std::type_identity<typename member::template holder<object_template>>{};
        }
        else
        {
            return std::type_identity<member>{};
        }
    }())::type;
}
namespace graft
{

    template<class object_descriptor>
    struct object
    {


        auto get(this auto&& self, const auto selector) -> decltype(auto)
        {
            constexpr auto index = index_for(selector);
            return std::get<index>(self.members_tuple);
        }

        private:
        using members_tuple_type = tuple_transform
        <
            typename object_descriptor::type,
            []<class type> { return std::type_identity<detail::to_holder_t<type, object>>{}; }
        >;
        members_tuple_type members_tuple;


        consteval auto static index_for(const auto selector) -> size_t
        {
          return []<size_t I = 0>(this auto self)
          {
            if constexpr (I == std::tuple_size_v<members_tuple_type>) { static_assert(false); }
            else if constexpr (std::is_same_v<decltype(selector), std::tuple_element_t<I, members_tuple_type>>) { return I; }
            else { return self.template operator()<I + 1>(); }
          }();
        }
    };
}


// template<class value_type>
// struct Test
// {
//     using value_type = value_type;
// };


// sprawdzić czy nie mógłbym w ten sposób zwracać rzeczy zamias
// []<class type> () -> detail::to_holder_t<type, object>>

template<class T>
struct InfoTest
{
    using type = typename T::Info;
};

struct AlbumTest;

struct ArtistTest
{
    using neighbour_type = InfoTest<AlbumTest>::type;
};

struct AlbumTest
{
    using Info = int;
};


int main()
{
    std::vector<int> vi;
    typename Album::Spec::type;
    graft::object<Artist::Spec> artist;
    graft::object<Album::Spec> album;
    // album.get(Album::releaseYear);
    // album.set(Album::releaseYear, 2003);
    // album.set(AlbumInfo::releaseYear, 2003);
    std::cout << "Hello, world" << '\n';
}
