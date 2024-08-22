#ifndef BOITATAH_BACKBUFFER_HPP
#define BOITATAH_BACKBUFFER_HPP

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>

#include <vulkan/vulkan.h>
#include <vector>
#include "../Renderer.hpp"
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

        private:
            Renderer* renderer;
            Handle<RenderPass> renderpass;
            int current;
            void clearBackBuffer();
    };
}

#endif //BOITATAH_BACKBUFFER_HPP