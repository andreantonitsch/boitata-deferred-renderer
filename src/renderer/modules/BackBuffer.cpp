#include "BackBuffer.hpp"
#include <algorithm>
#include <utils/utils.hpp>
namespace boitatah
{


    std::pair<ImageDesc, AttachmentDesc> createNewAttachmentDescriptions(
                                            ATTACHMENT_TYPE         type, 
                                            uint32_t                index,
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

            attachDesc.index =  index;

            // un-chained attachments dont need to be 
            // layout initialized
            attachDesc.initialLayout = IMAGE_LAYOUT::UNDEFINED;
            attachDesc.clear = clear;
            imageDesc.dimensions = dimensions;

            switch(type)
            {
                case ATTACHMENT_TYPE::NORMAL:
                case ATTACHMENT_TYPE::POSITION:
                case ATTACHMENT_TYPE::COLOR:
                {
                    imageDesc.usage = IMAGE_USAGE::RENDER_GRAPH_COLOR;
                    attachDesc.finalLayout = IMAGE_LAYOUT::COLOR_ATT;
                    attachDesc.layout = IMAGE_LAYOUT::COLOR_ATT;
                    break;
                };
                case ATTACHMENT_TYPE::DEPTH_STENCIL:{
                    imageDesc.usage = IMAGE_USAGE::RENDER_GRAPH_DEPTH;
                    attachDesc.finalLayout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    attachDesc.layout = IMAGE_LAYOUT::DEPTH_STENCIL_ATT;
                    attachDesc.clearColor = glm::vec4(0);
                    break;
                }
                default:
                    std::runtime_error("invalid attachment type");
            }

            return {imageDesc, attachDesc};        
    }

    BackBufferDesc2 BackBufferManager::BasicDeferredPipeline(uint32_t windowWidth, 
                                                             uint32_t windowHeight)
    {
        return BackBufferDesc2{
                                .dimensions = {windowWidth, windowHeight},
                                .render_stages = {
                                    RenderStageDesc{
                                        .type = StageType::CAMERA,
                                        .attachments = {ATTACHMENT_TYPE::COLOR,
                                                        ATTACHMENT_TYPE::POSITION,
                                                        ATTACHMENT_TYPE::NORMAL,
                                                        ATTACHMENT_TYPE::DEPTH_STENCIL},
                                        .attachmentFormats = {  IMAGE_FORMAT::BGRA_8_SRGB,
                                                                IMAGE_FORMAT::RGBA_32_SFLOAT,
                                                                IMAGE_FORMAT::RGBA_32_SFLOAT,
                                                                IMAGE_FORMAT::DEPTH_32_SFLOAT},
                                        .links = {},
                                    },
                                    RenderStageDesc{
                                        .type = StageType::SCREEN_QUAD,
                                        .attachments = {ATTACHMENT_TYPE::COLOR,},
                                        .attachmentFormats = {  IMAGE_FORMAT::BGRA_8_SRGB},
                                        .links = {
                                            .attachToTexture = { 
                                                    AttachToTextureLink{0, 0, 0},
                                                    AttachToTextureLink{0, 1, 1},
                                                    AttachToTextureLink{0, 2, 2},},
                                        },
                                    },
                                },
                                .present_link = {1, 0},
                                };
    }
    BackBufferDesc2 BackBufferManager::BasicForwardPipeline(uint32_t windowWidth, uint32_t windowHeight)
    {
        return BackBufferDesc2{
                                   .dimensions = {windowWidth, windowHeight},
                                   .render_stages = {
                                        RenderStageDesc{
                                            .type = StageType::CAMERA,
                                            .attachments = {ATTACHMENT_TYPE::COLOR,
                                                            ATTACHMENT_TYPE::DEPTH_STENCIL},
                                            .attachmentFormats = {  IMAGE_FORMAT::BGRA_8_SRGB,
                                                                    IMAGE_FORMAT::DEPTH_32_SFLOAT},
                                            .links = {},
                                        }
                                    },            
                                    .present_link {0,0}, //first attachment of first stage
                                   };
    };

    BackBufferDesc2 BackBufferManager::BasicMultiWriteForwardPipeline(uint32_t windowWidth,
                                                                      uint32_t windowHeight,
                                                                      uint32_t present_index){
        return BackBufferDesc2{
                                   .dimensions = {windowWidth, windowHeight},
                                   .render_stages = {
                                        RenderStageDesc{
                                            .type = StageType::CAMERA,
                                            .attachments = {ATTACHMENT_TYPE::COLOR,
                                                            ATTACHMENT_TYPE::POSITION,
                                                            ATTACHMENT_TYPE::NORMAL,
                                                            ATTACHMENT_TYPE::DEPTH_STENCIL},
                                            .attachmentFormats = {  IMAGE_FORMAT::BGRA_8_SRGB,
                                                                    IMAGE_FORMAT::BGRA_8_SRGB,
                                                                    IMAGE_FORMAT::BGRA_8_SRGB,       
                                                                    IMAGE_FORMAT::DEPTH_32_SFLOAT},
                                            .links = {},
                                        }
                                    },            
                                    .present_link {0, present_index},
                                   };
    };

    BackBufferManager::BackBufferManager(std::shared_ptr<RenderTargetManager> target_manager,
                                         std::shared_ptr<ImageManager> image_manager)
        : m_renderTargetManager(target_manager), m_imageManager(image_manager) {}

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
        m_stagePool = std::make_unique<Pool<RenderStage>>(PoolOptions{
            .size = 10, .dynamic = true, .name = "Render Stage Pool" });
        std::cout << "Starting Graph construction" << std::endl;
        for (int i = 0; i < desc.render_stages.size(); ++i)
            for (int j = 0; j < graphs.size(); ++j){
                auto new_stage = addRenderStageToGraph(
                    desc.render_stages[i],
                    desc.dimensions,
                    graphs[j]);
                auto new_stage_handle = m_stagePool->set(new_stage);
                graphs[j].push_back(new_stage_handle);
                std::cout << "Added new stage" << std::endl;
            }
        present_link = desc.present_link;
        stage_count = desc.render_stages.size();
        std::cout << "backbuffer setup2 finished" << std::endl;
    };


    void BackBufferManager::setup(BackBufferDesc &desc){
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
                                            static_cast<uint32_t>(i),
                                            desc.attachmentFormats[i],
                                            desc.dimensions,
                                            desc.samples,
                                            true);
\
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

    std::vector<Handle<RenderStage>>& BackBufferManager::getNext_Graph()
    {
        current = (current + 1) % graphs.size();
        return graphs[current];
    }

    std::vector<Handle<RenderStage>>& BackBufferManager::getCurrent_Graph()
    {
        return graphs[current];
    }

    RenderStage &BackBufferManager::getStage(Handle<RenderStage> &handle)
    {
        return m_stagePool->get(handle);
    }

    RenderStage &BackBufferManager::getStage(uint32_t index)
    {
        return m_stagePool->get(graphs[0][index]);
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

    uint32_t BackBufferManager::getStageCount()
    {
        return stage_count;
    }

    Handle<RenderStage> BackBufferManager::getCompatibleRenderStage(uint32_t stage_mask)
    {
        for(std::size_t stage_index = 0; stage_index < getStageCount(); ++stage_index){
            uint32_t m = static_cast<uint32_t>(1) << stage_index;
            std::cout << "testing compatible stage for " << stage_mask <<
                         " with mask " << m << 
                         "for stage index " << stage_index << std::endl;
            if((m && stage_mask) > 0){
                return graphs[getCurrentIndex()][stage_index];
            }
        }
        return Handle<RenderStage>();
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


    RenderStage BackBufferManager::addRenderStageToGraph(
                                        RenderStageDesc                    &desc,
                                        glm::vec2                          dimensions,
                                        std::vector<Handle<RenderStage>>   &graph)
    {
        RenderStage newStage;
        newStage.stage_index = graph.size();
        newStage.type = desc.type;
        newStage.clear = desc.clear;
        
        RenderTargetDesc target_desc;
        target_desc.dimensions = dimensions;

        target_desc.attachments.resize(desc.attachments.size());
        target_desc.attachmentImages.resize(desc.attachments.size());
        std::vector<bool> set_attachments(desc.attachments.size());
        std::fill(set_attachments.begin(), set_attachments.end(), false);

        std::cout << "new render stage data structures setup done" << std::endl;

        //no need to create a new render target
        if (desc.links.targetLink.prev_target_idx != UINT32_MAX) {
            auto prev_stage_handle = graph[desc.links.targetLink.prev_target_idx];
            auto& prev_stage = m_stagePool->get(prev_stage_handle);
            newStage.target = prev_stage.target;
        }
        //creating a new render target
        else {
            //links to previous stage attachments
            if (desc.links.attachToAttach.size() != 0) {
                for(auto& link :desc.links.attachToAttach){
                    auto[previous_target_idx, previous_attach_idx, attachment_index] = link;
                    
                    auto& prev_stage = m_stagePool->get(graph[previous_target_idx]);
                    auto& prev_target = m_renderTargetManager->get(prev_stage.target);
                    auto& prev_pass = m_renderTargetManager->get(prev_target.renderpass);
                    //set the attachment image
                    target_desc.attachmentImages[attachment_index] 
                        = prev_target.attachments[previous_attach_idx];
                    
                    //copy the previous attachment description
                    if(prev_stage.description.attachments[attachment_index] == 
                       ATTACHMENT_TYPE::DEPTH_STENCIL){
                            target_desc.attachments[attachment_index] = 
                                prev_pass.description.depth_attachment;
                       }
                    else {
                        target_desc.attachments[attachment_index]
                            = prev_pass.description.color_attachments[previous_attach_idx];
                    }
                    // set it to not clear the attachment 
                    target_desc.attachments[attachment_index].clear = false;
                    set_attachments[attachment_index] = true;
                }
            }

            //complete attachments
            for(std::size_t i = 0; i < target_desc.attachments.size(); i++){
                //if attachment is still not set.
                if(!set_attachments[i])
                {
                    std::cout << "making brand new attachment " << i << std::endl; 
                    auto[imageDesc, attachDesc] = createNewAttachmentDescriptions(
                                                desc.attachments[i],
                                                static_cast<uint32_t>(i),
                                                desc.attachmentFormats[i],
                                                dimensions,
                                                desc.samples,
                                                newStage.clear);
                    attachDesc.index = static_cast<uint32_t>(i);
                    std::cout << "created new attachment descriptions " << std::endl;
                    set_attachments[i] = true;
                    target_desc.attachments[i] = attachDesc;
                    target_desc.attachmentImages[i] = m_imageManager->createImage(imageDesc);

                }
            }
            auto pass = m_renderTargetManager->createMatchingRenderPass(target_desc);
            target_desc.renderpass = pass;
            newStage.target = m_renderTargetManager->createRenderTarget(target_desc);
        }

        //create material set binding.
        if(desc.links.attachToTexture.size() != 0){
            MaterialBinding binding;
            for(auto& link :desc.links.attachToTexture){
                auto prev_stage_handle = graph[link.prev_target_idx];
                auto& prev_stage = m_stagePool->get(prev_stage_handle);
                auto& prev_target = m_renderTargetManager->get(prev_stage.target);
                MaterialBindingAtt binding_att{
                    .type = DESCRIPTOR_TYPE::IMAGE,
                    .binding_handle = {.image = prev_target.attachments[link.prev_attach_idx]},
                };
                binding.bindings.push_back(binding_att);
            }
            newStage.materialBinding = binding;
        }

        newStage.description = desc;
        return newStage;

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