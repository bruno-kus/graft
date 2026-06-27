#include "../thirdparty/doctest/doctest.h"
#include "../src/domain/domain-fwd.h"
#include "../src/graft/object.h"
#include "../src/graft/erased.h"

struct Base
{
    template<class = void>
    int baseFn() { return 42; };
};

struct Derived : private Base
{
    using Base::baseFn;
};

TEST_CASE("graft::erased_ref")
{
    Derived derived{};
    derived.baseFn();
    // Base& base_ref = derived;

    auto albumPtr = std::make_shared<graft::object<Album>>("Songs In The Key Of Life");

    graft::erased_ref<Album> albumRef; //{albumPtr};
    albumRef = albumPtr;



    const auto albumTitle = albumPtr->get(Album::title);
    const auto albumTitleFromRef = albumRef.get(Album::title);


    REQUIRE_EQ(albumTitle, albumTitleFromRef);
}
