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
        bool signal = false;
    };

    struct VulkanWriterDrawCommand {};
    struct VulkanWriterBindPipelineCommand {};
    struct VulkanWriterCopyImageCommand {
        VkImageLayout srcLayout;
        VkImageLayout dstLayout;
        glm::vec2 extent;
        VkImage srcImage;
        VkImage dstImage;
    };
    struct VulkanWriterTransitionLayoutCommand {
        VkImageLayout src;
        VkImageLayout dst;
        VkImage image;
    };

    struct VulkanWriterCopyBufferCommand 
    {
        VkBuffer srcBuffer;
        uint32_t srcOffset;
        VkBuffer dstBuffer;
        uint32_t dstOffset;
        uint32_t size;
    };

    struct VulkanWriterCopyBufferToImageCommand{
        VkBuffer buffer;
        VkImage image;

        uint32_t buffOffset;
        //uint32_t buffRow;   //in case of padding
        //uint32_t buffHeight;//in case of padding

        VkImageAspectFlagBits aspect;
        //uint8_t mipLevel;
        //uint8_t baseLayer;
        //uint8_t layerCount;

        glm::int3 offset;
        glm::u32vec3 extent;

        VkImageLayout srcImgLayout;
        VkImageLayout dstImgLayout;
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
            using CopyBufferToImageCommand = boitatah::vk::VulkanWriterCopyBufferToImageCommand;
            using CommandBufferType = VkCommandBuffer;
            using SemaphoreType = VkSemaphore;
            using FenceType = VkFence;
    };

};

