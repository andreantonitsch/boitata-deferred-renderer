#include "RenderTargetManager.hpp"


namespace boitatah{
    RenderTargetManager::RenderTargetManager(std::shared_ptr<vk::Vulkan> vulkan,
                                             std::shared_ptr<ImageManager> imageManager)
    : m_vk(vulkan), m_imageManager(imageManager){
        m_passPool = std::make_unique<Pool<RenderPass>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "RenderTargetManager Pass Pool"
        });        
        m_targetPool = std::make_unique<Pool<RenderTarget>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "RenderTargetManager Target Pool"
        });
        m_buffersPool = std::make_unique<Pool<RenderTargetSync>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "RenderTargetManager Target Buffers Pool"
        });
    }

    Handle<RenderPass> RenderTargetManager::createMatchingRenderPass(RenderTargetDesc &desc)
    {
        RenderPassDesc pass_desc;
        auto color_attachments = desc.attachments;
        if (color_attachments.back().layout == IMAGE_LAYOUT::DEPTH_STENCIL_ATT)
        {
            pass_desc.depth_attachment = color_attachments.back();
            pass_desc.depth_attachment.index = color_attachments.size()-1;
            color_attachments.pop_back();
            pass_desc.use_depthStencil = true;
        }
        pass_desc.color_attachments = color_attachments;
        //pass_desc.attTransitions[];
    
        return createRenderPass(pass_desc);
    }

    Handle<RenderTarget> RenderTargetManager::createRenderTarget(const RenderTargetDesc &description)
    {
        std::vector<Handle<Image>> images(description.attachmentImages);
        if (images.empty())
        {
            // Create images.
            for (const auto &imageDesc : description.imageDesc)
            {
                Handle<Image> newImage = m_imageManager->createImage(imageDesc);
                images.push_back(newImage);
            }
        }

        Handle<RenderPass> passhandle = description.renderpass;
        RenderPass pass;
        if (passhandle.isNull())
        {
            passhandle = createRenderPass(description.renderpassDesc);
            if (!m_passPool->tryGet(passhandle, pass))
            {
                throw std::runtime_error("Failed to create renderpass.");
            }
        }
        else
        {
            if (!m_passPool->tryGet(passhandle, pass))
            {
                throw std::runtime_error("Failed to create renderpass.");
            }
        }

        std::vector<VkImageView> imageViews;

        for (auto &imagehandle : images)
        {
            Image image = m_imageManager->getImage(imagehandle);
            imageViews.push_back(image.view);
        }

        FramebufferDescVk vkDesc{
            .views = imageViews,
            .pass = pass.renderPass,
            .dimensions = description.dimensions,
        };

        RenderTarget framebuffer{
            .buffer = m_vk->createFramebuffer(vkDesc),
            .attachments = images,
            .renderpass = passhandle,
            .sync = createRenderTargetSyncData()};

        return m_targetPool->set(framebuffer);
    }

    Handle<RenderPass> RenderTargetManager::createRenderPass(const RenderPassDesc &description)
    {
        RenderPass pass{
            .renderPass = m_vk->createRenderPass(description),
            .description = description
        };
        for(auto& desc : description.color_attachments)
            pass.clearColors.push_back(desc.clearColor);
        
        if(description.use_depthStencil)
            pass.clearColors.push_back(description.depth_attachment.clearColor);

        return m_passPool->move_set(pass);
    }

    Handle<RenderTargetSync> RenderTargetManager::createRenderTargetSyncData()
    {
        RenderTargetSync sync = m_vk->allocateBufferSync(); 
        return m_buffersPool->set(sync);
    }

    void RenderTargetManager::destroyRenderPass(Handle<RenderPass> &handle)
    {
        if(!m_passPool->contains(handle))
            return;

        RenderPass& pass = m_passPool->get(handle);
        pass.description = {};
        m_vk->destroyRenderpass(pass);
        m_passPool->clear(handle);
    }
    void RenderTargetManager::destroyRenderTarget(Handle<RenderTarget> &handle) {
        if(!m_targetPool->contains(handle)){
            std::cout << "failed deleting RenderTarget" << std::endl;
            return;
        }

        RenderTarget& target = m_targetPool->get(handle);
        for (auto &imagehandle : target.attachments)
        {
            Image image = m_imageManager->getImage(imagehandle);
            if(!image.swapchain)
                m_imageManager->destroyImage(imagehandle);
        }

        //destroyRenderPass(target.renderpass); 
        RenderTargetSync data;
            if (m_buffersPool->clear(target.sync, data))
                m_vk->destroyRenderTargetCmdData(data);

        target.attachments.clear();
        m_vk->destroyFramebuffer(target);
        m_targetPool->clear(handle);
    }
    bool RenderTargetManager::isActive(const Handle<RenderTarget> &handle)
    {
        return m_targetPool->contains(handle);
    };
    bool RenderTargetManager::isActive(const Handle<RenderPass> &handle)
    {
        return m_passPool->contains(handle);
    };
    bool RenderTargetManager::isActive(const Handle<RenderTargetSync> &handle)
    {
        return m_buffersPool->contains(handle);
    };
    RenderTarget &RenderTargetManager::get(const Handle<RenderTarget> &handle) {
        return m_targetPool->get(handle);
    };
    RenderPass &RenderTargetManager::get(const Handle<RenderPass> &handle) {
        return m_passPool->get(handle);
    };
    RenderTargetSync &RenderTargetManager::get(const Handle<RenderTargetSync> &handle) {
        return m_buffersPool->get(handle);
    }
    RenderTargetSync &RenderTargetManager::get_sync_data(const Handle<RenderTarget> &handle) {
        auto& target = get(handle);
        return get(target.sync);
    };
};