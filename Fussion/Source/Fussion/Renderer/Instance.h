﻿#pragma once
#include "Fussion/Window.h"

namespace Fussion
{
    class Instance
    {
    public:
        virtual ~Instance() = default;

        static Instance* Create(const Window& window);
    };
}