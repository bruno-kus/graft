#include "../graft/associator/one.h"
#include "../graft/member.h"

struct Disc;
struct Track;

struct DiscNumberedTrack
{

    constexpr static auto metaclass_name = "DiscNumberedTrack"sv;

    constexpr static auto number = graft::member<"number", std::size_t>;

    constexpr static auto track = graft::member<"track", graft::one<Track>>;
    constexpr static auto disc = graft::member<"disc", graft::one<Disc>>;

    using members = std::tuple
    <
        decltype(DiscNumberedTrack::number),
        decltype(DiscNumberedTrack::track),
        decltype(DiscNumberedTrack::disc)
    >;
};
