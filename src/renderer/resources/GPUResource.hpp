#pragma once

#include "ResourceStructs.hpp"
//#include <renderer/modules/GPUResourceManager.hpp>
#include <collections/Pool.hpp>
#include <types/BttEnums.hpp>
#include <stdexcept>
#include <array>

namespace boitatah{

    class GPUResourceManager;
    template<typename Resource, int Copies>
    class GPUResource //gpu data + metadata object
    {
        friend GPUResourceManager;
        protected:
            Handle<Resource> m_resourceHandle;

            ResourceDescriptor m_descriptor;
            std::weak_ptr<GPUResourceManager> m_manager;

            uint8_t dirty = 255u;
            uint8_t commited = 255u;
            uint32_t last_updated_frame = 0U;

            std::array<typename ResourceTraits<Resource>::ContentType, Copies> replicated_content;


            Resource& self(){return *static_cast<Resource*>(this);};

            GPUResource() = default;
            GPUResource(const ResourceDescriptor &descriptor, std::shared_ptr<GPUResourceManager> manager)
                : m_manager(manager),
                  m_descriptor(descriptor)
            {
                for(int i = 0; i < Copies; i++)
                    replicated_content[i] = self().CreateGPUData();
            };
             
            void set_descriptor(const ResourceDescriptor &descriptor){
                this->m_descriptor = descriptor;
            }

            void clean_dirt(int frame_index = 0) 
            { dirty = dirty & ~(static_cast<uint8_t>(1u) << (frame_index%2)); };

            void clean_commit(int frame_index = 0) 
            { commited = 255u;};

            void release(){
                for(int i = 0; i < Copies; i++)
                    self().ReleaseData(replicated_content[i]);
                self().Release();
            };

            /// @brief commits to update this resource next time resources are updated
            void commit(uint32_t frame_index, ResourceTraits<Resource>::CommandBufferWriter& writer)
            {
                if(frame_index == last_updated_frame)
                    return;
                last_updated_frame = frame_index;
                set_commited(frame_index);
                clean_dirt(frame_index);
                self().WriteTransfer(replicated_content[frame_index % Copies], writer.self());
            };
            void ready_content( uint32_t                                      frame_index, 
                               ResourceTraits<Resource>::CommandBufferWriter   &writer){
                if((!ready_for_use(frame_index)||
                     check_dirt(frame_index) ))
                        commit(frame_index, writer);
            }
            // can only be used when writting a transfer buffer.
            ResourceTraits<Resource>::ContentType& get_content_commit_update(
                            uint32_t                                        frame_index, 
                            ResourceTraits<Resource>::CommandBufferWriter   &writer)
            {
                ready_content(frame_index, writer);
                return get_content(frame_index);
            };
            

            // can only be used when writting a transfer buffer.
            ResourceTraits<Resource>::RenderData get_render_data_commit_update(
                            uint32_t                                        frame_index, 
                            ResourceTraits<Resource>::CommandBufferWriter   &writer)
            {
                //data is not commited, and not ready // dirty
                ready_content(frame_index, writer);
                return get_render_data(frame_index);
            };

            bool check_dirt(uint32_t frame_index) 
            { return 0u < (dirty & (1u << (frame_index%2))); };

            void set_dirty(){dirty = 255u; commited = 255u;};

            void set_commited(uint32_t frame_index)
            {commited &= ~(static_cast<uint8_t>(1) << (frame_index%2)); };

            public:

                bool ready_for_use(uint32_t frame_index) { return self().ReadyForUse(replicated_content[frame_index % Copies]);};
                bool check_commited(uint32_t frame_index) { return 0u == ((commited << (frame_index % Copies))); };
            
                ResourceTraits<Resource>::RenderData get_render_data(uint32_t frame_index)
                {
                    return self().GetRenderData(frame_index);
                };
                
                ResourceTraits<Resource>::ContentType& get_content(uint32_t frame_index)
                {
                    return replicated_content[frame_index % Copies];
                };

                uint32_t get_data(void* const dstPtr, uint32_t frame_index) const {
                    if(commited)
                        return self().__impl_get_data(dstPtr, frame_index);

                    if(check_dirt(0) || check_dirt(1))
                        commit();

                    return self().__impl_get_data(dstPtr, frame_index);
                };
                

    };

}

