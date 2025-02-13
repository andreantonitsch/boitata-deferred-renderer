#pragma once

#include <vulkan/vulkan.h>

#include <buffers/Buffer.hpp>
#include "../Shader.hpp"
#include "../RenderTarget.hpp"
#include "CommandBuffer.hpp"
#include "../../collections/Pool.hpp"
#include "../BttEnums.hpp"

namespace boitatah
{
    using namespace boitatah::buffer;

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

    struct DrawCommand
    {
        CommandBuffer drawBuffer;
        uint32_t indexCount;
        // count, first
        glm::uvec2 vertexInfo;
        glm::uvec2 instanceInfo;
        bool indexed = true;
        //std::vector<PushConstant> pushConstants;

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

    struct BindPipelineCommand{
        CommandBuffer commandBuffer;
        Handle<Shader> shader;
    };

    struct PushConstantsCommand{
        CommandBuffer drawBuffer;
        ShaderLayout layout;
        std::vector<PushConstant> push_constants;
    };

    struct BindSetCommand{
        CommandBuffer drawBuffer;
        uint32_t set_index;
        DescriptorSetLayout set_layout;
        ShaderLayout shader_layout;
        std::vector<BindBindingDesc> bindings;
    };

    class Geometry;
    struct BindVertexBuffersCommand{
        CommandBuffer commandBuffer;
        uint32_t frame;
        Handle<Geometry> geometry;
        bool bindIndex = true;
    };

}
