//
// Created by ros on 10/10/25.
//

#ifndef TEST_FILES_PROOF_CHECKER_UUID_H
#define TEST_FILES_PROOF_CHECKER_UUID_H

#include <array>
#include <source_location>

using uuid_t = uint32_t;

constexpr uuid_t fnv1a_32(const char* str, uuid_t hash = 2166136261u) {
    if !consteval {
        static auto counter = 0L;
        hash += ++counter;
    }
    return *str ? fnv1a_32(str + 1, (hash ^ static_cast<uuid_t>(*str)) * 16777619u) : hash;
}

constexpr auto to_cstr(uuid_t N) {
    std::array<char, 12> buf{}; // enough for int
    int i = 0;
    while (N != 0) {
        buf[i++] = N % 10 + '0';
        N /= 10;
    }
    buf[i] = '\0';
    return buf;
}

constexpr uuid_t compose_hash(char const* first, auto... rest) {
    if constexpr (sizeof...(rest) == 0) {
        return fnv1a_32(first);
    } else {
        return fnv1a_32(first, compose_hash(rest...));
    }
}

constexpr uuid_t function_uuid(const std::source_location loc = std::source_location::current()) {
    return fnv1a_32(loc.function_name());
}

constexpr auto line_num(const std::source_location loc = std::source_location::current()) {
    return loc.line();
}

constexpr auto var_uuid(const std::source_location loc = std::source_location::current()) {
    uuid_t const hash = loc.line();
    auto const l = to_cstr(hash);
    return compose_hash(loc.function_name(), loc.file_name(), l.data());
}

#endif //TEST_FILES_PROOF_CHECKER_UUID_H