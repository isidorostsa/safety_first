//
// Created by ros on 10/29/25.
//

#pragma once

#include <concepts>
#include <ranges>

#define FWD(x) std::forward<decltype(x)>(x)

template<typename T, typename R>
concept view_of = std::ranges::view<T> && std::same_as<std::ranges::range_value_t<T>, R>;

template<typename T, typename R>
concept range_of = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, R>;
