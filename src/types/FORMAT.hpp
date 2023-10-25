#ifndef BOITATAH_FORMAT_HPP
#define BOITATAH_FORMAT_HPP

#include <vulkan/vulkan.h>

namespace boitatah
{

    // TODO We can decouple this in the future with a map.
    enum FORMAT
    {
        RGBA_8_SRGB = 1,
        BGRA_8_SRGB = 2,
        RGBA_8_UNORM = 3,
        BGRA_8_UNORM = 4,
    };

}
#endif // BOITATAH_FORMAT_HPP