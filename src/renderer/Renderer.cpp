#include "Renderer.hpp"

#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>



namespace boitatah
{
#pragma region Initialization

    Renderer::Renderer(RendererOptions opts)
    {
        m_options = opts;
        WindowDesc desc{.dimensions = m_options.windowDimensions,
                        .windowName = m_options.appName};

        m_window = new WindowManager(desc);
        createVulkan();

        m_window->initSurface(m_vk->getInstance());
        m_vk->attachWindow(m_window);
        m_vk->completeInit();

        createSwapchain();

        m_backBufferManager = new BackBufferManager(this);
        m_backBufferManager->setup(m_options.backBufferDesc);

        m_transferCommandBuffer = allocateCommandBuffer({.count = 1,
                                                         .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                         .type = COMMAND_BUFFER_TYPE::TRANSFER});
        m_transferFence = m_vk->createFence(true);

        m_cameraUniforms = reserveBuffer({
            .request = sizeof(FrameUniforms),
            .usage = BUFFER_USAGE::UNIFORM_BUFFER,
            .sharing = SHARING_MODE::CONCURRENT,
        });

        // camera uniforms
        //  FrameUniforms frameUniforms{}; //auto ptr
        //  m_cameraUniforms = createUniform<FrameUniforms>( frameUniforms );
        //  updateUniform(m_cameraUniforms, new frame uniforms

        m_baseLayout.layout = m_vk->createDescriptorLayout({.m_set = 0,
                                                            .bindingDescriptors = {
                                                                {.binding = 0,
                                                                 .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
                                                                 .stages = STAGE_FLAG::VERTEX}}});
    }

    void Renderer::handleWindowResize()
    {
        m_vk->waitIdle();

        swapchain->createSwapchain();

        auto newWindowSize = m_window->getWindowDimensions();

        m_options.backBufferDesc.dimensions = {
            static_cast<uint32_t>(newWindowSize.x),
            static_cast<uint32_t>(newWindowSize.y),
        };

        m_backBufferManager->setup(m_options.backBufferDesc);
    }

    void Renderer::createSwapchain()
    {
        swapchain = new Swapchain({.format = m_options.swapchainFormat,
                                   .useValidationLayers = m_options.debug});
        swapchain->attach(m_vk, this, m_window);
        swapchain->createSwapchain(); // options.windowDimensions, false, false);
    }

    void Renderer::createVulkan()
    {
        uint32_t extensionCount = 0;

        m_vk = new Vulkan({
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

    void Renderer::renderToRenderTarget(const SceneNode &scene, const Handle<RenderTarget> &rendertarget)
    {
        RenderTarget target;
        if (!renderTargetPool.get(rendertarget, target))
        {
            throw std::runtime_error("Failed to write command buffer \n\tRender Target");
        }
        RenderPass pass;
        if (!renderpassPool.get(target.renderpass, pass))
        {
            throw std::runtime_error("Failed to write command buffer \n\tRender Pass");
        }

        Image image;
        if (!imagePool.get(target.attachments[0], image))
        {
            throw std::runtime_error("Failed to write command buffer \n\tImage");
        }

        RenderTargetCmdBuffers buffers;
        if (!rtCmdPool.get(target.cmdBuffers, buffers))
            throw std::runtime_error("Failed to Render to Target");

        // for scene in scene
        Shader shader;
        if (!shaderPool.get(scene.shader, shader))
        {
            throw std::runtime_error("Failed to retrieve material");
        }

        // vertex and mesh data
        Geometry geom;
        if (!geometryPool.get(scene.geometry, geom))
        {
            throw std::runtime_error("Failed to retrieve geometry");
        }

        BufferReservation vertexBufferReservation;
        Handle<BufferReservation> vertexBufferHandle = geom.buffers.size() > 0 ? geom.buffers[0] : Handle<BufferReservation>();
        bufferReservPool.get(vertexBufferHandle, vertexBufferReservation);

        BufferReservation indexBufferReservation;
        bufferReservPool.get(geom.indexBuffer, indexBufferReservation);

        m_vk->waitForFrame(buffers);

        m_vk->resetCommandBuffer(buffers.drawBuffer.buffer);
        m_vk->resetCommandBuffer(buffers.transferBuffer.buffer);

        drawCommand({
            .drawBuffer = buffers.drawBuffer,
            .renderTarget = target,
            .renderPass = pass,
            .shader = shader,
            .dimensions = {
                static_cast<int>(image.dimensions.x),
                static_cast<int>(image.dimensions.y),
            },

            .vertexBuffer = vertexBufferHandle.isNull() ? VK_NULL_HANDLE : vertexBufferReservation.buffer->getBuffer(),
            .vertexBufferOffset = vertexBufferReservation.offset,

            .indexBuffer = geom.indexBuffer.isNull() ? VK_NULL_HANDLE : indexBufferReservation.buffer->getBuffer(),
            .indexBufferOffset = indexBufferReservation.offset,
            .indexCount = geom.indiceCount,

            .vertexInfo = geom.vertexInfo,
            .instanceInfo = {1, 0}, // scene.instanceInfo
        });

        // vk->submitDrawCmdBuffer({.bufferData = buffers,
        //                          .submitType = COMMAND_BUFFER_TYPE::GRAPHICS});
        m_vk->submitDrawCmdBuffer({.commandBuffer = buffers.drawBuffer.buffer,
                                   .fence = buffers.inFlightFen});
    }

    void Renderer::render(SceneNode &scene)
    {
        auto backbuffer = m_backBufferManager->getNext();
        renderSceneNode(scene, backbuffer);
        presentRenderTarget(backbuffer);
    }

    void Renderer::presentRenderTarget(Handle<RenderTarget> &rendertarget)
    {
        m_window->windowEvents();

        RenderTarget fb;
        if (!renderTargetPool.get(rendertarget, fb))
            throw std::runtime_error("failed to framebuffer for Presentation");

        RenderTargetCmdBuffers buffers;
        if (!rtCmdPool.get(fb.cmdBuffers, buffers))
            throw std::runtime_error("failed to framebuffer for Presentation");

        Image image;
        if (!imagePool.get(fb.attachments[0], image))
            throw std::runtime_error("failed to framebuffer for Presentation");

        m_vk->waitForFrame(buffers);
        // SubmitDrawCommand command{.bufferData = buffers, .submitType = COMMAND_BUFFER_TYPE::PRESENT};
        auto swapchainImage = swapchain->getNext(buffers.schainAcqSem);

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

        for (const auto &node : nodes)
        {
            if (node->shader.isNull())
                continue;
            renderToRenderTarget(*node, rendertarget);
        }
    }

    void Renderer::render(SceneNode &scene, Camera &camera)
    {
        auto backbuffer = m_backBufferManager->getNext();

        renderSceneNode(scene, camera, backbuffer);

        presentRenderTarget(backbuffer);
    }

    void Renderer::renderSceneNode(SceneNode &scene, Camera &camera, Handle<RenderTarget> &rendertargetHandle)
    {
        std::vector<SceneNode *> nodes;
        scene.sceneAsList(nodes);

        // TODO cullings and whatever
        // TRANSFORM UPDATES
        // BUILD UPDATE TRANSFER BUFFER FOR CHANGED UNIFORMS
        // SORT BY MATERIALS
        // ETC

        // UPDATE UNIFORMS
        updateUniforms(nodes);


        RenderTarget renderTarget;
        renderTargetPool.get(rendertargetHandle, renderTarget);
        RenderTargetCmdBuffers renderTargetBuffers;
        rtCmdPool.get(renderTarget.cmdBuffers, renderTargetBuffers);
        RenderPass pass;
        renderpassPool.get(renderTarget.renderpass, pass);
        Image image;
        imagePool.get(renderTarget.attachments[0], image);


        // update camera uniforms
        FrameUniforms frameUniforms;
        copyDataToBuffer({.reservation = m_cameraUniforms,
                          .data = &frameUniforms,
                          .dataSize = sizeof(FrameUniforms)});

        beginBuffer({.buffer = renderTargetBuffers.drawBuffer});

        beginRenderpass({
            .commandBuffer = renderTargetBuffers.drawBuffer,
            .pass = pass,
            .target = renderTarget,

            .clearColor = glm::vec4(0, 0, 0, 1),
            .scissorDims = image.dimensions,
            .scissorOffset = glm::vec2(0, 0),
        });

        // bind dummy pipeline

        // bind camera uniforms
        // bindUniformsCommand({});

        std::cout << "rendernode with camera" << std::endl;

        // parallellizable?
        for (const auto &node : nodes)
        {
            // if material changed,
            //  bind pipeline
            // bindPipelineCommand({});

            // bind object matrix
            // push constant?
            // draw object
            if (node->shader.isNull())
                continue;
        }
        // End Render Pass
        // End Buffer
        // Submit buffer
    }

#pragma endregion Rendering

#pragma region CleanUp/Destructor
    void Renderer::cleanup()
    {
        for (auto &buffer : buffers)
            delete buffer;

        m_vk->destroyFence(m_transferFence);

        delete swapchain;
        m_window->destroySurface(m_vk->getInstance());
        delete m_backBufferManager;
        delete m_vk;
        delete m_window;
    }

    Renderer::~Renderer(void)
    {
        cleanup();
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
            .pipeline = command.shader.pipeline,
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
        });
    }

    void Renderer::clearCommandBuffer(const CommandBuffer &buffer)
    {
        m_vk->resetCommandBuffer(buffer.buffer);
    }

    void Renderer::queueTransferUniform(const TransferUniformCommand &command)
    {
        Uniform uniform;
        uniformPool.get(command.uniform, uniform);

        // TODO change usage to type depending on uniform type.
        queueUploadBuffer({
            .dataSize = uniform.get_size(),
            .data = uniform.get_data(),
            .usage = BUFFER_USAGE::UNIFORM_BUFFER
        });
    }

    void Renderer::transferImage(const TransferImageCommand &command)
    {
        RenderTarget dstBuffer;
        if (!renderTargetPool.get(command.dst, dstBuffer))
            throw std::runtime_error("failed to transfer buffers");

        RenderTarget srcBuffer;
        if (!renderTargetPool.get(command.src, srcBuffer))
            throw std::runtime_error("failed to transfer buffers");

        Image dstImage;
        if (!imagePool.get(dstBuffer.attachments[0], dstImage))
            throw std::runtime_error("failed to transfer buffers");

        Image srcImage;
        if (!imagePool.get(srcBuffer.attachments[0], srcImage))
            throw std::runtime_error("failed to transfer buffers");

        // begin buffer

        m_vk->CmdCopyImage({.buffer = command.buffer.buffer,
                            .srcImage = srcImage.image,
                            .srcImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            .dstImage = dstImage.image,
                            .dstImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            .extent = srcImage.dimensions});

        // submit buffer
    }

    void Renderer::copyBuffer(const CopyBufferCommand &command)
    {
        BufferReservation srcReservation;
        BufferReservation dstReservation;
        if (!bufferReservPool.get(command.src, srcReservation))
            throw std::runtime_error("failed to copy buffer");
        if (!bufferReservPool.get(command.dst, dstReservation))
            throw std::runtime_error("failed to copy buffer");

        // vk->waitIdle();
        m_vk->waitForFence(m_transferFence);
        m_vk->beginCmdBuffer(command.buffer.buffer);

        m_vk->CmdCopyBuffer({
            .commandBuffer = command.buffer.buffer,
            .srcBuffer = srcReservation.buffer->getBuffer(),
            .srcOffset = srcReservation.offset,
            .dstBuffer = dstReservation.buffer->getBuffer(),
            .dstOffset = dstReservation.offset,
            .size = srcReservation.size,
        });

        m_vk->submitCmdBuffer({
            .commandBuffer = command.buffer.buffer,
            .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
            .fence = m_transferFence,
        });
    }

    void Renderer::beginBuffer(const BeginBufferCommand &command)
    {
        m_vk->beginBufferCommand({.commandBuffer = command.buffer.buffer});
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

    void Renderer::submitBuffer(const SubmitBufferCommand &command)
    {
        m_vk->submitCmdBuffer({
            .commandBuffer = command.buffer.buffer,
            .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
            .fence = m_transferFence,
        });
    }

    void Renderer::bindDummyPipeline()
    {
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
        if (!pipelineLayoutPool.get(data.layout, layout))
            throw std::runtime_error("failed to get pipeline layout");

        // Get rendertarget and renderpass
        // from backbuffer
        // or from description
        RenderTarget buffer;
        RenderPass pass;
        if (data.framebuffer.isNull())
        {
            if (!renderpassPool.get(m_backBufferManager->getRenderPass(), pass))
                throw std::runtime_error("failed to back buffer render pass");
        }
        else
        {
            if (!renderTargetPool.get(data.framebuffer, buffer))
                throw std::runtime_error("failed to get framebuffer");
            if (!renderpassPool.get(buffer.renderpass, pass))
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
                .layout = layout.layout,
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
            if (!renderpassPool.get(passhandle, pass))
            {
                throw std::runtime_error("Failed to create renderpass.");
            }
        }
        else
        {
            if (!renderpassPool.get(passhandle, pass))
            {
                throw std::runtime_error("Failed to create renderpass.");
            }
        }

        std::vector<VkImageView> imageViews;

        for (auto &imagehandle : images)
        {
            Image image;
            if (imagePool.get(imagehandle, image))
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

    Handle<ShaderLayout> Renderer::createShaderLayout(const ShaderLayoutDesc &desc)
    {
        VkDescriptorSetLayout materialLayout = m_vk->createDescriptorLayout(desc.materialLayout);
        ShaderLayout layout{.layout = m_vk->createShaderLayout(
                                {
                                    .materialLayout = materialLayout,
                                    .baseLayout = m_baseLayout.layout,
                                })};

        return pipelineLayoutPool.set(layout);
    }

    Handle<Geometry> Renderer::createGeometry(const GeometryDesc &desc)
    {
        Geometry geo{};
        if (desc.vertexDataSize != 0)
        {
            Handle<BufferReservation> bufferHandle = uploadBuffer({
                .dataSize = desc.vertexDataSize,
                .data = desc.vertexData,
                .usage = BUFFER_USAGE::TRANSFER_DST_VERTEX,
            });

            geo.buffers.push_back(bufferHandle);
        }
        std::cout << "copied vertex buffer " << std::endl;
        if (desc.indexCount != 0)
        {
            geo.indexBuffer = uploadBuffer({.dataSize = static_cast<uint32_t>(desc.indexCount * sizeof(uint32_t)),
                                            .data = desc.indexData,
                                            .usage = BUFFER_USAGE::TRANSFER_DST_INDEX});
        }
        std::cout << "copied index buffer " << std::endl;
        geo.vertexInfo = desc.vertexInfo;
        geo.vertexSize = desc.vertexSize;
        geo.indiceCount = desc.indexCount;

        return geometryPool.set(geo);
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

#pragma region Buffers

    Handle<Uniform> Renderer::createUniform(void *data, uint32_t size, DESCRIPTOR_TYPE type)
    {

        return Handle<Uniform>{};
    }

    Buffer *Renderer::createBuffer(const BufferDesc &desc)
    {
        buffers.push_back(new Buffer(desc, m_vk));

        return buffers.back();
    }

    Handle<BufferReservation> Renderer::reserveBuffer(const BufferReservationRequest &request)
    {
        // Find Compatible Buffer
        Buffer *buffer = findOrCreateCompatibleBuffer({.requestSize = request.request,
                                                       .usage = request.usage,
                                                       .sharing = request.sharing});
        // allocate in buffer
        BufferReservation reservation = buffer->reserve(request.request);

        return bufferReservPool.set(reservation);
    }

    std::pair<Handle<BufferReservation>, Handle<BufferReservation>> Renderer::getUploadBufferHandles(const BufferUploadDesc &desc)
    {
        Handle<BufferReservation> stagingHandle = reserveBuffer({.request = desc.dataSize,
                                                                 .usage = BUFFER_USAGE::TRANSFER_SRC,
                                                                 .sharing = SHARING_MODE::CONCURRENT});

        Handle<BufferReservation> resHandle = reserveBuffer({.request = desc.dataSize,
                                                             .usage = desc.usage,
                                                             .sharing = SHARING_MODE::CONCURRENT});
        if (resHandle.isNull() || stagingHandle.isNull())
        {
            unreserveBuffer(stagingHandle);
            unreserveBuffer(resHandle);
            return std::pair(Handle<BufferReservation>(), Handle<BufferReservation>());
        }
        return std::pair(stagingHandle, resHandle);
    }

    Handle<BufferReservation> Renderer::uploadBuffer(const BufferUploadDesc &desc)
    {
        auto pair = getUploadBufferHandles(desc);

        Handle<BufferReservation> stagingHandle = pair.first;
        Handle<BufferReservation> resHandle = pair.second;

        if (resHandle.isNull())
            return Handle<BufferReservation>();

        BufferReservation stagingReservation;
        bufferReservPool.get(stagingHandle, stagingReservation);

        m_vk->copyDataToBuffer({
            .memory = stagingReservation.buffer->getMemory(),
            .offset = stagingReservation.offset,
            .size = desc.dataSize,
            .data = desc.data,
        });

        copyBuffer({.src = stagingHandle,
                    .dst = resHandle,
                    .buffer = m_transferCommandBuffer});

        unreserveBuffer(stagingHandle);

        return resHandle;
    }

    Handle<BufferReservation> Renderer::queueUploadBuffer(const BufferUploadDesc &desc)
    {
        auto pair = getUploadBufferHandles(desc);

        Handle<BufferReservation> stagingHandle = pair.first;
        Handle<BufferReservation> resHandle = pair.second;

        if (resHandle.isNull())
            return Handle<BufferReservation>();

        stagingBufferQueue.emplace_back(stagingHandle);

        
    }

    void Renderer::clearUploadBufferQueue()
    {

        // for (size_t i = 0; i < stagingBufferQueue.size(); i++)
        // {
        //     unreserveBuffer(stagingBufferQueue[i]);
        // }

        stagingBufferQueue.clear();
    }

    void Renderer::copyDataToBuffer(const CopyDataToBufferDesc &desc)
    {

        BufferReservation reservation;
        bufferReservPool.get(desc.reservation, reservation);

        m_vk->copyDataToBuffer(
            {
                .memory = reservation.buffer->getMemory(),
                .offset = reservation.offset,
                .size = desc.dataSize,
                .data = desc.data,
            });
    }

    void Renderer::unreserveBuffer(Handle<BufferReservation> &reservation)
    {
    }

    Buffer *Renderer::findOrCreateCompatibleBuffer(const BufferCompatibility &compatibility)
    {
        // Find buffer
        uint32_t bufferIndex = findCompatibleBuffer(compatibility);
        if (bufferIndex != UINT32_MAX)
        {
            // return reference
            return buffers[bufferIndex];
        }
        else
        {
            Buffer *buffer = createBuffer({
                .estimatedElementSize = compatibility.requestSize,
                .partitions = 1 << 10,
                .usage = compatibility.usage,
                .sharing = compatibility.sharing,
            });
            // return reference
            return buffer;
        }
    }

    uint32_t Renderer::findCompatibleBuffer(const BufferCompatibility &compatibility)
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            if (buffers[i]->checkCompatibility(compatibility))
                return i;
        }
        return UINT32_MAX;
    }

#pragma endregion Buffers

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
