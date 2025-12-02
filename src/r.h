//
// Created by ros on 10/13/25.
//

#pragma once

#include <cassert>

#include <type_traits>
#include <print>

#include "uuid.h"

struct r {
    friend struct Case;
protected:
    std::optional<uuid_t> uuid;

public:
    constexpr uuid_t get_uuid() const {
        assert(uuid);
        return *uuid;
    }

    constexpr r(r&& other) noexcept : uuid(other.get_uuid()) {
        other.uuid.reset();
    }

private:
    explicit constexpr r(const uuid_t _uuid = get_new_uuid()) : uuid(_uuid) {}
};
