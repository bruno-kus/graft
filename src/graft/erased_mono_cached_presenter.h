#pragma once
#include "object.h"

#include "erased.h"
namespace graft
{




    template<class metaclass, template<class> class neighbour_effective_metaclass = std::type_identity_t>
    struct erased_mono_cached_presenter : object<metaclass, neighbour_effective_metaclass>

    {

        using base = object<metaclass, neighbour_effective_metaclass>;
        using base::base;
        using base::get;

        using cache_metaclass = tuple_remove_if_t
            <
                typename metaclass::members,
                []<class member> { return some_associator<member>; }
            >;

        erased_ref<metaclass> m_source_ref; // assigned once in constructor, used by push_upstream()

        // auto push_upstream() -> void;
    };
}
