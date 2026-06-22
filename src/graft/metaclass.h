#pragma once

namespace graft
{
    template<class source_metaclass_t, class... p_members>
    struct metaclass
    {
        using source_metaclass = source_metaclass_t;
        constexpr static auto metaclass_name = source_metaclass::metaclass_name;
        using members = std::tuple<std::remove_cvref_t<p_members>...>;

    };
}
