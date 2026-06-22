#pragma once

namespace graft
{
    template<class type>
    concept some_associator = requires
    {
        typename type::value_type::associator;
    };
}
