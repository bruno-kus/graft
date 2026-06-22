

namespace graft
{
    template<class object_type> // e.g. object<Artist>
    struct contributor
    {
        object_type* m_source;

        template<class member>
        auto get(this auto&& self, const member) -> decltype(auto)
        {
            return m_source->get(member);
        }
    }
}
// nie muszę zapisywać, które typy, bo przechowuję tylko wskaźnik
// ale dla cached_contributor już muszę...

namespace graft
{
    template<some_metaclass metaclass>
    struct cached_contributor
    {
        // potrzebuję znać metaklasę, żeby na jej podstawie przetrzymywać atrybuty
    }
    // idea jest taka, że jeżeli muszę wybierać to za pomocą prezentera?
}
namespace graft
{
    // żeby mieć prezenter muszę sprecyzować selektory
    // czyli member
    template<class... members>
    struct presenter
    {
        // unikalne typy źródłowe
        // std::tuple<object<Album>, object<Artist>>;
    }
}
// presenter analogicznie powinien działać dla pojedynczego typu, jak dla wielu

namespace graft
{
    template<class... members> // wspólny typ jest domniemany?
    // jeżeli dla każdego atrybutu mam osobny wskaźnik, to ułatwia implementację
    struct presenter_cell
    {
        // rzecz w tym, że bez cache'owania, nie muszę wybierać parametrów
        // object<Artist>* m_source;
    }
}
namespace graft
{
    template<class... members>
    struct cached_presenter_cell
    {
        // object<Artist>* m_source;
        // std::tuple<MEMBER_HOLDER>
        // <- krotkę z holderami mogę zrobić zwyczajnie, to w sumie żaden problem
    }
}
namespace graft
{
    // template<class... members>
    // struct member_cache
    // {
    //     using member_holders_type = tuple_transform_t
    //     <
    //         typename metaclass::members,
    //         []<class type> { return std::type_identity<detail::to_holder_t<type, object>>{}; }
    //     >;

    //     member_holders_type member_holders;
    //     // w sumie, to nawet nie muszę mieć osobnych skrytek, wystarczy mi jedna
    // };

    // przy okazji sorted unique :)
    template<class sources_type_list, class members_type_list>
    struct cached_presenter
    {
        using source_ptrs = decltype(
           []<std::size_t... Is>(std::index_sequence<Is...>)
           {
               return std::type_identity<std::tuple<std::tuple_element_t<Is, sources_type_list>*...>>{};
           }(std::make_index_sequence<std::tuple_size_v<sources_type_list>>())
        )::type;

        source_ptrs m_source_ptrs;

        using member_holders_type = tuple_transform_t
        <
            typename metaclass::members,
            []<class type> { return std::type_identity<detail::to_holder_t<type, object>>{}; }
        >;

        member_holders_type m_member_holders;

        template<class member>
        auto set(const member, auto value) -> void
        {
            constexpr auto member_index = get_member_index<member>();
            std::get<member_index>(self.m_member_holders) = value;
        }
        auto push_upstream() -> void
        {

            []<std::size_t I = 0>(this auto self)
            {
                if constexpr (I == std::tuple_size_v<members_type_list>) { return; }
                else
                {
                    []<std::size_t J = 0>(this auto self)
                    {
                        if constexpr (J == std::tuple_size_v<sources_type_list>) { return; }
                        else
                        {
                            using current_member = std::tuple_element_t<I, members_type_list>;
                            using current_source = std::tuple_element_t<J, sources_type_list>;

                            constexpr auto current_source_has_current_member = requires (current_source&& cs)
                            {
                                cs.set(current_member{}, cached_value);
                            };
                            // to jest bardzo sprytna sztuczka :)

                            if constexpr (current_source_has_current_member)
                            {
                                const auto& source_ptr = std::get<current_source>(m_source_ptrs);
                                const auto& cached_value = std::get<current_member>(m_member_holders); // holder to jest samo w sobie value
                                source_ptr->set(current_member{}, cached_value);
                            }
                        }
                    }
                }
            }();

            // muszę przeiterować po wszystkich schowkach

            // w każdym miejscu skrytki jeszcze potrzebuję flagi?
            // w zasadzie, to nie potrzebuję, bo self assignment powinien działać
            // czyli na chama!
            // aczkolwiek z logiką transakcyjności
            // wtedy nie potrzeba flagi, po prostu jest silna gwarancja
            // a silna gwarancja jest poprzez swap
            // cached presenter nie wydaje się straszny w takim razie
            // aczkolwiek zostałbym chyba przy tym, że każdy atrybut ma pojedynczy wskaźnik
            // <- bo w ten sposób wspieram rekurencyjne relacje
            //
            // czyli za każdym razem mam wskaźnik plus selektor
            // tylko wtedy nie mam jak odróżnić pomiędzy selektorami
            // to wtedy potrzebowałbym jeszcze aliasów
            // to na razie bym sobie darował może
            // ale tak czy siak muszę wiedzieć z jakiej klasy pochodzą membery
        }
        // unikalne typy źródłowe
        // krotka ze skrytą
        //
        template<class member>
        consteval static auto get_member_index() -> size_t
        {
            return []<size_t i = 0>(this auto self)
            {
                if constexpr (i = std::tuple_size_v<member_tuple>) { static_assert(false); }
                else if constexpr (std::is_same_v<member, std::tuple_element_t<I, typename metaclass::members>>) { return I; }
                else { return self.template operator()<I + 1>(); }
            }
        }
    }
}
