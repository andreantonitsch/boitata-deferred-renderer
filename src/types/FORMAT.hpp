#ifndef BOITATAH_FORMAT_HPP
#define BOITATAH_FORMAT_HPP

#include <vulkan/vulkan.h>

namespace boitatah
{

    // TODO We can decouple this in the future with a map.
    enum FORMAT
    {
        RGBA_8_SRGB = VK_FORMAT_R8G8B8A8_SRGB,
        RGBA_8_UNORM = VK_FORMAT_B8G8R8A8_UNORM,
        BGRA_8_SRGB = VK_FORMAT_B8G8R8A8_SRGB
    };

}
#endif // BOITATAH_FORMAT_HPP