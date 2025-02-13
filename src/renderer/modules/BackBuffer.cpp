#include "BackBuffer.hpp"
#include "../Renderer.hpp"
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

    void BackBufferManager::setup(BackBufferDesc &desc)
    {
        clearBackBuffer();

        std::vector<ImageDesc> imageDescriptions;
        std::vector<AttachmentDesc> attachmentDescriptions;
        for (int i = 0; i < desc.attachments.size(); i++)
        {

            ImageDesc imageDesc;
            AttachmentDesc attachDesc;

            imageDesc.format = desc.attachmentFormats[i];
            attachDesc.format = desc.attachmentFormats[i];

            if (desc.attachments[i] == ATTACHMENT_TYPE::COLOR)
            {
                imageDesc.usage = IMAGE_USAGE::COLOR_ATT_TRANSFER_SRC;
                imageDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
                imageDesc.samples = desc.samples;
                imageDesc.mipLevels = 1;

                attachDesc.finalLayout = IMAGE_LAYOUT::COLOR_ATT;
                attachDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
                attachDesc.layout = IMAGE_LAYOUT::COLOR_ATT;
                attachDesc.index = i;
                attachDesc.samples = desc.samples;

            }

            //TODO implement depth and normal.

            imageDesc.dimensions = desc.dimensions;
            imageDescriptions.push_back(imageDesc);
            attachmentDescriptions.push_back(attachDesc);
        }

        RenderPass pass;
        renderpass = renderer->createRenderPass({.attachments = attachmentDescriptions });


        RenderTargetDesc targetDesc{
            .renderpass = renderpass,
            .attachments = attachmentDescriptions,
            .imageDesc = imageDescriptions,
            .dimensions = desc.dimensions,
        };

        buffers.push_back(renderer->createRenderTarget(targetDesc));
        buffers.push_back(renderer->createRenderTarget(targetDesc));
    }

    Handle<RenderPass> BackBufferManager::getRenderPass()
    {
        return renderpass;
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

    uint32_t BackBufferManager::getCurrentIndex()
    {
        return current;
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