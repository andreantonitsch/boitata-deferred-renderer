#ifndef BOITATAH_BUFFER_HPP
#define BOITATAH_BUFFER_HPP

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "../vulkan/Vulkan.hpp"

#include "../collections/Pool.hpp"
#include "BufferStructs.hpp"
#include "BufferAllocator.hpp"

#include "../types/BttEnums.hpp"

namespace boitatah::buffer
{
   

    class Buffer
    {
    public:
        Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan);
        ~Buffer(void);

        Handle<BufferReservation> reserve(const uint32_t request);
        bool unreserve(const Handle<BufferReservation> reservation);

        //void updateBufferSync(const Handle<BufferReservation> handle, void * data);
        void queueUpdate(const Handle<BufferReservation> handle, void * data);

        bool checkCompatibility(const BufferReservationRequest &compatibility);

        VkBuffer getBuffer();
        VkDeviceMemory getMemory();

    private:
        VkBuffer buffer;
        VkDeviceMemory memory;
        const vk::Vulkan *vulkan;

        BufferDesc description;

        BUFFER_USAGE usage;
        SHARING_MODE sharing;

        uint32_t alignment;
        uint32_t actualSize;
        uint32_t largestchunk;

        Pool<BufferReservation> reservationPool{{.size = 50, .name = "buffer pool"}};

        BufferAllocator *allocator;


    };

}

#endif // BOITATAH_BUFFER_HPP