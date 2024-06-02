#pragma once
#include "vulkan/vulkan.h"
#include "vulkan/vk_enum_string_helper.h"
#include "Engin5/Log/Log.h"

#define VK_CHECK(Function) \
    { \
        auto vk_check__result = Function; \
        if (vk_check__result != VK_SUCCESS) { \
            LOG_FATALF("VK_CHECK: Vulkan call `{}` failed with {}", #Function, string_VkResult(vk_check__result)); \
        } \
    }
