#include "src/graft/graft.h"
#include "src/graft/graph.h"
#include "src/domain/Album.h"
#include "src/domain/Artist.h"
#include "src/domain/Track.h"
#include "src/domain/Disc.h"
#include "src/domain/AlbumNumberedTrack.h"
#include "src/domain/DiscNumberedTrack.h"
#include <iostream>
#include "src/graft/presenter.h"
#include "src/graft/cached_presenter.h"
#include "src/graft/patch.h"

#include <cassert>
#include <sstream>


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

// void inferAlbumNumbering(auto& graph, const auto& album)
// {
//      auto presenters = graph.present<Album, Disc, DiscNumberedTrack>(album);

//      std::size_t albumTrackNumber = 0;
//      for (const auto& presenter : presenters)
//      {
//          ++albumTrackNumber;

//          const auto& albumNumberedTrackPtr = presenter.get(Disc::discNumberedTrack);
//          albumNumberedTrackPtr->set(AlbumNumberedTrack::number, albumTrackNumber);
//      }

//      // tylko tutaj problem jest taki, że nie ma transakcji!
//      // a ja chciałem transakcję, żebym mógł na przykład cofnąć
//      // no bo teraz tej akcji to w zasadzie nie da się cofnąć w ogóle! W OGÓLE!
// }

void inferAlbumNumberingTransactional(auto& graph, const auto& album)
{
    auto proxy = graph.template make_proxy<Album, Disc, DiscNumberedTrack>(album); // za mało!
    // i teraz na tymjjj
}


auto prepare_graph() -> auto
{
    using Graph = graft::graph<graft::object, Artist, Album, Disc, Track, AlbumNumberedTrack, DiscNumberedTrack>;

    Graph graph;

    auto artist_ptr = graph.create<Artist>("Stevie Wonder");

    auto album_ptr = graph.create<Album>("Songs In The Key Of Life", 1976);

    auto disc1_ptr = graph.create<Disc>(1); // tego nie chcę!
    auto disc2_ptr = graph.create<Disc>(2);

    auto track1_1_ptr = graph.create<Track>("Love's In Need Of Love Today");
    auto track1_2_ptr = graph.create<Track>("Have A Talk With God");

    auto track2_1_ptr = graph.create<Track>("Isn't She Lovely");
    auto track2_2_ptr = graph.create<Track>("Joy Inside My Tears");

    graph.add_association(*artist_ptr, *album_ptr);

    graph.add_association(*album_ptr, *disc1_ptr);
    graph.add_association(*album_ptr, *disc2_ptr);

    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_2_ptr, 2);

    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_2_ptr, 2);

    return graph;
}
void test_associations()
{
    using Graph = graft::graph<graft::object, Artist, Album, Disc, Track, AlbumNumberedTrack, DiscNumberedTrack>;

    Graph graph;

    auto artist_ptr = graph.create<Artist>("Stevie Wonder");

    auto album_ptr = graph.create<Album>("Songs In The Key Of Life", 1976);

    auto disc1_ptr = graph.create<Disc>(1); // tego nie chcę!
    auto disc2_ptr = graph.create<Disc>(2);

    auto track1_1_ptr = graph.create<Track>("Love's In Need Of Love Today");
    auto track1_2_ptr = graph.create<Track>("Have A Talk With God");

    auto track2_1_ptr = graph.create<Track>("Isn't She Lovely");
    auto track2_2_ptr = graph.create<Track>("Joy Inside My Tears");

    graph.add_association(*artist_ptr, *album_ptr);

    graph.add_association(*album_ptr, *disc1_ptr);
    graph.add_association(*album_ptr, *disc2_ptr);

    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_2_ptr, 2);

    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_2_ptr, 2);
}

void test_project()
{
    using Graph = graft::graph<graft::object, Album, Artist>;
    Graph graph;

    auto album_ptr = graph.create<Album>("Rubber Souls", 1967);
    auto artist_ptr = graph.create<Artist>("The Beatles");

    // idea jest taka, że najpierw robię złączenie, a potem je filtruję?
    // no chyba tak...
    // graph.project_from<Album>().to<Artist>();
    // graph.project<Album, Artist>(); // idea jest taka, że mogę w różne strony chodzić...
    // zacznijmy od najprostszej wersji,czyli parameter pack w kolejności
    // i wtedy mogę przeglądać wszystkie albumy wraz z tą tam resztą...
    // tylko idea jest taka, że mi się rzeczy poszerzają jako drzewa...
    // no bo jeden album ma wiele dysków
    // ale o to właśnie chodzi
    // to jest joinowanie po prostu
    // czyli jeżeli jeden artysta ma wiele albumów
    // to dostaję Beatles — Abir, Beatles — Rubbers,
    // hmm...
    // czy jest różnica między Artist, Album, Collection? a Collection, Album, Artist?
    // jest w kolejności iteracji
    // tylko do czego to miało być przydatne? do zapytań głównie
    // bo ja chciałem jakiś use case mieć
    // właśnie, to zacznijmy od numbering service!
    //
}

// void test_cached_presenter()
// {
//     using Graph = graft::graph<graft::object, Album, Artist>;
//     Graph graph;

//     auto album_ptr = graph.create<Album>("Rubber Souls", 1967);
//     auto artist_ptr = graph.create<Artist>("The Beatles");

//     std::cout << *album_ptr << '\n';
//     std::cout << *artist_ptr << '\n';

//     auto presenter = graft::presenter<graft::object<Album>, graft::object<Artist>>{ album_ptr, artist_ptr }; // możliwe, że prezenter powinien być type erased...

//     // static_assert(std::is_same_v<decltype(Album::title), std::tuple_element_t<0, Album::members>>);
//     // album_ptr->set(Album::title, "Revolver");
//     // album_ptr->set(Album::releaseYear, 1969);
//     // static_assert(tuple_contains_v<typename Artist::members, decltype(Artist::name)>);
//     // artist_ptr->set(Artist::name, "Beatles");
//     // presenter.set(Album::title, "Revolver");
//     // presenter.set(Album::releaseYear, 1969);
//     // presenter.set(Artist::name, "Beatles");

//     // auto value = "Beatles";e
//     // constexpr bool r = requires (std::tuple_element_t<0, decltype(presenter)::source_ptrs_tuple>&& current_source_ptr)
//     // {
//     //     current_source_ptr->set(Artist::name, std::move(value));
//     // };
//     // static_assert(std::is_same_v<std::shared_ptr<graft::object<Album>>, std::tuple_element_t<0, decltype(presenter)::source_ptrs_tuple>>);
//     // static_assert(not requires (std::shared_ptr<graft::object<Album>>&& source_ptr) { source_ptr->set(Artist::name, value); });
//     // static_assert(r);
//     // album_ptr->set(Artist::name, value);

//     std::cout << *album_ptr << '\n';

//     std::cout << *artist_ptr << '\n';

//     auto cached_presenter = graft::cached_presenter
//     <
//         std::tuple<graft::object<Album>, graft::object<Artist>>,
//         std::tuple<decltype(Album::title), decltype(Album::releaseYear), decltype(Artist::name)>
//     >{ album_ptr, artist_ptr };

//     std::cout << "cached_presenter: " << cached_presenter << '\n';

//     cached_presenter.set(Album::title, "Abbey Road");
//     cached_presenter.set(Album::releaseYear, 1970);
//     cached_presenter.set(Artist::name, "Subibi");

//     std::cout << "cached_presenter: " << cached_presenter << '\n';

//     std::cout << *album_ptr << '\n';
//     std::cout << *artist_ptr << '\n';

//     std::cout << "push_upstream()" << '\n';
//     cached_presenter.push_upstream();

//     std::cout << "cached_presenter: " << cached_presenter << '\n';

//     std::cout << *album_ptr << '\n';
//     std::cout << *artist_ptr << '\n';
// }

void test_compute_edge_count()
{
    constexpr static auto edge_count = graft::compute_edge_count<Album, Disc, AlbumNumberedTrack, DiscNumberedTrack, Track>();
    std::cout << "edge count : " << edge_count << '\n';
}

void test_compute_edges()
{
    constexpr auto edges = graft::get_edges<Album, Disc, Track, DiscNumberedTrack, AlbumNumberedTrack>();

    using EdgeInput = std::tuple<Album, Disc, Track, DiscNumberedTrack, AlbumNumberedTrack>;

    [&edges]<std::size_t I = 0>(this auto&& self)
    {
        if constexpr (I == edges.size()) { return; }
        else
        {
            std::cout << "(" << std::tuple_element_t<edges.at(I).left_class_index, EdgeInput>::metaclass_name << ", "
                      << std::tuple_element_t<edges.at(I).right_class_index, EdgeInput>::metaclass_name << ")";

            std::cout << ", (" << std::tuple_element_t<edges.at(I).left_to_right_associator_index, typename std::tuple_element_t<edges.at(I).left_class_index, EdgeInput>::members>::name << ", "
                      << std::tuple_element_t<edges.at(I).right_to_left_associator_index, typename std::tuple_element_t<edges.at(I).right_class_index, EdgeInput>::members>::name << ")";
            std::cout << '\n';

            return self.template operator()<I + 1>();
        }
    }();
}

void test_clone_reachable()
{

    using Graph = graft::graph<graft::object, Artist, Album, Disc, Track, AlbumNumberedTrack, DiscNumberedTrack>;

    Graph graph;

    auto artist_ptr = graph.create<Artist>("Stevie Wonder");

    auto album_ptr = graph.create<Album>("Songs In The Key Of Life", 1976);

    auto album_2_ptr = graph.create<Album>("Extra Album", 2000);

    auto disc1_ptr = graph.create<Disc>(1); // tego nie chcę!
    auto disc2_ptr = graph.create<Disc>(2);

    auto track1_1_ptr = graph.create<Track>("Love's In Need Of Love Today");
    auto track1_2_ptr = graph.create<Track>("Have A Talk With God");

    auto track2_1_ptr = graph.create<Track>("Isn't She Lovely");
    auto track2_2_ptr = graph.create<Track>("Joy Inside My Tears");

    graph.add_association(*artist_ptr, *album_ptr);
    graph.add_association(*artist_ptr, *album_2_ptr);

    graph.add_association(*album_ptr, *disc1_ptr);
    graph.add_association(*album_ptr, *disc2_ptr);


    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_2_ptr, 2);

    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_2_ptr, 2);

    assert(graph.exists_association(*artist_ptr, *album_ptr));

    // assert(graph.exists_association(*album_ptr, *disc1_ptr));
    // assert(graph.exists_association(*album_ptr, *disc2_ptr));

    // auto cloned_graph = graft::clone_reachable(graph, album_ptr);

    auto cloned_graph = graft::clone_reachable(graph, track1_1_ptr);
    auto cloned_graph_acyclic = graft::clone_reachable_acyclic(graph, track1_1_ptr);
// teraz będę musiał sprawdzić to na większym grafie !
    std::cout << "graph\n" << graph << '\n';
    std::cout << "cloned_graph\n"  << cloned_graph << '\n';
    std::cout << "cloned_graph_acyclic\n" << cloned_graph_acyclic << '\n';

    std::stringstream ss;
    ss << graph;
    auto graph_str = ss.str();

    ss = std::stringstream{};
    ss << cloned_graph;
    auto cloned_graph_str = ss.str();

    ss = std::stringstream{};
    ss << cloned_graph_acyclic;
    auto cloned_graph_acyclic_str =ss.str();


    std::cout << "(cloned_graph == cloned_graph_acyclic)" << std::boolalpha << (cloned_graph == cloned_graph_acyclic) << '\n';
    std::cout << "(cloned_graph_str == cloned_graph_acyclic_str)" << std::boolalpha << (cloned_graph_str == cloned_graph_acyclic_str) << '\n';

    std::cout << "(graph == cloned_graph)" << std::boolalpha << (graph == cloned_graph) << '\n';
    std::cout << "(graph_str == cloned_graph_str)" << std::boolalpha << (graph_str == cloned_graph_str) << '\n';

    std::cout << "acyclic num albums: " << std::get<1>(cloned_graph_acyclic.m_storages).m_objects.size() << '\n';

    // przygotuje graf
    // sklonuj graf
    // porównaj obydwa grafy :D
    // jeszcze by się przydał jakiś hash dla całego grafu
}

void test_creating_objects()
{
    using Graph = graft::graph<graft::object, Album>;

    Graph graph;
    // auto album_ptr = graph.create<Album>(2000);
    // std::cout << *album_ptr << '\n';

    // PRZYPADEK 1: dajemy wszystkie parametry
    {
        auto album_ptr = graph.create<Album>("albumTitle", 2026);
        std::cout << *album_ptr << '\n';
    }

    // PRZYPADEK 2: za początkowe wstawiamy std::nullopt
    {
        auto album_ptr = graph.create<Album>(std::nullopt, 2026);
        std::cout << *album_ptr << '\n';
    }
    // PRZYPADEK 3: pomijamy końcowe parametry

    {
        auto album_ptr = graph.create<Album>("albumTitle");
        std::cout << *album_ptr << '\n';
    }
    // PRZYPADEK 4 (BŁĄD): pomijamy pierwsze parametry
    {
        // -> tutaj musiałbym mieć na poziomie sygnatury i deklaracji requires właściwie powtórzoną logikę konstrukotra
        // co jest w gruncie rzeczy uczciwym pomysłem, ale teraz nie chce mi się implementować
        // const auto fn = [&graph](auto&& arg) { graph.create<Album>(std::forward<decltype(arg)>(arg)); };
        // constexpr bool failure = std::invocable<decltype(fn), std::vector<int>>;
        // static_assert(not failure);
        // std::cout << std::boolalpha << failure << '\n';
    }

    // std::string s;
    // s = 2026;
    // std::tuple<std::string> t;
    // std::get<0>(t) = 2026;
}

void test_acyclic_patch()
{
    using Graph = graft::graph<graft::object, Artist, Album, Disc, Track, AlbumNumberedTrack, DiscNumberedTrack>;

    Graph graph;

    auto artist_ptr = graph.create<Artist>("Stevie Wonder");

    auto album_ptr = graph.create<Album>("Songs In The Key Of Life", 1976);

    auto album_2_ptr = graph.create<Album>("Extra Album", 2000);

    auto disc1_ptr = graph.create<Disc>(1); // tego nie chcę!
    auto disc2_ptr = graph.create<Disc>(2);

    auto track1_1_ptr = graph.create<Track>("Love's In Need Of Love Today");
    auto track1_2_ptr = graph.create<Track>("Have A Talk With God");

    auto track2_1_ptr = graph.create<Track>("Isn't She Lovely");
    auto track2_2_ptr = graph.create<Track>("Joy Inside My Tears");

    graph.add_association(*artist_ptr, *album_ptr);
    graph.add_association(*artist_ptr, *album_2_ptr);

    graph.add_association(*album_ptr, *disc1_ptr);
    graph.add_association(*album_ptr, *disc2_ptr);


    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc1_ptr,  *track1_2_ptr, 2);

    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_1_ptr, 1);
    addDiscNumberedTrack(graph, *disc2_ptr,  *track2_2_ptr, 2);

    std::cout << graph << '\n';



    // auto patch = graft::make_reachable_acyclic_patch(graph, track1_1_ptr);
    auto patch = graft::make_reachable_acyclic_patch(graph, album_ptr);

    // std::cout << decltype(patch)::debug_info();

    std::cout << patch << '\n';


    auto patch_album_ptr = patch.m_graph.get_storage<Album>().m_objects.front();
    // using root = typename decltype(patch_album_ptr)::element_type::metaclass;
    static_assert(graft::compute_member_blacklist_size<typename decltype(patch_album_ptr)::element_type::metaclass>() ==2);

    // constexpr static auto blacklist = graft::compute_member_blacklist<metaclasses, Album>();

    // std::cout << blacklist.size() << '\n';
    // []<std::size_t blacklist_index = 0>(this auto&& self)
    // {
    //     if constexpr (blacklist_index == blacklist.size()) { return; }
    //     else
    //     {
    //         constexpr auto member_location = blacklist.at(blacklist_index);
    //         using metaclass = std::tuple_element_t<member_location.metaclass_index, metaclasses>;
    //         constexpr auto metaclass_name  = metaclass::metaclass_name;
    //         constexpr auto member_name = std::tuple_element_t<member_location.member_index, typename metaclass::members>::name;
    //         std::cout << metaclass_name << "::" << member_name << "(" << member_location.metaclass_index << ", " << member_location.member_index << ")"  << '\n';
    //         return self.template operator()<blacklist_index + 1>();
    //     }

    // }();
    // std::cout << patch.exists_association()
    // auto patch_rec_1 = graft::make_reachable_acyclic_patch(patch, patch_album_ptr);
}


void test_acyclic_patch_result_type()

{
    using Graph = graft::graph<graft::object,  Album, Artist, Disc, Track, AlbumNumberedTrack, DiscNumberedTrack>;

    using x = graft::make_reachable_acyclic_patch_adhoc_metaclasses_t<typename Graph::metaclasses_tuple, Track>;
    // static_assert(std::is_same_v<void, x>);
    using patch_type = tuple_specialise_template_t
    <
        tuple_prepend_t
        <
            // typename Graph::metaclasses_tuple, // OK!
            graft::make_reachable_acyclic_patch_adhoc_metaclasses_t<typename Graph::metaclasses_tuple, Track>,
            Graph
        >,
        graft::patch
    >;
    patch_type patch;

    Graph graph;
    auto graph_album_ptr = graph.create<Album>("Extra Album", 2001);

    patch.create<Album>(graph_album_ptr, std::vector<int>{});

    auto album_ptr = patch.create<Album>("Songs In The Key Of Life", 2000);
    auto artist_ptr = patch.create<Artist>("Stevie Wonder");



    auto album_2_ptr = patch.create<Album>("Extra Album", 2000);

    auto disc1_ptr = patch.create<Disc>(1); // tego nie chcę!
    auto disc2_ptr = patch.create<Disc>(2);

    auto track1_1_ptr = patch.create<Track>("Love's In Need Of Love Today");
    auto track1_2_ptr = patch.create<Track>("Have A Talk With God");

    auto track2_1_ptr = patch.create<Track>("Isn't She Lovely");
    auto track2_2_ptr = patch.create<Track>("Joy Inside My Tears");

    std::cout << patch.exists_association(*album_ptr, *artist_ptr) << '\n'; // i to już jest true, a nie powinno być true!
    patch.add_association(*album_ptr, *artist_ptr);
    std::cout << patch.exists_association(*album_ptr, *artist_ptr) << '\n';

}


int main()
{
    test_acyclic_patch();
    test_acyclic_patch_result_type();
    using metaclasses = std::tuple<Album, Artist, Disc, Track, AlbumNumberedTrack, DiscNumberedTrack>;
    constexpr static auto blacklist = graft::compute_member_blacklist<metaclasses, Album>();

    std::cout << tuple_find_v<typename Artist::members, decltype(Artist::albums)> << '\n';
    std::cout << blacklist.size() << '\n';
    []<std::size_t blacklist_index = 0>(this auto&& self)
    {
        if constexpr (blacklist_index == blacklist.size()) { return; }
        else
        {
            constexpr auto member_location = blacklist.at(blacklist_index);
            using metaclass = std::tuple_element_t<member_location.metaclass_index, metaclasses>;
            constexpr auto metaclass_name  = metaclass::metaclass_name;
            constexpr auto member_name = std::tuple_element_t<member_location.member_index, typename metaclass::members>::name;
            std::cout << metaclass_name << "::" << member_name << "(" << member_location.metaclass_index << ", " << member_location.member_index << ")"  << '\n';
            return self.template operator()<blacklist_index + 1>();
        }

    }();
    // std::is_same_v<graft::patch<graft::graph<graft::object, Album, Artist, Disc, Track, AlbumNumberedTrack, DiscNumberedTrack>,
    // graft::metaclass<Album,
    //     const graft::member_t<static_string<5>{{"title"}}, std::string, graft::visibility::k_public>,
    //     const graft::member_t<static_string<11>{{"releaseYear"}}, unsigned long, graft::visibility::k_public>,
    //     const graft::member_t<static_string<7>{{"artists"}}, graft::many<Artist>, graft::visibility::k_public>,
    //     const graft::member_t<static_string<19>{{"albumNumberedTracks"}}, graft::many<AlbumNumberedTrack>, graft::visibility::k_public>>,
    // graft::metaclass<Artist, const graft::member_t<static_string<4>{{"name"}}, std::string, graft::visibility::k_public>>,
    // graft::metaclass<Disc, const graft::member_t<static_string<6>{{"number"}},
    // unsigned long, graft::visibility::k_public>, const graft::member_t<static_string<5>{{"album"}}, graft::one<Album>, graft::visibility::k_public>>,
    // graft::metaclass<Track, const graft::member_t<static_string<5>{{"title"}}, std::string, graft::visibility::k_public>, const
    // graft::member_t<static_string<16>{{"duration_seconds"}}, unsigned long, graft::visibility::k_public>, const graft::member_t<static_string<17>{{"discNumberedTrack"}},
    // graft::one<DiscNumberedTrack>, graft::visibility::k_public>, const graft::member_t<static_string<18>{{"albumNumberedTrack"}}, graft::one<AlbumNumberedTrack>,
    // graft::visibility::k_public>>, graft::metaclass<AlbumNumberedTrack, const graft::member_t<static_string<6>{{"number"}}, unsigned long, graft::visibility::k_public>,
    // const graft::member_t<static_string<5>{{"track"}}, graft::one<Track>, graft::visibility::k_public>, const graft::member_t<static_string<5>{{"album"}},
    // graft::one<Album>, graft::visibility::k_public>>, graft::metaclass<DiscNumberedTrack, const graft::member_t<static_string<6>{{"number"}}, unsigned long,
    // graft::visibility::k_public>, const graft::member_t<static_string<5>{{"track"}}, graft::one<Track>, graft::visibility::k_public>, const
    // graft::member_t<static_string<4>{{"disc"}}, graft::one<Disc>, graft::visibility::k_public>>>, void>;

}
