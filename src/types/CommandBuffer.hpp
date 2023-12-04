#ifndef BOITATAH_COMMANDS_HPP
#define BOITATAH_COMMANDS_HPP
#include <vulkan/vulkan.h>

#include "RenderTarget.hpp"
#include "../collections/Pool.hpp"
#include "BttEnums.hpp"


namespace boitatah
{
    struct TransitionLayoutCmdVk{
        VkCommandBuffer buffer;
        VkImageLayout src;
        VkImageLayout dst;
        VkImage image;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        VkAccessFlags srcAccess;
        VkAccessFlags dstAccess;
    };

    struct CopyImageCommandVk{
        VkCommandBuffer buffer;
        //VkQueue queue;
        VkImage srcImage;
        VkImageLayout srcImgLayout;
        VkImage dstImage;
        VkImageLayout dstImgLayout;
        glm::u32vec2 extent;
    };

    struct TransferCommand{
        Handle<RenderTarget> src;
        Handle<RenderTarget> dst;
        CommandBuffer buffer;
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

    struct DrawCommand
    {
        CommandBuffer drawBuffer;
        RenderTarget renderTarget;
        RenderPass renderPass;
        Shader shader;
        glm::ivec2 dimensions;

        VkBuffer vertexBuffer;
        uint32_t vertexBufferOffset;

        // count, first
        glm::uvec2 vertexInfo;
        glm::uvec2 instanceInfo;
    };

    struct SubmitCommand{
        RTCmdBuffers bufferData;
        COMMAND_BUFFER_TYPE submitType;
    };

    struct CommandBufferDesc
    {
        uint32_t count;
        COMMAND_BUFFER_LEVEL level;
        COMMAND_BUFFER_TYPE type;
    };

    struct CopyToBufferOp{
        VkDeviceMemory memory;
        uint32_t offset;
        uint64_t size;
        void* data;
    };

}

#endif // BOITATAH_COMMANDS_HPP