#pragma once
#include "graft.h"
#include "../util/meta.hpp"
#include "associator/some_associator.h"

namespace graft
{


    template<class... members>
    struct erased_ref_body
    {
        erased_ref_body() = default;

        template<class T>
        erased_ref_body(T t) : ptr{new impl<T, members...>(t)}
        {

        }

        template<class... members_1>
        struct interface_chain;

        template<>
        struct interface_chain<> {};

        template<class head_member>
        struct interface_chain<head_member> : interface_chain<>
        {
            virtual ~interface_chain() = default;

            using value_type = typename head_member::value_type;
            virtual const value_type& get(std::type_identity<std::remove_cvref_t<head_member>>) = 0;
        };

        template<class head_member, class... tail_members>
        struct interface_chain<head_member, tail_members...> : interface_chain<tail_members...>
        {
            using interface_chain<tail_members...>::get;

            virtual ~interface_chain() = default;

            using value_type = typename head_member::value_type;
            virtual const value_type& get(std::type_identity<std::remove_cvref_t<head_member>>) = 0;
        };

        template<class self, class interface, class... members_1>
        struct impl_chain;

        template<class self, class interface>
        struct impl_chain<self, interface> {};

        template<class self, class interface, class head_member>
        struct impl_chain<self, interface, head_member> : interface
        {
            using value_type = typename head_member::value_type;

            const value_type& get(std::type_identity<std::remove_cvref_t<head_member>>) override
            {
                return static_cast<self*>(this)->model->get(head_member{});
            }
        };

        template<class self, class interface, class head_member, class... tail_members>
        struct impl_chain<self, interface, head_member, tail_members...> : impl_chain<self, interface, tail_members...>
        {

            using value_type = typename head_member::value_type;

            const value_type& get(std::type_identity<std::remove_cvref_t<head_member>>) override
            {
                return static_cast<self*>(this)->model->get(head_member{});
            }
        };

        template<class T, class... members_1>
        struct impl : impl_chain<impl<T, members_1...>, interface_chain<members_1 ...>, members_1...>
        {
            T model;

            impl(T t) : model{std::move(t)} {}
        };

        template<class member>
        auto get(member) -> decltype(auto)
        {
            return ptr->get(std::type_identity<member>{});
        }
        interface_chain<members...> *ptr;
    };


    template<class metaclass>
    struct erased_ref
    {

        using erased_ref_body_type = tuple_specialise_template_t
            <
                tuple_remove_if_t
                <
                    typename metaclass::members,
                    []<class member> { return  some_associator<member>; }
                >,
                erased_ref_body
            >;

        erased_ref_body_type m_body;


        // erased_ref(auto t) :m_body{std::move(t)} {}
        auto operator=(auto object_ptr) { m_body = std::move(object_ptr); return *this; }

        auto get(const auto member)
        {
            return m_body.get(std::remove_cvref_t<decltype(member)>{});
        }
    };

}
