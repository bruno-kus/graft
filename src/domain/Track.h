#include "../graft/associator/many.h"
#include "../graft/member.h"
#include "../graft/associator/one.h"

struct DiscNumberedTrack;
struct AlbumNumberedTrack;

struct Track
{
    using nominal_metaclass = Track;
    constexpr static auto metaclass_name = "Track"sv;

    constexpr static auto title = graft::member<"title", std::string>;
    constexpr static auto duration_seconds = graft::member<"duration_seconds", std::size_t>;

    constexpr static auto discNumberedTrack = graft::member<"discNumberedTrack", graft::one<DiscNumberedTrack>>;
    constexpr static auto albumNumberedTrack = graft::member<"albumNumberedTrack", graft::one<AlbumNumberedTrack>>;

    using members = std::tuple
    <
        decltype(Track::title),
        decltype(Track::duration_seconds),
        decltype(Track::discNumberedTrack),
        decltype(Track::albumNumberedTrack)
    >;
};
