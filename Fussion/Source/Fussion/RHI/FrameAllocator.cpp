#include "FussionPCH.h"
#include "FrameAllocator.h"

#include "Device.h"

namespace Fussion::RHI {
    constexpr s32 DEFAULT_MAX_RESOURCES = 200;

    void FrameAllocator::Init(u32 frames, std::string const& name)
    {
        m_Name = name;
        m_MaxFrames = frames;

        for (size_t i = 0; i < frames; i++) {
            m_UsedPools.emplace_back();
            m_FreePools.emplace_back();
            m_CurrentFramePools.push_back(nullptr);
        }
    }

    Ref<Resource> FrameAllocator::Alloc(Ref<ResourceLayout> const& layout, std::string const& name)
    {
        if (m_CurrentFramePools[m_CurrentFrame] == nullptr) {
            m_CurrentFramePools[m_CurrentFrame] = GetAvailablePool(DEFAULT_MAX_RESOURCES);
            m_UsedPools[m_CurrentFrame].push_back(m_CurrentFramePools[m_CurrentFrame]);
        }

        auto result = m_CurrentFramePools[m_CurrentFrame]->Allocate(layout, name);
        if (result.is_error()) {
            using enum ResourcePool::AllocationError;

            if (auto err = result.error();
                err == FragmentedPool || err == OutOfMemory) {
                auto pool = GetAvailablePool(DEFAULT_MAX_RESOURCES);
                m_CurrentFramePools[m_CurrentFrame] = pool;
                m_UsedPools[m_CurrentFrame].push_back(pool);

                auto new_result = pool->Allocate(layout);
                VERIFY(new_result.is_value());
                return new_result.value();
            }
        }
        return result.value();
    }

    void FrameAllocator::Reset()
    {
        m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFrames;
        for (auto const& pool : m_UsedPools[m_CurrentFrame]) {
            pool->Reset();
            m_FreePools[m_CurrentFrame].push_back(pool);
        }

        m_CurrentFramePools[m_CurrentFrame] = nullptr;
        m_UsedPools[m_CurrentFrame].clear();
    }

    Ref<ResourcePool> FrameAllocator::GetAvailablePool(s32 max_resources)
    {
        if (!m_FreePools[m_CurrentFrame].empty()) {
            auto pool = m_FreePools[m_CurrentFrame].back();
            m_FreePools[m_CurrentFrame].pop_back();
            return pool;
        }

        auto spec = ResourcePoolSpecification{
            .MaxSets = max_resources,
        };

        static std::unordered_map<ResourceType, f32> multipliers{
            { ResourceType::CombinedImageSampler, 1.0f },
            { ResourceType::InputAttachment, 1.0f },
            { ResourceType::UniformBuffer, 1.0f },
            { ResourceType::StorageBuffer, 1.0f },
            { ResourceType::StorageBufferDynamic, 1.0f },
        };

        for (auto const& [type, multiplier] : multipliers) {
            spec.ResourceLimits.push_back(ResourceLimit{
                .Type = type,
                .Limit = CAST(s32, multiplier * max_resources),
            });
        }

        return Device::Instance()->CreateResourcePool(spec);
    }
}
