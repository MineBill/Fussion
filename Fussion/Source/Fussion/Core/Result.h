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
        Result(ValueType const& value): m_value(value) {}
        Result(ValueType&& value): m_value(std::move(value)) {}

        Result(ErrorT const& error): m_error(error) {}
        Result(ErrorT&& error): m_error(std::move(error)) {}

        ValueType& value()
        {
            return m_value.value();
        }

        ErrorType& error()
        {
            return m_error.value();
        }

        [[nodiscard]]
        bool is_value() const { return m_value.has_value(); }

        [[nodiscard]]
        bool is_error() const { return m_error.has_value(); }

        ValueType& operator*()
        {
            return m_value.value();
        }

    private:
        Maybe<ValueType> m_value{};
        Maybe<ErrorType> m_error{};
    };

    template<typename ErrorT>
    class [[nodiscard]] Result<void, ErrorT> {
    public:
        using ValueType = void;
        using ErrorType = ErrorT;

        constexpr Result() = default;
        constexpr Result(ErrorT const& error): m_error(error) {}
        constexpr Result(ErrorT&& error): m_error(std::move(error)) {}

        constexpr ErrorType& error()
        {
            return m_error.value();
        }

        [[nodiscard]]
        constexpr bool is_error() const { return m_error.has_value(); }

        [[nodiscard]]
        constexpr bool is_value() const { return !m_error.has_value(); }

    private:
        Maybe<ErrorType> m_error{};
    };
}
