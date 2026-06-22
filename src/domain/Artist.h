#include "../graft/member.h"

#include "../graft/associator/many.h"

struct Album;

struct Artist
{
    constexpr static auto metaclass_name = "Artist"sv;

    constexpr static auto name = graft::member<"name", std::string, graft::visibility::k_public>;
    constexpr static auto albums = graft::member<"albums", graft::many<Album>, graft::visibility::k_public>;

    using members = std::tuple
    <
        decltype(Artist::name),
        decltype(Artist::albums)
    >;
};
