#pragma once

#include <vulkan/vulkan.h>
#include <boitatah/BoitatahEnums.hpp>

namespace boitatah{

    struct MemoryDesc{
        uint64_t size;
        MEMORY_PROPERTY type;
        uint32_t typeBits;
    };

    struct Memory{
        VkDeviceMemory memory;
    };

}


