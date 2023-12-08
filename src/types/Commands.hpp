#ifndef BOITATAH_COMMANDS_HPP
#define BOITATAH_COMMANDS_HPP
#include <vulkan/vulkan.h>

#include "Buffer.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"
#include "CommandBuffer.hpp"
#include "../collections/Pool.hpp"
#include "BttEnums.hpp"

namespace boitatah
{

    struct CopyBufferCommand
    {
        Handle<BufferReservation> src;
        Handle<BufferReservation> dst;
        CommandBuffer buffer;
    };


    struct TransferImageCommand
    {
        Handle<RenderTarget> src;
        Handle<RenderTarget> dst;
        CommandBuffer buffer;
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
    };

    struct SubmitDrawCommand
    {
        RenderTargetCmdBuffers bufferData;
        COMMAND_BUFFER_TYPE submitType;
    };

    struct SubmitCommand
    {
        COMMAND_BUFFER_TYPE submitType;
    };

    struct BeginBufferCmmand{
        //command buffer
        CommandBuffer buffer;
    };

    struct SubmitBufferCommand{
        //command buffer
        CommandBuffer buffer; 
        //type of submission
        COMMAND_BUFFER_TYPE submitType;
    };


}

#endif // BOITATAH_COMMANDS_HPP