#ifndef BOITATAH_COLOR_SPACE_HPP
#define BOITATAH_COLOR_SPACE_HPP

#include <vulkan/vulkan.h>

namespace boitatah{

    //TODO We can decouple this in the future with a map.
    enum COLOR_SPACE {
        SRGB_NON_LINEAR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

}
#endif //BOITATAH_COLOR_SPACE_HPP