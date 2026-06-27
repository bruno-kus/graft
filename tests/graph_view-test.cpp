#include "../thirdparty/doctest/doctest.h"
#include "../src/domain/DomainGraph.h"
#include "../src/graft/erased_mono_cached_presenter.h"
#include "../src/graft/reachable_acyclic.h"

#include "test-util.h"
#include <ranges>

// template<class A>
// void fn()
// {

// }
TEST_CASE("graft::graph_view")
{
    // fn<struct {int a; char b;}>();
    using Graph = DomainGraph<graft::object, std::type_identity_t, Artist, Album, AlbumNumberedTrack, Disc, DiscNumberedTrack, Track>;

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

    auto graph_view = graft::make_reachable_acyclic_graph_view<graft::erased_mono_cached_presenter>(graph, album1Ptr);


    // tu widzę, że rozbieżność się rozrasta, ale to w porządku
    // będziemy pisać przeciążenia make_reachable_acyclic
    // auto graph_view = graft::make_reachable_acyclic_copy<graft::graph_view, DomainGraph, graft::erased_mono_cached_presenter>();
    // auto graph_view = graft::make_reachable_acyclic_copy<DomainGraphView>();
    // auto graph_view = graft::make_reachable_acyclic_copy<DomainGraphBufferedView>();


    // teoretycznie mógłbym dla zasady parametryzować DomainGraphView na cached prezenterach?
    // tylko, co jeżeli będę miał dwie implementacje
    // jedną z prezenterami bezpośrednimi, a drugą z mapą
    // to tak czy siak muszę najpierw zaimplementować logikę, a potem odważnie wpisywać przeciążenia, kiedy wiem już *NA CZYM* są przeciążane...
    /*
     * nie ma co udawać, że rzeczy są te same
     * ave wizytor...
     * dla algorytmu
     */

    std::cout << "graph_view:\n" << graph_view << '\n';

    std::cout << graph_view.m_dirty_objects.size() << '\n';

    // wygląda dobrze :)
    // to teraz pora na update'owanie do góry :o
    SUBCASE("push_upstream | create")
    {
        // auto graph_copy = graph;
        graph_view.create<Album>("Innervisions");
        graph_view.push_upstream();
        // teraz musz
        const auto& graph_view_albums = graph_view.get_storage<Album>().m_objects;
        const auto& graph_albums = graph.get_storage<Album>().m_objects;

        auto p = &decltype(graph)::get_storage<Album>; // todo mega ważne

        auto it1 = std::ranges::find_if(graph_view_albums, [](const auto& album_ptr){ return album_ptr->get(Album::title) == "Innervisions"; });
        auto it2 = std::ranges::find_if(graph_albums, [](const auto& album_ptr){ return album_ptr->get(Album::title) == "Innervisions"; });
        // auto it = std::ranges::find(graph_albums, nullptr);
        bool found_in_graph_view = it1 != graph_view_albums.end();
        bool found_in_graph = it2 != graph_albums.end();
        REQUIRE(found_in_graph_view == found_in_graph);



        // REQUIRE(compare_graphs(graph, graph_view));

    };
}
