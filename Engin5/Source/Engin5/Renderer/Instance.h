#pragma once
#include "Engin5/Window.h"

namespace Engin5
{
    class Instance
    {
    public:
        virtual ~Instance() = default;

        static Instance* Create(const Window& window);
    };
}