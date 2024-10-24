﻿#include "FussionPCH.h"
#include "Uuid.h"

#include <random>

namespace {
    std::random_device g_RandomDevice;
    std::mt19937_64 g_Engine(g_RandomDevice());
}

namespace Fussion {
    Uuid::Uuid()
        : m_ID(g_Engine())
    { }

    Uuid Uuid::Invalid { CAST(u64, -1) };
}
