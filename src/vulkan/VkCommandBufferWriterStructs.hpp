#ifndef BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_STRUCTS_HPP
#define BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_STRUCTS_HPP

#include <vulkan/vulkan.h>
#include "../command_buffers/CommandBufferWriterStructs.hpp"

namespace boitatah::vk{

    class Vulkan;
    class VkCommandBufferWriter;
    struct VulkanWriterBeginCommand {} ;


    struct VulkanWriterResetCommand {} ;

    struct VulkanWriterEndCommand {} ;

    struct VulkanWriterSubmitCommand {};

    struct VulkanWriterDrawCommand {};
    struct VulkanWriterBindPipelineCommand {};
    struct VulkanWriterCopyImageCommand {};
    struct VulkanWriterTransitionLayoutCommand {};

    struct VulkanWriterCopyBufferCommand 
    {
        uint32_t srcOffset;
        uint32_t dstOffset;
        uint32_t size;
        VkBuffer srcBuffer;
        VkBuffer dstBuffer;
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
    };

};

   
#endif