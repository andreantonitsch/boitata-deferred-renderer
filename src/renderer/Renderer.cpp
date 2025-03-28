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

        //Create a backbuffer
        m_backBufferManager = std::make_shared<BackBufferManager>(m_renderTargetManager);
        m_backBufferManager->setup(m_options.backBufferDesc);

        m_transferCommandBuffer = allocateCommandBuffer({.count = 1,
                                                         .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                         .type = COMMAND_BUFFER_TYPE::TRANSFER});
        
        m_transferFence = m_vk->createFence(true); //create fence signaled

        //Initialize the VulkanBuffer manager
        m_bufferManager = std::make_shared<BufferManager>(m_vk);

        //Initializethe command buffer writer
        m_ResourceManagerTransferWriter = std::make_shared<VkCommandBufferWriter>(m_vk);
        m_ResourceManagerTransferWriter->setCommandBuffer(allocateCommandBuffer({.count = 1,
                                                         .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                         .type = COMMAND_BUFFER_TYPE::TRANSFER}).buffer);
        m_ResourceManagerTransferWriter->setFence(m_vk->createFence(true));
        m_ResourceManagerTransferWriter->setSignal(m_vk->createSemaphore());
        

        //Initialize the renderer Modules
        m_resourceManager = std::make_shared<GPUResourceManager>(m_vk, m_bufferManager, m_imageManager, m_ResourceManagerTransferWriter);
        m_descriptorManager= std::make_shared<DescriptorSetManager>(m_vk, 4096);
        m_materialMngr = std::make_shared<MaterialManager>(m_vk, 
                                                           m_renderTargetManager, 
                                                           m_descriptorManager,
                                                           m_resourceManager); 
        auto& shader_mngr = m_materialMngr->getShaderManager();

        // Initialize base renderer data(base material, base layout etc.)
        m_frameUniformsBuffer = m_resourceManager->create(GPUBufferCreateDescription{
            .size = sizeof(FrameUniforms2),
            .usage = BUFFER_USAGE::UNIFORM_BUFFER,
            .sharing_mode = SHARING_MODE::EXCLUSIVE,
            });
        // camera layout
       base_setLayout = m_descriptorManager->getLayout({
                                                            .bindingDescriptors = {
                                                                {//.binding = 0,
                                                                .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
                                                                .stages = STAGE_FLAG::ALL_GRAPHICS,
                                                                .descriptorCount= 1,
                                                                }
                                                            }
                                                        });

        m_frameUniforms = m_materialMngr->createBinding(base_setLayout);
        auto& binding_ref = m_materialMngr->getBinding(m_frameUniforms);
        binding_ref.bindings[0].binding_handle.buffer = m_frameUniformsBuffer;

        //create base layout with push constants for model matrices
        Handle<ShaderLayout> m_baseShaderLayout = shader_mngr.makeShaderLayout({.setLayouts={base_setLayout}});

        m_dummyPipeline = createShader({
            .vert = {.byteCode = utils::readFile("./src/09_shader_base_vert.spv"),
                     .entryFunction = "main"},
            .frag = {.byteCode = utils::readFile("./src/09_shader_base_frag.spv"),
                     .entryFunction = "main"},
            .renderPass = m_backBufferManager->getRenderPass(),
            .layout = m_baseShaderLayout,
            .vertexBindings = {}
            });

        m_baseMaterial = m_materialMngr->createMaterial({
            .shader = m_dummyPipeline,
             .bindings = {m_frameUniforms},
            .name = "base material",
        });

        m_materialMngr->setBaseMaterial(m_baseMaterial);
        m_materialMngr->setupBaseMaterials(m_backBufferManager->getRenderPass());
        std::cout << "Renderer Initialization Complete " << std::endl;
    }

    void Renderer::updateCameraUniforms(Camera &camera)
    {
        frame_uniforms.camera = camera.getCameraUniforms();

    }

    void Renderer::updateFrameUniforms(uint32_t frame_index)
    {
        GPUBuffer& buffer = m_resourceManager->getResource(m_frameUniformsBuffer);
        buffer.copyData(&frame_uniforms, sizeof(FrameUniforms2));
        //m_resourceManager->commitResourceCommand(m_frameUniformsBuffer, frame_index);
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

        m_backBufferManager->setup(m_options.backBufferDesc);
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
        auto& layout = m_descriptorManager->getLayoutContent(base_setLayout);
        m_vk->destroyDescriptorSetLayout(layout.layout);
        
        if(m_vk->checkFenceStatus(m_transferFence))
            m_vk->waitForFence(m_transferFence);

        m_vk->destroyFence(m_transferFence);
        
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

    void Renderer::renderToRenderTarget(SceneNode &scene, const Handle<RenderTarget> &rendertarget, uint32_t frameIndex = 0)
    {
        if (!m_renderTargetManager->isActive(rendertarget))
            throw std::runtime_error("Failed to write command buffer \n\tRender Target");
        RenderTarget& target = m_renderTargetManager->get(rendertarget);

        if (!m_renderTargetManager->isActive(target.renderpass))
            throw std::runtime_error("Failed to write command buffer \n\tRender Pass");
        RenderPass& pass = m_renderTargetManager->get(target.renderpass);

        if (!m_renderTargetManager->isActive(target.cmdBuffers)){
            throw std::runtime_error("Failed to Render to Target");}
        RenderTargetSync& buffers = m_renderTargetManager->get(target.cmdBuffers);

        // vertex and mesh data
        Geometry geom = m_resourceManager->getResource(scene.geometry);
        
        //auto indexBufferData = m_resourceManager->getCommitResourceAccessData(geom.IndexBuffer(), frameIndex);
        //auto indexVkBuffer = indexBufferData.buffer->getBuffer();

        drawCommand({
            .drawBuffer = buffers.drawBuffer,
            .indexCount = geom.IndexCount(),
            .vertexInfo = geom.VertexInfo(),
            .instanceInfo = {1, 0},
            .indexed = true,
        });
    }


    void Renderer::presentRenderTargetNow(Handle<RenderTarget> &rendertarget, uint32_t attachment_index = 0)
    {
        m_window->windowEvents();

        if (!m_renderTargetManager->isActive(rendertarget))
            throw std::runtime_error("Failed to write command buffer \n\tRender Pass");
        RenderTarget& target = m_renderTargetManager->get(rendertarget);

        if (!m_renderTargetManager->isActive(target.cmdBuffers)){
            throw std::runtime_error("Failed to Render to Target");}
        RenderTargetSync& buffers = m_renderTargetManager->get(target.cmdBuffers);
        if(!m_imageManager->contains(target.attachments[attachment_index]))
        throw std::runtime_error("failed to get framebuffer for Presentation");
        Image& image = m_imageManager->getImage(target.attachments[attachment_index]);

        //m_vk->waitForFrame(buffers);
        // SubmitDrawCommand command{.bufferData = buffers, .submitType = COMMAND_BUFFER_TYPE::PRESENT};
        auto swapchainImage = m_swapchain->getNext(buffers.schainAcqSem);

        // failed to find swapchain image.
        if (swapchainImage.index == UINT32_MAX) // Fail case.
        {
            handleWindowResize();
            return;
        }
        std::vector<VkSemaphore> waits;
        waits.push_back(buffers.schainAcqSem);
        waits.push_back(buffers.writeSem);
        bool successfullyPresent = m_vk->presentFrame(image,
                                                      swapchainImage.image,
                                                      swapchainImage.sc,
                                                      swapchainImage.index,
                                                      {
                                                          .commandBuffer = buffers.transferBuffer.buffer,
                                                          .waitSemaphores = waits,
                                                          .signalSemaphore = buffers.transferSem,
                                                          .fence = buffers.inFlightFen,
                                                      });

        // If present was unsucessful we must remake the swapchain
        // and recreate our backbuffer.
        if (!successfullyPresent)
        {
            handleWindowResize();
        }
    }

    void Renderer::renderSceneNode(SceneNode &scene, Handle<RenderTarget> &rendertarget)
    {
        std::vector<SceneNode *> nodes;
        scene.sceneAsList(nodes);

        //std::cout << "drawing scene with " << nodes.size() << " nodes "<<std::endl;
        // TODO cullings and whatever
        // TRANSFORM UPDATES
        // ETC
        
        auto ordered_nodes = nodes; //orderSceneNodes(nodes);
        
        RenderTarget& target = m_renderTargetManager->get(rendertarget);
        RenderTargetSync& buffers = m_renderTargetManager->get(target.cmdBuffers);
        RenderPass& pass = m_renderTargetManager->get(target.renderpass);
        Image& image = m_imageManager->getImage(target.attachments[0]);

        uint32_t frame_index = m_backBufferManager->getCurrentIndex();
        
        m_vk->waitForFrame(buffers);

        m_vk->resetCmdBuffer(buffers.drawBuffer.buffer);
        m_vk->resetCmdBuffer(buffers.transferBuffer.buffer);
        m_descriptorManager->resetPools(frame_index);

        auto& shader_mngr = m_materialMngr->getShaderManager();
        
        m_resourceManager->waitForTransfers();
        m_resourceManager->beginCommitCommands();

        beginBuffer({.buffer = buffers.drawBuffer});

        //std::cout << "began buffer" << std::endl;
        beginRenderpass({
            .commandBuffer = buffers.drawBuffer,
            .pass = pass,
            .target = target,

            .clearColor = glm::vec4(0, 0, 0, 1),
            .scissorDims = image.dimensions,
            .scissorOffset = glm::vec2(0, 0),
        });

        //binds dummy pipeline
        if(!m_materialMngr->BindMaterial(m_baseMaterial, frame_index, buffers.drawBuffer))
            std::runtime_error("failed to bind material");

        //Bind Pipeline <-- relevant when shader is reused.
        Handle<Shader> boundPipeline;
        Handle<Geometry> boundVertices;
        std::vector<VERTEX_BUFFER_TYPE> boundVertexTypes;

        for (const auto &node : ordered_nodes)
        {
            if (!node->material)
            {
                std::cout << "skip drawing node" << std::endl;
                continue;
            }
            
            auto& material = m_materialMngr->getMaterialContent(node->material);
            m_materialMngr->BindMaterial(node->material, frame_index, buffers.drawBuffer);
            
            Handle<Shader>& shader = material.shader;
            
            // TODO separate to avoid rebinding when drawing a lot of the same object
            bindVertexBuffers({
                .commandBuffer = buffers.drawBuffer,
                .frame = m_backBufferManager->getCurrentIndex(),
                .geometry = node->geometry,
                .bindIndex = true
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
                    .stages = STAGE_FLAG::ALL_GRAPHICS
                }}}
            );

            renderToRenderTarget(*node, rendertarget, m_backBufferManager->getCurrentIndex());
        }
 
        m_vk->endRenderpassCommand({.commandBuffer = buffers.drawBuffer.buffer});

        std::vector<VkSemaphore> wait_semaphores;
        wait_semaphores.push_back(*(m_resourceManager->getCommandBufferWriter().getSignal()));
 
        m_resourceManager->submitCommitCommands();
        m_vk->submitDrawCmdBuffer({.commandBuffer = buffers.drawBuffer.buffer,
                            .fence = buffers.inFlightFen,
                            .wait_semaphores = wait_semaphores,
                            .signal_semaphore = &buffers.writeSem
                            });

        m_materialMngr->resetBindings();
        //std::cout << "Submit Draw Command \n";
    }
    void Renderer::render(SceneNode &scene)
    {
        auto backbuffer = m_backBufferManager->getNext();
        updateFrameUniforms(m_backBufferManager->getCurrentIndex());
        renderSceneNode(scene, backbuffer);
        presentRenderTargetNow(backbuffer);
    }
    
    void Renderer::render(SceneNode &scene, Camera &camera)
    {
        auto backbuffer = m_backBufferManager->getNext();
        renderSceneNode(scene, camera, backbuffer);
        presentRenderTargetNow(backbuffer);
    }

    void Renderer::render_graph(SceneNode &scene, Camera &camera)
    {
        // auto backbuffer = m_backBufferManager->getNext_Graph();

        // for(const auto& stage : backbuffer){
        //     render_graph_stage(scene, camera, stage);

        // }
        // auto present_target = m_backBufferManager->getPresentTarget();
        // auto present_target_index = m_backBufferManager->getPresentTargetIndex();
        // presentRenderTargetNow(present_target, present_target_index);
    }

    void Renderer::renderSceneNode(SceneNode &scene, Camera &camera, Handle<RenderTarget> &rendertargetHandle)
    {
        updateCameraUniforms(camera);
        updateFrameUniforms(m_backBufferManager->getCurrentIndex());
        renderSceneNode(scene, rendertargetHandle);
    }

#pragma endregion Rendering



#pragma region Command Buffers

    CommandBuffer Renderer::allocateCommandBuffer(const CommandBufferDesc &desc)
    {
        return m_vk->allocateCommandBuffer(desc);
    }

    void Renderer::bindVertexBuffers(const BindVertexBuffersCommand& command ){

        auto geom = m_resourceManager->getResource(command.geometry);
        auto vertexBufferHandle = geom.getBuffer(VERTEX_BUFFER_TYPE::POSITION);
        auto colorBufferHandle = geom.getBuffer(VERTEX_BUFFER_TYPE::COLOR);
        auto uvBufferHandle = geom.getBuffer(VERTEX_BUFFER_TYPE::UV);
        std::array<BufferAccessData, 3> bufferData;
        bufferData[0] = m_resourceManager->getCommitResourceAccessData(vertexBufferHandle, command.frame);
        bufferData[1] = m_resourceManager->getCommitResourceAccessData(uvBufferHandle, command.frame);
        bufferData[2] = m_resourceManager->getCommitResourceAccessData(colorBufferHandle, command.frame);

        std::vector<VkDeviceSize> offsets; 
        std::vector<VkBuffer> buffers;

        for (std::size_t i = 0; i < 3; i++)
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

    void Renderer::copyBuffer(const CopyBufferCommand &command)
    {   
        bool fetchedBuffers = true;
        Buffer * srcBuffer;
        Buffer *dstBuffer;

        BufferReservation srcReservation;
        BufferReservation dstReservation;

        fetchedBuffers &= m_bufferManager->getAddressBuffer(command.src, srcBuffer);
        fetchedBuffers &= m_bufferManager->getAddressBuffer(command.dst, dstBuffer);

        m_bufferManager->getAddressReservation(command.src,srcReservation);
        m_bufferManager->getAddressReservation(command.dst, dstReservation);

        if(!fetchedBuffers)
            throw std::runtime_error("Failed to fetch buffers to copy");

        // vk->waitIdle();
        m_vk->waitForFence(m_transferFence);
        m_vk->beginCmdBuffer({.commandBuffer = command.buffer.buffer});
    }

    void Renderer::beginBuffer(const BeginBufferCommand &command)
    {
        m_vk->beginCmdBuffer({.commandBuffer = command.buffer.buffer});
    }

    void Renderer::beginRenderpass(const BeginRenderpassCommand &command)
    {
        m_vk->beginRenderpassCommand({
            .commandBuffer = command.commandBuffer.buffer,
            .pass = command.pass.renderPass,
            .frameBuffer = command.target.buffer,
            .clearColor = command.clearColor,

            .scissorDims = command.scissorDims,
            .scissorOffset = command.scissorOffset,
            .depth = command.pass.description.use_depthStencil
            //.viewportDims = command.viewportDims,
            //.viewportOffset = command.viewportOffset,
        });
    }

    void Renderer::setCameraUniforms(Camera &camera)
    {
        frame_uniforms.camera = camera.getCameraUniforms();
    }

    void Renderer::submitBuffer(const SubmitBufferCommand &command)
    {
        m_vk->submitCmdBuffer({
            .commandBuffer = command.buffer.buffer,
            .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
            .fence = m_transferFence,
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
        auto d = description;
        if(!description.parent)
            d.parent = m_baseMaterial;

        return m_materialMngr->createMaterial(d);
    }

    Handle<ShaderLayout> Renderer::createShaderLayout(const ShaderLayoutDesc &desc)
    {
        return m_materialMngr->getShaderManager().makeShaderLayout(desc);
    }

    Handle<RenderPass> Renderer::getBackBufferRenderPass()
    {
        return m_backBufferManager->getRenderPass();
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
