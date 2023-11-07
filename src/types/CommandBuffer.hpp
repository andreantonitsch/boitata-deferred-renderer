#ifndef BOITATAH_COMMANDS_HPP
#define BOITATAH_COMMANDS_HPP
#include <vulkan/vulkan.h>

#include "../collections/Pool.hpp"
#include "BttEnums.hpp"
#include "Framebuffer.hpp"

namespace boitatah
{

    struct DrawCommandVk
    {
        VkCommandBuffer drawBuffer;
        VkRenderPass pass;
        VkFramebuffer frameBuffer;
        Vector2<int> areaDims;
        Vector2<int> areaOffset;
        uint32_t vertexCount;
        uint32_t instaceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

    };

    struct DrawCommand
    {
        Handle<Framebuffer> buffer;
    };

    struct CommandBufferDesc
    {
        uint32_t count;
        COMMAND_BUFFER_LEVEL level;
    };

    struct CommandBuffer
    {
        VkCommandBuffer buffer;
    };
}

#endif // BOITATAH_COMMANDS_HPP