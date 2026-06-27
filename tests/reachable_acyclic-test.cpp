#include "../thirdparty/doctest/doctest.h"

#include "../src/graft/reachable_acyclic.h"
#include "../src/graft/graph.h"
#include "../src/graft/object.h"
#include "../src/graft/erased_mono_cached_presenter.h"

#include "test-util.h"
#include <iostream>
#include <sstream>



TEST_CASE("graft::compute_member_blacklist")
{
    using metaclasses_tuple = std::tuple<Artist, Album, AlbumNumberedTrack, Disc, DiscNumberedTrack, Track>;
    constexpr static auto member_blacklist = graft::compute_member_blacklist<metaclasses_tuple, Album>();

    std::stringstream ss;
    [&ss]<std::size_t blacklist_index = 0>(this auto&& self)
    {
        if constexpr (blacklist_index == member_blacklist.size()) { return; }
        else
        {
            constexpr auto member_location = member_blacklist.at(blacklist_index);
            using metaclass = std::tuple_element_t<member_location.metaclass_index, metaclasses_tuple>;
            constexpr auto metaclass_name = metaclass::metaclass_name;
            constexpr auto member_name = std::tuple_element_t<member_location.member_index, typename metaclass::members>::name;
            ss << metaclass_name << "::" << member_name << '\n';
            return self.template operator()<blacklist_index + 1>();
        }
    }();
    std::cout << ss.str() <<'\n';
}

TEST_CASE("graft::make_reachable_acyclic_copy | graft::patch")
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


    auto mono_cached_graph = make_reachable_acyclic_copy<graft::graph, graft::erased_mono_cached_presenter>(graph, album1Ptr);

    std::cout << "mono_cached_graph:" << '\n';
    std::cout << mono_cached_graph << '\n';
}


TEST_CASE("graft::make_reachable_acyclic_copy | graft::graph")
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


    auto copy = make_reachable_acyclic_copy<graft::graph, graft::object>(graph, album1Ptr); // some_graph
    std::cout << copy << '\n';

    // using copy_artist_metaclass = decltype(copy.create<Artist>())::element_type::metaclass;
    // using copy_album_metaclass = decltype(copy.create<Album>())::element_type::metaclass;

    // using copy_disc_metaclass = decltype(copy.create<Disc>())::element_type::metaclass;

    // static_assert(graft::directed_edge_exists_v<copy_album_metaclass, copy_artist_metaclass>);
    // static_assert(not graft::directed_edge_exists_v<copy_artist_metaclass, copy_album_metaclass>);
    // static_assert(graft::directed_edge_exists_v<copy_disc_metaclass, copy_album_metaclass>);
    // static_assert(graft::directed_edge_exists_v< copy_album_metaclass, copy_disc_metaclass>);

}

TEST_CASE("graft::make_perfect_copy | graft::patch")
{

}

TEST_CASE("graft::make_perfect_copy | graft::graph")
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

    // co teraz




}
