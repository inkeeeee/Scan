#include "format_string.hpp"
#include "scan.hpp"
#include <gtest/gtest.h>
#include <string_view>

using namespace stdx;
using namespace stdx::details;
TEST(ScanTest, BasicScan) {
    constexpr auto fmt = "Name: {}, Age: {}, Score: {}"_fs;
    constexpr auto source = fixed_string<34>("Name: Alice, Age: 25, Score: 95");

    constexpr auto result = scan<fmt, source, std::string_view, int, int>();
    const auto &[name, age, score] = result.values();

    EXPECT_EQ(std::string_view(name), "Alice");
    EXPECT_EQ(age, 25);
    EXPECT_EQ(score, 95);
}

TEST(ScanTest, UnsignedIntegers) {
    constexpr auto fmt = "A: {}, B: {}, C: {}"_fs;
    constexpr auto source = fixed_string("A: 255, B: 65535, C: 42");

    constexpr auto result = scan<fmt, source, std::uint8_t, std::uint16_t, std::uint32_t>();
    const auto &[a, b, c] = result.values();

    EXPECT_EQ(a, 255);
    EXPECT_EQ(b, 65535);
    EXPECT_EQ(c, 42);
}

TEST(ScanTest, SignedIntegers) {
    constexpr auto fmt = "X: {}, Y: {}, Z: {}"_fs;
    constexpr auto source = fixed_string<25>("X: -128, Y: 32767, Z: -1");

    constexpr auto result = scan<fmt, source, std::int8_t, std::int16_t, std::int32_t>();
    const auto &[x, y, z] = result.values();

    EXPECT_EQ(x, -128);
    EXPECT_EQ(y, 32767);
    EXPECT_EQ(z, -1);
}

TEST(ScanTest, FormatSpecifiers) {
    constexpr auto fmt = "ID: {%d}, Name: {%s}, Count: {%u}"_fs;
    constexpr auto source = fixed_string<30>("ID: 42, Name: Bob, Count: 100");

    constexpr auto result = scan<fmt, source, int, std::string_view, unsigned int>();
    const auto &[id, name, count] = result.values();

    EXPECT_EQ(id, 42);
    EXPECT_EQ(std::string_view(name), "Bob");
    EXPECT_EQ(count, 100);
}
TEST(ScanTest, CVQualifiedTypes) {
    constexpr auto fmt = "Value: {}, Text: {}"_fs;
    constexpr auto source = fixed_string("Value: 123, Text: Hello");

    constexpr auto result = scan<fmt, source, int, std::string_view>();
    const auto &[value, text] = result.values();

    EXPECT_EQ(value, 123);
    EXPECT_EQ(std::string_view(text), "Hello");
}

TEST(ScanTest, MixedSpecifiers) {
    constexpr auto fmt = "Num: {%d}, Str: {}, Flag: {%u}"_fs;
    constexpr auto source = fixed_string("Num: -10, Str: Test, Flag: 1");

    constexpr auto result = scan<fmt, source, int, std::string_view, uint64_t>();
    const auto &[num, str, flag] = result.values();

    EXPECT_EQ(num, -10);
    EXPECT_EQ(std::string_view(str), "Test");
    EXPECT_EQ(flag, 1);
}
TEST(ScanTest, BoundaryValues) {
    constexpr auto fmt = "Min: {}, Max: {}, Zero: {}"_fs;
    constexpr auto source = fixed_string<35>("Min: -32768, Max: 32767, Zero: 0");

    constexpr auto result = scan<fmt, source, std::int16_t, std::int16_t, int>();
    const auto &[min, max, zero] = result.values();

    EXPECT_EQ(min, -32768);
    EXPECT_EQ(max, 32767);
    EXPECT_EQ(zero, 0);
}

TEST(ScanTest, LongValues) {
    constexpr auto fmt = "Number: {}, Text: {}"_fs;
    constexpr auto source = fixed_string("Number: 2147483647, Text: LongStringValue");

    constexpr auto result = scan<fmt, source, std::int32_t, std::string_view>();
    const auto &[number, text] = result.values();

    EXPECT_EQ(number, 2147483647);
    EXPECT_EQ(std::string_view(text), "LongStringValue");
}

TEST(ScanTest, Int64Types) {
    constexpr auto fmt = "I64: {}, U64: {}"_fs;
    constexpr auto source = fixed_string("I64: -9223372036854775807, U64: 18446744073709551615");

    constexpr auto result = scan<fmt, source, std::int64_t, std::uint64_t>();
    const auto &[i64, u64] = result.values();

    EXPECT_EQ(i64, -9223372036854775807LL);
    EXPECT_EQ(u64, 18446744073709551615ULL);
}

TEST(ScanTest, MultipleStrings) {
    constexpr auto fmt = "First: {}, Second: {}, Third: {}"_fs;
    constexpr auto source = fixed_string("First: A, Second: BB, Third: CCC");

    constexpr auto result = scan<fmt, source, std::string_view, std::string_view, std::string_view>();
    const auto &[first, second, third] = result.values();

    EXPECT_EQ(std::string_view(first), "A");
    EXPECT_EQ(std::string_view(second), "BB");
    EXPECT_EQ(std::string_view(third), "CCC");
}

TEST(ScanTest, AllSpecifiers) {
    constexpr auto fmt = "Signed: {%d}, Unsigned: {%u}, String: {%s}"_fs;
    constexpr auto source = fixed_string("Signed: -100, Unsigned: 200, String: Text");

    constexpr auto result = scan<fmt, source, int, unsigned int, std::string_view>();
    const auto &[signed_val, unsigned_val, str] = result.values();

    EXPECT_EQ(signed_val, -100);
    EXPECT_EQ(unsigned_val, 200);
    EXPECT_EQ(std::string_view(str), "Text");
}

TEST(ScanTest, OnlyNumbers) {
    constexpr auto fmt = "{}, {}, {}"_fs;
    constexpr auto source = fixed_string<15>("100, 200, 300");

    constexpr auto result = scan<fmt, source, int, int, int>();
    const auto &[a, b, c] = result.values();

    EXPECT_EQ(a, 100);
    EXPECT_EQ(b, 200);
    EXPECT_EQ(c, 300);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
