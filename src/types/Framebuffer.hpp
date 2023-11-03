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

    // If VK_NULL_HANDLE a new image is created.
    struct FramebufferDesc{
        RenderPassDesc renderpassDesc;
        std::vector<AttachmentDesc> attachments;
        Vector2<uint32_t> dimensions;
        VkImage image = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
    };

    //attachments here are images
    struct Framebuffer{
        VkFramebuffer buffer;
        std::vector<Handle<Image>> attachments;
        Handle<RenderPass> renderpass;
    };


}

#endif //BOITATAH_FRAME_BUFFER_HPP