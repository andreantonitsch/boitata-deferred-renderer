#pragma once
#include <vulkan/vulkan.h>

namespace boitatah{

    struct TextureAccessData{
        VkImageView view;
        VkImage image;
        VkImageLayout layout;
        VkSampler sampler;
    };

}