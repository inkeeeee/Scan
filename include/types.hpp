#pragma once
#include <algorithm>
#include <stdexcept>
#include <tuple>

namespace stdx::details {

template <std::size_t N>
    requires(N > 0)
class fixed_string {
  public:
    char buf[N];

    constexpr fixed_string(const char (&buf_)[N]) { std::copy(buf_, buf_ + N, buf); }

    template <std::size_t I>
        requires(I < N && I > 0)
    constexpr fixed_string(const char (&buf_)[I]) {
        std::copy(buf_, buf_ + I, buf);
        std::fill(buf + I, buf + N, '\0');
    }

    constexpr fixed_string(const char *first, const char *last) {
        const std::ptrdiff_t dist = last - first;
        if (dist > N) throw std::invalid_argument("Invalid size!");

        std::copy(first, last, buf);
        std::fill(buf + dist, buf + N, char{});
    }

    constexpr const char *data() const noexcept { return buf; }
    constexpr std::size_t size() const noexcept { return N; }
    constexpr operator const char *() const noexcept { return buf; }
};

class parse_error : public fixed_string<128> {
  public:
    using fixed_string<128>::fixed_string;
};

template <typename... Types> class scan_result {
  private:
    std::tuple<Types...> values_;

  public:
    constexpr scan_result(std::tuple<Types...> values) : values_(std::move(values)) {}

    constexpr scan_result(Types... values) : values_(std::forward<Types>(values)...) {}

    constexpr const std::tuple<Types...> &values() const & noexcept { return values_; }

    constexpr std::tuple<Types...> &values() & noexcept { return values_; }

    constexpr std::tuple<Types...> &&values() && noexcept { return std::move(values_); }
};

} // namespace stdx::details
