#pragma once
#include "Core.h"
#include <concepts>
#include "Types.h"

namespace Fussion {
// template<typename T>
// class Ok {
// public:
//     explicit constexpr Ok(T value) : Value(std::move(value)) {}
//
//     constexpr T&& TakeValue() { return std::move(Value); }
//
//     T Value;
// };
//
// template<typename T>
// class Err {
// public:
//     explicit constexpr Err(T value) : Value(std::move(value)) {}
//
//     constexpr T&& TakeValue() { return std::move(Value); }
//
//     T Value;
// };
//
// template<typename OkT, typename ErrT>
// class Result {
// public:
//     using VariantT = std::variant<Ok<OkT>, Err<ErrT>>;
//
//     constexpr Result(Ok<OkT> value) : m_Variant(std::move(value)) {}
//
//     constexpr Result(Err<ErrT> value) : m_Variant(std::move(value)) {}
//
//     require_results constexpr bool IsValue() const { return std::holds_alternative<Ok<OkT>>(m_Variant); }
//     require_results constexpr bool IsError() const { return std::holds_alternative<Err<ErrT>>(m_Variant); }
//
//     require_results constexpr OkT Value() const { return std::get<Ok<OkT>>(m_Variant).Value; }
//     require_results constexpr ErrT Error() const { return std::get<Err<ErrT>>(m_Variant).Value; }
//
//     require_results constexpr OkT&& TakeValue() { return std::get<Ok<OkT>>(m_Variant).TakeValue(); }
//     require_results constexpr ErrT&& TakeError() { return std::get<Err<ErrT>>(m_Variant).TakeValue(); }
//
// private:
//     VariantT m_Variant;
// };

template<typename ValueT, typename ErrorT>
class [[nodiscard]] Result {
public:
    using ValueType = ValueT;
    using ErrorType = ErrorT;

    Result() = default;
    Result(ValueType const& value): m_Value(value) {}
    Result(ValueType&& value): m_Value(std::move(value)) {}

    Result(ErrorT const& error): m_Error(error) {}
    Result(ErrorT&& error): m_Error(std::move(error)) {}

    ValueType& Value()
    {
        return m_Value.value();
    }

    ErrorType& Error()
    {
        return m_Error.value();
    }

    [[nodiscard]]
    bool IsValue() const { return m_Value.has_value(); }

    [[nodiscard]]
    bool IsError() const { return m_Error.has_value(); }

private:
    std::optional<ValueType> m_Value{};
    std::optional<ErrorType> m_Error{};
};

template<typename ErrorT>
class [[nodiscard]] Result<void, ErrorT> {
public:
    using ValueType = void;
    using ErrorType = ErrorT;

    Result() = default;
    Result(ErrorT const& error): m_Error(error) {}
    Result(ErrorT&& error): m_Error(std::move(error)) {}

    ErrorType& Error()
    {
        return m_Error.value();
    }

    [[nodiscard]]
    bool IsError() const { return m_Error.has_value(); }

    [[nodiscard]]
    bool IsValue() const { return !m_Error.has_value(); }

private:
    std::optional<ErrorType> m_Error{};
};
}
