#pragma once

#include "../src/domain/domain-fwd.h"

auto addAlbumNumberedTrack(auto& graph, auto& album, auto& track, auto&&... args) -> auto
{
    const auto albumNumberedTrackPtr = graph.template create<AlbumNumberedTrack>(std::forward<decltype(args)>(args)...);
    graph.add_association(track, *albumNumberedTrackPtr);
    graph.add_association(album, *albumNumberedTrackPtr);

    return albumNumberedTrackPtr;
}

auto addDiscNumberedTrack(auto& graph, auto& disc,  auto& track, auto&&... args) -> auto
{
    const auto discNumberedTrackPtr = graph.template create<DiscNumberedTrack>(std::forward<decltype(args)>(args)...);

    graph.add_association(disc, *discNumberedTrackPtr);
    graph.add_association(track, *discNumberedTrackPtr);

    return discNumberedTrackPtr;
}
