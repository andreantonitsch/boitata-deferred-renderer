#ifndef BOITATAH_MEMORY_HPP
#define BOITATAH_MEMORY_HPP


#include <vulkan/vulkan.h>
#include "BttEnums.hpp"
namespace boitatah{

    struct MemoryDesc{
        uint64_t size;
        MEMORY_PROPERTY type;
        uint32_t typeBits;
        uint32_t index;
    };

    struct Memory{
        VkDeviceMemory memory;
    };

}


#endif //BOITATAH_MEMORY_HPP