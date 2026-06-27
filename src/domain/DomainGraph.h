#include "../graft/graph.h"
#include "domain-fwd.h"
#include "../graft/object.h"

template
<
    template<class, template<class>class > class object_template_arg,
    template<class> class effective_metaclass_template,
    class... metaclasses
>
 struct DomainGraph /* final */:
    private graft::graph<object_template_arg, effective_metaclass_template, metaclasses...>
{
    // private:
    using base = graft::graph<graft::object, std::type_identity_t, Artist, Album, AlbumNumberedTrack, Disc, DiscNumberedTrack, Track>;
    public:
    using base::base;
    using base::create;
    using base::add_association;
    using base::exists_association;
    using base::metaclasses_tuple;
    using base::get_storage;
    // te wszystkie usingi piszę sam, ale one nic mnie nie kosztują prawie
    //

    // template<std::same_as<Disc>>
    // auto create(auto& album, auto&&... args) -> auto
    // {
    //     // COMPOSITION
    //     auto album_ptr = base::get_storage<Album>().find(album);

    //     auto disc_ptr = base::create<Disc>(std::forward<decltype(args)>(args)...);

    //     base::add_association(album_ptr, disc_ptr);

    //     return disc_ptr;
    // }

    // template<std::same_as<AlbumNumberedTrack>>
    // auto create(auto&&...) = delete;

    // template<std::same_as<DiscNumberedTrack>>
    // auto create(auto&&...) = delete;
};
/*
 * jeżeli rozważam hierarchię final, to równie dobrze mogę w ogóle nie mieć hierarchii
 */
