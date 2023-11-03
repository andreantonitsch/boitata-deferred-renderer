#ifndef BOITATAH_FRAME_BUFFER_HPP
#define BOITATAH_FRAME_BUFFER_HPP

#include <vulkan/vulkan.h>
#include "BttEnums.hpp"
#include "../collections/Pool.hpp"
#include "Vector.hpp"

namespace boitatah{

    struct ImageDesc{

    };

    struct Image{
        VkImage image;
        VkImageView view;
        Vector2<uint32_t> dimensions;
        bool swapchain  = false;
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
    struct FramebufferDesc{
        //Either RenderPassDesc or pass need to be non null.
        RenderPassDesc renderpassDesc;
        RenderPass pass;

        std::vector<AttachmentDesc> attachments;
        std::vector<Handle<Image>> attachmentImages;
        Vector2<uint32_t> dimensions;
        //If image handles are VK_NULL_HANDLE imageDesc is mandatory.
        ImageDesc imageDesc;
    };

    //attachments here are images
    struct Framebuffer{
        VkFramebuffer buffer;
        std::vector<Handle<Image>> attachments;
        Handle<RenderPass> renderpass;
    };


}

#endif //BOITATAH_FRAME_BUFFER_HPP