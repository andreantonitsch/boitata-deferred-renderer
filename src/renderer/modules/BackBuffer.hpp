#pragma once
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>

#include <vulkan/vulkan.h>
#include <vector>
#include "../../types/RenderTarget.hpp"
#include "../../types/BackBufferDesc.hpp"

namespace boitatah{

    class Renderer;


    class BackBufferManager{
        public:
        BackBufferManager(Renderer *renderer);
        ~BackBufferManager(void);

        void setup(BackBufferDesc &desc);
        Handle<RenderPass> getRenderPass();
        Handle<RenderTarget> getNext();
        Handle<RenderTarget> getCurrent();
        std::vector<Handle<RenderTarget>> buffers;
        uint32_t getCurrentIndex();

        private:
            Renderer* renderer;
            Handle<RenderPass> renderpass;
            int current;
            void clearBackBuffer();
    };
}
