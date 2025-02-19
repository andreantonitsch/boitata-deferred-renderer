#pragma once

#include <types/RenderTarget.hpp>
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah{

    class RenderPassManager{

        private:
            std::shared_ptr<GPUResourceManager> m_resourceManager;
        public:

            Handle<RenderTarget> getMatchingRenderTarget(Handle<RenderPass>& handle);
            

    };
};