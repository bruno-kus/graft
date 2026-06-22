#pragma once
#include "patch."
namespace graft
{

    // derive adhoc przyjmuje metaklasy poprzedniego grafu, czyli w domyśle oryginalnego!


    // SPRAWDŹMY, CZY DA SIĘ W OGÓLE ODPALIĆ COMPUTE MEMBER na łacie -> da się
    template<class metaclasses_tuple, class root_metaclass>
    using derive_adhoc_metaclasses = decltype
    ([]{
        constexpr static auto member_blacklist = compute_member_blacklist<metaclasses_tuple, root_metaclass>();


        constexpr auto derive_adhoc_associator =
            []<class associator>
            {
                using x = typename associator::value_type::neighbour_metaclass;
                // dla x muszę sprawdzić... co muszę sprawdzić?
                // znowu jestem na etapie trudnego algorytmu
                // jaki byłby kształt
                // ale czy to nie jest self reference problem?
            };

        constexpr auto derive_adhoc_metaclass =
            []<class metaclass>
            {
                using members = typename metaclass::members;
                // filtrowanie jest konieczne
                // oraz transformowanie!
                // czy mogę najpierw odfiltrować
                // zróbmy najpierw filtrowanie
            };

    }())::type();

    // pytanie za sto punktów brzmi, co z reachable, jeżeli chodzę po upośledzonym grafie
    // nie powinno być żadnego problemu, ponieważ usunęliśmy tylko krawędzie po których tak czy siak NIE SZLIŚMY!

    // blacklista już działa doskonale, ale czy na pewno?
    // dwie rzeczy: -> blacklista od łaty powinna być całkowicie pusta
}
