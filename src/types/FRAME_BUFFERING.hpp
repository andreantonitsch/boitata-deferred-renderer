#ifndef BOITATAH_FRAMEBUFFERING_HPP
#define BOITATAH_FRAMEBUFFERING_HPP

#include <vulkan/vulkan.h>

namespace boitatah{

    //TODO We can decouple this in the future with a map.
    enum FRAME_BUFFERING {
        NO_BUFFER = VK_PRESENT_MODE_IMMEDIATE_KHR,
        VSYNC = VK_PRESENT_MODE_FIFO_KHR,
        TRIPLE_BUFFER = VK_PRESENT_MODE_MAILBOX_KHR,
    };

}
#endif //BOITATAH_FRAMEBUFFERING_HPP