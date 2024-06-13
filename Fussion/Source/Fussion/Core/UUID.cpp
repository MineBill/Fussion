#include "e5pch.h"
#include "UUID.h"

#include <random>

namespace
{
    std::random_device g_RandomDevice;
    std::mt19937_64 g_Engine(g_RandomDevice());
}

namespace Fussion
{
    UUID::UUID()
        : m_Id(g_Engine())
    {
    }
}