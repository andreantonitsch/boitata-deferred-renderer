#ifndef BOITATAH_BUFFER_MANAGER_HPP
#define BOITATAH_BUFFER_MANAGER_HPP

#include <vector>

#include "../vulkan/Vulkan.hpp"
#include "BufferStructs.hpp"
#include "Buffer.hpp"

#include "../collections/Pool.hpp"
#include "../vulkan/VkCommandBufferWriter.hpp"

namespace boitatah::buffer
{
    class Buffer;
    using namespace boitatah::vk;
    using namespace boitatah::command_buffers;

    class BufferManager : public std::enable_shared_from_this<BufferManager>
    {
        private:
            uint32_t partitionsPerBuffer = 1u << 10;

            std::shared_ptr<vk::Vulkan>  m_vk;
            std::vector<Handle<Buffer *>> activeBuffers;

            Pool<Buffer *> bufferPool = Pool<Buffer *>({.size = 1<<16, .name = "uniforms pool"});
            Pool<std::shared_ptr<Buffer>> stagingBufferPool = Pool<std::shared_ptr<Buffer>>({.size = 1<<16, .name = "uniforms pool"});
            
            Pool<BufferAddress> addressPool = Pool<BufferAddress>({.size = 1<<20, .name = "uniforms pool"});
            CommandBuffer transferBuffer;
            VkFence m_transferFence;

            Handle<Buffer*> createBuffer(const BufferDesc &&description);

            void releaseBuffer(Handle<Buffer*> handle);

            Handle<Buffer *> findOrCreateCompatibleBuffer(const BufferReservationRequest &compatibility);
            uint32_t findCompatibleBuffer(const BufferReservationRequest &compatibility);
            

        public:
            BufferManager(std::shared_ptr<vk::Vulkan>  vk_instance);
            ~BufferManager(void);
            Handle<BufferAddress> reserveBuffer(const BufferReservationRequest &request);
            bool copyToBuffer(const BufferUploadDesc &desc);
            
            bool queueCopy(const Handle<BufferAddress> src, const Handle<BufferAddress> dst);
            
            template<class T>
            bool queueCopy(const Handle<BufferAddress> src, const Handle<BufferAddress> dst, CommandBufferWriter<T>& writer);

            //user is responsible for releasing the staged buffer
            void memoryCopy(uint32_t dataSize, void* data, Handle<BufferAddress>& handle);

            void queueingBufferUpdates(); //queues updates
            void startBufferUpdates(); //setup queue buffer updates
            void endBufferUpdates();    // ship queue buffer updates


            bool getAddressReservation(const Handle<BufferAddress> handle, BufferReservation& reservation);
            Handle<BufferReservation> getAddressReservation(const Handle<BufferAddress> handle);
            bool getAddressBuffer(const Handle<BufferAddress> handle, Buffer*& buffer);
            

            void freeBufferReservation(Handle<BufferAddress> handle);
             
            bool areTransfersFinished() const;
            void waitForTransferToFinish() const;
            CommandBuffer getTransferBuffer();
    };


};

#endif //BOITATAH_BUFFER_MANAGER_HPP