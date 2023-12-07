#include "Buffer.hpp"
#include "../vulkan/Vulkan.hpp"
#include <bit>
namespace boitatah
{

    Buffer::Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan) : vulkan(vulkan)
    {

        this->description = desc;

        // get the vulkan buffer
        // allocate the device memory
        auto reqs = this->vulkan->getBufferAlignmentMemoryType({
            .size = desc.estimatedElementSize,
            .usage = desc.usage,
            .sharing = desc.sharing,
        });
        alignment = reqs.alignment;

        // initialize the allocator
        allocator = new BufferAllocator({.alignment = alignment,
                                         .partitionSize = desc.estimatedElementSize,
                                         .height = static_cast<uint32_t>(std::bit_width(desc.partitions)) - 1u,
                                         });

        auto objs = this->vulkan->createBuffer({
            .size = allocator->getSize(),
            .usage = desc.usage,
            .sharing = desc.sharing,
        });

        sharing = desc.sharing;
        usage = desc.usage;
        actualSize = objs.actualSize;
        buffer = objs.buffer;
        memory = objs.memory;
    
    }

    Buffer::~Buffer(void)
    {
        vulkan->destroyBuffer({.buffer = buffer, .memory = memory});
        delete allocator;
    }
    BufferReservation Buffer::reserve(const uint32_t request)
    {

        auto blockHandle = allocator->allocate(request);

        // failed to reserve a buffer.
        if (blockHandle.isNull())
            return BufferReservation{};

        Block block;
        BufferReservation reservation{
            .buffer = this,
        };
        allocator->getBlockData(blockHandle, reservation.offset, reservation.size);

        reservation.reservedBlock = blockHandle;
        reservation.requestSize = request;

        return reservation;
    }
    bool Buffer::checkCompatibility(const BufferCompatibility &compatibility)
    {
        std::cout << " testing compatibility " << static_cast<int>(usage) <<
        " " << static_cast<int>(compatibility.usage) << std::endl;
        return usage == compatibility.usage && 
        allocator->freeSpace() >= compatibility.requestSize;
    }
    VkBuffer Buffer::getBuffer()
    {
        return buffer;
        ;
    }
    VkDeviceMemory Buffer::getMemory()
    {
        return memory;
    }
}
