//
// Created by ros on 10/13/25.
//

#ifndef TEST_FILES_R_H
#define TEST_FILES_R_H

#include <type_traits>
#include <print>

#include "uuid.h"

struct r;
struct Case;

struct r {
    friend struct Case;
    mutable uuid_t uuid;

    constexpr r(r&& other) noexcept : uuid(other.uuid) {
        other.uuid = -1;
        if !consteval {
            std::println("Moved with id= {}", uuid);
        }
    }

private:
    explicit constexpr r(const uuid_t _uuid = var_uuid()) : uuid(_uuid) {
        if !consteval {
            std::println("New uuid = {}", uuid);
        }
    }
};

#endif //TEST_FILES_R_H