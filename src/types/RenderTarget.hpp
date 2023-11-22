#ifndef BOITATAH_RENDERTARGET_HPP
#define BOITATAH_RENDERTARGET_HPP

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include "BttEnums.hpp"
#include "../collections/Pool.hpp"
#include "Image.hpp"

#include <glm/vec2.hpp>

namespace boitatah{

    struct CommandBuffer
    {
        VkCommandBuffer buffer;
        COMMAND_BUFFER_TYPE type;
    };
    struct RTCmdBuffers{

        CommandBuffer drawBuffer;
        CommandBuffer transferBuffer;

        VkSemaphore schainAcqSem;
        VkSemaphore transferSem;
        VkFence inFlightFen;
    };

    struct AttachmentDesc{
        uint32_t index;
        FORMAT format;
        IMAGE_LAYOUT layout;
        SAMPLES samples = SAMPLES::SAMPLES_1;
        IMAGE_LAYOUT initialLayout;
        IMAGE_LAYOUT finalLayout;
    };

    //same attachment list as the framebufferdescription.
    struct RenderPassDesc{
        //FORMAT format;
        std::vector<AttachmentDesc> attachments;
    };

    struct RenderPass{
        VkRenderPass renderPass;
    };

    struct FramebufferDescVk{
        std::vector<VkImageView> views;
        VkRenderPass pass;
        glm::u32vec2 dimensions;
    };

    // If VK_NULL_HANDLE a new image is created.
    struct RenderTargetDesc{
        //Either RenderPassDesc or pass need to be non null.
        RenderPassDesc renderpassDesc;
        Handle<RenderPass> renderpass;

        std::vector<AttachmentDesc> attachments;
        std::vector<Handle<Image>> attachmentImages;
        //If attachmentImages are nullptr imageDesc is mandatory.
        std::vector<ImageDesc> imageDesc;
        glm::u32vec2 dimensions;
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