#ifndef BOITATAH_COMMANDS_VK_HPP
#define BOITATAH_COMMANDS_VK_HPP
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace boitatah::vk
{
    struct PresentCommandVk
    {
        VkCommandBuffer commandBuffer;
        VkSemaphore waitSemaphore;
        VkSemaphore signalSemaphore;
        VkFence fence;

        //RenderTargetCmdBuffers bufferData;
        //COMMAND_BUFFER_TYPE submitType;
    };

    struct SubmitDrawCommandVk
    {
        VkCommandBuffer commandBuffer;
        VkFence fence;

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
        uint64_t size;
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
        VkRenderPass pass;
        VkFramebuffer frameBuffer;
        VkPipeline pipeline;

        VkBuffer vertexBuffer;
        uint32_t vertexBufferOffset;

        glm::ivec2 areaDims;
        glm::ivec2 areaOffset;
        uint32_t vertexCount;
        uint32_t instaceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };



}
#endif //BOITATAH_COMMANDS_VK_HPP