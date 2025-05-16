#pragma once

#include <vulkan/vulkan.h>
#include "../command_buffers/CommandBufferWriterStructs.hpp"
#include "../types/BttEnums.hpp"
#include <vector>

namespace boitatah::vk{

    class VkCommandBufferWriter;
    struct VulkanWriterBegin {} ;


    struct VulkanWriterReset {} ;

    struct VulkanWriterEnd {} ;

    struct VulkanWriterEndRenderpass {} ;

    struct VulkanWriterSubmit {
        COMMAND_BUFFER_TYPE submitType;
        bool signal = false;
    };

    struct VulkanWriterDraw {
        uint32_t vertexCount;
        uint32_t instaceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

        bool indexed;
        uint32_t indexCount;
    };

    struct VulkanWriterBindPipeline {
        VkPipeline pipeline;
    };


    struct VulkanWriterCopyImage {
        VkImageLayout srcLayout;
        VkImageLayout dstLayout;
        glm::vec2 extent;
        VkImage srcImage;
        VkImage dstImage;
    };
    struct VulkanWriterTransitionLayout {
        VkImageLayout src;
        VkImageLayout dst;
        VkImage image;
        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;
    };

    struct VulkanWriterCopyBuffer {
        VkBuffer srcBuffer;
        uint32_t srcOffset;
        VkBuffer dstBuffer;
        uint32_t dstOffset;
        uint32_t size;
    };

    struct VulkanWriterCopyBufferToImage {
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

    struct VulkanWriterBeginRenderpass {
        VkRenderPass pass;
        VkFramebuffer frame_buffer;

        std::vector<glm::vec4> clearColors;

        glm::ivec2 scissorDims;
        glm::ivec2 scissorOffset;

        bool depth = false;
        uint32_t attachment_count = 1;
    };

    struct VulkanWriterBindVertexBuffer {
        std::vector<VkBuffer> buffers;
        std::vector<VkDeviceSize> offsets;
    };

    struct VulkanWriterBindIndexBuffer {
        VkBuffer buffers;
        VkDeviceSize offsets;
    };

    struct VulkanWriterBindSet {
        VkPipelineLayout layout;
        VkDescriptorSet  set;
        uint32_t         set_index;
    };

    struct VulkanPushConstant{
        void* ptr;
        uint32_t offset;
        uint32_t size;
        VkShaderStageFlags stages;
    };
    
    struct VulkanPushConstants{
        VkPipelineLayout layout;
        std::vector<VulkanPushConstant> push_constants;
    };

};

// CommandBuffer Writer Type trait Definitions
namespace boitatah::command_buffers{

    template<>
    class CommandWriterTraits<boitatah::vk::VkCommandBufferWriter> {
        public :
            using BeginCommand = boitatah::vk::VulkanWriterBegin;
            using ResetCommand = boitatah::vk::VulkanWriterReset;
            using EndCommand = boitatah::vk::VulkanWriterEnd;
            using SubmitCommand = boitatah::vk::VulkanWriterSubmit;

            using BeginRenderpassCommand = boitatah::vk::VulkanWriterBeginRenderpass;
            using EndRenderpassCommand   = boitatah::vk::VulkanWriterEndRenderpass;

            using BindPipelineCommand = boitatah::vk::VulkanWriterBindPipeline;
            using BindVertexBufferCommand = boitatah::vk::VulkanWriterBindVertexBuffer;
            using BindIndexBufferCommand = boitatah::vk::VulkanWriterBindIndexBuffer;
            using BindSetCommand = boitatah::vk::VulkanWriterBindSet;

            using DrawCommand = boitatah::vk::VulkanWriterDraw;

            using CopyImageCommand = boitatah::vk::VulkanWriterCopyImage;
            using CopyBufferCommand = boitatah::vk::VulkanWriterCopyBuffer;
            using TransitionLayoutCommand = boitatah::vk::VulkanWriterTransitionLayout;
            using CopyBufferToImageCommand = boitatah::vk::VulkanWriterCopyBufferToImage;

            using PushConstantsCommand = boitatah::vk::VulkanPushConstants;

            using CommandBufferType = VkCommandBuffer;
            using SemaphoreType = VkSemaphore;
            using FenceType = VkFence;
    };

};

