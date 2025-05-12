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
        m_buffer_writer->set_commandbuffer(allocateCommandBuffer({.count = 1,
                                                    .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                    .type = COMMAND_BUFFER_TYPE::TRANSFER}).buffer);
        m_buffer_writer->set_fence(m_vk->createFence(true));
        m_buffer_writer->set_signal(m_vk->createSemaphore());
        

        //Initialize the renderer Modules
        m_resourceManager = std::make_shared<GPUResourceManager>(   m_vk, m_bufferManager, 
                                                                    m_imageManager,
                                                                    20);
        
        m_descriptorManager= std::make_shared<DescriptorSetManager>(m_vk, 4096);
        m_materialMngr = std::make_shared<MaterialManager>(m_vk, 
                                                           m_renderTargetManager, 
                                                           m_descriptorManager,
                                                           m_resourceManager); 

        //Create a backbuffer
        m_backBufferManager = std::make_shared<BackBufferManager>(m_renderTargetManager,
                                                                  m_imageManager,
                                                                  m_materialMngr,
                                                                  m_resourceManager,
                                                                  m_vk);
        

        m_backBufferManager->setup(m_options.backBufferDesc2);
        

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

        m_backBufferManager->setup(m_options.backBufferDesc2);
    }

    void Renderer::createSwapchain()
    {
        m_swapchain = std::make_shared<Swapchain>(SwapchainOptions{.format = m_options.swapchainFormat,
                                   .useValidationLayers = m_options.debug});
        m_swapchain->attach(m_vk, m_window);
        m_swapchain->createSwapchain(); // options.windowDimensions, false, false);
    }

    std::vector<std::shared_ptr<RenderScene>> 
    Renderer::orderSceneNodes(const std::vector<std::shared_ptr<RenderScene>> &nodes) const
    {
        // BIN THE NODES ACCORDING TO THE MATERIAL ORDER 
        auto cpy_nodes = nodes;
        std::vector<std::vector<std::shared_ptr<RenderScene>>>  bins;
        auto& material_order = m_materialMngr->orderMaterials();
        for(auto& material : material_order){
            bins.push_back({});

            auto it = cpy_nodes.begin();
            it = std::find_if(it, cpy_nodes.end(),[&material](std::shared_ptr<RenderScene> node){
                return node->content.material == material;
            });
            while(it != cpy_nodes.end()){
                bins.back().push_back(*it);
                it = cpy_nodes.erase(it);
                            it = std::find_if(it, cpy_nodes.end(),[&material](std::shared_ptr<RenderScene> node){
                return node->content.material == material;
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

    template<typename T>
    void Renderer::write_draw_command(
            CommandBufferWriter<T> &writer,
            RenderScene &scene,
            const Handle<RenderTarget> &rendertarget,
            uint32_t frameIndex){
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
        Geometry geom = m_resourceManager->getResource(scene.content.geometry);

        // TODO ???
        writer.draw(typename CommandBufferWriter<T>::DrawCommand{
            static_cast<uint32_t>(geom.VertexInfo().x),
            static_cast<uint32_t>(1),
            static_cast<uint32_t>(geom.VertexInfo().y),
            static_cast<uint32_t>(0),
            true,
            static_cast<uint32_t>(geom.IndexCount()),
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

        //waits for previous tranfers to finish


        auto swapchainImage = m_swapchain->getNext(buffers.sc_aquired_semaphore);
        
        // failed to find swapchain image.
        if (swapchainImage.index == UINT32_MAX) // Fail case.
        {
            handleWindowResize();
            return;
        }
        
        // failed to find swapchain image.
        if (swapchainImage.index == UINT32_MAX -1u) // Fail case.
        {
            //handleWindowResize();
            return;
        }

        std::vector<VkSemaphore> waits;
        waits.push_back(buffers.sc_aquired_semaphore);
        waits.push_back(stage_wait);

        auto present_writer = VkCommandBufferWriter(m_vk);
        present_writer.set_commandbuffer(buffers.present_buffer.buffer);
        present_writer.set_fence(buffers.in_flight_fence);
        //TODO fix this garbo
        m_vk->reset_fence(buffers.in_flight_fence);

        //sets semaphores
        present_writer.setWait(waits);
        present_writer.set_signal(buffers.transfer_semaphore);
        
        
        present_writer.reset({});
        present_writer.begin({});
        present_writer.copy_image({ 
            .srcLayout = castEnum<VkImageLayout>(IMAGE_LAYOUT::COLOR_ATT),
            .dstLayout = castEnum<VkImageLayout>(IMAGE_LAYOUT::PRESENT_SRC),
            .extent = image.dimensions,
            .srcImage = image.image,
            .dstImage = swapchainImage.image.image,
        });
        
        present_writer.submit({.submitType = COMMAND_BUFFER_TYPE::TRANSFER, .signal= true});

        //present image
        bool successfullyPresent = m_vk->presentFrame(swapchainImage.image,
                                                      swapchainImage.sc,
                                                      swapchainImage.index,
                                                      {
                                                          .waitSemaphores = {buffers.transfer_semaphore},
                                                      });

        // If present was unsucessful we must remake the swapchain
        // and recreate our backbuffer.
        if (!successfullyPresent)
        {
            handleWindowResize();
        }
    }

    void Renderer::render_graph(RenderScene &scene, BufferedCamera &camera)
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

    VkSemaphore Renderer::render_graph_stage(RenderScene &scene, 
                                      BufferedCamera &camera, 
                                      Handle<RenderStage> stage_handle,
                                      VkSemaphore wait_for_last_stage)
    {

        std::vector<std::weak_ptr<RenderScene>> nodes;
        scene.sceneAsList(nodes);

        // TODO cullings and whatever
        // ETC
        
        auto ordered_nodes = nodes;//orderSceneNodes(nodes);
        
        // Unpack data structures.
        auto& stage = m_backBufferManager->getStage(stage_handle);
        //std::cout << "drawing stage " << stage.stage_index <<std::endl;
        RenderTarget& target = m_renderTargetManager->get(stage.target);
        RenderTargetSync& buffers = m_renderTargetManager->get(target.sync);
        RenderPass& pass = m_renderTargetManager->get(target.renderpass);
        
        //get image for dimension setting purposes.
        Image& image = m_imageManager->getImage(target.attachments[0]);
        
        uint32_t frame_index = m_backBufferManager->getCurrentIndex();
        
        
        auto& shader_mngr = m_materialMngr->getShaderManager();
        
        //m_vk->waitForFence(buffers.in_flight_fence);
        m_resourceManager->beginCommitCommands();
        auto& resource_writer = m_resourceManager->getCurrentBufferWriter();

        if(wait_for_last_stage != VK_NULL_HANDLE)
            resource_writer.setWait({wait_for_last_stage});
        

        auto writer = VkCommandBufferWriter(m_vk);
        writer.set_commandbuffer(buffers.draw_buffer.buffer);
        writer.set_signal(buffers.draw_semaphore);
        writer.setWait({*resource_writer.get_signal()});
        writer.reset({});
        writer.begin({});

        writer.begin_renderpass({
            .pass = pass.renderPass,
            .frame_buffer = target.buffer,
            .clearColors = pass.clearColors,
            .scissorDims = image.dimensions,
            .scissorOffset = glm::vec2(0, 0),
            .depth = pass.description.use_depthStencil,
            .attachment_count = static_cast<uint32_t>(target.attachments.size()),
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
        //Bind Pipeline <-- relevant when shader is reused.
        Handle<Shader> boundPipeline;
        Handle<Geometry> boundVertices;
        std::vector<VERTEX_BUFFER_TYPE> boundVertexTypes;
        for (const auto &node_weak : ordered_nodes)
        {
            auto node = std::shared_ptr<RenderScene>(node_weak);
            //skip empty node
            if (!node->content.material)
            {
                continue;
            }
            auto& material = m_materialMngr->getMaterialContent(node->content.material);

            //skip wrong stage node
            if( !((1u << stage.stage_index) &  material.stage_mask)){
                continue;
            }
            m_materialMngr->BindMaterial(writer, node->content.material, frame_index);
            
            Handle<Shader>& shader = material.shader;
            // TODO separate to avoid rebinding when drawing a lot of the same object
            bindVertexBuffers(
                m_backBufferManager->getCurrentIndex(),
                node->content.geometry,
                true,
                material.vertexBufferBindings,
                writer);

            glm::mat4 model_mat = node->getGlobalMatrix();

            pushPushConstants({
                .drawBuffer = buffers.draw_buffer,
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
            write_draw_command(writer,
                               *node, 
                               stage.target, 
                               m_backBufferManager->getCurrentIndex());
        }
 
        writer.end_renderpass({});

        m_resourceManager->submitCommitCommands();

        //writer.set_fence(buffers.in_flight_fence);
        writer.submit({ .submitType = COMMAND_BUFFER_TYPE::GRAPHICS,
                        .signal = true});
        
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
        buffer_writer.setWait({buffers.draw_semaphore});
        //buffer_writer.set_fence(buffers.in_flight_fence);
        m_resourceManager->submitCommitCommands();

        return *buffer_writer.get_signal();
    }

#pragma endregion Rendering



#pragma region Command Buffers

    CommandBuffer Renderer::allocateCommandBuffer(const CommandBufferDesc &desc)
    {
        return m_vk->allocateCommandBuffer(desc);
    }

    void Renderer::bindVertexBuffers(uint32_t           frame_index, 
                                    Handle<Geometry>    geometry, 
                                    bool                indexed, 
                                    std::vector<VERTEX_BUFFER_TYPE> vertex_buffers, 
                                    VkCommandBufferWriter           &writer) {
        auto geom = m_resourceManager->getResource(geometry);
        std::vector<BufferAccessData> bufferData;

        for(uint32_t i = 0; i < vertex_buffers.size(); ++i){
            auto buffer_handle = geom.getBuffer(vertex_buffers[i]);
            bufferData.push_back(
                m_resourceManager->getCommitResourceAccessData(buffer_handle, frame_index)
            );
        }

        std::vector<VkDeviceSize> offsets; 
        std::vector<VkBuffer> buffers;

        for (std::size_t i = 0; i < vertex_buffers.size(); i++)
        {
            offsets.push_back(static_cast<VkDeviceSize>(bufferData[i].offset));
            buffers.push_back(bufferData[i].buffer->getBuffer());
        }
        
        writer.bind_vertexbuffers({
            .buffers = buffers,
            .offsets = offsets
        });

         if(indexed)  
        {
            auto indexHandle = geom.IndexBuffer();
            auto indexData = m_resourceManager->getCommitResourceAccessData(indexHandle, frame_index);
            // m_vk->CmdBindIndexBuffer({.drawBuffer = command.commandBuffer.buffer,
            //                         .buffers = {indexData.buffer->getBuffer()},
            //                         .offsets = {indexData.offset}});
            writer.bind_indexbuffer({
                                    .buffers = {indexData.buffer->getBuffer()},
                                    .offsets = {indexData.offset}});
        }
    
    };

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


}
