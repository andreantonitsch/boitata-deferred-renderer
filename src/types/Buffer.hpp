#ifndef BOITATAH_BUFFER_HPP
#define BOITATAH_BUFFER_HPP

#include "../vulkan/Vulkan.hpp"
#include "BufferStructs.hpp"
#include "../collections/Pool.hpp"
#include "../collections/BufferAllocator.hpp"
//#include "../renderer/Renderer.hpp"
#include <vulkan/vulkan.h>
#include "BttEnums.hpp"
#include <glm/glm.hpp>

namespace boitatah
{

    struct BufferDesc
    {
        // uint32_t alignment;
        uint32_t size;
        // power of 2.
        uint32_t partitions;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    class Buffer
    {
    public:
        Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan);
        ~Buffer(void);

        BufferReservation reserve(const uint32_t request);
        bool unreserve(const Handle<BufferReservation> reservation);

        //BufferReservation checkReservation(const Handle<BufferReservation> reservation);

        bool checkCompatibility(const BufferCompatibility &compatibility);

        VkBuffer getBuffer();

    private:
        VkBuffer buffer;
        VkDeviceMemory memory;

        BUFFER_USAGE usage;
        SHARING_MODE sharing;

        uint32_t alignment;
        uint32_t actualSize;

        const vk::Vulkan *vulkan;

        Pool<BufferReservation> reservationPool{{.size = 50, .name = "buffer pool"}};

        BufferAllocator *allocator;

        //VkDeviceMemory getMemory();

    };

}

#endif // BOITATAH_BUFFER_HPP