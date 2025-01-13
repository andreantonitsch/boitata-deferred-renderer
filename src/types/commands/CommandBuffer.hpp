#pragma once

#include <vulkan/vulkan.h>

#include "../BttEnums.hpp"

namespace boitatah{


    struct CommandBufferDesc
    {
        uint32_t count;
        COMMAND_BUFFER_LEVEL level;
        COMMAND_BUFFER_TYPE type;
    };
    
    struct CommandBuffer
    {
        VkCommandBuffer buffer;
        COMMAND_BUFFER_TYPE type;
    };

    //Render Target Command Buffers
    struct RenderTargetCmdBuffers{

        CommandBuffer drawBuffer;
        CommandBuffer transferBuffer;

        VkSemaphore schainAcqSem;
        VkSemaphore transferSem;
        VkFence inFlightFen;
    };
}
