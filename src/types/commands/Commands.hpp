#pragma once

#include <vulkan/vulkan.h>

#include "../../buffers/Buffer.hpp"
#include "../Shader.hpp"
#include "../RenderTarget.hpp"
#include "CommandBuffer.hpp"
#include "../../collections/Pool.hpp"
#include "../BttEnums.hpp"
#include "../Material.hpp"

namespace boitatah
{

    struct SubmitDrawCommand
    {
        RenderTargetCmdBuffers bufferData;
        COMMAND_BUFFER_TYPE submitType;
    };

    struct SubmitCommand
    {
        COMMAND_BUFFER_TYPE submitType;
    };

    struct BeginBufferCommand
    {
        // command buffer
        CommandBuffer buffer;
    };

    struct SubmitBufferCommand
    {
        // command buffer
        CommandBuffer buffer;
        // type of submission
        COMMAND_BUFFER_TYPE submitType;
    };

    struct CopyBufferCommand
    {
        Handle<BufferAddress> src;
        Handle<BufferAddress> dst;
        CommandBuffer buffer;
    };

    struct TransferImageCommand
    {
        Handle<RenderTarget> src;
        Handle<RenderTarget> dst;
        CommandBuffer buffer;
    };

    struct CopyDataToBufferDesc
    {
        Handle<BufferAddress> reservation;
        const void *data;
        uint64_t dataSize;
    };

    struct TransferUniformCommand{
        Handle<Uniform> uniform;
        uint32_t frameIndex;
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

        VkBuffer indexBuffer;
        uint32_t indexBufferOffset;
        uint32_t indexCount;

        // count, first
        glm::uvec2 vertexInfo;
        glm::uvec2 instanceInfo;

        std::vector<PushConstant> pushConstants;

    };

    struct BeginRenderpassCommand
    {
        CommandBuffer commandBuffer;
        RenderPass pass;
        RenderTarget target;

        glm::vec4 clearColor;

        glm::ivec2 scissorDims;
        glm::ivec2 scissorOffset;

        // glm::ivec2 viewportDims;
        // glm::ivec2 viewportOffset;
    };

}
