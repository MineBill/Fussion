﻿#pragma once
#include "Resources/ResourcePool.h"

namespace Fussion::RHI {
class FrameAllocator {
public:
    using FramePools = std::vector<Ref<ResourcePool>>;

    void Init(u32 frames);
    Ref<Resource> Alloc(Ref<ResourceLayout> layout);
    void Reset();

private:
    Ref<ResourcePool> GetAvailablePool(s32 max_resources);

    std::vector<FramePools> m_UsedPools;
    std::vector<FramePools> m_FreePools;

    FramePools m_CurrentFramePools;

    u32 m_CurrentFrame{ 0 };
    u32 m_MaxFrames{ 0 };
};

}