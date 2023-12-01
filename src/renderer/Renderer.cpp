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

        RTCmdBuffers buffers;
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
        Handle<BufferReservation> vertexBufferHandle = geom.reservations[0];
        bufferReservPool.get(vertexBufferHandle, vertexBufferReservation);


        vk->waitForFrame(buffers);

        vk->resetCommandBuffer(buffers.drawBuffer.buffer);
        vk->resetCommandBuffer(buffers.transferBuffer.buffer);

        recordCommand({
            .drawBuffer = buffers.drawBuffer,
            .renderTarget = target,
            .renderPass = pass,
            .shader = shader,
            .dimensions = {
                static_cast<int>(image.dimensions.x),
                static_cast<int>(image.dimensions.y),
            },
            .vertexBuffer = vertexBufferReservation.buffer->getBuffer(),
            .vertexBufferOffset = vertexBufferReservation.offset,
            .vertexInfo = geom.vertexInfo,
            .instanceInfo = {1, 0}, // scene.instanceInfo
            
        });

        vk->submitDrawCmdBuffer({.bufferData = buffers,
                                 .submitType = COMMAND_BUFFER_TYPE::GRAPHICS});
    }

    void Renderer::render(SceneNode &scene)
    {
        auto backbuffer = backBufferManager->getNext();
        renderSceneNode(scene, backbuffer);
        presentRenderTarget(backbuffer);
    }

    void Renderer::presentRenderTarget(Handle<RenderTarget> &rendertarget)
    {
        window->windowEvents();

        RenderTarget fb;
        if (!renderTargetPool.get(rendertarget, fb))
            throw std::runtime_error("failed to framebuffer for Presentation");

        RTCmdBuffers buffers;
        if (!rtCmdPool.get(fb.cmdBuffers, buffers))
            throw std::runtime_error("failed to framebuffer for Presentation");

        Image image;
        if (!imagePool.get(fb.attachments[0], image))
            throw std::runtime_error("failed to framebuffer for Presentation");

        vk->waitForFrame(buffers);
        SubmitCommand command{.bufferData = buffers, .submitType = COMMAND_BUFFER_TYPE::PRESENT};
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
                                                    command);

        // If present was unsucessful we must remake the swapchain
        // and recreate our backbuffer.
        if (!successfullyPresent)
        {
            handleWindowResize();
        }
    }

    void Renderer::renderSceneNode(SceneNode &scene, Handle<RenderTarget> &rendertarget)
    {
        std::vector<SceneNode> nodes;
        scene.sceneAsList(nodes);

        // TODO cullings and whatever
        // TRANSFORM UPDATES
        // ETC

        for (const auto &node : nodes)
        {
            // std::cout << node.name << std::endl;
            if (node.shader.isNull())
                continue;
            renderToRenderTarget(node, rendertarget);
        }
    }

#pragma endregion Rendering

#pragma region CleanUp/Destructor
    void Renderer::cleanup()
    {
        for (auto &buffer : buffers)
            delete buffer;

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

    void Renderer::recordCommand(const DrawCommand &command)
    {

        vk->recordCommand({
            .drawBuffer = command.drawBuffer.buffer,
            .pass = command.renderPass.renderPass,
            .frameBuffer = command.renderTarget.buffer,
            .pipeline = command.shader.pipeline,
            .vertexBuffer = command.vertexBuffer,
            .vertexBufferOffset = command.vertexBufferOffset,
            .areaDims = {static_cast<int>(command.dimensions.x),
                         static_cast<int>(command.dimensions.y)},
            .areaOffset = {0, 0},
            .vertexCount = 3,
            .instaceCount = 1,
            .firstVertex = 0,
            .firstInstance = 0,
        });
    }

    void Renderer::clearCommandBuffer(const CommandBuffer &buffer)
    {
        vk->resetCommandBuffer(buffer.buffer);
    }

    void Renderer::transferImage(const TransferCommand &command)
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

        vk->CmdCopyImage({.buffer = command.buffer.buffer,
                          .srcImage = srcImage.image,
                          .srcImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          .dstImage = dstImage.image,
                          .dstImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          .extent = srcImage.dimensions});
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

            vkbindings.push_back(bindingDesc);
            uint32_t runningOffset = 0;
            for (int j = 0; j < binding.attributes.size(); j++)
            {
                auto attribute = binding.attributes[j];
                VkVertexInputAttributeDescription attributeDesc;
                attributeDesc.binding = i;
                attributeDesc.format = castEnum<FORMAT, VkFormat>(attribute.format);
                runningOffset = runningOffset + formatSize(attribute.format);
                attributeDesc.offset = runningOffset;
                attributeDesc.location = j;
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
        // TODO seperate responsabilities
        // Create Image
        // Create Buffer Memory
        // Bind Buffer Memory
        Image image = vk->createImage(desc);

        // TODO separate view from image?
        // Create Image View
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

        Handle<BufferReservation> reservation = reserveBuffer({.request = desc.dataSize,
                                                               .usage = BUFFER_USAGE::VERTEX,
                                                               .sharing = SHARING_MODE::EXCLUSIVE});
        
        std::vector<Handle<BufferReservation>> reservations{reservation};
        
        Geometry geo = {
            .reservationCount = reservations.size(),
            .reservations = reservations.data(),
            .vertexInfo = desc.vertexInfo,
        };

        return geometryPool.set(geo);
    }

    Handle<RenderPass> Renderer::getBackBufferRenderPass()
    {
        return backBufferManager->getRenderPass();
    }

    Handle<RTCmdBuffers> Renderer::createRenderTargetCmdData()
    {
        RTCmdBuffers sync{
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
            // if buffer NOT found
            // compute new buffer size
            uint32_t newBufferSize = estimateNewBufferSize(compatibility);
            //      create new buffer
            Buffer *buffer = createBuffer({
                .size = newBufferSize,
                .partitions = 1 << 10,
                .usage = compatibility.usage,
                .sharing = compatibility.sharing,
            });
            //      return reference
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

    uint32_t Renderer::estimateNewBufferSize(const BufferCompatibility &compatibility)
    {

        if (compatibility.usage == BUFFER_USAGE::VERTEX)
        {
            return compatibility.requestSize * (1 << 5);
        }

        return 0;
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

            RTCmdBuffers data;
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
