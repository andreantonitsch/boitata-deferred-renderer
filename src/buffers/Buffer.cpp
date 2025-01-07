#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "../vulkan/Vulkan.hpp"
#include <bit>
namespace boitatah::buffer
{


    Buffer::Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan) : vulkan(vulkan)
    {
        description = desc;

        // get the vulkan buffer
        // allocate the device memory
        auto reqs = this->vulkan->getBufferAlignmentMemoryType({
            .size = desc.estimatedElementSize,
            .usage = desc.usage,
            .sharing = desc.sharing,
        });

        alignment = reqs.alignment;
        sharing = desc.sharing;
        usage = desc.usage;
        buffer_id = buffer_quantity;
        
        // if(sharing == SHARING_MODE::EXCLUSIVE)
        //     bufferManager = desc.bufferManager;

        setupBuffer(desc.estimatedElementSize, desc.partitions);

        blockSize = mainAllocator->getPartitionSize();
        buffer_quantity += 1;
        bufferManager = desc.bufferManager;
    }

    Buffer::~Buffer(void)
    {
        if(sharing == SHARING_MODE::CONCURRENT)
            vulkan->unmapMemory({bufferData.memory});

        vulkan->destroyBuffer(bufferData);
        buffer_quantity -= 1;
    }

    uint32_t Buffer::getID() const
    {
        return buffer_id;
    }

    bool Buffer::getReservationData(const Handle<BufferReservation> handle, BufferReservation &reservation) const
    {
        if(mainReservPool->tryGet(handle, reservation))
            return true;
        return false;
    }

    Handle<BufferReservation> Buffer::reserve(const uint32_t request)
    {
        auto blockHandle = mainAllocator->allocate(request);

        // failed to reserve a buffer.
        if (blockHandle.isNull())
            return Handle<BufferReservation>{};

        BufferReservation reservation{};
        mainAllocator->getBlockData(blockHandle, reservation.offset, reservation.size);

        reservation.reservedBlock = blockHandle;
        reservation.requestSize = request;

        Handle<BufferReservation> reservationHandle = mainReservPool->set(reservation);
        return reservationHandle;
    }


    bool Buffer::unreserve(const Handle<BufferReservation> reservation)
    {
        if (reservation.isNull())
            return false;

        BufferReservation bufferReservation;
        if(!mainReservPool->tryGet(reservation, bufferReservation))
            throw std::runtime_error("reservation double release");
        
        mainAllocator->release(bufferReservation.reservedBlock);

        return true;
    }

    void Buffer::copyData(const Handle<BufferReservation> handle, void *data)
    {
        BufferReservation reservation;
            if(!mainReservPool->tryGet(handle, reservation)) 
                throw std::runtime_error("failed to copy data to gpu buffer");

        std::cout << "Buffer " << getID() << " copy data reseevation offset" << reservation.offset <<
                     " size " << reservation.size << std::endl;

        if(sharing == SHARING_MODE::CONCURRENT){
            vulkan->copyToMappedMemory({
                .offset = reservation.offset,
                .elementSize = reservation.size,
                .elementCount = static_cast<uint32_t>(1),
                .map = mappedMemory,
                .data = data,
            });
        }
        // else{
        //     if(queueTransfer){
        //         std::shared_ptr<BufferManager> manager(bufferManager);

        //         auto stagingAddress = manager->reserveBuffer({
        //             .request = reservation.requestSize,
        //             .usage = BUFFER_USAGE::TRANSFER_SRC,
        //             .sharing = SHARING_MODE::CONCURRENT,
        //         });

        //         manager->copyToBuffer({.address = stagingAddress, 
        //                                 .dataSize = reservation.requestSize,
        //                                 .data = data, 
        //         });

        //         queuedTransfers.emplace_back(stagingAddress, handle);
        //     }
        // }
    }

    // void Buffer::copyDataFromBuffer(const Handle<BufferReservation> dst, const Handle<BufferAddress> src)
    // {
    //         std::shared_ptr<BufferManager> manager(bufferManager);

    //         Buffer* srcBuffer;

    //         BufferReservation srcReservation;
    //         BufferReservation dstReservation;

    //         if(!manager->getAddressBuffer(src, srcBuffer))
    //             std::runtime_error("buffer transfer failed, invalid staging buffer");
            
    //         if(!manager->getAddressReservation(src,srcReservation))
    //             std::runtime_error("buffer transfer failed, invalid staging reservation");

    //         if(!getReservationData(dst,dstReservation))
    //             std::runtime_error("buffer transfer failed, invalid staging reservation");

    //         vulkan->CmdCopyBuffer({
    //             .commandBuffer = manager->getTransferBuffer().buffer,
    //             .srcBuffer = srcBuffer->getBuffer(),
    //             .srcOffset = srcReservation.offset,
    //             .dstBuffer = getBuffer(),
    //             .dstOffset = dstReservation.offset,
    //             .size = dstReservation.requestSize,
    //         });
    // }

    bool Buffer::hasUpdates()
    {
        return queuedTransfers.size() > 0;
    }

    // void Buffer::queueTransfers()
    // {
    //     std::shared_ptr<BufferManager> manager(bufferManager);
    //     //do the queues

    //     for(auto& transfer : queuedTransfers)
    //     {
    //         Buffer* srcBuffer;
    //         Buffer* dstBuffer;

    //         BufferReservation srcReservation;
    //         BufferReservation dstReservation;

    //         if(!manager->getAddressBuffer(transfer.stagingBufferAddress,srcBuffer))
    //             std::runtime_error("buffer transfer failed, invalid staging buffer");
            
    //         if(mainReservPool->tryGet(transfer.finalBufferReservation ,dstReservation))
    //             std::runtime_error("buffer transfer failed, invalid final reservation");

    //         if(!manager->getAddressReservation(transfer.stagingBufferAddress,srcReservation))
    //             std::runtime_error("buffer transfer failed, invalid staging reservation");

    //         vulkan->CmdCopyBuffer({
    //             .commandBuffer = manager->getTransferBuffer().buffer,
    //             .srcBuffer = srcBuffer->getBuffer(),
    //             .srcOffset = srcReservation.offset,
    //             .dstBuffer = this->getBuffer(),
    //             .dstOffset = dstReservation.offset,
    //             .size = dstReservation.requestSize,
    //         });
    //     }

    // }

    // /// @brief Transfer from src into this buffer at BufferReservation dst
    // /// @param src 
    // /// @param dst 
    // void Buffer::queueTransfer(Handle<BufferAddress> src, Handle<BufferReservation> dst)
    // {
    //         std::shared_ptr<BufferManager> manager(bufferManager);

    //         Buffer* srcBuffer;

    //         BufferReservation srcReservation;
    //         BufferReservation dstReservation;

    //         if(!manager->getAddressBuffer(src, srcBuffer))
    //             std::runtime_error("buffer transfer failed, invalid staging buffer");
            
    //         if(!manager->getAddressReservation(src,srcReservation))
    //             std::runtime_error("buffer transfer failed, invalid staging reservation");

    //         if(!getReservationData(dst,dstReservation))
    //             std::runtime_error("buffer transfer failed, invalid staging reservation");

    //         vulkan->CmdCopyBuffer({
    //             .commandBuffer = manager->getTransferBuffer().buffer,
    //             .srcBuffer = srcBuffer->getBuffer(),
    //             .srcOffset = srcReservation.offset,
    //             .dstBuffer = getBuffer(),
    //             .dstOffset = dstReservation.offset,
    //             .size = dstReservation.requestSize,
    //         });
    // }

    // void Buffer::clearTransferQueue()
    // {
    //     std::shared_ptr<BufferManager> manager(bufferManager);
    //     for(auto& transfer : queuedTransfers){
    //         manager ->freeBufferReservation(transfer.stagingBufferAddress);
    //     }
    //     queuedTransfers.clear();
    // }

    bool Buffer::checkCompatibility(const BufferReservationRequest &compatibility)
    {
        return usage == compatibility.usage && 
               sharing == compatibility.sharing &&
               mainAllocator->getLargestFreeBlockSize() >= compatibility.request;
    }

    VkBuffer Buffer::getBuffer() const
    {
            return bufferData.buffer;
    }
    VkDeviceMemory Buffer::getMemory() const
    {
        return bufferData.memory;
    }

    void Buffer::setupBuffer(uint32_t desiredSize, uint32_t partitions){

        mainAllocator.reset(new BufferAllocator({.alignment = alignment,
                                        .partitionSize = desiredSize,
                                        .height = static_cast<uint32_t>(std::bit_width(partitions)) - 1u,
                                        }));

        bufferData = this->vulkan->createBuffer({
            .size = mainAllocator->getSize(),
            .usage = usage,
            .sharing = sharing,
        });
        
        mainReservPool.reset(new Pool<BufferReservation>(
            {.size = partitions, .name = "buffer pool"}));

        if(sharing == SHARING_MODE::CONCURRENT){
            //std::cout << "mapping memory" << std::endl;
            mappedMemory = vulkan->mapMemory({.memory = bufferData.memory, .offset = 0, .size= bufferData.actualSize});
            if(mappedMemory == nullptr) throw std::runtime_error("Failed to map memory");
        }



    }

    // template <class T>
    // inline void Buffer::queueTransfer(Handle<BufferAddress> src, Handle<BufferReservation> dst, CommandBufferWriter<T> &writer)
    // {
    //         std::shared_ptr<BufferManager> manager(bufferManager);

    //         Buffer* srcBuffer;

    //         BufferReservation srcReservation;
    //         BufferReservation dstReservation;

    //         if(!manager->getAddressBuffer(src, srcBuffer))
    //             std::runtime_error("buffer transfer failed, invalid staging buffer");
            
    //         if(!manager->getAddressReservation(src,srcReservation))
    //             std::runtime_error("buffer transfer failed, invalid staging reservation");

    //         if(!getReservationData(dst,dstReservation))
    //             std::runtime_error("buffer transfer failed, invalid staging reservation");

    //         writer.copyBuffer({
    //             .srcOffset = srcReservation.offset,
    //             .dstOffset = dstReservation.offset,
    //             .size = dstReservation.requestSize,
    //             .srcBuffer = srcBuffer->getBuffer(),
    //             .dstBuffer = getBuffer(),
    //         });
    // }
}

