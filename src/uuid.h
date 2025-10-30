//
// Created by ros on 10/10/25.
//

#pragma once

#include <array>
#include <source_location>
#include <random>

using uuid_t = uint32_t;

constexpr uuid_t fnv1a_32(const char *str, uuid_t hash = 2166136261u) noexcept {
    return *str ? fnv1a_32(str + 1, (hash ^ static_cast<uuid_t>(*str)) * 16777619u) : hash;
}

constexpr auto to_cstr(uuid_t N) noexcept {
    std::array<char, 12> buf{}; // enough for int
    int i = 0;
    while (N != 0) {
        buf[i++] = N % 10 + '0';
        N /= 10;
    }
    buf[i] = '\0';
    return buf;
}

constexpr uuid_t compose_hash(char const *first, auto... rest) noexcept {
    if constexpr (sizeof...(rest) == 0) {
        return fnv1a_32(first);
    } else {
        return fnv1a_32(first, compose_hash(rest...));
    }
}

constexpr uuid_t get_code_point_uuid(std::source_location const &loc = std::source_location::current()) noexcept {
    auto const line = to_cstr(loc.line());
    auto const col  = to_cstr(loc.column());
    return compose_hash(loc.file_name(), line.data(), col.data());
}

uuid_t get_new_uuid() noexcept {
    thread_local std::mt19937 gen(std::random_device{}());
    thread_local std::uniform_int_distribution<uint32_t> dist;

    return dist(gen);
};

constexpr std::pair<uuid_t, uuid_t> get_call_uuid(
    const std::source_location loc = std::source_location::current()) noexcept {
    return {fnv1a_32(loc.function_name()), get_new_uuid()};
}
