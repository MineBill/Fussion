#pragma once
#include <concepts>

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

        ValueType& operator*()
        {
            return m_Value.value();
        }

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
