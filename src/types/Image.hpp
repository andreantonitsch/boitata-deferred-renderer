#ifndef BOITATAH_IMAGE_HPP
#define BOITATAH_IMAGE_HPP

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include "BttEnums.hpp"
#include "../collections/Pool.hpp"

namespace boitatah
{
    struct ImageDesc
    {
        FORMAT format;
        glm::u32vec2 dimensions;
        uint32_t mipLevels = 1;
        IMAGE_LAYOUT initialLayout;
        IMAGE_USAGE usage;
        SAMPLES samples = SAMPLES::SAMPLES_1;
    };

    struct Image
    {
        VkImage image;
        VkImageView view;
        glm::u32vec2 dimensions;
        bool swapchain = false;
        VkDeviceMemory memory;
    };

}

#endif // BOITATAH_IMAGE_HPP