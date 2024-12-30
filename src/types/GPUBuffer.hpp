#ifndef BOITATAH_GPU_BUFFER_HPP
#define BOITATAH_GPU_BUFFER_HPP

#include <memory>

#include <glm/glm.hpp>
#include <vector>
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "../collections/Pool.hpp"
#include "GPUResource.hpp"
#include "../renderer/modules/GPUResourceManager.hpp"
#include "../vulkan/VkCommandBufferWriter.hpp"

#include <array>

namespace boitatah
{
    using namespace boitatah::command_buffers;
    class GPUBuffer;


    struct GPUBufferCreateDescription{
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing_mode;
    };

    struct BufferGPUData : ResourceGPUContent<GPUBuffer>{
        Handle<BufferAddress> buffer;
        uint32_t buffer_capacity;
        bool dirty;
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
            void copyData(void * data, uint32_t frameIndex = 0){
                dirty = true;
                //Stages a transfer
                if(descriptor.sharing == SHARING_MODE::EXCLUSIVE){
                    auto manager = std::shared_ptr(m_manager); 
                    auto bufferManager = manager->getBufferManager();


                    if(!stagingBuffer.isNull())
                        stagingBuffer = bufferManager->reserveBuffer({
                            .request = size,
                            .usage = usage,
                            .sharing = SHARING_MODE::CONCURRENT,
                        });

                    bufferManager->memoryCopy(size, data, stagingBuffer);
                }
                else{
                    auto manager = std::shared_ptr(m_manager); 
                    auto bufferManager = manager->getBufferManager();

                    auto& frame_buffer = std::static_cast<BufferGPUData>(self().get_content(frameIndex));
                    bufferManager->memoryCopy(size, data, frame_buffer.buffer);
                }
            };

            //TODO: Implement
            bool readData(void* dstPtr){
                if(descriptor.sharing == SHARING_MODE::CONCURRENT){

                }
                else{

                }
                return false;
            };

            //TODO: Implement
            bool asyncReadData(void* dstPtr){
                if(descriptor.sharing == SHARING_MODE::CONCURRENT){

                }
                else{

                }
                return false;
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

                if(descriptor.sharing == SHARING_MODE::CONCURRENT)
                    return true;
                
                return static_cast<BufferGPUData>(content).dirty;
            }

            // create Resource
            BufferGPUData __impl_create_managed_resource(){
                auto manager = std::shared_ptr(m_manager); 
                return BufferGPUData{.buffer =  manager->getBufferManager()->reserveBuffer({
                                                                .request = size,
                                                                .usage = usage,
                                                                .sharing = SHARING_MODE::EXCLUSIVE,}),
                        .buffer_capacity = this->size};
            }

            void __impl_write_transfer(BufferGPUData& data, CommandBufferWriter<VkCommandBufferWriter> &writer){
                if(descriptor.sharing == SHARING_MODE::EXCLUSIVE){
                    auto manager = std::shared_ptr(m_manager);
                    auto bufferManager = manager->getBufferManager();
                    data.dirty = false;
                    
                    bufferManager->queueCopy(stagingBuffer, data.buffer);
                }
            }

    };
    

}

#endif