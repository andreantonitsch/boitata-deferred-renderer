#ifndef BOITATAH_COMMANDS_HPP
#define BOITATAH_COMMANDS_HPP
#include <vulkan/vulkan.h>

#include "../collections/Pool.hpp"
#include "BttEnums.hpp"
#include "RenderTarget.hpp"

namespace boitatah
{

    struct EndCommandBuffer{
        
    };

    struct CommandBuffer
    {
        VkCommandBuffer buffer;
        COMMAND_BUFFER_TYPE type;
    };
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
        Vector2<uint32_t> extent;
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
        Vector2<int> areaDims;
        Vector2<int> areaOffset;
        uint32_t vertexCount;
        uint32_t instaceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

    };

    struct DrawCommand
    {
        Handle<RenderTarget> buffer;
    };

    struct CommandBufferDesc
    {
        uint32_t count;
        COMMAND_BUFFER_LEVEL level;
        COMMAND_BUFFER_TYPE type;
    };

}

#endif // BOITATAH_COMMANDS_HPP