#pragma once

#include "member_buffer.h"

// template<class metaclass>
// using object = member_buffer<metaclass, object>;
namespace graft
{
    template<class metaclass>
    struct object : member_buffer<metaclass, object>
    {
        private:
        using base = member_buffer<metaclass, object>;
        constexpr static auto metaclass_name = base::metaclass_name;

        public:
        using base::base;
    };
}
