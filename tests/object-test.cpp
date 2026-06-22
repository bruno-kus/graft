#include "../thirdparty/doctest/doctest.h"

#include "../src/graft/object.h"
#include "../src/domain/domain-fwd.h"


TEST_CASE("graft::object")
{
    graft::object<Album> album{"Songs In The Key Of Life", 1976};

}
