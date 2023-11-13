#ifndef BOITATAH_RENDERTARGET_HPP
#define BOITATAH_RENDERTARGET_HPP

#include <vulkan/vulkan.h>
#include "BttEnums.hpp"
#include "../collections/Pool.hpp"
#include "Vector.hpp"
#include "Image.hpp"
//#include "CommandBuffer.hpp"

namespace boitatah{

    struct CommandBuffer
    {
        VkCommandBuffer buffer;
        COMMAND_BUFFER_TYPE type;
    };
    struct RTCmdBuffers{

        CommandBuffer drawBuffer;
        CommandBuffer transferBuffer;

        VkSemaphore acquireSem;
        VkSemaphore transferSem;
        VkFence inFlightFen;
    };

    struct AttachmentDesc{
        uint32_t index;
        FORMAT format;
        IMAGE_LAYOUT layout;
        SAMPLES samples = SAMPLES_1;
        IMAGE_LAYOUT initialLayout;
        IMAGE_LAYOUT finalLayout;
    };

    //same attachment list as the framebufferdescription.
    struct RenderPassDesc{
        FORMAT format;
        std::vector<AttachmentDesc> attachments;
    };

    struct RenderPass{
        VkRenderPass renderPass;
    };

    struct FramebufferDescVk{
        std::vector<VkImageView> views;
        VkRenderPass pass;
        Vector2<uint32_t> dimensions;
    };

    // If VK_NULL_HANDLE a new image is created.
    struct RenderTargetDesc{
        //Either RenderPassDesc or pass need to be non null.
        RenderPassDesc renderpassDesc;
        RenderPass pass;

        std::vector<AttachmentDesc> attachments;
        std::vector<Handle<Image>> attachmentImages;
        //If attachmentImages are nullptr imageDesc is mandatory.
        std::vector<ImageDesc> imageDesc;
        Vector2<uint32_t> dimensions;
    };

    //attachments here are images
    struct RenderTarget{
        VkFramebuffer buffer;
        std::vector<Handle<Image>> attachments;
        Handle<RenderPass> renderpass;
        Handle<RTCmdBuffers> cmdBuffers;
    };



}

#endif //BOITATAH_RENDERTARGET_HPP