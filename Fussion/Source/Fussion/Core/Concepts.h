#pragma once
#include <concepts>
#include <ranges>

namespace Fussion {
    template<class T, template<class...> class U>
    inline constexpr bool IsInstanceOf = std::false_type {};

    template<template<class...> class U, class... Vs>
    inline constexpr bool IsInstanceOf<U<Vs...>, U> = std::true_type {};

    template<typename T>
    concept ScalarType = std::integral<T> || std::floating_point<T>;

    template<typename R, typename T>
    concept SpanCompatible = std::ranges::contiguous_range<R>
        && std::ranges::sized_range<R>
        && !std::is_array_v<std::remove_cvref_t<R>>
        && (std::ranges::borrowed_range<R> || std::is_const_v<T>)
        && std::is_convertible_v<std::remove_reference_t<std::ranges::range_reference_t<R>> (*)[], T (*)[]>;
}
