#include <iostream>
#include <stdexcept>

#include "Renderer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>
#include "../types/Swapchain.hpp"

namespace boitatah
{
#pragma region Initialization

    Renderer::Renderer(RendererOptions opts)
    {
        options = opts;
        WindowDesc desc{.dimensions = options.windowDimensions,
                        .windowName = options.appName};

        window = new WindowManager(desc);
        createVulkan();

        window->initSurface(vk->getInstance());
        vk->attachWindow(window);
        vk->completeInit();

        createSwapchain();

        backBufferManager = new BackBufferManager(this);
        backBufferManager->setup(options.backBufferDesc);

        transferCommandBuffer = allocateCommandBuffer({.count = 1,
                                                .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                .type = COMMAND_BUFFER_TYPE::TRANSFER});
        transferFence = vk->createFence(true);
    }

    void Renderer::handleWindowResize()
    {
        vk->waitIdle();

        swapchain->createSwapchain();

        auto newWindowSize = window->getWindowDimensions();

        options.backBufferDesc.dimensions = {
            static_cast<uint32_t>(newWindowSize.x),
            static_cast<uint32_t>(newWindowSize.y),
        };

        backBufferManager->setup(options.backBufferDesc);
    }

    void Renderer::createSwapchain()
    {
        swapchain = new Swapchain({.format = options.swapchainFormat,
                                   .useValidationLayers = options.debug});
        swapchain->attach(vk, this, window);
        swapchain->createSwapchain(); // options.windowDimensions, false, false);
    }

    void Renderer::createVulkan()
    {
        uint32_t extensionCount = 0;

        vk = new Vulkan({
            .appName = (char *)options.appName,
            .extensions = window->requiredWindowExtensions(),
            .useValidationLayers = options.debug,
            .debugMessages = options.debug,
            .window = window->window,
        });
    }
#pragma endregion Initialization

#pragma region Rendering
    void Renderer::waitIdle()
    {
        vk->waitIdle();
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

        vk->waitForFrame(buffers);

        vk->resetCommandBuffer(buffers.drawBuffer.buffer);
        vk->resetCommandBuffer(buffers.transferBuffer.buffer);

        recordDrawCommand({
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
        vk->submitDrawCmdBuffer({.commandBuffer = buffers.drawBuffer.buffer,
        .fence = buffers.inFlightFen});
    }

    void Renderer::render(SceneNode &scene)
    {
        auto backbuffer = backBufferManager->getNext();
        renderSceneNode(scene, backbuffer);
        presentRenderTarget(backbuffer);
    }

    void Renderer::render(SceneNode &scene, Camera &camera)
    {
        auto backbuffer = backBufferManager->getNext();

        renderSceneNode()

        presentRenderTarget(backbuffer);

    }

    void Renderer::presentRenderTarget(Handle<RenderTarget> &rendertarget)
    {
        window->windowEvents();

        RenderTarget fb;
        if (!renderTargetPool.get(rendertarget, fb))
            throw std::runtime_error("failed to framebuffer for Presentation");

        RenderTargetCmdBuffers buffers;
        if (!rtCmdPool.get(fb.cmdBuffers, buffers))
            throw std::runtime_error("failed to framebuffer for Presentation");

        Image image;
        if (!imagePool.get(fb.attachments[0], image))
            throw std::runtime_error("failed to framebuffer for Presentation");

        vk->waitForFrame(buffers);
        //SubmitDrawCommand command{.bufferData = buffers, .submitType = COMMAND_BUFFER_TYPE::PRESENT};
        auto swapchainImage = swapchain->getNext(buffers.schainAcqSem);

        // failed to find swapchain image.
        if (swapchainImage.index == UINT32_MAX) // Fail case.
        {
            handleWindowResize();
            return;
        }

        bool successfullyPresent = vk->presentFrame(image,
                                                    swapchainImage.image,
                                                    swapchainImage.sc,
                                                    swapchainImage.index,
                                                    {.commandBuffer = buffers.transferBuffer.buffer,
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
        std::vector<SceneNode*> nodes;
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

    void Renderer::renderSceneNode(SceneNode &scene, Camera &camera, Handle<RenderTarget> &rendertarget)
    {
        std::vector<SceneNode*> nodes;
        scene.sceneAsList(nodes);

        // TODO cullings and whatever
        // TRANSFORM UPDATES
        // ETC
    }

#pragma endregion Rendering

#pragma region CleanUp/Destructor
    void Renderer::cleanup()
    {
        for (auto &buffer : buffers)
            delete buffer;

        vk->destroyFence(transferFence);

        delete swapchain;
        window->destroySurface(vk->getInstance());
        delete backBufferManager;
        delete vk;
        delete window;
    }

    Renderer::~Renderer(void)
    {
        cleanup();
    }
    bool Renderer::isWindowClosed()
    {
        return window->isWindowClosed();
    }
#pragma endregion CleanUp / Destructor

#pragma region Command Buffers

    CommandBuffer Renderer::allocateCommandBuffer(const CommandBufferDesc &desc)
    {
        CommandBuffer buffer{
            .buffer = vk->allocateCommandBuffer(desc),
            .type = desc.type};

        return buffer;
    }

    void Renderer::recordDrawCommand(const DrawCommand &command)
    {
        vk->recordDrawCommand({
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
        vk->resetCommandBuffer(buffer.buffer);
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

        //begin buffer

        vk->CmdCopyImage({.buffer = command.buffer.buffer,
                          .srcImage = srcImage.image,
                          .srcImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          .dstImage = dstImage.image,
                          .dstImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          .extent = srcImage.dimensions});

        //submit buffer
    }

    void Renderer::copyBuffer(const CopyBufferCommand &command)
    {
        BufferReservation srcReservation;
        BufferReservation dstReservation;
        if(!bufferReservPool.get(command.src, srcReservation))
            throw std::runtime_error("failed to copy buffer");
        if(!bufferReservPool.get(command.dst, dstReservation))
            throw std::runtime_error("failed to copy buffer");

        //vk->waitIdle();
        vk->waitForFence(transferFence);
        vk->beginCmdBuffer(command.buffer.buffer);

        vk->CmdCopyBuffer({
            .commandBuffer = command.buffer.buffer,
            .srcBuffer = srcReservation.buffer->getBuffer(),
            .srcOffset = srcReservation.offset,
            .dstBuffer = dstReservation.buffer->getBuffer(),
            .dstOffset = dstReservation.offset,
            .size = srcReservation.size,
        });
        
        vk->submitCmdBuffer({
            .commandBuffer = command.buffer.buffer,
            .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
            .fence = transferFence,
        });

    }

    void Renderer::beginBuffer(const BeginBufferCmmand &command)
    {
    }

    void Renderer::submitBuffer(const SubmitBufferCommand &command)
    {
    }

#pragma endregion Command Buffers

#pragma region Create Vulkan Objects

    Handle<Shader> Renderer::createShader(const ShaderDesc &data)
    {
        Shader shader{
            .name = data.name,
            .vert = {.shaderModule = vk->createShaderModule(data.vert.byteCode),
                     .entryFunction = data.vert.entryFunction},
            .frag = {.shaderModule = vk->createShaderModule(data.frag.byteCode),
                     .entryFunction = data.vert.entryFunction}};

        PipelineLayout layout;
        if (!pipelineLayoutPool.get(data.layout, layout))
            throw std::runtime_error("failed to get pipeline layout");

        // Get rendertarget and renderpass
        // from backbuffer
        // or from description
        RenderTarget buffer;
        RenderPass pass;
        if (data.framebuffer.isNull())
        {
            if (!renderpassPool.get(backBufferManager->getRenderPass(), pass))
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
                attributeDesc.format = castEnum<FORMAT, VkFormat>(attribute.format);
                attributeDesc.offset = runningOffset;
                runningOffset = runningOffset + formatSize(attribute.format);
                attributeDesc.location = j;
                vkattributes.push_back(attributeDesc);
            }
        }

        vk->buildShader(
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
            .buffer = vk->createFramebuffer(vkDesc),
            .attachments = images,
            .renderpass = passhandle,
            .cmdBuffers = createRenderTargetCmdData()};

        return renderTargetPool.set(framebuffer);
    }

    Handle<RenderPass> Renderer::createRenderPass(const RenderPassDesc &data)
    {
        RenderPass pass{
            .renderPass = vk->createRenderPass(data)};

        return renderpassPool.set(pass);
    }

    Handle<Image> Renderer::addImage(Image image)
    {
        return imagePool.set(image);
    }

    Handle<Image> Renderer::createImage(const ImageDesc &desc)
    {
        Image image = vk->createImage(desc);
        image.view = vk->createImageView(image.image, desc);

        // Add to Image Pool.
        return imagePool.set(image);
    }

    Handle<PipelineLayout> Renderer::createPipelineLayout(const PipelineLayoutDesc &desc)
    {
        PipelineLayout layout{.layout = vk->createPipelineLayout(desc)};
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
        if (desc.indexCount != 0){
            geo.indexBuffer = uploadBuffer({
                .dataSize = static_cast<uint32_t>(desc.indexCount * sizeof(uint32_t)),
                .data = desc.indexData,
                .usage = BUFFER_USAGE::TRANSFER_DST_INDEX
            });
        }
        std::cout << "copied index buffer " << std::endl;
        geo.vertexInfo = desc.vertexInfo;
        geo.vertexSize = desc.vertexSize;
        geo.indiceCount = desc.indexCount;

        return geometryPool.set(geo);
    }

    Handle<RenderPass> Renderer::getBackBufferRenderPass()
    {
        return backBufferManager->getRenderPass();
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
            .schainAcqSem = vk->createSemaphore(),
            .transferSem = vk->createSemaphore(),
            .inFlightFen = vk->createFence(true),
        };

        return rtCmdPool.set(sync);
    }

#pragma endregion Create Vulkan Objects

#pragma region Buffers

    Buffer *Renderer::createBuffer(const BufferDesc &desc)
    {
        buffers.push_back(new Buffer(desc, vk));

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

    Handle<BufferReservation> Renderer::uploadBuffer(const BufferUploadDesc &desc)
    {
        Handle<BufferReservation> stagingHandle = reserveBuffer({.request = desc.dataSize,
                                                                    .usage = BUFFER_USAGE::TRANSFER_SRC,
                                                                    .sharing = SHARING_MODE::CONCURRENT});

        Handle<BufferReservation> resHandle = reserveBuffer({.request = desc.dataSize,
                                                                .usage = desc.usage,
                                                                .sharing = SHARING_MODE::CONCURRENT});
        

        if (resHandle.isNull() || stagingHandle.isNull())
            return Handle<BufferReservation>();

        BufferReservation stagingReservation;
        bufferReservPool.get(stagingHandle, stagingReservation);

        vk->copyDataToBuffer({
            .memory = stagingReservation.buffer->getMemory(),
            .offset = stagingReservation.offset,
            .size = desc.dataSize,
            .data = desc.data,
        });

        copyBuffer({
            .src = stagingHandle,
            .dst = resHandle,
            .buffer = transferCommandBuffer
        });

        unreserveBuffer(stagingHandle);


        return resHandle;
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
            vk->destroyShader(shader);
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
                        vk->destroyImage(image);
                    }
                }
            }

            destroyRenderPass(framebuffer.renderpass);

            RenderTargetCmdBuffers data;
            if (rtCmdPool.clear(framebuffer.cmdBuffers, data))
                vk->destroyRenderTargetCmdData(data);

            vk->destroyFramebuffer(framebuffer);
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
            vk->destroyRenderpass(pass);
        }
    }

    void Renderer::destroyLayout(Handle<PipelineLayout> layouthandle)
    {
        PipelineLayout layout;
        if (pipelineLayoutPool.clear(layouthandle, layout))
        {
            vk->destroyPipelineLayout(layout);
        }
    }

#pragma endregion Destroy Vulkan Objects

}
