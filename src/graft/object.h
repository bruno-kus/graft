#pragma once

#include "member_buffer.h"

template<template<class, template<class> class> class object_template, template<class> class neighbour_effective_metaclass_template>
struct adapt_object_template_to_neighbour_template
{
    template<class nominal_metaclass>
    using neighbour_template = object_template<neighbour_effective_metaclass_template<nominal_metaclass>, neighbour_effective_metaclass_template>;
};

namespace graft
{
    template<class metaclass, template<class> class neighbour_effective_metaclass_template = std::type_identity_t>
    struct object : member_buffer<metaclass, adapt_object_template_to_neighbour_template<object, neighbour_effective_metaclass_template>::template neighbour_template>
    {
        using base = member_buffer<metaclass, adapt_object_template_to_neighbour_template<object, neighbour_effective_metaclass_template>::template neighbour_template>;
        constexpr static auto metaclass_name = base::metaclass_name;

        public:
        using base::base;

        // template<class p_metaclass>
        // using neighbour_template = object<p_metaclass, neighbour_effective_metaclass_template>;

        // member_buffer<metaclass, neighbour_template> m_member_buffer;
    };
}


// std::shared_ptr<graft::object<graft::metaclass<Disc, const graft::member_t<static_string<7UL - 1>{{"number"}}, unsigned long, graft::visibility::k_public>, const
//     graft::member_t<static_string<6UL - 1>{{"album"}}, graft::one<Album>, graft::visibility::k_public>>, graft::reachable_acyclic_object_template<graft::object, std::tuple<Artist, Album,
    // AlbumNumberedTrack, Disc, DiscNumberedTrack, Track>, Album>::neighbour_effective_metaclass_template>> &>'


    // FROM:
    // std::shared_ptr
    // <
    //     graft::object
    //         <
    //             graft::metaclass
    //             <
    //                 Disc,
    //                 const graft::member_t<static_string<7UL - 1>{{"number"}}, unsigned long, graft::visibility::k_public>,
    //                 const graft::member_t<static_string<6UL - 1>{{"album"}}, graft::one<Album>, graft::visibility::k_public>
    //             >,
    //             graft::reachable_acyclic_object_template<graft::object, std::tuple<Artist, Album, AlbumNumberedTrack, Disc, DiscNumberedTrack, Track>,Album>::neighbour_effective_metaclass_template
    //             >
    //         >
    // >
//     TO:
    // shared_ptr<object<Disc, neighbour_effective_metaclass_template>>
