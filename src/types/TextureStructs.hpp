#pragma once
#include <vulkan/vulkan.h>

namespace boitatah{

    struct TextureAccessData{
        VkImageView image;
        VkImageLayout layout;
        VkSampler sampler;
    };

}