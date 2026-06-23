#pragma once

// #include "associator/many.h"
// #include "associator/one.h"
#include "../util/meta.hpp"
#include "associator/some_associator.h"
#include "graft.h"

namespace graft
{
    template<class metaclass_arg, template<class> class neighbour_object_template>
    struct member_buffer
    {
        using metaclass = metaclass_arg;
        constexpr static auto metaclass_name = metaclass_arg::metaclass_name;

        member_buffer(auto&&... args)
        {

            if constexpr (sizeof...(args) > 0)
            {
                [this]<std::size_t member_index = 0, std::size_t arg_index = 0>(this auto&& self, auto&&... args)
                {
                    constexpr bool member_index_past_last = (member_index == std::tuple_size_v<member_holders_type>);
                    constexpr bool arg_index_past_last = (arg_index == sizeof...(args));

                    if constexpr(member_index_past_last or arg_index_past_last) { return; }
                    else
                    {
                        using current_member = std::tuple_element_t<member_index, typename metaclass::members>;
                        if constexpr (some_associator<current_member>)
                        {
                            return self.template operator()<member_index + 1, arg_index>(std::forward<decltype(args)>(args)...);
                        }
                        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(args...[arg_index])>, std::nullopt_t>)
                        {
                            return self.template operator()<member_index + 1, arg_index + 1>(std::forward<decltype(args)>(args)...);
                        }
                        else
                        {
                            std::get<member_index>(member_holders) = std::forward<decltype(args...[arg_index])>(args...[arg_index]);
                            return self.template operator()<member_index + 1, arg_index +1>(std::forward<decltype(args)>(args)...);
                        }
                    }
                }(std::forward<decltype(args)>(args)...);
            }
        }


        friend auto operator<<(std::ostream& os, const member_buffer& member_buffer) -> std::ostream&
        {
            os << "{";
            // iteruj po memberach
            [&os, &member_buffer]<std::size_t I = 0>(this auto&& self)
            {
                if constexpr (I == std::tuple_size_v<typename metaclass::members>) { return;}
                else
                {
                    using current_member = std::tuple_element_t<I, typename metaclass::members>;
                    if constexpr (some_associator<current_member>)
                    {
                        os << " " << (current_member::name) << " : " << std::get<I>(member_buffer.member_holders).size() <<",";
                        return self.template operator()<I + 1>();
                    }
                    else
                    {
                        // static_assert(std::is_same_v<decltype(object.member_holders), void>);
                        // static_assert(not some_many<current_member>);
                        os << " " << (current_member::name) << " : " << std::get<I>(member_buffer.member_holders) << ",";
                        return self.template operator()<I + 1>();
                    }
                }
            }();
            os << "}";
            return os;
        }


        /*
         * iteruję po memberach
         */
        auto exists_link(const auto& supposed_neighbour) const -> bool // mono_cahed_presenter<...>
        {
            return [this, &supposed_neighbour]<std::size_t member_index = 0>(this auto&& self)
            {
                if constexpr (member_index == std::tuple_size_v<typename metaclass::members>) /* blacklisted metaclass */ { static_assert(false); }
                else
                {
                    using current_member = std::tuple_element_t<member_index, typename metaclass::members>;

                    if constexpr(some_associator<current_member>)
                    {
                        using member_neighbour_metaclass = typename current_member::value_type::neighbour_metaclass; // domenowa metaklasa sąsiada
                        // if constexpr (std::is_same_v<typename std::remove_cvref_t<decltype(supposed_neighbour)>::metaclass, member_neighbour_metaclass>)
                        if constexpr (std::remove_cvref_t<decltype(supposed_neighbour)>::metaclass_name == current_member::value_type::neighbour_metaclass::metaclass_name)
                        {
                            for (const auto& existing_neighbour : std::get<member_index>(member_holders))
                            {
                            if ((void*)existing_neighbour.get() == (void*)&supposed_neighbour)
                            {
                                return true;
                            }
                            }
                            return false;
                        } else
                        {
                            return self.template operator()<member_index + 1>();
                        }
                    }
                    else
                    {
                        return self.template operator()<member_index + 1>();
                    }
                }
            }();
        }

        template<class neighbour> // class member? class neigobject?
        auto insert_link(std::shared_ptr<neighbour> neighbour_ptr) -> void
        {
            [this, neighbour_ptr = std::move(neighbour_ptr)]<std::size_t member_holder_index = 0>(this auto&& self)
            {
                if constexpr (member_holder_index == std::tuple_size_v<member_holders_type>) { static_assert(false); }
                else
                {
                    if constexpr(some_associator<std::tuple_element_t<member_holder_index, typename metaclass::members>>)
                    {
                        using associator = typename std::tuple_element_t<member_holder_index, typename metaclass::members>;
                        if constexpr
                        (
                            // std::is_same_v<typename neighbour::metaclass, typename associator::value_type::neighbour_metaclass>
                            // std::is_same_v<typename neighbour::metaclass, typename associator::value_type::neighbour_metaclass>
                            neighbour::metaclass::metaclass_name == associator::value_type::neighbour_metaclass::metaclass_name

                        )
                        {
                            // std::cout << "Inserting " << *neighbour_ptr << " as " << neighbour::metaclass::metaclass_name << " into " << metaclass::metaclass_name << "\n";
                            std::get<member_holder_index>(member_holders).insert(std::move(neighbour_ptr));
                            return;
                        }
                        else { return self.template operator()<member_holder_index + 1>(); }
                    }
                    else { return self.template operator()<member_holder_index + 1>(); }
                }
            }();
        }

        template<class member>
        requires (tuple_contains_v<typename metaclass::members, const member>)
        auto get(this auto&& self, member) -> decltype(auto)
        {
            constexpr auto member_index = get_member_index<member>();
            return std::get<member_index>(self.member_holders);
        }

        template<class member>
        requires (tuple_contains_v<typename metaclass::members, const member>) and (not some_associator<std::remove_cvref_t<member>>)
        auto set(member, auto value) -> void
        {
            constexpr auto member_index = get_member_index<member>();
            std::get<member_index>(member_holders) = std::move(value);
        }

        // private: public:
        using member_holders_type = tuple_transform_t
        <
            typename metaclass::members,
            []<class member> { return std::type_identity<detail::to_holder_t<member, neighbour_object_template>>{}; }
        >;

        member_holders_type member_holders{};

        template<class member>
        consteval static auto get_member_index() -> size_t
        {
            return []<std::size_t I = 0>(this auto&& self) -> std::size_t
            {
                if constexpr (I == std::tuple_size_v<typename metaclass::members>) { static_assert(false); }
                else if constexpr (std::is_same_v<std::remove_cvref_t<member>, std::remove_cvref_t<std::tuple_element_t<I, typename metaclass::members>>>) { return I; }
                else { return self.template operator()<I + 1>(); }
            }();
        }
    };
}
