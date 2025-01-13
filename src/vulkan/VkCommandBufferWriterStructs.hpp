#pragma once

#include <vulkan/vulkan.h>
#include "../command_buffers/CommandBufferWriterStructs.hpp"
#include "../types/BttEnums.hpp"

namespace boitatah::vk{

    class VkCommandBufferWriter;
    struct VulkanWriterBeginCommand {} ;


    struct VulkanWriterResetCommand {} ;

    struct VulkanWriterEndCommand {} ;

    struct VulkanWriterSubmitCommand {
        //VkFence fence;
        //VkSemaphore signal;
        //VkSemaphore wait;
        COMMAND_BUFFER_TYPE submitType;
    };

    struct VulkanWriterDrawCommand {};
    struct VulkanWriterBindPipelineCommand {};
    struct VulkanWriterCopyImageCommand {};
    struct VulkanWriterTransitionLayoutCommand {};

    struct VulkanWriterCopyBufferCommand 
    {
        VkBuffer srcBuffer;
        uint32_t srcOffset;
        VkBuffer dstBuffer;
        uint32_t dstOffset;
        uint32_t size;
    };
};

// CommandBuffer Writer Type trait Definitions
namespace boitatah::command_buffers{

    template<>
    class CommandWriterTraits<boitatah::vk::VkCommandBufferWriter> {
        public :
            using BeginCommand = boitatah::vk::VulkanWriterBeginCommand;
            using ResetCommand = boitatah::vk::VulkanWriterResetCommand;
            using EndCommand = boitatah::vk::VulkanWriterEndCommand;
            using SubmitCommand = boitatah::vk::VulkanWriterSubmitCommand;
            using DrawCommand = boitatah::vk::VulkanWriterDrawCommand;
            using BindPipelineCommand = boitatah::vk::VulkanWriterBindPipelineCommand;
            using CopyImageCommand = boitatah::vk::VulkanWriterCopyImageCommand;
            using CopyBufferCommand = boitatah::vk::VulkanWriterCopyBufferCommand;
            using TransitionLayoutCommand = boitatah::vk::VulkanWriterTransitionLayoutCommand;
            using CommandBufferType = VkCommandBuffer;
            using SemaphoreType = VkSemaphore;
            using FenceType = VkFence;
    };

};

