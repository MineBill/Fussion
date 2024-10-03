#pragma once
#include <functional>
#include <vector>

namespace Fussion {
    template<typename Signature = void()>
    class Delegate {
    public:
        using Function = std::function<Signature>;

        void Subscribe(Function const& function)
        {
            m_Subscribers.push_back(function);
        }

        template<typename... Args>
        void Fire(Args&&... args)
        {
            for (auto const& sub : m_Subscribers) {
                sub(std::forward<Args>(args)...);
            }
        }

        friend void operator+=(Delegate& lhs, Function const& func)
        {
            lhs.m_Subscribers.push_back(func);
        }

    private:
        std::vector<Function> m_Subscribers {};
    };
}
