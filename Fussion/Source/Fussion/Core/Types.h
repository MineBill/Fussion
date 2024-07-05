#pragma once
#include "e5pch.h"

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

constexpr s8 operator""_s8(unsigned long long int v) { return static_cast<s8>(v); }
constexpr s16 operator""_s16(unsigned long long int v) { return static_cast<s16>(v); }
constexpr s32 operator""_s32(unsigned long long int v) { return static_cast<s32>(v); }
constexpr s64 operator""_s64(unsigned long long int v) { return static_cast<s64>(v); }
constexpr u8 operator""_u8(unsigned long long int v) { return static_cast<u8>(v); }
constexpr u16 operator""_u16(unsigned long long int v) { return static_cast<u16>(v); }
constexpr u32 operator""_u32(unsigned long long int v) { return static_cast<u32>(v); }
constexpr u64 operator""_u64(unsigned long long int v) { return static_cast<u64>(v); }

using f32 = float;
using f64 = double;

// using Vector2 = glm::vec2;
// using Vector3 = glm::vec3;
// using Vector4 = glm::vec4;

using Quaternion = glm::quat;

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

template<typename T>
using Ptr = std::unique_ptr<T>;

template<typename T, typename... Args>
Ptr<T> MakePtr(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename... Args>
Ref<T> MakeRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
concept ScalarType = std::integral<T> || std::floating_point<T>;