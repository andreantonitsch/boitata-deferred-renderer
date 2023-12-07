#ifndef BOITATAH_BUFFER_HPP
#define BOITATAH_BUFFER_HPP

#include "../vulkan/Vulkan.hpp"
#include "../collections/Pool.hpp"
#include "../collections/BufferAllocator.hpp"
//#include "../renderer/Renderer.hpp"
#include <vulkan/vulkan.h>
#include "BttEnums.hpp"
#include <glm/glm.hpp>

namespace boitatah
{

    class Buffer;

    struct BufferCompatibility
    {
        uint32_t requestSize;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferReservationRequest
    {
        uint32_t request;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferReservation
    {
        Buffer* buffer;
        uint32_t size;
        uint32_t requestSize;
        uint32_t offset;
        Handle<Block> reservedBlock;
    };

    struct BufferDesc
    {
        // uint32_t alignment;
        uint32_t estimatedElementSize;
        // power of 2.
        uint32_t partitions;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferUploadDesc{
        uint32_t dataSize;
        void* data;
        BUFFER_USAGE usage;
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


        Pool<BufferReservation> reservationPool{{.size = 50, .name = "buffer pool"}};

        BufferAllocator *allocator;


    };

}

#endif // BOITATAH_BUFFER_HPP