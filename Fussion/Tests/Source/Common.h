#pragma once
#include <array>
#include <Fussion/Core/Types.h>
#include <Fussion/Log/Log.h>

std::array<u8, 16> uuidv7();

struct DebugObject {
    DebugObject()
    {
        LOG_DEBUGF("DebugObject::Constructed");
    }

    ~DebugObject()
    {
        LOG_DEBUGF("DebugObject::Destructed. Was Moved: {}", Moved);
    }

    DebugObject(DebugObject const& other)
    {
        LOG_DEBUGF("DebugObject::Copied");
    }

    DebugObject(DebugObject&& other) noexcept
    {
        other.Moved = true;
        LOG_DEBUGF("DebugObject::Moved");
    }

    DebugObject& operator=(DebugObject const& other)
    {
        if (this == &other)
            return *this;
        LOG_DEBUGF("DebugObject::Assigned");
        return *this;
    }

    DebugObject& operator=(DebugObject&& other) noexcept
    {
        if (this == &other)
            return *this;
        LOG_DEBUGF("DebugObject::Assigned-Moved?");
        return *this;
    }

    bool Moved{ false };
};