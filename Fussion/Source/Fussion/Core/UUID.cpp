#include "e5pch.h"
#include "Uuid.h"

#include <random>

namespace
{
    std::random_device g_RandomDevice;
    std::mt19937_64 g_Engine(g_RandomDevice());
}

namespace Fussion
{
    Uuid::Uuid()
        : m_Id(g_Engine())
    {
    }
}