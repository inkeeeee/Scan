#pragma once
#include <algorithm>
#include <charconv>
#include <concepts>
#include <cstdint>
#include <optional>
#include <string_view>
#include <system_error>

#include "format_string.hpp"
#include "types.hpp"

namespace stdx::details {
template <typename T>
concept allowed_type =
    (std::signed_integral<T> || std::unsigned_integral<T> || std::is_same_v<std::remove_cv_t<T>, std::string_view>) &&
    !std::is_reference_v<T>;

template <std::integral T, fixed_string input> consteval std::optional<T> parse_value() {
    std::remove_cv_t<T> value{};
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), value);
    if (ec == std::errc{} && ptr > input.data()) {
        return value;
    }
    return std::nullopt;
}

template <int I, format_string fmt, fixed_string source> consteval auto get_current_source_for_parsing() {
    static_assert(I >= 0 && I < fmt.number_placeholders, "Invalid placeholder index");

    constexpr auto to_sv = [](const auto &fs) { return std::string_view(fs.data(), fs.size() - 1); };

    constexpr auto fmt_sv = to_sv(fmt.value);
    constexpr auto src_sv = to_sv(source);
    constexpr auto &positions = fmt.placeholder_positions;

    constexpr auto pos_i = positions[I];
    constexpr std::size_t fmt_start = pos_i.first;
    constexpr std::size_t fmt_end = pos_i.second;

    constexpr std::size_t src_start = [&] {
        if constexpr (I == 0) {
            return fmt_start;
        } else {
            constexpr auto prev_bounds = get_current_source_for_parsing<I - 1, fmt, source>();
            constexpr auto prev_end = prev_bounds.second;

            constexpr auto prev_pos = positions[I - 1];
            constexpr auto prev_fmt_end = prev_pos.second;
            constexpr auto sep_size = fmt_start - (prev_fmt_end + 1);

            if constexpr (sep_size > 0) {
                constexpr auto sep = fmt_sv.substr(prev_fmt_end + 1, sep_size);
                constexpr auto pos = src_sv.find(sep, prev_end);
                if constexpr (pos != std::string_view::npos) {
                    return pos + sep.size();
                }
            }
            return prev_end;
        }
    }();

    constexpr std::size_t src_end = [&] {
        if constexpr (fmt_end == (fmt_sv.size() - 1)) {
            return src_sv.size();
        }

        constexpr auto next_start = (I < fmt.number_placeholders - 1) ? positions[I + 1].first : fmt_sv.size();
        constexpr auto sep_size = next_start - (fmt_end + 1);

        if constexpr (sep_size > 0) {
            constexpr auto sep = fmt_sv.substr(fmt_end + 1, sep_size);
            constexpr auto pos = src_sv.find(sep, src_start);
            if constexpr (pos != std::string_view::npos) return pos;
        }
        return src_sv.size();
    }();

    return std::pair{src_start, std::min(src_end, source.size() - 1)};
}

template <int I, format_string fmt, fixed_string source, allowed_type T> consteval auto parse_input() {
    static_assert(I >= 0 && I < fmt.number_placeholders, "Invalid placeholder index");
    constexpr auto bounds = get_current_source_for_parsing<I, fmt, source>();
    constexpr auto start = bounds.first;
    constexpr auto end = bounds.second;

    static_assert(start < end, "No data found for placeholder");
    static_assert(end <= source.size() - 1, "Data range exceeds source string");

    constexpr std::size_t data_size = end - start;

    constexpr auto specifier = [&]() constexpr -> char {
        constexpr auto &positions = fmt.placeholder_positions;
        constexpr auto pos_i = positions[I];
        constexpr auto placeholder_start = pos_i.first;
        constexpr auto placeholder_end = pos_i.second;

        if (placeholder_start + 2 < placeholder_end && fmt.value.data()[placeholder_start + 1] == '%') {
            return fmt.value.data()[placeholder_start + 2];
        }
        return '\0';
    }();

    if constexpr (specifier != '\0') {
        if constexpr (std::integral<T>) {
            static_assert(specifier == 'd' || specifier == 'u', "Invalid format specifier for integral type");
        } else if constexpr (std::same_as<T, std::string_view>) {
            static_assert(specifier == 's', "Invalid format specifier for string type");
        }
    }

    if constexpr (std::same_as<std::remove_cv_t<T>, std::string_view>) {
        return std::string_view(source.data() + start, data_size);
    } else {
        constexpr fixed_string<data_size> value_src(source.data() + start, source.data() + end);
        constexpr auto parsed_value = parse_value<T, value_src>();
        static_assert(parsed_value.has_value(), "Failed to parse value from source data");
        return parsed_value.value();
    }
}

} // namespace stdx::details
