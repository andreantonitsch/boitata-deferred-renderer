#ifndef BOITATAH_BUFFERVK_STRUCTS_HPP
#define BOITATAH_BUFFERVK_STRUCTS_HPP

#include <vulkan/vulkan.h>
#include "BttEnums.hpp"

namespace boitatah{


    struct BufferCompatibility
    {
        uint32_t requestSize;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferObjects{
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint32_t alignment;
        uint32_t actualSize;
    };

    struct BufferReservationRequest
    {
        uint32_t request;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferDescVk
    {
        // uint32_t alignment;
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };


}
#endif //BOITATAH_BUFFERVK_STRUCTS_HPP