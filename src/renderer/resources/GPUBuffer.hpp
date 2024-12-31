#ifndef BOITATAH_GPU_BUFFER_HPP
#define BOITATAH_GPU_BUFFER_HPP

#include <memory>

#include <glm/glm.hpp>
#include <vector>
#include "ResourceStructs.hpp"
#include "../../buffers/BufferStructs.hpp"
#include "../../buffers/Buffer.hpp"
#include "../../collections/Pool.hpp"
#include "GPUResource.hpp"
#include "../modules/GPUResourceManager.hpp"
#include "../../vulkan/VkCommandBufferWriter.hpp"

#include <array>

namespace boitatah
{
    using namespace boitatah::command_buffers;
    class GPUBuffer;
    class GPUResourceManager;

    struct BufferGPUData;
    template<>
    struct ResourceGPUContent<GPUBuffer>{
        using ContentType = BufferGPUData;
    };
    struct BufferGPUData : public ResourceGPUContent<GPUBuffer>{
        Handle<BufferAddress> buffer;
        uint32_t buffer_capacity;
        bool dirty;
    };


    struct BufferMetaData : ResourceMetaContent<GPUBuffer>{
        void* buffer_ptr;
    };
 
    class GPUBuffer : public MutableGPUResource<GPUBuffer>
    {
        friend class GPUResource<MutableGPUResource, GPUBuffer>;
        using MutableGPUResource<GPUBuffer>::__impl_ready_for_use;
        
        GPUBuffer(GPUBufferCreateDescription &createDescription, std::shared_ptr<GPUResourceManager> manager){ 
            m_manager = manager;
            size = createDescription.size;
            usage = createDescription.usage;
            set_descriptor({
                            .sharing = createDescription.sharing_mode,
                            .type = RESOURCE_TYPE::GPU_BUFFER,
                            .mutability = RESOURCE_MUTABILITY::MUTABLE,
            });
        };

        ~GPUBuffer(void) = default;

        public :
            void copyData(void * data, uint32_t frameIndex = 0);

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
            bool __impl_ready_for_use(ResourceGPUContent<GPUBuffer>& content);
            void __ImpSetContent(ResourceGPUContent<GPUBuffer>& content);

            // create Resource
            BufferGPUData __impl_create_managed_resource();

            void __impl_write_transfer(BufferGPUData& data, CommandBufferWriter<VkCommandBufferWriter> &writer);

    };
    

}

#endif