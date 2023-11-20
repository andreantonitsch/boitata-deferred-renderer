#include "BackBuffer.hpp"
#include "RenderTarget.hpp"
#include "../collections/Pool.hpp"
#include "../renderer/Renderer.hpp"

namespace boitatah
{
    BackBufferManager::BackBufferManager(Renderer *renderer)
    {
        this->renderer = renderer;
    }

    BackBufferManager::~BackBufferManager(void)
    {
        clearBackBuffer();
    }

    void BackBufferManager::setup(RenderTargetDesc &desc)
    {
        clearBackBuffer();
        buffers.push_back(renderer->createRenderTarget(desc));
        buffers.push_back(renderer->createRenderTarget(desc));
    }

    Handle<RenderTarget> BackBufferManager::getNext()
    {
        current = (current + 1) % buffers.size();
        return buffers[current];
    }

    Handle<RenderTarget> boitatah::BackBufferManager::getCurrent()
    {
        return buffers[current];
    }

    void BackBufferManager::clearBackBuffer()
    {
        for (auto &attach : buffers)
        {
            renderer->destroyRenderTarget(attach);
        }
        buffers.clear();
    }
}