#pragma once

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include "BttEnums.hpp"
#include "commands/CommandBuffer.hpp"
#include "../collections/Pool.hpp"
#include "Image.hpp"


#include <glm/vec2.hpp>

namespace boitatah{

    enum class AttachmentDescType{
        NEW_IMAGE,
        TEXTURE_TO_ATTACH,
        ATTACH_TO_ATTACH
    };

    struct AttachmentDesc{
        uint32_t index;
        IMAGE_FORMAT format;
        IMAGE_LAYOUT layout;
        SAMPLES samples = SAMPLES::SAMPLES_1;
        IMAGE_LAYOUT initialLayout;
        IMAGE_LAYOUT finalLayout;
        bool clear;
        //AttachmentDescType type = AttachmentDescType::NEW_IMAGE;
    };

    //same attachment list as the framebufferdescription.
    struct RenderPassDesc{
        //FORMAT format;
        std::vector<AttachmentDesc> color_attachments;
        bool use_depthStencil = false;
        AttachmentDesc depth_attachment;
        std::vector<std::pair<IMAGE_LAYOUT, IMAGE_LAYOUT>> attTransitions;
    };



    struct RenderPass{
        VkRenderPass renderPass;
        RenderPassDesc description;
    };

    struct FramebufferDescVk{
        std::vector<VkImageView> views;
        VkRenderPass pass;
        glm::u32vec2 dimensions;
    };

    struct RenderTargetDesc{
        //Either RenderPassDesc or pass need to be non null.
        RenderPassDesc renderpassDesc;
        Handle<RenderPass> renderpass;

        std::vector<AttachmentDesc> attachments;
        //If attachmentImages.size = 0 imageDesc is mandatory.
        std::vector<Handle<Image>> attachmentImages;
        std::vector<ImageDesc> imageDesc;
        glm::u32vec2 dimensions;
    };

    //attachments here are images
    struct RenderTarget{
        VkFramebuffer buffer;
        std::vector<Handle<Image>> attachments;
        Handle<RenderPass> renderpass;
        Handle<RenderTargetSync> cmdBuffers;
    };




}

