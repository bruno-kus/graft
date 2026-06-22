#pragma once
#include "graft.h"

// robimy skurwiela od zera całkiem!

namespace graft
{

    template<class metaclass>
    struct erased
    {
        // ina opcja, to jest preparować memberów
        // i tak powinienem robić w ogóle!
        // bo to mapowanie podawania funkcji jest kurwa głupie
        // gdzie dedukuję metaklasy? w łacie
    };

    template<>
    struct erased_mono_cached_presenter
    {

    }
    // tylko, że erased, nie erased, nie wiem, jak wydedukować brakujących sąsiadów!
    //

}
