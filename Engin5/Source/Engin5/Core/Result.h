#pragma once
#include "Core.h"
#include <concepts>

#include "Types.h"

namespace Engin5
{
    template <typename T>
    class Ok {
    public:
        explicit constexpr Ok(T value) : Value(std::move(value)) {}

        constexpr T&& TakeValue() { return std::move(Value); }

        T Value;
    };

    template <typename T>
    class Err {
    public:
        explicit constexpr Err(T value) : Value(std::move(value)) {}

        constexpr T&& TakeValue() { return std::move(Value); }

        T Value;
    };

    template <typename OkT, typename ErrT>
    class Result {
    public:
        using VariantT = std::variant<Ok<OkT>, Err<ErrT>>;

        constexpr Result(Ok<OkT> value) : m_Variant(std::move(value))
        {
        }

        constexpr Result(Err<ErrT> value) : m_Variant(std::move(value))
        {
        }

        require_results constexpr bool IsValue() const { return std::holds_alternative<Ok<OkT>>(m_Variant); }
        require_results constexpr bool IsError() const { return std::holds_alternative<Err<ErrT>>(m_Variant); }

        require_results constexpr OkT Value() const { return std::get<Ok<OkT>>(m_Variant).Value; }
        require_results constexpr ErrT Error() const { return std::get<Err<ErrT>>(m_Variant).Value; }

        require_results constexpr OkT&& TakeValue() { return std::get<Ok<OkT>>(m_Variant).TakeValue(); }
        require_results constexpr ErrT&& TakeError() { return std::get<Err<ErrT>>(m_Variant).TakeValue(); }

    private:
        VariantT m_Variant;
    };
}