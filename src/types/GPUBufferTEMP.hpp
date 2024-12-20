#ifndef BOITATAH_GPU_BUFFER_HPP
#define BOITATAH_GPU_BUFFER_HPP

#include <memory>

#include <glm/glm.hpp>
#include <vector>
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "../collections/Pool.hpp"
#include "GPUResourceTEMP.hpp"
#include "GPUResourceManagerTEMP.hpp"
#include "../vulkan/VkCommandBufferWriter.hpp"

#include <array>

namespace boitatah
{
    using namespace boitatah::command_buffers;
    class GPUBuffer;
    class GPUResourceManager;


    struct GPUBufferCreateDescription{
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing_mode;
    };

    struct BufferGPUData : ResourceGPUContent<GPUBuffer>{
        Handle<BufferAddress> buffer;
        uint32_t buffer_capacity;
    };

    struct BufferMetaData : ResourceMetaContent<GPUBuffer>{
        void* buffer_ptr;
    };
 
    struct GPUBuffer : MutableGPUResource<GPUBuffer>
    {
        friend class GPUResource<GPUBuffer>;
        using MutableGPUResource<GPUBuffer>::ready_for_use;

        GPUBuffer() = default;
        GPUBuffer(const GPUBufferCreateDescription &createDescription, std::shared_ptr<GPUResourceManager> manager){
            m_manager = manager;
            size = createDescription.size;
            usage = createDescription.usage;
            set_descriptor({
                            .sharing = createDescription.sharing_mode,
                            .type = RESOURCE_TYPE::GPU_BUFFER,
                            .mutability = RESOURCE_MUTABILITY::MUTABLE,
            });
        };

        ~GPUBuffer(void){
            //TODO release staging buffer
        };

        public : 

            void copyData(void * data, uint32_t size){
                
                if(descriptor.sharing == SHARING_MODE::EXCLUSIVE){
                    auto manager = std::shared_ptr(m_manager); 
                    auto bufferManager = manager->getBufferManager();

                    if(!stagingBuffer.isNull())
                        stagingBuffer = bufferManager->stageCopy(size, data);
                    else
                        stagingBuffer = bufferManager->stageCopy(stagingBuffer, data);
                }

            };


        private:
            uint32_t size;
            BUFFER_USAGE usage;
            Handle<BufferAddress> stagingBuffer;
            BufferMetaData meta_data;

            /// @brief ready for use for buffers is trivially handled by MutableGPUResource<T>
            /// @param content 
            /// @return that this buffer is ready for use.
            bool __impl_ready_for_use(ResourceGPUContent<GPUBuffer>& content){

                // auto manager = std::shared_ptr(m_manager); 
                // auto buffer_manager = manager->getBufferManager();
                // Buffer* buffer;
                // auto allocation = buffer_manager->getAddressBuffer(content.buffer, buffer);

                return true;
            }

            // create Resource
            BufferGPUData __impl_create_managed_resource(){
                auto manager = std::shared_ptr(m_manager); 
                return {.buffer =  manager->getBufferManager()->reserveBuffer({
                                                                .request = size,
                                                                .usage = usage,
                                                                .sharing = SHARING_MODE::EXCLUSIVE,}),
                    .buffer_capacity = size};
            }

            void __impl_write_transfer(BufferGPUData& data, CommandBufferWriter<VkCommandBufferWriter> &writer){
                auto manager = std::shared_ptr(m_manager);
                auto bufferManager = manager->getBufferManager();
                
                bufferManager->queueCopy(stagingBuffer, data.buffer);

            }



    };
    

}

#endif