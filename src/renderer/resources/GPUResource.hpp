#pragma once

#include "ResourceStructs.hpp"
//#include <renderer/modules/GPUResourceManager.hpp>
#include <collections/Pool.hpp>
#include <types/BttEnums.hpp>
#include <stdexcept>
#include <array>

namespace boitatah{

    class GPUResourceManager;
    template<template <typename > class DerivedResource, typename Resource>
    class GPUResource //gpu data + metadata object
    {
        friend GPUResourceManager;
        protected:
            Handle<Resource> m_resourceHandle;

            ResourceDescriptor m_descriptor;
            std::weak_ptr<GPUResourceManager> m_manager;

            uint8_t dirty = 255u;
            bool commited = 255u;

            DerivedResource<Resource>& self(){return *static_cast<DerivedResource<Resource> *>(this);};

            GPUResource() = default;
            GPUResource(const ResourceDescriptor &descriptor, std::shared_ptr<GPUResourceManager> manager)
                : m_manager(manager),
                  m_descriptor(descriptor)
            {
            };
             
            void set_descriptor(const ResourceDescriptor &descriptor){
                this->m_descriptor = descriptor;
            }

            void clean_dirt(int frameIndex = 0) 
            { dirty = dirty & ~(static_cast<uint8_t>(1u) << (frameIndex%2)); };

            void clean_commit(int frameIndex = 0) 
            { commited = 0u;};

            void release(){
                self().__impl_release();
            };

            /// @brief commits to update this resource next time resources are updated
            void commit(uint32_t frame_index, ResourceTraits<Resource>::CommandBufferWriter& writer)
            {
                commited = true;
                self().__impl_commit(frame_index, writer);
                set_commited(frame_index);
            };

            // can only be used when writting a transfer buffer.
            ResourceTraits<Resource>::ContentType& get_content_commit_update(
                            uint32_t                                        frameIndex, 
                            ResourceTraits<Resource>::CommandBufferWriter   &writer)
            {
                if(!check_content_ready(frameIndex)){
                    commit(frameIndex, writer);
                }
                return get_content(frameIndex);
            };
            
            ResourceTraits<Resource>::RenderData& get_render_data(uint32_t frameIndex)
            {
                return self().__impl_get_render_data(frameIndex);
            };

            // can only be used when writting a transfer buffer.
            ResourceTraits<Resource>::RenderData& commit_update_get_render_data(
                            uint32_t                                        frameIndex, 
                            ResourceTraits<Resource>::CommandBufferWriter   &writer)
            {
                //data is not commited, and not ready // dirty
                if((!check_content_ready(frameIndex)
                   ||check_dirt(frameIndex) ) && 
                    !check_commited(frameIndex))
                    {
                        commit(frameIndex, writer);
                    }
                return get_render_data(frameIndex);
            };
            void check_content_ready(int frameIndex){
                return self().__impl_check_content_ready(frameIndex%2);
            };

            bool check_dirt(uint32_t frameIndex) 
            { return static_cast<uint8_t>(0) < (dirty & (static_cast<uint8_t>(1) << (frameIndex%2))); };

            void set_dirty(){dirty = 255u;};

            void set_commited(uint32_t frame_index)
            {commited = commited & ~(static_cast<uint8_t>(1) << (frame_index%2)); };

            public:

                bool ready_for_use(uint32_t frameIndex) { return self().__impl_ready_for_use(frameIndex);};
                bool check_commited(uint32_t frameIndex) { return static_cast<uint8_t>(0u) == (dirty & (static_cast<uint32_t>(commited) << (frameIndex%2))); };
                
                ResourceTraits<Resource>::ContentType& get_content(uint32_t frameIndex)
                {
                    return self().__impl_get_resource_content(frameIndex);
                };

                uint32_t get_data(void* const dstPtr, uint32_t frame_index) const {
                    if(commited)
                        return self().__impl_get_data(dstPtr, frame_index);

                    if(check_dirt(0) || check_dirt(1))
                        commit();

                    return self().__impl_get_data(dstPtr, frame_index);
                };

    };

    template<typename Resource>
    class  MutableGPUResource : public GPUResource<MutableGPUResource, Resource>// gpu data + metadata object
    {
        friend class GPUResource<MutableGPUResource, Resource>;

        protected :
            std::array<typename ResourceTraits<Resource>::ContentType, 2> replicated_content; //<< clear up on release

            MutableGPUResource() = default;
            MutableGPUResource(const ResourceDescriptor &descriptor, std::shared_ptr<GPUResourceManager> manager) 
                              : GPUResource<MutableGPUResource, Resource>(descriptor, manager){

                replicated_content[0] = resource().CreateGPUData();
                replicated_content[1] = resource().CreateGPUData();
            }; //Constructor

        public :
            using GPUResource<MutableGPUResource, Resource>::get_content;
            using GPUResource<MutableGPUResource, Resource>::self;
            using GPUResource<MutableGPUResource, Resource>::commit;

            Resource& resource(){return *static_cast<Resource *>(this); };

            bool __impl_check_content_ready(uint32_t frame_index){
                return true;
            }

            ResourceTraits<Resource>::ContentType& __impl_get_resource_content(uint32_t frame_index){
                return replicated_content[frame_index % 2];
            };

            ResourceTraits<Resource>::RenderData& __impl_get_render_data(uint32_t frameIndex)
            {
                return resource().GetRenderData(frameIndex);
            };

            bool __impl_ready_for_use(uint32_t frame_index){
                return self().check_dirt(frame_index) ;
            };

            void __impl_release(){
                resource().ReleaseData(replicated_content[0]);
                resource().ReleaseData(replicated_content[1]);
                resource().Release();
            };

            void __impl_commit(uint32_t frame_index, ResourceTraits<Resource>::CommandBufferWriter& writer){
                resource().WriteTransfer(replicated_content[frame_index % 2], writer.self());
            }

    };

    template<typename Resource>
    class  ImmutableGPUResource : public GPUResource<ImmutableGPUResource, Resource>/// gpu data + metadata object
    {   

        friend class GPUResource<ImmutableGPUResource, Resource>;

        protected :
            ResourceTraits<Resource>::ContentType content; //<< clear up on release

            ImmutableGPUResource() = default;
            ImmutableGPUResource(const ResourceDescriptor &descriptor, std::shared_ptr<GPUResourceManager> manager) 
                              : GPUResource<ImmutableGPUResource, Resource>(descriptor, manager){

                content = resource().CreateGPUData();
            }; //Constructor

        public :
            using GPUResource<ImmutableGPUResource, Resource>::get_content;
            using GPUResource<ImmutableGPUResource, Resource>::self;
            using GPUResource<ImmutableGPUResource, Resource>::commit;

            Resource& resource(){return *static_cast<Resource *>(this); };

            bool __impl_check_content_ready(uint32_t frame_index){
                return true;
            }

            ResourceTraits<Resource>::ContentType& __impl_get_resource_content(uint32_t frame_index){
                return content;
            };

            uint32_t __impl_get_data(void* dstPtr, uint32_t frame_index){
                return 0;
            };

            ResourceTraits<Resource>::RenderData& __impl_get_render_data(uint32_t frameIndex)
            {
                return resource().GetRenderData(frameIndex);
            };


            bool __impl_ready_for_use(uint32_t frame_index){
                return self().check_dirt(frame_index) ;
            };

            void __impl_release(std::shared_ptr<GPUResourceManager> manager){
                resource().ReleaseData(content);
                resource().Release();
                
            };

            void __impl_commit(uint32_t frame_index, ResourceTraits<Resource>::CommandBufferWriter& writer){
                resource().WriteTransfer(content, writer.self());
            }
    };



}

