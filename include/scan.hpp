#pragma once

#include <tuple>
#include <utility>

#include "format_string.hpp"
#include "parse.hpp"
#include "types.hpp"

namespace stdx {

namespace details {

template <format_string fmt, fixed_string source, typename... Ts, std::size_t... Is>
consteval auto scan_impl(std::index_sequence<Is...>) {
    return scan_result<Ts...>(parse_input<Is, fmt, source, Ts>()...);
}

} // namespace details

template <details::format_string fmt, details::fixed_string source, typename... Ts>
consteval details::scan_result<Ts...> scan() {
    static_assert(sizeof...(Ts) == fmt.number_placeholders, "Number of types must match number of placeholders");
    static_assert((details::is_allowed_type<Ts> && ...), "Unsupported type detected");

    return details::scan_impl<fmt, source, Ts...>(std::index_sequence_for<Ts...>{});
}

} // namespace stdx
