#pragma once
#include <concepts>

namespace Fussion {
    template<class T, template<class...> class U>
    inline constexpr bool IsInstanceOf = std::false_type{};

    template<template<class...> class U, class... Vs>
    inline constexpr bool IsInstanceOf<U<Vs...>, U> = std::true_type{};

    template<typename T>
    concept ScalarType = std::integral<T> || std::floating_point<T>;
}
