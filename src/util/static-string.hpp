 #pragma once
 #include <array>
 #include <string>
 #include <algorithm>
 #include <ranges>
 #include <ostream>





 template<size_t N>
 struct static_string
 {
     std::array<char, N + 1> char_array{};

     constexpr static_string() = default;
     constexpr operator std::string() const { return {std::from_range, char_array | std::views::take(N)}; }
     constexpr operator std::string_view() const { return {char_array.data(), N}; }

     constexpr static_string(const std::string_view& sv)
     {
         std::copy_n(sv.data(), N, char_array.data());
         char_array[N] = '\0';
     }

     template<std::size_t str_size>
     constexpr static_string(const char (&str)[str_size])
     {
         std::copy_n(str, N, char_array.data());
         char_array[N] = '\0';
     }


     constexpr auto size() const  -> std::size_t{ return N;}
     constexpr auto data() const -> const char* { return char_array.data(); }
     // constexpr auto data() -> char* {return char_array.data(); }
     constexpr bool operator==(const static_string &) const = default;

     friend auto operator<<(std::ostream& os, const static_string& static_string) -> std::ostream&
     {
         return os << std::string_view{static_string};
     }
 };

 template<std::size_t str_size>
 static_string(const char(&str)[str_size]) -> static_string<str_size -1>;

 template<class T>
 struct is_static_string : std::false_type {};

 template<std::size_t N>
 struct is_static_string<static_string<N>> : std::true_type {};

 template<class T>
 concept some_static_string = is_static_string<T>::value;




 // template<auto&... static_strings>
 // consteval std::string_view static_string_concat()
 // {
 //     constexpr static auto concatenation_size = (std::size_t{0} + ... + static_strings.size()) + 1;
 //     constexpr static auto std::array<char, concatenation_size> string_buffer = []
 //         {
 //             std::array<char, concatenation_size> result;
 //             result.back() = '\n';
 //             std::size_t result_back = 0;
 //             [&result, &result_back]<std::size_t static_string_index = 0>(this auto&& self)
 //             {
 //                 if constexpr (static_string_index == sizeof...(string_views)) { return; }
 //                 else
 //                 {
 //                     const auto& current_static_string = string_views...[static_string_index];
 //                     for (int i = 0; i < current_static_string.char_array.size(); ++i)
 //                     {
 //                         result.at(result_back + i) = current_static_string.char_array..at(i);
 //                     }
 //                 }
 //             }();
 //             return result;
 //         }();

 //     return std::string_view{string_buffer};
 // }
