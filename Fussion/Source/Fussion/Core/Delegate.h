#pragma once
#include <vector>
#include <functional>

namespace Fussion {
template<typename Signature = void()>
class Delegate {
public:
    using Function = std::function<Signature>;

    void subscribe(Function const& function)
    {
        m_subscribers.push_back(function);
    }

    template<typename... Args>
    void fire(Args&&... args)
    {
        for (auto const& sub : m_subscribers) {
            sub(std::forward<Args>(args)...);
        }
    }

    friend void operator+=(Delegate& lhs, Function const& func)
    {
        lhs.m_subscribers.push_back(func);
    }

private:
    std::vector<Function> m_subscribers{};
};
}
