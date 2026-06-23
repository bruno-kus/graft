#include "../thirdparty/doctest/doctest.h"

#include "../src/graft/reachable_acyclic.h"
#include "../src/graft/graph.h"
#include "../src/graft/object.h"

#include "test-util.h"
#include <iostream>
#include <sstream>


TEST_CASE("graft::make_reachable_acyclic_copy | graft::patch")
{

}

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

    // teraz chcę zrobić kopię grafu, ale w taki sposób, że niedostępne są nieobecne!
    // tak, to jest ważny krok!
    // i grafowi mogę normalnie podać parametr, który to tłumaczy... ale taki parametry nie może być wewnątrz funkcji!

    auto copy = make_reachable_acyclic_copy<graft::graph, graft::object>(graph, track1_1Ptr); // some_graph
    std::cout << copy << '\n';
    // static_assert(std::is_same_v<decltype(copy), void>);

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

    // co teraz?




}
