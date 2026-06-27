#include "../thirdparty/doctest/doctest.h"
#include "../src/graft/graph.h"
#include "../src/domain/domain-fwd.h"
#include "../src/graft/object.h"
#include "test-util.h"



TEST_CASE("graft::directed_edge_exists_v")
{
    using Graph = graft::graph<graft::object, std::type_identity_t, Artist, Album, AlbumNumberedTrack, Disc, DiscNumberedTrack, Track>;
    Graph graph;

    static_assert(graft::directed_edge_exists_v<Artist, Album>);
    static_assert(graft::directed_edge_exists_v<Album, Artist>);
}
TEST_CASE("graft::graft<graft::object, ...>")
{
    using Graph = graft::graph<graft::object, std::type_identity_t, Artist, Album, AlbumNumberedTrack, Disc, DiscNumberedTrack, Track>;
    Graph graph;


    auto artistPtr = graph.create<Artist>("Stevie Wonder");

    auto album1Ptr = graph.create<Album>("Songs In The Key Of Life", 1976);

    auto album2Ptr = graph.create<Album>("Extra Album", 2000);

    auto disc1Ptr = graph.create<Disc>(1);
    auto disc2Ptr = graph.create<Disc>(2);

    auto track1_1Ptr = graph.create<Track>("Love's In Need Of Love Today");
    auto track1_2Ptr = graph.create<Track>("Have A Talk With God");

    auto track2_1Ptr = graph.create<Track>("Isn't She Lovely");
    auto track2_2Ptr = graph.create<Track>("Joy Inside My Tears");

    graph.add_association(*artistPtr, *album1Ptr);
    graph.add_association(*artistPtr, *album2Ptr);

    graph.add_association(*album1Ptr, *disc1Ptr);
    graph.add_association(*album1Ptr, *disc2Ptr);

    addDiscNumberedTrack(graph, *disc1Ptr,  *track1_1Ptr, 1);
    addDiscNumberedTrack(graph, *disc1Ptr,  *track1_2Ptr, 2);

    addDiscNumberedTrack(graph, *disc2Ptr,  *track2_1Ptr, 1);
    addDiscNumberedTrack(graph, *disc2Ptr,  *track2_2Ptr, 2);

}


// template<template<class, template<class> class> class, template<class> class, class...> class graph_template, template<class, template<class>> class object_template_arg, template<class> class >
// template
// <
//     template<class, template<class>> class object_template_arg,
//     template<class> class effective_metaclass_template,
//     class... metaclasses
// >

// i teraz to mogę podawać jako source graph type, ale...
/*
 * problem nadal leży w tym, że te ograniczniea tutaj zaimplementowane
 * nie będą się odnosiły do tego nowo utworzonego grafu
 * ale to ma sens, ponieważ te
 */

/*
 * TODO:
 */
