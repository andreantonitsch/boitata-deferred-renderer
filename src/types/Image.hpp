#ifndef BOITATAH_IMAGE_HPP
#define BOITATAH_IMAGE_HPP

#include <vulkan/vulkan.h>
#include "Vector.hpp"
#include "BttEnums.hpp"
#include "../collections/Pool.hpp"

namespace boitatah
{
    struct ImageDesc
    {
        FORMAT format;
        Vector2<uint32_t> dimensions;
        uint32_t mipLevels = 1;
        IMAGE_LAYOUT initialLayout;
        USAGE usage;
        SAMPLES samples = SAMPLES_1;
    };

    struct Image
    {
        VkImage image;
        VkImageView view;
        Vector2<uint32_t> dimensions;
        bool swapchain = false;
        VkDeviceMemory memory;
    };

}

#endif // BOITATAH_IMAGE_HPP