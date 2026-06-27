#include "../graft/associator/many.h"
#include "../graft/member.h"

struct Artist;
struct AlbumNumberedTrack;
struct Disc;

struct Album
{
    using nominal_metaclass = Album;

    constexpr static auto metaclass_name = "Album"sv;

    constexpr static auto title = graft::member<"title", std::string>;
    constexpr static auto releaseYear = graft::member<"releaseYear", std::size_t>;

    constexpr static auto artists = graft::member<"artists", graft::many<Artist>>;

    constexpr static auto albumNumberedTracks = graft::member<"albumNumberedTracks", graft::many<AlbumNumberedTrack>>;

    constexpr static auto discs = graft::member<"discs", graft::many<Disc>>;


    using members = std::tuple
    <
        decltype(Album::title),
        decltype(Album::releaseYear),
        decltype(Album::artists),

        decltype(Album::albumNumberedTracks),

        decltype(Album::discs)
    >;
};
