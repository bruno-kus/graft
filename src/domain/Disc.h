#include "../graft/associator/many.h"
#include "../graft/member.h"

struct DiscNumberedTrack;
struct Album;

struct Disc
{
    constexpr static auto metaclass_name = "Disc"sv;

    constexpr static auto number = graft::member<"number", std::size_t>;

    constexpr static auto discNumberedTracks = graft::member<"discNumberedTracks", graft::many<DiscNumberedTrack>>;

    constexpr static auto album = graft::member<"album", graft::one<Album>>;

    using members = std::tuple
    <
        decltype(Disc::number),
        decltype(Disc::discNumberedTracks),
        decltype(Disc::album)
    >;
};
