#ifndef BOITATAH_COMMANDS_HPP
#define BOITATAH_COMMANDS_HPP
#include <vulkan/vulkan.h>

#include "../collections/Pool.hpp"
#include "BttEnums.hpp"
namespace boitatah{


    struct CommandBufferDesc{
        uint32_t count;
        COMMAND_BUFFER_LEVEL level;
    };

    struct CommandBuffer{
        VkCommandBuffer buffer;
    };
}

#endif //BOITATAH_COMMANDS_HPP