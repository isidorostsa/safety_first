//
// Created by ros on 10/10/25.
//

#pragma once

#include <array>
#include <source_location>
#include <random>
#include <string_view>

enum class uuid_kind : uint8_t { value, code_point, function_call };

template<uuid_kind K>
struct typed_uuid {
    uint32_t raw;
    auto operator<=>(const typed_uuid&) const = default;
};

using value_uuid_t         = typed_uuid<uuid_kind::value>;
using code_point_uuid_t    = typed_uuid<uuid_kind::code_point>;
using function_call_uuid_t = typed_uuid<uuid_kind::function_call>;

struct function_point_t {
    std::string_view function_name;
    std::string_view file_name;
    uint32_t line;
    uint32_t column;
    auto operator<=>(const function_point_t&) const = default;
};

constexpr uint32_t fnv1a_32(const char *str, uint32_t hash = 2166136261u) noexcept {
    return *str ? fnv1a_32(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619u) : hash;
}

constexpr auto to_cstr(uint32_t N) noexcept {
    std::array<char, 12> buf{};
    int i = 0;
    while (N != 0) {
        buf[i++] = N % 10 + '0';
        N /= 10;
    }
    buf[i] = '\0';
    return buf;
}

constexpr uint32_t compose_hash(char const *first, auto... rest) noexcept {
    if constexpr (sizeof...(rest) == 0) {
        return fnv1a_32(first);
    } else {
        return fnv1a_32(first, compose_hash(rest...));
    }
}

constexpr code_point_uuid_t get_code_point_uuid(
    std::source_location const &loc = std::source_location::current()) noexcept {
    auto const line = to_cstr(loc.line());
    auto const col  = to_cstr(loc.column());
    return {compose_hash(loc.file_name(), line.data(), col.data())};
}

inline uint32_t get_new_raw_uuid() noexcept {
    thread_local std::mt19937 gen(std::random_device{}());
    thread_local std::uniform_int_distribution<uint32_t> dist;
    return dist(gen);
}

inline value_uuid_t get_new_uuid() noexcept {
    return {get_new_raw_uuid()};
}

inline std::pair<function_point_t, function_call_uuid_t> get_call_uuid(
    const std::source_location loc = std::source_location::current()) noexcept {
    function_point_t fp{loc.function_name(), loc.file_name(), loc.line(), loc.column()};
    return {fp, function_call_uuid_t{get_new_raw_uuid()}};
}
