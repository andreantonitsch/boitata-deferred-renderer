#include "BackBuffer.hpp"

#include <utils/utils.hpp>
namespace boitatah
{
    BackBufferManager::BackBufferManager(std::shared_ptr<RenderTargetManager> targetManager)
    : m_renderTargetManager(targetManager){ };

    BackBufferManager::~BackBufferManager(void)
    {
        clearBackBuffer();
    }

    void BackBufferManager::setup(BackBufferDesc &desc)
    {
        clearBackBuffer();

        std::vector<ImageDesc> imageDescriptions;
        std::vector<AttachmentDesc> colorAttDescs;
        AttachmentDesc depthAttDesc;
        ImageDesc depthAttImageDesc;
        bool useDepth = false;
        for (int i = 0; i < desc.attachments.size(); i++)
        {

            ImageDesc imageDesc;
            AttachmentDesc attachDesc;

            imageDesc.format = desc.attachmentFormats[i];
            attachDesc.format = desc.attachmentFormats[i];

            switch(desc.attachments[i])
            {
                case ATTACHMENT_TYPE::COLOR:{
                    imageDesc.usage = IMAGE_USAGE::COLOR_ATT_TRANSFER_SRC;
                    imageDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
                    imageDesc.samples = desc.samples;
                    imageDesc.mipLevels = 1;
                    imageDesc.dimensions = desc.dimensions;
                    imageDescriptions.push_back(imageDesc);

                    attachDesc.finalLayout = IMAGE_LAYOUT::COLOR_ATT;
                    attachDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
                    attachDesc.layout = IMAGE_LAYOUT::COLOR_ATT;
                    attachDesc.samples = desc.samples;
                    colorAttDescs.push_back(attachDesc);
                    break;
                };
                case ATTACHMENT_TYPE::DEPTH_STENCIL:{
                    imageDesc.usage = IMAGE_USAGE::DEPTH_STENCIL;
                    imageDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
                    imageDesc.samples = desc.samples;
                    imageDesc.mipLevels = 1;
                    imageDesc.dimensions = desc.dimensions;

                    attachDesc.finalLayout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    attachDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
                    attachDesc.layout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    attachDesc.samples = desc.samples;
                    depthAttDesc = attachDesc;
                    depthAttImageDesc = imageDesc;
                    useDepth = true;
                    break;
                }
                default:
                    std::runtime_error("invalid attachment type");
            }
        }

        for(int i = 0; i < colorAttDescs.size(); i++)
            colorAttDescs[i].index= i;


        RenderPassDesc renderPassDesc{.color_attachments = colorAttDescs};
        if(useDepth){
            depthAttDesc.index = colorAttDescs.size();
            renderPassDesc.use_depthStencil = true;
            renderPassDesc.depth_attachment = depthAttDesc; 
        }
        renderpass = m_renderTargetManager->createRenderPass(renderPassDesc);


        colorAttDescs.push_back(depthAttDesc);
        imageDescriptions.push_back(depthAttImageDesc);
        
        RenderTargetDesc targetDesc{
            .renderpass = renderpass,
            .attachments = colorAttDescs,
            .imageDesc = imageDescriptions,
            .dimensions = desc.dimensions,
        };

        buffers.push_back(m_renderTargetManager->createRenderTarget(targetDesc));
        buffers.push_back(m_renderTargetManager->createRenderTarget(targetDesc));
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
            m_renderTargetManager->destroyRenderTarget(attach);
        }
        m_renderTargetManager->destroyRenderPass(renderpass);
        buffers.clear();
    }
}