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
    std::optional<value_uuid_t> uuid;

public:
    constexpr value_uuid_t get_uuid() const {
        assert(uuid);
        return *uuid;
    }

    constexpr r(r&& other) noexcept : uuid(other.get_uuid()) {
        other.uuid.reset();
    }

private:
    explicit constexpr r(const value_uuid_t _uuid) : uuid(_uuid) {}
};
