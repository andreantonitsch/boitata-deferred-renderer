#pragma once

#include <memory>
#include <vulkan/Vulkan.hpp>
#include <types/RenderTarget.hpp>
#include <renderer/modules/ImageManager.hpp>

namespace boitatah{


    class RenderTargetManager{

        private:
            std::shared_ptr<vk::VulkanInstance> m_vk;
            std::shared_ptr<ImageManager> m_imageManager;
            std::unique_ptr<Pool<Renderpass>> m_passPool;
            std::unique_ptr<Pool<RenderTarget>> m_targetPool;
            std::unique_ptr<Pool<RenderTargetSync>> m_buffersPool;

        public:
            RenderTargetManager(std::shared_ptr<vk::VulkanInstance> vulkan, std::shared_ptr<ImageManager> imageManager);
            Handle<RenderTarget> createMatchingRenderTarget(Handle<Renderpass>& handle, glm::u32vec2 dimensions);
            Handle<RenderTarget> createMatchingRenderTarget(Handle<Renderpass>& handle, const std::vector<Handle<Image>>& images);
            Handle<RenderTarget> createImageRenderTarget(Handle<Renderpass>& renderpassHandle,
                                                      const std::vector<Image>& images );
            Handle<Renderpass> createMatchingRenderPass(Handle<RenderTarget>& handle);
            Handle<Renderpass> createMatchingRenderPass(RenderTargetDesc& desc);
            
            // Handle<RenderTarget> createImageRenderTarget(Handle<RenderPass>& renderpassHandle,
            //                                           const std::vector<RenderTexture>& textures );
            Handle<RenderTarget> createRenderTarget(const RenderTargetDesc& description);   
            Handle<Renderpass>   create_renderpass(const RenderPassDesc& description);   
            Handle<RenderTargetSync> createRenderTargetSyncData();
            void destroyRenderPass(Handle<Renderpass>& handle);
            void destroyRenderTarget(Handle<RenderTarget>& handle);

            bool isActive(const Handle<RenderTarget>& handle);
            bool isActive(const Handle<Renderpass>& handle);
            bool isActive(const Handle<RenderTargetSync>& handle);
            RenderTarget& get(const Handle<RenderTarget>& handle);
            Renderpass& get(const Handle<Renderpass>& handle);
            RenderTargetSync& get(const Handle<RenderTargetSync>& handle);
            RenderTargetSync& get_sync_data(const Handle<RenderTarget>& handle);


    };
};