#include "BackBuffer.hpp"

#include <utils/utils.hpp>
namespace boitatah
{


    std::pair<ImageDesc, AttachmentDesc> createNewAttachmentDescriptions(
                                            ATTACHMENT_TYPE         type, 
                                            IMAGE_FORMAT            format,
                                            glm::uvec2              dimensions, 
                                            SAMPLES                 samples,
                                            bool clear){

            ImageDesc imageDesc;
            AttachmentDesc attachDesc;

            imageDesc.format = format;
            attachDesc.format = format;

            imageDesc.samples = samples;
            attachDesc.samples = samples;

            imageDesc.mipLevels = 1;

            // un-chained attachments dont need to be 
            // layout initialized
            attachDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
            attachDesc.clear = clear;
            imageDesc.dimensions = dimensions;

            switch(type)
            {
                case ATTACHMENT_TYPE::POSITION:
                case ATTACHMENT_TYPE::NORMAL:
                case ATTACHMENT_TYPE::COLOR:{
                    imageDesc.usage = IMAGE_USAGE::RENDER_GRAPH_COLOR;
                    attachDesc.finalLayout = IMAGE_LAYOUT::COLOR_ATT;
                    attachDesc.layout = IMAGE_LAYOUT::COLOR_ATT;
                    break;
                };
                case ATTACHMENT_TYPE::DEPTH_STENCIL:{
                    imageDesc.usage = IMAGE_USAGE::RENDER_GRAPH_DEPTH;
                    attachDesc.finalLayout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    attachDesc.layout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    break;
                }
                default:
                    std::runtime_error("invalid attachment type");
            }

            return {imageDesc, attachDesc};        
    }

    BackBufferManager::BackBufferManager(std::shared_ptr<RenderTargetManager> targetManager)
    : m_renderTargetManager(targetManager){ }

    // BackBufferManager::BackBufferManager(
    //                 std::shared_ptr<RenderTargetManager>    targetManager,
    //                 std::shared_ptr<MaterialManager>        materialManager) 
    // : m_renderTargetManager(targetManager), m_materialManager(materialManager){ };

    BackBufferManager::~BackBufferManager(void)
    {
        clearBackBuffer();
    };


    void BackBufferManager::setup2(BackBufferDesc2 &desc){

        clearBackBuffer();


        std::vector<ImageDesc> imageDescriptions;
        std::vector<AttachmentDesc> attachments;

        std::vector<RenderStage> stages;


        for(int i = 0; i < desc.render_stages.size(); i++){
            

        }


    };


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
            auto[imageDesc, attachDesc] = createNewAttachmentDescriptions(
                                            desc.attachments[i],
                                            desc.attachmentFormats[i],
                                            desc.dimensions,
                                            desc.samples,
                                            true);

            attachDesc.index = i;
            if(desc.attachments[i] == ATTACHMENT_TYPE::DEPTH_STENCIL)
            {
                useDepth = true;
                depthAttDesc = attachDesc;
                depthAttImageDesc = imageDesc;
            }
            else{
                imageDescriptions.push_back(imageDesc);
                colorAttDescs.push_back(attachDesc);
            }
            
        }

        RenderPassDesc renderPassDesc{.color_attachments = colorAttDescs};
        if(useDepth){
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

    Handle<RenderTarget> BackBufferManager::getPresentTarget()
    {

        return m_stagePool->get(
                graphs[getCurrentIndex()][present_link.target_idx]
            ).target;
        
    }

    uint32_t BackBufferManager::getPresentTargetIndex()
    {
        return present_link.attach_idx;
    }


    AttachmentDesc createExistingImageAttachmentDescription(
                                            ATTACHMENT_TYPE         type, 
                                            IMAGE_FORMAT            format,
                                            glm::uvec2              dimensions, 
                                            SAMPLES                 samples,
                                            bool clear){
        AttachmentDesc attachDesc;
        attachDesc.format = format;
        attachDesc.samples = samples;
        attachDesc.clear = clear;
        switch(type)
            {
                case ATTACHMENT_TYPE::POSITION:
                case ATTACHMENT_TYPE::NORMAL:
                case ATTACHMENT_TYPE::COLOR:{
                    attachDesc.initialLayout = IMAGE_LAYOUT::COLOR_ATT;
                    attachDesc.finalLayout = IMAGE_LAYOUT::COLOR_ATT;
                    attachDesc.layout = IMAGE_LAYOUT::COLOR_ATT;
                    break;
                };
                case ATTACHMENT_TYPE::DEPTH_STENCIL:{
                    attachDesc.initialLayout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    attachDesc.finalLayout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    attachDesc.layout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    break;
                }
                default:
                    std::runtime_error("invalid attachment type");
            }
        return attachDesc;
    };


    void BackBufferManager::addRenderStageToGraph(
                                        RenderStageDesc                    &desc,
                                        glm::vec2                          dimensions,
                                        std::vector<Handle<RenderStage>>   &graph)
    {
        RenderStage newStage;
        newStage.stage_index = graph.size();
        newStage.type = desc.type;
        newStage.clear = desc.clear;
        RenderTargetDesc target_desc;

        target_desc.attachments.resize(desc.attachments.size());
        target_desc.attachmentImages.resize(desc.attachments.size());
        std::vector<bool> set_attachments(desc.attachments.size());

        //no need to create a new render target
        if(desc.links.targetLink.previous_target_idx == UINT32_MAX){
            auto prev_stage_handle = graph[desc.links.targetLink.previous_target_idx];
            auto& prev_stage = m_stagePool->get(prev_stage_handle);
            newStage.target = prev_stage.target;
        }
        //creating a new render target
        else{

            //links to previous stage attachments
            if(desc.links.attachToAttach.size() != 0){
                
            }

            //complete attachments
            for(std::size_t i = 0; i < target_desc.attachments.size(); i++){
                //if attachment is still not set.
                if(!set_attachments[i])
                {
                    auto[imageDesc, attachDesc] = createNewAttachmentDescriptions(
                                                desc.attachments[i],
                                                desc.attachmentFormats[i],
                                                dimensions,
                                                desc.samples,
                                                newStage.clear);
                    set_attachments[i] = true;
                    target_desc.attachments[i] = attachDesc;
                    target_desc.attachmentImages[i] = m_imageManager->createImage(imageDesc);
                }
            }
            newStage.target = m_renderTargetManager->createRenderTarget(target_desc);
        }

 

        //create material set binding.
        if(desc.links.attachToTexture.size() != 0){    
        }


        newStage.description = desc;
        auto stage_handle = m_stagePool->set(newStage);

        graph.push_back(stage_handle);

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