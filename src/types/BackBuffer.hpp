#ifndef BOITATAH_BACKBUFFER_HPP
#define BOITATAH_BACKBUFFER_HPP

#include <vulkan/vulkan.h>
#include "RenderTarget.hpp"
#include <vector>
#include "../renderer/Renderer.hpp"

namespace boitatah{

    class Renderer;

    class BackBufferManager{
        public:
        BackBufferManager(Renderer *renderer);
        ~BackBufferManager(void);

        void setup(RenderTargetDesc &desc);

        Handle<RenderTarget> getNext();
        Handle<RenderTarget> getCurrent();
        std::vector<Handle<RenderTarget>> buffers;

        private:
            Renderer* renderer;
            int current;
            void clearBackBuffer();
    };
}

#endif //BOITATAH_BACKBUFFER_HPP