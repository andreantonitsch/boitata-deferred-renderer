#pragma once

#include <memory>
#include <vulkan/Vulkan.hpp>
#include <types/RenderTarget.hpp>
#include <renderer/modules/ImageManager.hpp>

namespace boitatah{

    struct AttachmentSlot{

    };

    class RenderTargetManager{

        private:
            std::shared_ptr<vk::Vulkan> m_vk;
            std::shared_ptr<ImageManager> m_imageManager;
            std::unique_ptr<Pool<RenderPass>> m_passPool;
            std::unique_ptr<Pool<RenderTarget>> m_targetPool;
            std::unique_ptr<Pool<RenderTargetSync>> m_buffersPool;

        public:
            RenderTargetManager(std::shared_ptr<vk::Vulkan> vulkan, std::shared_ptr<ImageManager> imageManager);
            Handle<RenderTarget> createMatchingRenderTarget(Handle<RenderPass>& handle, glm::u32vec2 dimensions);
            Handle<RenderTarget> createMatchingRenderTarget(Handle<RenderPass>& handle, const std::vector<Handle<Image>>& images);
            Handle<RenderTarget> createImageRenderTarget(Handle<RenderPass>& renderpassHandle,
                                                      const std::vector<Image>& images );
            // Handle<RenderTarget> createImageRenderTarget(Handle<RenderPass>& renderpassHandle,
            //                                           const std::vector<RenderTexture>& textures );
            Handle<RenderTarget> createRenderTarget(const RenderTargetDesc& description);   
            Handle<RenderPass>   createRenderPass(const RenderPassDesc& description);   
            Handle<RenderTargetSync> createRenderTargetSyncData();
            void destroyRenderPass(Handle<RenderPass>& handle);
            void destroyRenderTarget(Handle<RenderTarget>& handle);

            bool isActive(const Handle<RenderTarget>& handle);
            bool isActive(const Handle<RenderPass>& handle);
            bool isActive(const Handle<RenderTargetSync>& handle);
            RenderTarget& get(const Handle<RenderTarget>& handle);
            RenderPass& get(const Handle<RenderPass>& handle);
            RenderTargetSync& get(const Handle<RenderTargetSync>& handle);


    };
};