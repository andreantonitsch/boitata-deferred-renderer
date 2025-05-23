#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace boitatah::vk
{
    struct PresentCommandVk
    {
        //VkCommandBuffer commandBuffer;
        std::vector<VkSemaphore> waitSemaphores;
        VkFence fence;
    };


    struct BindPipelineCommandVk{
        VkCommandBuffer drawBuffer;
        VkPipeline pipeline;
    };

    struct SubmitDrawCommandVk
    {
        VkCommandBuffer commandBuffer;
        VkFence fence;
        std::vector<VkSemaphore>  wait_semaphores;
        VkSemaphore*  signal_semaphore = nullptr;
    };

    struct BeginCommandVk{
        VkCommandBuffer commandBuffer;
    };

    struct BeginRenderpassCommandVk{
        VkCommandBuffer commandBuffer;
        VkRenderPass pass;
        VkFramebuffer frameBuffer;

        std::vector<glm::vec4> clearColors;

        glm::ivec2 scissorDims;
        glm::ivec2 scissorOffset;

        bool depth = false;
        uint32_t attachmentCount = 1;
    };
    struct EndRenderpassCommandVk{
        VkCommandBuffer commandBuffer;
    };

    struct SubmitCommandVk
    {
        VkCommandBuffer commandBuffer;
        COMMAND_BUFFER_TYPE submitType;
        VkFence fence;
        VkSemaphore signal;
        VkSemaphore wait;
    };


    struct CopyToBufferVk
    {
        VkDeviceMemory memory;
        uint32_t offset;
        uint32_t size;
        void *data;
    };

    struct CopyBufferCommandVk
    {
        VkCommandBuffer commandBuffer;
        VkBuffer srcBuffer;
        uint32_t srcOffset;
        VkBuffer dstBuffer;
        uint32_t dstOffset;
        uint32_t size;
    };

    struct CopyImageCommandVk
    {
        VkCommandBuffer buffer;
        // VkQueue queue;
        VkImage srcImage;
        VkImageLayout srcImgLayout;
        VkImage dstImage;
        VkImageLayout dstImgLayout;
        glm::u32vec2 extent;
    };

    struct TransitionLayoutCmdVk
    {
        VkCommandBuffer buffer;
        VkImageLayout src;
        VkImageLayout dst;
        VkImage image;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        VkAccessFlags srcAccess;
        VkAccessFlags dstAccess;
    };

    struct DrawCommandVk
    {
        VkCommandBuffer drawBuffer;
        uint32_t vertexCount;
        uint32_t instaceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

        bool indexed;
        uint32_t indexCount;
        //std::vector<PushConstant> pushConstants;
    };


    struct MapMemoryVk{
        VkDeviceMemory memory;
        uint64_t offset;
        uint64_t size;
    };
    
    struct UnmapMemoryVk{
        VkDeviceMemory memory;
    };

    // sizes and offsets in bytes
    struct CopyMappedMemoryVk
    {
        uint32_t offset;
        uint32_t elementSize;
        uint32_t elementCount;
        void *map;
        void *data;
    };

}