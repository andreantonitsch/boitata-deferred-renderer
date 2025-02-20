#pragma once
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include "../../types/RenderTarget.hpp"
#include "../../types/BackBufferDesc.hpp"
#include <renderer/modules/RenderTargetManager.hpp>

namespace boitatah{

    class BackBufferManager{
        public:
        BackBufferManager(std::shared_ptr<RenderTargetManager> targetManager);
        ~BackBufferManager(void);

        void setup(BackBufferDesc &desc);
        Handle<RenderPass> getRenderPass();
        Handle<RenderTarget> getNext();
        Handle<RenderTarget> getCurrent();
        std::vector<Handle<RenderTarget>> buffers;
        uint32_t getCurrentIndex();

        private:
            std::shared_ptr<RenderTargetManager> m_renderTargetManager;
            Handle<RenderPass> renderpass;
            int current;
            void clearBackBuffer();
    };
}
