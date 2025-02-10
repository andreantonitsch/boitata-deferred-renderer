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
        createVulkan();

        m_window->initSurface(m_vk);
        m_vk->attachWindow(m_window);
        m_vk->completeInit();

        createSwapchain();

        m_backBufferManager = std::make_shared<BackBufferManager>(this);
        m_backBufferManager->setup(m_options.backBufferDesc);

        m_transferCommandBuffer = allocateCommandBuffer({.count = 1,
                                                         .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                         .type = COMMAND_BUFFER_TYPE::TRANSFER});\
        
        m_transferFence = m_vk->createFence(true); //create fence signaled
        m_bufferManager = std::make_shared<BufferManager>(m_vk);
        m_ResourceManagerTransferWriter = std::make_shared<VkCommandBufferWriter>(m_vk);
        m_ResourceManagerTransferWriter->setCommandBuffer(m_transferCommandBuffer.buffer);
        m_ResourceManagerTransferWriter->setFence(m_transferFence);
        m_ResourceManagerTransferWriter->setSignal(nullptr);
        
        m_resourceManager = std::make_shared<GPUResourceManager>(m_vk, m_bufferManager, m_ResourceManagerTransferWriter);

        // frame uniforms
        m_frameUniform = m_resourceManager->create(GPUBufferCreateDescription{
            .size = sizeof(FrameUniforms2),
            .usage = BUFFER_USAGE::UNIFORM_BUFFER,
            .sharing_mode = SHARING_MODE::EXCLUSIVE,
            });

        
        base_setLayout = createDescriptorLayout({
                                                    .bindingDescriptors = {
                                                    {//.binding = 0,
                                                        .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
                                                        .stages = STAGE_FLAG::ALL_GRAPHICS,
                                                        .descriptorCount= 1,
                                                        }}});

        //create base layout with push constants for model matrices
        Handle<ShaderLayout> m_baseShaderLayout = createShaderLayout({});

        m_descriptorManager = std::make_shared<DescriptorPoolManager>(m_vk, m_resourceManager, 4096);
        m_dummyPipeline = createShader({
            .vert = {.byteCode = utils::readFile("./src/09_shader_base_vert.spv"),
                     .entryFunction = "main"},
            .frag = {.byteCode = utils::readFile("./src/09_shader_base_frag.spv"),
                     .entryFunction = "main"},
                                            
            .layout = m_baseShaderLayout,
            .bindings = {}
            });
        
    }

    void Renderer::updateCameraUniforms(Camera &camera)
    {
        frame_uniforms.camera = camera.getCameraUniforms();

    }

    void Renderer::updateFrameUniforms(uint32_t frame_index)
    {
        GPUBuffer& buffer = m_resourceManager->getResource(m_frameUniform);
        buffer.copyData(&frame_uniforms, sizeof(FrameUniforms2));
        m_resourceManager->forceCommitResource(m_frameUniform, frame_index);
        
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
        m_swapchain->attach(m_vk, this, m_window);
        m_swapchain->createSwapchain(); // options.windowDimensions, false, false);
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

#pragma region Rendering
    void Renderer::waitIdle()
    {
        m_vk->waitIdle();
    }

    void Renderer::renderToRenderTarget(SceneNode &scene, const Handle<RenderTarget> &rendertarget, uint32_t frameIndex = 0)
    {
        RenderTarget target;
        if (!renderTargetPool.tryGet(rendertarget, target))
        {
            throw std::runtime_error("Failed to write command buffer \n\tRender Target");
        }
        RenderPass pass;
        if (!renderpassPool.tryGet(target.renderpass, pass))
        {
            throw std::runtime_error("Failed to write command buffer \n\tRender Pass");
        }

        Image image;
        if (!imagePool.tryGet(target.attachments[0], image))
        {
            throw std::runtime_error("Failed to write command buffer \n\tImage");
        }

        RenderTargetCmdBuffers buffers;
        if (!rtCmdPool.tryGet(target.cmdBuffers, buffers))
            throw std::runtime_error("Failed to Render to Target");

        // for scene in scene
        Shader shader;
        if (!shaderPool.tryGet(scene.shader, shader))
        {
            throw std::runtime_error("Failed to retrieve material");
        }

        // vertex and mesh data
        Geometry geom = m_resourceManager->getResource(scene.geometry);

        Handle<GPUBuffer> vertexBufferHandle = geom.getBuffer(VERTEX_BUFFER_TYPE::POSITION);

        auto vertexGPUBuffer = m_resourceManager->getResource(vertexBufferHandle);
        auto vertexGPUBufferContent = vertexGPUBuffer.get_content(frameIndex);
        

        Buffer* vertexBuffer;
        BufferReservation vertexBufferReservation;
        m_bufferManager->getAddressReservation(vertexGPUBufferContent.buffer, vertexBufferReservation);
        m_bufferManager->getAddressBuffer(vertexGPUBufferContent.buffer, vertexBuffer);

        auto indexBufferAddressHandle = m_resourceManager->getResource(geom.IndexBuffer()).get_content(frameIndex).buffer;
        Buffer* indexBuffer;
        BufferReservation indexBufferReservation;
        m_bufferManager->getAddressBuffer(indexBufferAddressHandle, indexBuffer);
        
        m_bufferManager->getAddressReservation(indexBufferAddressHandle, indexBufferReservation);

        auto indexVkBuffer = indexBuffer->getBuffer();
        auto vertexVkBuffer = vertexBuffer->getBuffer();

        drawCommand({
            .drawBuffer = buffers.drawBuffer,
            .renderTarget = target,
            .renderPass = pass,
            //.shader = shader,
            .dimensions = {
                static_cast<int>(image.dimensions.x),
                static_cast<int>(image.dimensions.y),
            },

            //.vertexBuffer = vertexBufferHandle.isNull() ? VK_NULL_HANDLE : vertexBuffer->getBuffer(),
            .vertexBuffer = vertexVkBuffer,
            .vertexBufferOffset = vertexBufferReservation.offset,
            //.indexBuffer = geom.indexBuffer.isNull() ? VK_NULL_HANDLE : indexBuffer->getBuffer(),
            .indexBuffer = indexVkBuffer,
            .indexBufferOffset  = indexBufferReservation.offset,
            .indexCount = geom.IndexCount(),

            .vertexInfo = geom.VertexInfo(),
            .instanceInfo = {1, 0}, // scene.instanceInfo

        });
    }
    void Renderer::render(SceneNode &scene)
    {
        auto backbuffer = m_backBufferManager->getNext();
        updateFrameUniforms(m_backBufferManager->getCurrentIndex());
        renderSceneNode(scene, backbuffer);
        presentRenderTarget(backbuffer);
    }

    void Renderer::presentRenderTarget(Handle<RenderTarget> &rendertarget)
    {
        m_window->windowEvents();

        RenderTarget fb;
        if (!renderTargetPool.tryGet(rendertarget, fb))
            throw std::runtime_error("failed to framebuffer for Presentation");

        RenderTargetCmdBuffers buffers;
        if (!rtCmdPool.tryGet(fb.cmdBuffers, buffers))
            throw std::runtime_error("failed to framebuffer for Presentation");

        Image image;
        if (!imagePool.tryGet(fb.attachments[0], image))
            throw std::runtime_error("failed to framebuffer for Presentation");

        m_vk->waitForFrame(buffers);
        // SubmitDrawCommand command{.bufferData = buffers, .submitType = COMMAND_BUFFER_TYPE::PRESENT};
        auto swapchainImage = m_swapchain->getNext(buffers.schainAcqSem);

        // failed to find swapchain image.
        if (swapchainImage.index == UINT32_MAX) // Fail case.
        {
            handleWindowResize();
            return;
        }

        bool successfullyPresent = m_vk->presentFrame(image,
                                                      swapchainImage.image,
                                                      swapchainImage.sc,
                                                      swapchainImage.index,
                                                      {
                                                          .commandBuffer = buffers.transferBuffer.buffer,
                                                          .waitSemaphore = buffers.schainAcqSem,
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

        // TODO cullings and whatever
        // TRANSFORM UPDATES
        // ETC
        
        RenderTarget target = renderTargetPool.get(rendertarget);
        RenderTargetCmdBuffers buffers = rtCmdPool.get(target.cmdBuffers);
        RenderPass pass = renderpassPool.get(target.renderpass);
        Image image = imagePool.get(target.attachments[0]);

        m_vk->waitForFrame(buffers);

        m_vk->resetCmdBuffer(buffers.drawBuffer.buffer);
        m_vk->resetCmdBuffer(buffers.transferBuffer.buffer);
        m_descriptorManager->resetPools(m_backBufferManager->getCurrentIndex());

        beginBuffer({.buffer = buffers.drawBuffer});

        //std::cout << "began buffer" << std::endl;
        beginRenderpass({
            .commandBuffer = buffers.drawBuffer,
            .pass = pass,
            .target = target,

            .clearColor = glm::vec4(0, 1, 0, 1),
            .scissorDims = image.dimensions,
            .scissorOffset = glm::vec2(0, 0),
        });

        //binds dummy pipeline
        bindPipelineCommand({.commandBuffer = buffers.drawBuffer, .shader = m_dummyPipeline});
        auto& dummyPipeline = shaderPool.get(m_dummyPipeline);
        //bind camera uniforms.

        bindDescriptorSetCommand({
                                   .drawBuffer = buffers.drawBuffer,
                                   .set_index = 0,
                                   .set_layout = setLayoutPool.get(base_setLayout),
                                   .shader_layout = dummyPipeline.layout,
                                   .bindings = {{
                                            .binding = 0,
                                            .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
                                            .buffer = m_frameUniform
                                  }}});

        //Bind Pipeline <-- relevant when shader is reused.
        //std::cout << "began RenderPass" << std::endl;
        Handle<Shader> boundPipeline;
        for (const auto &node : nodes)
        {
            if (node->shader.isNull())
                {
                    std::cout << "skip drawing node" << std::endl;
                    continue;
                }
            if(boundPipeline != node->shader){
                bindPipelineCommand({
                    .commandBuffer = buffers.drawBuffer,
                    .shader = node->shader
                });
                boundPipeline = node->shader;
            }

            glm::mat4 model_mat = scene.getGlobalMatrix();

            pushPushConstants({
                .drawBuffer = buffers.drawBuffer,
                .layout = shaderPool.get(node->shader).layout.pipeline,
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

        m_vk->submitDrawCmdBuffer({.commandBuffer = buffers.drawBuffer.buffer,
                            .fence = buffers.inFlightFen});

        //std::cout << "Submit Draw Command \n";
    }

    void Renderer::render(SceneNode &scene, Camera &camera)
    {
        updateCameraUniforms(camera);
        auto backbuffer = m_backBufferManager->getNext();
        updateFrameUniforms(m_backBufferManager->getCurrentIndex());
        renderSceneNode(scene, camera, backbuffer);
        presentRenderTarget(backbuffer);
    }

    void Renderer::renderSceneNode(SceneNode &scene, Camera &camera, Handle<RenderTarget> &rendertargetHandle)
    {
        renderSceneNode(scene, rendertargetHandle);
    }

#pragma endregion Rendering

#pragma region CleanUp/Destructor
    void Renderer::cleanup()
    {
        m_vk->waitIdle();

        auto& layout = setLayoutPool.get(base_setLayout);
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


    bool Renderer::isWindowClosed()
    {
        return m_window->isWindowClosed();
    }
#pragma endregion CleanUp / Destructor

#pragma region Command Buffers

    CommandBuffer Renderer::allocateCommandBuffer(const CommandBufferDesc &desc)
    {
        CommandBuffer buffer{
            .buffer = m_vk->allocateCommandBuffer(desc),
            .type = desc.type};

        return buffer;
    }

    void Renderer::drawCommand(const DrawCommand &command)
    {
        m_vk->recordDrawCommand({
            .drawBuffer = command.drawBuffer.buffer,
            .pass = command.renderPass.renderPass,
            .frameBuffer = command.renderTarget.buffer,
            //.pipeline = command.shader.pipeline,
            //.layout = command.shader.layout.layout,
            .vertexBuffer = command.vertexBuffer,
            .vertexBufferOffset = command.vertexBufferOffset,
            .indexBuffer = command.indexBuffer,
            .indexBufferOffset = command.indexBufferOffset,
            .indexCount = command.indexCount,
            .areaDims = {static_cast<int>(command.dimensions.x),
                         static_cast<int>(command.dimensions.y)},
            .areaOffset = {0, 0},
            .vertexCount = command.vertexInfo.x,
            .instaceCount = 1,
            .firstVertex = command.vertexInfo.y,
            .firstInstance = 0,
            //.pushConstants = command.pushConstants,
        });
    }

    void Renderer::clearCommandBuffer(const CommandBuffer &buffer)
    {
        m_vk->resetCmdBuffer(buffer.buffer);
    }


    void Renderer::transferImage(const TransferImageCommand &command)
    {
        RenderTarget dstBuffer;
        if (!renderTargetPool.tryGet(command.dst, dstBuffer))
            throw std::runtime_error("failed to transfer buffers");

        RenderTarget srcBuffer;
        if (!renderTargetPool.tryGet(command.src, srcBuffer))
            throw std::runtime_error("failed to transfer buffers");

        Image dstImage;
        if (!imagePool.tryGet(dstBuffer.attachments[0], dstImage))
            throw std::runtime_error("failed to transfer buffers");

        Image srcImage;
        if (!imagePool.tryGet(srcBuffer.attachments[0], srcImage))
            throw std::runtime_error("failed to transfer buffers");


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

        // m_vk->CmdCopyBuffer({
        //     .commandBuffer = command.buffer.buffer,
        //     .srcBuffer = srcBuffer->getBuffer(),
        //     .srcOffset = srcReservation.offset,
        //     .dstBuffer = dstBuffer->getBuffer(),
        //     .dstOffset = dstReservation.offset,
        //     .size = srcReservation.size,
        // });

        // m_vk->submitCmdBuffer({
        //     .commandBuffer = command.buffer.buffer,
        //     .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
        //     .fence = m_transferFence,
        // });
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

    void Renderer::bindDummyPipeline(const BindPipelineCommand &command)
    {
        m_vk->bindPipelineCommand({
            .drawBuffer =  command.commandBuffer.buffer,
            .pipeline = shaderPool.get(m_dummyPipeline).pipeline
        });
    }

    void Renderer::bindPipelineCommand(const BindPipelineCommand &command)
    {
        m_vk->bindPipelineCommand({
            .drawBuffer = command.commandBuffer.buffer,
            .pipeline = shaderPool.get(command.shader).pipeline,
        });
    }

    void Renderer::bindDescriptorSetCommand(const BindSetCommand &command)
    {
        //allocate
        auto set = m_descriptorManager->getSet(command.set_layout, m_backBufferManager->getCurrentIndex());
        
        auto binds = std::span<const BindBindingDesc>(command.bindings);
        // //write set
        m_descriptorManager->writeSet(binds, set, m_backBufferManager->getCurrentIndex());
        // //bind
        m_descriptorManager->bindSet(command.drawBuffer,
                                     command.shader_layout, 
                                     set, command.set_index,  
                                     m_backBufferManager->getCurrentIndex());
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

    Handle<Shader> Renderer::createShader(const ShaderDesc &data)
    {
        Shader shader{
            .name = data.name,
            .vert = {.shaderModule = m_vk->createShaderModule(data.vert.byteCode),
                     .entryFunction = data.vert.entryFunction},
            .frag = {.shaderModule = m_vk->createShaderModule(data.frag.byteCode),
                     .entryFunction = data.vert.entryFunction}};

        ShaderLayout layout;
        if (!pipelineLayoutPool.tryGet(data.layout, layout))
            throw std::runtime_error("failed to get pipeline layout");
        shader.layout = layout;
        // Get rendertarget and renderpass
        // from backbuffer
        // or from description
        RenderTarget buffer;
        RenderPass pass;
        if (data.framebuffer.isNull())
        {
            if (!renderpassPool.tryGet(m_backBufferManager->getRenderPass(), pass))
                throw std::runtime_error("failed to back buffer render pass");
        }
        else
        {
            if (!renderTargetPool.tryGet(data.framebuffer, buffer))
                throw std::runtime_error("failed to get framebuffer");
            if (!renderpassPool.tryGet(buffer.renderpass, pass))
                throw std::runtime_error("failed to get renderpass");
        }

        // TODO Convert bindings in vulkan class?
        std::vector<VkVertexInputAttributeDescription> vkattributes;
        std::vector<VkVertexInputBindingDescription> vkbindings;

        for (int i = 0; i < data.bindings.size(); i++)
        {
            auto binding = data.bindings[i];
            VkVertexInputBindingDescription bindingDesc{};
            bindingDesc.stride = binding.stride;
            bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDesc.binding = i;
            std::cout << data.bindings[i].stride;
            vkbindings.push_back(bindingDesc);
            uint32_t runningOffset = 0;

            for (int j = 0; j < binding.attributes.size(); j++)
            {
                auto attribute = binding.attributes[j];
                VkVertexInputAttributeDescription attributeDesc;
                attributeDesc.binding = i;
                attributeDesc.format = castEnum<VkFormat>(attribute.format);
                attributeDesc.offset = runningOffset;
                runningOffset = runningOffset + formatSize(attribute.format);
                attributeDesc.location = j;
                vkattributes.push_back(attributeDesc);
            }
        }

        m_vk->buildShader(
            {
                .name = shader.name,
                .vert = shader.vert,
                .frag = shader.frag,
                .renderpass = pass.renderPass,
                .layout = layout.pipeline,
                .bindings = vkbindings,
                .attributes = vkattributes,
            },
            shader);

        return shaderPool.set(shader);
    }

    Handle<RenderTarget> Renderer::createRenderTarget(const RenderTargetDesc &data)
    {
        std::vector<Handle<Image>> images(data.attachmentImages);
        if (images.empty())
        {
            // Create images.
            for (const auto &imageDesc : data.imageDesc)
            {
                Handle<Image> newImage = createImage(imageDesc);
                images.push_back(newImage);
            }
        }

        Handle<RenderPass> passhandle = data.renderpass;
        RenderPass pass;
        if (passhandle.isNull())
        {
            passhandle = createRenderPass(data.renderpassDesc);
            if (!renderpassPool.tryGet(passhandle, pass))
            {
                throw std::runtime_error("Failed to create renderpass.");
            }
        }
        else
        {
            if (!renderpassPool.tryGet(passhandle, pass))
            {
                throw std::runtime_error("Failed to create renderpass.");
            }
        }

        std::vector<VkImageView> imageViews;

        for (auto &imagehandle : images)
        {
            Image image;
            if (imagePool.tryGet(imagehandle, image))
                imageViews.push_back(image.view);
        }

        FramebufferDescVk vkDesc{
            .views = imageViews,
            .pass = pass.renderPass,
            .dimensions = data.dimensions,
        };

        RenderTarget framebuffer{
            .buffer = m_vk->createFramebuffer(vkDesc),
            .attachments = images,
            .renderpass = passhandle,
            .cmdBuffers = createRenderTargetCmdData()};

        return renderTargetPool.set(framebuffer);
    }

    Handle<RenderPass> Renderer::createRenderPass(const RenderPassDesc &data)
    {
        RenderPass pass{
            .renderPass = m_vk->createRenderPass(data)};

        return renderpassPool.set(pass);
    }

    Handle<Image> Renderer::addImage(Image image)
    {
        return imagePool.set(image);
    }

    Handle<Image> Renderer::createImage(const ImageDesc &desc)
    {
        Image image = m_vk->createImage(desc);
        image.view = m_vk->createImageView(image.image, desc);

        // Add to Image Pool.
        return imagePool.set(image);
    }

    Handle<DescriptorSetLayout> Renderer::createDescriptorLayout(const DescriptorSetLayoutDesc &desc)
    {
        DescriptorSetLayout layout;
        //std::vector<DescriptorSetRatio> ratios;
        std::vector<BindingDesc> bindingDesc;
        layout.layout = m_vk->createDescriptorLayout(desc);

        for(auto& bindDesc : desc.bindingDescriptors){
            for (size_t i = 0; i <= layout.ratios.size(); i++)
            {
                if(i == layout.ratios.size()){
                    DescriptorSetRatio ratio;
                    ratio.type = bindDesc.type;
                    ratio.quantity = bindDesc.descriptorCount;
                    layout.ratios.push_back(ratio);
                    break;
                }
                if(layout.ratios[i].type == bindDesc.type)
                {
                    layout.ratios[i].quantity += bindDesc.descriptorCount;
                    break;
                }
            }            
        }
        return setLayoutPool.set(layout);
    }

    Handle<ShaderLayout> Renderer::createShaderLayout(const ShaderLayoutDesc &desc)
    {
        auto& baseLayout = setLayoutPool.get(base_setLayout);
        VkDescriptorSetLayout materialLayout = m_vk->createDescriptorLayout(desc.materialLayout);
        ShaderLayout layout{.pipeline = m_vk->createShaderLayout(
                                {
                                    .materialLayout = materialLayout,
                                    .baseLayout = baseLayout.layout,
                                    .pushConstants ={
                                        PushConstantDesc{
                                        .offset = 0, //<-- must be larger or equal than sizeof(glm::mat4)
                                        .size = sizeof(glm::mat4), //<- M matrices
                                        .stages = STAGE_FLAG::ALL_GRAPHICS}},
                                })};

        return pipelineLayoutPool.set(layout);
    }

    Handle<RenderPass> Renderer::getBackBufferRenderPass()
    {
        return m_backBufferManager->getRenderPass();
    }

    Handle<RenderTargetCmdBuffers> Renderer::createRenderTargetCmdData()
    {
        RenderTargetCmdBuffers sync{
            .drawBuffer = allocateCommandBuffer({.count = 1,
                                                 .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                 .type = COMMAND_BUFFER_TYPE::GRAPHICS}),
            .transferBuffer = allocateCommandBuffer({.count = 1,
                                                     .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                     .type = COMMAND_BUFFER_TYPE::TRANSFER}),
            .schainAcqSem = m_vk->createSemaphore(),
            .transferSem = m_vk->createSemaphore(),
            .inFlightFen = m_vk->createFence(true),
        };

        return rtCmdPool.set(sync);
    }

#pragma endregion Create Vulkan Objects


#pragma region Destroy Vulkan Objects

    void Renderer::destroyShader(Handle<Shader> handle)
    {
        Shader shader;
        if (shaderPool.clear(handle, shader))
        {
            m_vk->destroyShader(shader);
        }
    }

    void Renderer::destroyRenderTarget(Handle<RenderTarget> bufferhandle)
    {
        RenderTarget framebuffer;
        if (renderTargetPool.clear(bufferhandle, framebuffer))
        {
            for (auto &imagehandle : framebuffer.attachments)
            {
                Image image;
                if (imagePool.clear(imagehandle, image))
                {
                    if (!image.swapchain)
                    {
                        m_vk->destroyImage(image);
                    }
                }
            }

            destroyRenderPass(framebuffer.renderpass);

            RenderTargetCmdBuffers data;
            if (rtCmdPool.clear(framebuffer.cmdBuffers, data))
                m_vk->destroyRenderTargetCmdData(data);

            m_vk->destroyFramebuffer(framebuffer);
        }
        else
        {
            std::cout << "failed delete" << std::endl;
        }
    }

    void Renderer::destroyRenderPass(Handle<RenderPass> passhandle)
    {
        RenderPass pass;
        if (renderpassPool.clear(passhandle, pass))
        {
            m_vk->destroyRenderpass(pass);
        }
    }

    void Renderer::destroyLayout(Handle<ShaderLayout> layouthandle)
    {
        ShaderLayout layout;
        if (pipelineLayoutPool.clear(layouthandle, layout))
        {
            m_vk->destroyPipelineLayout(layout);
        }
    }

#pragma endregion Destroy Vulkan Objects

}
