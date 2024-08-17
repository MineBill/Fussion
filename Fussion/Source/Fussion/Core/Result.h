#pragma once
#include <Fussion/Core/Maybe.h>
#include <Fussion/Core/Concepts.h>

namespace Fussion {
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
            return m_Value.Value();
        }

        ErrorType& Error()
        {
            return m_Error.Value();
        }

        [[nodiscard]]
        bool IsValue() const { return m_Value.HasValue(); }

        [[nodiscard]]
        bool IsError() const { return m_Error.HasValue(); }

        ValueType& operator*()
        {
            return m_Value.Value();
        }

    private:
        Maybe<ValueType> m_Value{};
        Maybe<ErrorType> m_Error{};
    };

    template<typename ErrorT>
    class [[nodiscard]] Result<void, ErrorT> {
    public:
        using ValueType = void;
        using ErrorType = ErrorT;

        constexpr Result() = default;
        constexpr Result(ErrorT const& error): m_Error(error) {}
        constexpr Result(ErrorT&& error): m_Error(std::move(error)) {}

        constexpr ErrorType& Error()
        {
            return m_Error.Value();
        }

        [[nodiscard]]
        constexpr bool IsError() const { return m_Error.HasValue(); }

        [[nodiscard]]
        constexpr bool IsValue() const { return !m_Error.HasValue(); }

    private:
        Maybe<ErrorType> m_Error{};
    };
}
