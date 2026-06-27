#include "../graft/associator/many.h"
#include "../graft/associator/one.h"
#include "../graft/member.h"

struct Track;
struct Album;


struct AlbumNumberedTrack
{

    using nominal_metaclass = AlbumNumberedTrack;

    constexpr static auto metaclass_name = "AlbumNumberedTrack"sv;

    constexpr static auto number = graft::member<"number", std::size_t>;
    constexpr static auto track = graft::member<"track", graft::one<Track>>;

    constexpr static auto album = graft::member<"album", graft::one<Album>>;

    using members = std::tuple
    <
        decltype(AlbumNumberedTrack::number),
        decltype(AlbumNumberedTrack::track),
        decltype(AlbumNumberedTrack::album)
    >;
};
