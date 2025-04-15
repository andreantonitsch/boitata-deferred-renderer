#include "Renderer.hpp"

#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>
#include <utils/utils.hpp>


namespace boitatah
{
#pragma region Initialization

    Renderer::Renderer(RendererOptions opts)
    {
        m_options = opts;
        WindowDesc desc{.dimensions = m_options.windowDimensions,
                        .windowName = m_options.appName};

        m_window = std::make_shared<WindowManager>(desc);

        // Initialize Vulkan
        createVulkan();
        m_window->initSurface(m_vk);
        m_vk->attachWindow(m_window);
        m_vk->completeInit();

        // Initialize Image and FrameBuffer Managers
        m_imageManager = std::make_shared<ImageManager>(m_vk);
        m_renderTargetManager = std::make_shared<RenderTargetManager>(m_vk, m_imageManager);

        //Create the swapchain
        createSwapchain();

        m_lightpool = std::make_unique<Pool<LightArray>>(PoolOptions{
            .size = 5,
            .dynamic =true,
            .name = " light pool" 
        });

        //Initialize the VulkanBuffer manager
        m_bufferManager = std::make_shared<BufferManager>(m_vk);

        //Initializethe command buffer writer
        m_buffer_writer = std::make_shared<VkCommandBufferWriter>(m_vk);
        m_buffer_writer->setCommandBuffer(allocateCommandBuffer({.count = 1,
                                                    .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                    .type = COMMAND_BUFFER_TYPE::TRANSFER}).buffer);
        m_buffer_writer->setFence(m_vk->createFence(true));
        m_buffer_writer->setSignal(m_vk->createSemaphore());
        

        //Initialize the renderer Modules
        m_resourceManager = std::make_shared<GPUResourceManager>(   m_vk, m_bufferManager, 
                                                                    m_imageManager,
                                                                    10);
        
        m_descriptorManager= std::make_shared<DescriptorSetManager>(m_vk, 4096);
        m_materialMngr = std::make_shared<MaterialManager>(m_vk, 
                                                           m_renderTargetManager, 
                                                           m_descriptorManager,
                                                           m_resourceManager); 

        //Create a backbuffer
        m_backBufferManager = std::make_shared<BackBufferManager>(m_renderTargetManager,
                                                                  m_imageManager,
                                                                  m_materialMngr,
                                                                  m_resourceManager);
        

        m_backBufferManager->setup2(m_options.backBufferDesc2);
        

        std::cout << "starting base material creation" << std::endl;
        // Initialize Base Materials
        m_baseMaterials = std::make_shared<Materials>(
                                                        m_materialMngr,
                                                        m_descriptorManager,
                                                        m_renderTargetManager,
                                                        m_backBufferManager);




        std::cout << "Renderer Initialization Complete " << std::endl;
    }

    void Renderer::handleWindowResize()
    {
        m_vk->waitIdle();

        m_swapchain->createSwapchain();

        auto newWindowSize = m_window->getWindowDimensions();

        m_options.backBufferDesc.dimensions = {
            static_cast<uint32_t>(newWindowSize.x),
            static_cast<uint32_t>(newWindowSize.y),
        };

        m_backBufferManager->setup2(m_options.backBufferDesc2);
    }

    void Renderer::createSwapchain()
    {
        m_swapchain = std::make_shared<Swapchain>(SwapchainOptions{.format = m_options.swapchainFormat,
                                   .useValidationLayers = m_options.debug});
        m_swapchain->attach(m_vk, m_window);
        m_swapchain->createSwapchain(); // options.windowDimensions, false, false);
    }

    std::vector<SceneNode *> Renderer::orderSceneNodes(const std::vector<SceneNode *> &nodes) const
    {
        // BIN THE NODES ACCORDING TO THE MATERIAL ORDER 
        auto cpy_nodes = nodes;
        std::vector<std::vector<SceneNode*>>bins;
        auto& material_order = m_materialMngr->orderMaterials();
        for(auto& material : material_order){
            bins.push_back({});

            auto it = cpy_nodes.begin();
            it = std::find_if(it, cpy_nodes.end(),[&material](SceneNode*& node){
                return node->material == material;
            });
            while(it != cpy_nodes.end()){
                bins.back().push_back(*it);
                it = cpy_nodes.erase(it);
                            it = std::find_if(it, cpy_nodes.end(),[&material](SceneNode*& node){
                return node->material == material;
            });
            }
        }
        return boitatah::utils::flatten(bins);
    }

    void Renderer::createVulkan()
    {
        uint32_t extensionCount = 0;

        m_vk = Vulkan::create(VulkanOptions{
            .appName = (char *)m_options.appName,
            .extensions = m_window->requiredWindowExtensions(),
            .useValidationLayers = m_options.debug,
            .debugMessages = m_options.debug,
            .window = m_window->window,
        });
    }
#pragma endregion Initialization


#pragma region CleanUp/Destructor
    void Renderer::cleanup()
    {
        m_vk->waitIdle();
    }

    Renderer::~Renderer(void)
    {
        cleanup();
    }

    BufferManager &Renderer::getBufferManager()
    {
        if(m_bufferManager == nullptr)
            throw std::runtime_error("null buffer manager");
        return *m_bufferManager;
    }

    GPUResourceManager &Renderer::getResourceManager()
    {
        if(m_resourceManager == nullptr){
            throw std::runtime_error("null resource manager");
        }
        return *m_resourceManager;
    }

    MaterialManager &Renderer::getMaterialManager()
    {
        if(m_materialMngr == nullptr){
            throw std::runtime_error("null material manager");
        }
        return *m_materialMngr;
    }

    DescriptorSetManager &Renderer::getDescriptorManager()
    {
        if(m_descriptorManager == nullptr){
            throw std::runtime_error("null descriptor manager");
        }
        return *m_descriptorManager;
    }

    Materials &Renderer::getMaterials()
    {
        return *m_baseMaterials;
    }

    BufferedCamera Renderer::createCamera(const CameraDesc &desc)
    {
        return BufferedCamera(desc, m_resourceManager);
    }

    void Renderer::setLightArray(const Handle<LightArray> &array)
    {
        lights = array;
    }

    Handle<LightArray> Renderer::createLightArray(uint32_t size)
    {

        LightArray array(size, 
                        Light{.color = glm::vec4(1.0, 1.0, 1.0, 0.0),
                            .intensity = 1.0,
                            .type = LIGHT_TYPE::POINT}, 
                        m_resourceManager);

        return m_lightpool->set(array);
    }

    LightArray &Renderer::getLightArray(Handle<LightArray> handle)
    {
        return m_lightpool->get(handle);
    }

    bool Renderer::isWindowClosed()
    {
        return m_window->isWindowClosed();
    }
#pragma endregion CleanUp / Destructor

#pragma region Rendering
    void Renderer::waitIdle()
    {
        m_vk->waitIdle();
    }

    void Renderer::write_draw_command(SceneNode &scene,
                                        const Handle<RenderTarget> &rendertarget,
                                        uint32_t frameIndex = 0)
    {
        if (!m_renderTargetManager->isActive(rendertarget))
            throw std::runtime_error("Failed to write command buffer \n\tRender Target");
        RenderTarget& target = m_renderTargetManager->get(rendertarget);

        if (!m_renderTargetManager->isActive(target.renderpass))
            throw std::runtime_error("Failed to write command buffer \n\tRender Pass");
        RenderPass& pass = m_renderTargetManager->get(target.renderpass);

        if (!m_renderTargetManager->isActive(target.sync)){
            throw std::runtime_error("Failed to Render to Target");}
        RenderTargetSync& buffers = m_renderTargetManager->get(target.sync);

        // vertex and mesh data
        Geometry geom = m_resourceManager->getResource(scene.geometry);
        drawCommand({
            .drawBuffer = buffers.drawBuffer,
            .indexCount = geom.IndexCount(),
            .vertexInfo = geom.VertexInfo(),
            .instanceInfo = {1, 0},
            .indexed = true,
        });
    }


    void Renderer::presentRenderTargetNow(Handle<RenderTarget> &rendertarget,
                                          VkSemaphore stage_wait,
                                          uint32_t attachment_index = 0)
    {
        m_window->windowEvents();

        if (!m_renderTargetManager->isActive(rendertarget))
            throw std::runtime_error("Failed to write command buffer \n\tRender Pass");
        RenderTarget& target = m_renderTargetManager->get(rendertarget);

        if (!m_renderTargetManager->isActive(target.sync)){
            throw std::runtime_error("Failed to Render to Target");}
        RenderTargetSync& buffers = m_renderTargetManager->get(target.sync);

        if(!m_imageManager->contains(target.attachments[attachment_index]))
            throw std::runtime_error("failed to get framebuffer for Presentation");
            
        Image& image = m_imageManager->getImage(target.attachments[attachment_index]);

        //m_vk->waitForFrame(buffers);
        // SubmitDrawCommand command{.bufferData = buffers, .submitType = COMMAND_BUFFER_TYPE::PRESENT};
        auto swapchainImage = m_swapchain->getNext(buffers.sc_aquired_semaphore);

        // failed to find swapchain image.
        if (swapchainImage.index == UINT32_MAX) // Fail case.
        {
            handleWindowResize();
            return;
        }
        std::vector<VkSemaphore> waits;
        waits.push_back(buffers.sc_aquired_semaphore);
        waits.push_back(stage_wait);
        
        bool successfullyPresent = m_vk->presentFrame(image,
                                                      swapchainImage.image,
                                                      swapchainImage.sc,
                                                      swapchainImage.index,
                                                      {
                                                          .commandBuffer = buffers.transferBuffer.buffer,
                                                          .waitSemaphores = waits,
                                                          .signalSemaphore = buffers.transfer_semaphore,
                                                          .fence = buffers.in_flight_fence,
                                                      });

        // If present was unsucessful we must remake the swapchain
        // and recreate our backbuffer.
        if (!successfullyPresent)
        {
            handleWindowResize();
        }
    }

    void Renderer::render_graph(SceneNode &scene, BufferedCamera &camera)
    {
        auto& backbuffer = m_backBufferManager->getNext_Graph();
        m_descriptorManager->resetPools(m_backBufferManager->getCurrentIndex());

        VkSemaphore last_stage_wait = VK_NULL_HANDLE;

        for(const auto& stage : backbuffer){
            last_stage_wait = render_graph_stage(scene, camera, stage, last_stage_wait);
        }

        auto present_target = m_backBufferManager->getPresentTarget();
        auto present_target_index = m_backBufferManager->getPresentTargetIndex();
        presentRenderTargetNow(present_target, last_stage_wait, present_target_index);
    }

    VkSemaphore Renderer::render_graph_stage(SceneNode &scene, 
                                      BufferedCamera &camera, 
                                      Handle<RenderStage> stage_handle,
                                      VkSemaphore wait_for_last_stage)
    {

        std::vector<SceneNode *> nodes;
        scene.sceneAsList(nodes);

        // TODO cullings and whatever
        // ETC
        
        auto ordered_nodes = orderSceneNodes(nodes);
        
        auto& stage = m_backBufferManager->getStage(stage_handle);
        //std::cout << "drawing stage " << stage.stage_index <<std::endl;
        RenderTarget& target = m_renderTargetManager->get(stage.target);
        RenderTargetSync& buffers = m_renderTargetManager->get(target.sync);
        RenderPass& pass = m_renderTargetManager->get(target.renderpass);

        //get image for dimension setting purposes.
        Image& image = m_imageManager->getImage(target.attachments[0]);

        uint32_t frame_index = m_backBufferManager->getCurrentIndex();
        
        //m_vk->waitForFrame(buffers);
        m_vk->resetCmdBuffer(buffers.drawBuffer.buffer);
        m_vk->resetCmdBuffer(buffers.transferBuffer.buffer);
        

        auto& shader_mngr = m_materialMngr->getShaderManager();

        m_resourceManager->beginCommitCommands();
        auto& writer = m_resourceManager->getCurrentBufferWriter();

        m_vk->beginCmdBuffer({.commandBuffer = buffers.drawBuffer.buffer});

        beginRenderpass({
            .commandBuffer = buffers.drawBuffer,
            .pass = pass,
            .target = target,
            .scissorDims = image.dimensions,
            .scissorOffset = glm::vec2(0, 0),
            .attachment_count = static_cast<uint32_t>(target.attachments.size())
        });


        auto base_mat_handle = m_baseMaterials->getStageBaseMaterial(stage.stage_index);
        //Bind base data to material
        switch(stage.type){
            //bind camera info to set 0 binding 0 of base material bindings
            case StageType::CAMERA:{
                m_materialMngr->setBufferBindingAttribute(base_mat_handle,
                                                          camera.getCameraBuffer(),
                                                          0, 0);
                break;
            }
        }

        //binds the base pipeline for this stage type;
        // if(!m_materialMngr->BindMaterial(base_mat_handle, frame_index, buffers.drawBuffer))
        //     std::runtime_error("failed to bind base pipeline");

        //Bind Pipeline <-- relevant when shader is reused.
        Handle<Shader> boundPipeline;
        Handle<Geometry> boundVertices;
        std::vector<VERTEX_BUFFER_TYPE> boundVertexTypes;
        for (const auto &node : ordered_nodes)
        {
            //skip empty node
            if (!node->material)
            {
                continue;
            }
            auto& material = m_materialMngr->getMaterialContent(node->material);

            //skip wrong stage node
            if( !((1u << stage.stage_index) &  material.stage_mask)){
                continue;
            }
            m_materialMngr->BindMaterial(node->material, frame_index, buffers.drawBuffer);
            
            Handle<Shader>& shader = material.shader;
            // TODO separate to avoid rebinding when drawing a lot of the same object
            bindVertexBuffers({
                .commandBuffer = buffers.drawBuffer,
                .frame = m_backBufferManager->getCurrentIndex(),
                .geometry = node->geometry,
                .bindIndex = true,
                .vertex_buffers = material.vertexBufferBindings
            });

            glm::mat4 model_mat = node->getGlobalMatrix();

            pushPushConstants({
                .drawBuffer = buffers.drawBuffer,
                .layout = shader_mngr.get(shader).layout.pipeline,
                .push_constants = {
                PushConstant{ //camera constant
                    .ptr = &model_mat,
                    .offset = 0,
                    .size = sizeof(glm::mat4),
                    .stages = SHADER_STAGE::ALL_GRAPHICS
                }}}
            );
            // //draw one node to target.
            write_draw_command(*node, stage.target, m_backBufferManager->getCurrentIndex());
        }
 
        m_vk->endRenderpassCommand({.commandBuffer = buffers.drawBuffer.buffer});

        std::vector<VkSemaphore> wait_semaphores;
        wait_semaphores.push_back(*writer.getSignal());


        if(wait_for_last_stage != VK_NULL_HANDLE)
            wait_semaphores.push_back(wait_for_last_stage);

        m_resourceManager->submitCommitCommands();
        
        m_vk->submitDrawCmdBuffer({.commandBuffer = buffers.drawBuffer.buffer,
                            .fence = buffers.in_flight_fence,
                            .wait_semaphores = wait_semaphores,
                            .signal_semaphore = &buffers.draw_semaphore
                            });
        
        m_materialMngr->resetBindings();
        
        // copies the frames into read textures
        m_resourceManager->beginCommitCommands();

        auto& buffer_writer = m_resourceManager->getCurrentBufferWriter();
        auto& stage_textures =  m_backBufferManager->getStageTextures(stage_handle);
        for(int i = 0; i < stage_textures.size(); i++){  
            m_resourceManager->getResource(stage_textures[i])
                              .CmdCopyImageFromImage(target.attachments[i],
                                                     IMAGE_LAYOUT::COLOR_ATT);
        }
        buffer_writer.setWait(buffers.draw_semaphore);
        m_resourceManager->submitCommitCommands();

        return *buffer_writer.getSignal();
    }

#pragma endregion Rendering



#pragma region Command Buffers

    CommandBuffer Renderer::allocateCommandBuffer(const CommandBufferDesc &desc)
    {
        return m_vk->allocateCommandBuffer(desc);
    }

    void Renderer::bindVertexBuffers(const BindVertexBuffersCommand& command ){


        auto geom = m_resourceManager->getResource(command.geometry);
        std::vector<BufferAccessData> bufferData;

        for(uint32_t i = 0; i < command.vertex_buffers.size(); ++i){
            auto buffer_handle = geom.getBuffer(command.vertex_buffers[i]);
            bufferData.push_back(
                m_resourceManager->getCommitResourceAccessData(buffer_handle, command.frame)
            );
        }

        std::vector<VkDeviceSize> offsets; 
        std::vector<VkBuffer> buffers;

        for (std::size_t i = 0; i < command.vertex_buffers.size(); i++)
        {
            offsets.push_back(static_cast<VkDeviceSize>(bufferData[i].offset));
            buffers.push_back(bufferData[i].buffer->getBuffer());
        }
        

        m_vk->CmdBindVertexBuffers({
            .drawBuffer = command.commandBuffer.buffer,
            .buffers = buffers,
            .offsets = offsets
        });

        if(command.bindIndex)
        {
            auto indexHandle = geom.IndexBuffer();
            auto indexData = m_resourceManager->getCommitResourceAccessData(indexHandle, command.frame);
            m_vk->CmdBindIndexBuffer({.drawBuffer = command.commandBuffer.buffer,
                                    .buffers = {indexData.buffer->getBuffer()},
                                    .offsets = {indexData.offset}});
        }
    };

    void Renderer::drawCommand(const DrawCommand &command)
    {
        m_vk->recordDrawCommand({
            .drawBuffer = command.drawBuffer.buffer,
            .vertexCount = command.vertexInfo.x,
            .instaceCount = 1,
            .firstVertex = command.vertexInfo.y,
            .firstInstance = 0,
            .indexed = command.indexed,
            .indexCount = command.indexCount,
        });
    }

    void Renderer::clearCommandBuffer(const CommandBuffer &buffer)
    {
        m_vk->resetCmdBuffer(buffer.buffer);
    }


    void Renderer::transferImage(const TransferImageCommand &command)
    {
        if (!m_renderTargetManager->isActive(command.src))
            throw std::runtime_error("Failed to write transfer command buffer \n\tRender Pass");
        RenderTarget& srcBuffer = m_renderTargetManager->get(command.src);

        if (!m_renderTargetManager->isActive(command.dst))
            throw std::runtime_error("Failed to write transfer command buffer \n\tRender Pass");
        RenderTarget& dstBuffer = m_renderTargetManager->get(command.src);

        Image& dstImage = m_imageManager->getImage(dstBuffer.attachments[0]);
        Image& srcImage = m_imageManager->getImage(srcBuffer.attachments[0]);

        m_vk->CmdCopyImage({.buffer = command.buffer.buffer,
                            .srcImage = srcImage.image,
                            .srcImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            .dstImage = dstImage.image,
                            .dstImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            .extent = srcImage.dimensions});

    }

    void Renderer::beginRenderpass(const BeginRenderpassCommand &command)
    {
        m_vk->beginRenderpassCommand({
            .commandBuffer = command.commandBuffer.buffer,
            .pass = command.pass.renderPass,
            .frameBuffer = command.target.buffer,
            .clearColors = command.pass.clearColors,
            .scissorDims = command.scissorDims,
            .scissorOffset = command.scissorOffset,
            .depth = command.pass.description.use_depthStencil,
        });
    }


    void Renderer::pushPushConstants(const PushConstantsCommand &command)
    {
        for(auto& push_constant : command.push_constants){
        vkCmdPushConstants(
            command.drawBuffer.buffer,
            command.layout.pipeline,
            castEnum<VkShaderStageFlags>(push_constant.stages),
            push_constant.offset,
            push_constant.size,
            push_constant.ptr
        );
    }
    }

#pragma endregion Command Buffers

#pragma region Create Vulkan Objects

    Handle<Shader> Renderer::createShader(const MakeShaderDesc &data)
    {
        return m_materialMngr->getShaderManager().makeShader(data);
    }

    Handle<RenderTarget> Renderer::createRenderTarget(const RenderTargetDesc &data)
    {
        return m_renderTargetManager->createRenderTarget(data);
    }

    Handle<RenderPass> Renderer::createRenderPass(const RenderPassDesc &data)
    {
        return m_renderTargetManager->createRenderPass(data);
    }

    Handle<Image> Renderer::createImage(const ImageDesc &desc)
    {
        return m_imageManager->createImage(desc);
    }


    Handle<Material> Renderer::createMaterial(const MaterialCreate &description)
    {
        return m_materialMngr->createMaterial(description);
    }

    Handle<ShaderLayout> Renderer::createShaderLayout(const ShaderLayoutDesc &desc)
    {
        return m_materialMngr->getShaderManager().makeShaderLayout(desc);
    }

    Handle<RenderTargetSync> Renderer::createRenderTargetCmdData()
    {
        return m_renderTargetManager->createRenderTargetSyncData();
    }

#pragma endregion Create Vulkan Objects


#pragma region Destroy Vulkan Objects

    void Renderer::destroyShader(Handle<Shader>& handle)
    {
        auto& shader_mngr = m_materialMngr->getShaderManager();
        shader_mngr.destroy(handle);
    }

    void Renderer::destroyRenderTarget(Handle<RenderTarget>& bufferhandle)
    {
        m_renderTargetManager->destroyRenderTarget(bufferhandle);
    }

    void Renderer::destroyRenderPass(Handle<RenderPass>& passhandle)
    {
        m_renderTargetManager->destroyRenderPass(passhandle);
    }

    void Renderer::destroyLayout(Handle<ShaderLayout>& layouthandle)
    {
        m_materialMngr->getShaderManager().destroy(layouthandle);
    }

#pragma endregion Destroy Vulkan Objects

}
