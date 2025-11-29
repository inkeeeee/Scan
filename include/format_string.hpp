#pragma once
#include "types.hpp"
#include <algorithm>
#include <array>
#include <expected>
#include <utility>

namespace stdx::details {

template <fixed_string str> class format_string {
  public:
    static consteval std::expected<size_t, parse_error> get_number_placeholders() {
        constexpr size_t N = str.size();
        if (!N) return 0;
        size_t placeholder_count = 0;
        size_t pos = 0;
        const size_t size = N - 1;

        while (pos < size) {
            if (str.data()[pos] != '{') {
                ++pos;
                continue;
            }

            if (pos + 1 >= size) {
                return std::unexpected(parse_error{"Unclosed last placeholder"});
            }

            ++placeholder_count;
            ++pos;

            if (str.data()[pos] == '%') {
                ++pos;
                if (pos >= size) {
                    return std::unexpected(parse_error{"Unclosed last placeholder"});
                }

                const char spec = str.data()[pos];
                constexpr char valid_specs[] = {'d', 'u', 's'};
                bool valid = false;

                for (const char s : valid_specs) {
                    if (spec == s) {
                        valid = true;
                        break;
                    }
                }

                if (!valid) {
                    return std::unexpected(parse_error{"Invalid specifier."});
                }
                ++pos;
            }

            if (pos >= size || str.data()[pos] != '}') {
                return std::unexpected(parse_error{"\'}\' hasn't been found in appropriate place"});
            }
            ++pos;
        }

        return placeholder_count;
    }

    static consteval auto get_placeholder_positions() {
        constexpr size_t count = number_placeholders;
        std::array<std::pair<size_t, size_t>, count> positions{};
        constexpr size_t N = str.size();
        size_t placeholder_index = 0;
        size_t pos = 0;
        const size_t size = N - 1;

        while (pos < size && placeholder_index < count) {
            if (str.data()[pos] != '{') {
                ++pos;
                continue;
            }

            size_t start = pos;
            ++pos;

            if (pos < size && str.data()[pos] == '%') {
                ++pos;
                if (pos < size) {
                    ++pos;
                }
            }

            while (pos < size && str.data()[pos] != '}') {
                ++pos;
            }

            if (pos < size && str.data()[pos] == '}') {
                size_t end = pos;
                positions[placeholder_index] = {start, end};
                ++placeholder_index;
                ++pos;
            }
        }

        return positions;
    }

    static constexpr fixed_string value = str;

    static constexpr size_t number_placeholders = []() {
        constexpr auto result = get_number_placeholders();
        static_assert(result.has_value(), "Format string parsing failed");
        return result.value();
    }();

    static constexpr auto placeholder_positions = get_placeholder_positions();

    static constexpr const char *data() noexcept { return str.data(); }
    static constexpr size_t size() noexcept { return str.size(); }
};

template <typename T, T... Chars> constexpr auto operator""_fs() {
    constexpr char str[sizeof...(Chars) + 1] = {Chars..., '\0'};
    constexpr auto fs = fixed_string<sizeof...(Chars) + 1>(str);

    constexpr auto check_result = format_string<fs>::get_number_placeholders();
    static_assert(check_result.has_value(), "Invalid format string");

    return format_string<fs>{};
}
} // namespace stdx::details
