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

    BackBufferDesc BackBufferManager::BasicDeferredPipeline(uint32_t windowWidth, 
                                                             uint32_t windowHeight)
    {
        return BackBufferDesc{
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
    BackBufferDesc BackBufferManager::BasicForwardPipeline(uint32_t windowWidth, uint32_t windowHeight)
    {
        return BackBufferDesc{
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

    BackBufferDesc BackBufferManager::BasicMultiWriteForwardPipeline(uint32_t windowWidth,
                                                                      uint32_t windowHeight,
                                                                      uint32_t present_index){
        return BackBufferDesc{
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
                                         std::shared_ptr<ImageManager> image_manager,
                                         std::shared_ptr<MaterialManager> material_manager,
                                         std::shared_ptr<GPUResourceManager>  resource_manager,
                                         std::shared_ptr<VulkanInstance> vulkan_instance)
        : m_renderTargetManager(target_manager), m_imageManager(image_manager), 
          m_material_manager(material_manager), m_resource_mngr(resource_manager),
          m_vulkan(vulkan_instance) {}

    BackBufferManager::~BackBufferManager(void){
        clearBackBuffer();
    };


    void BackBufferManager::setup(BackBufferDesc &desc){
        m_stagePool = std::make_unique<Pool<RenderStage>>(PoolOptions{
            .size = 10, .dynamic = true, .name = "Render Stage Pool" });
        sampler = m_imageManager->createSampler({});

        m_stage_textures.clear();
        m_stage_textures.resize(desc.render_stages.size());
        m_stage_bindings.clear();
        m_stage_bindings.resize(desc.render_stages.size());

        for (int i = 0; i < desc.render_stages.size(); ++i)
            for (int j = 0; j < m_graphs.size(); ++j){
                auto new_stage = create_renderstage(
                    desc.render_stages[i],
                    desc.dimensions,
                    m_graphs[j]);
                auto new_stage_handle = m_stagePool->set(new_stage);
                m_graphs[j].push_back(new_stage_handle);
            }
        present_link = desc.present_link;
        stage_count = desc.render_stages.size();

    }
    void BackBufferManager::regenerate_backbuffer(BackBufferDesc &desc) {
        
    };

    uint32_t BackBufferManager::getCurrentIndex(){
        return current;
    }

    std::vector<Handle<RenderStage>>& BackBufferManager::getNext_Graph(){
        current = (current + 1) % m_graphs.size();

        auto present_handle = getPresentTarget();
        auto& present_fence = m_renderTargetManager
                                    ->get_sync_data(present_handle)
                                    .in_flight_fence;
        m_vulkan->wait_for_fence(present_fence);
        m_vulkan->reset_fence(present_fence);
        return m_graphs[current];
    }

    std::vector<Handle<RenderStage>>& BackBufferManager::getCurrent_Graph(){
        return m_graphs[current];
    }

    RenderStage &BackBufferManager::getStage(Handle<RenderStage> &handle){
        return m_stagePool->get(handle);
    }

    RenderStage &BackBufferManager::getStage(uint32_t index){
        return m_stagePool->get(m_graphs[0][index]);
    }

    Handle<MaterialBinding> BackBufferManager::getStageBinding(Handle<RenderStage> &handle) {
        auto& stage = getStage(handle);
        return m_stage_bindings[stage.stage_index];
    }

    Handle<MaterialBinding> BackBufferManager::getStageBinding(uint32_t stage_index)
    {
        return m_stage_bindings[stage_index];
    }

    Handle<RenderTarget> BackBufferManager::getPresentTarget(){
        return m_stagePool->get(
                m_graphs[getCurrentIndex()][present_link.target_idx]
            ).target;
        
    }

    const std::vector<Handle<RenderTexture>>& BackBufferManager::getStageTextures(Handle<RenderStage>& handle)
    {
        auto& stage = getStage(handle);
        return m_stage_textures[stage.stage_index];
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
        for(std::size_t stage_index = 0; stage_index < getStageCount(); stage_index++){
            uint32_t m = 1u << stage_index;
            if((m & stage_mask)){
                return m_graphs[getCurrentIndex()][stage_index];
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


    RenderStage BackBufferManager::create_renderstage(
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
                                prev_pass.description.depth_attachment;}
                    else {
                        target_desc.attachments[attachment_index]
                            = prev_pass.description.color_attachments[previous_attach_idx];}

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
            

            //if the textures havent been created yet (one of the graph copies will do this.
            // but textures themselves are doubled)
            if(m_stage_textures[newStage.stage_index].empty()){

                // for each attachment create a texture that links to it
                for(auto& attachment : target_desc.attachments){
                    if( attachment.format== IMAGE_FORMAT::DEPTH_24_UNORM_UINT_STENCIL ||
                        attachment.format== IMAGE_FORMAT::DEPTH_32_SFLOAT ||
                        attachment.format== IMAGE_FORMAT::DEPTH_32_SFLOAT_UINT_STENCIL)
                            continue;
                    
                    TextureCreateDescription tex_desc{};
                    tex_desc.depth              = 1,
                    tex_desc.width              = target_desc.dimensions.x;
                    tex_desc.height             = target_desc.dimensions.y;
                    tex_desc.textureMode        = TextureMode::READ;
                    tex_desc.format             = attachment.format;
                    tex_desc.samplerInfo        = m_imageManager->getSampler(sampler).data;
                    auto new_attach_copy_tex    = m_resource_mngr->create(tex_desc);

                    m_stage_textures[newStage.stage_index].push_back(new_attach_copy_tex);
                }

                //create material set binding.
                if(desc.links.attachToTexture.size() > 0){
                    std::vector<MaterialBindingAtt> bindings;
                    for(auto& link :desc.links.attachToTexture){
                        MaterialBindingAtt binding_att{
                            .type = DESCRIPTOR_TYPE::COMBINED_IMAGE_SAMPLER,
                        };
                        binding_att.binding_handle.renderTex = m_stage_textures[link.prev_stage_idx]
                                                                               [link.prev_attach_idx];
                        bindings.push_back(binding_att);
                    }
                    
                    auto binding = m_material_manager->createBinding(bindings);
                    m_stage_bindings[newStage.stage_index] = binding;
                }
            }   

            newStage.target = m_renderTargetManager->createRenderTarget(target_desc);
        }

        //TODO make this less contrived
        if(newStage.stage_index> 0)
        {
            auto prev_stage_handle = graph[newStage.stage_index-1];
            auto& prev_stage = m_stagePool->get(prev_stage_handle);
            auto& prev_target = m_renderTargetManager->get(prev_stage.target);
            newStage.wait_list.push_back(prev_target.sync);
        }

        newStage.description = desc;
        return newStage;

    }

    void BackBufferManager::destroy_renderstage(Handle<RenderStage> &handle)
    {
        RenderStage stage;
        if(!m_stagePool->tryGet(handle, stage))
            return;

        m_renderTargetManager->destroyRenderTarget(stage.target);
    }

    void BackBufferManager::clearBackBuffer()
    {   
        for(auto graph : m_graphs)
            for(auto stage : graph)
                destroy_renderstage(stage);

        m_imageManager->destroySampler(sampler);

        for(auto& stage_textures : m_stage_textures)
            for(auto& tex : stage_textures)
                m_resource_mngr->destroy(tex);
                
        for(auto& bind : m_stage_bindings)
                m_material_manager->destroy_binding(bind);
    }
}