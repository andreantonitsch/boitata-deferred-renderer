#pragma once

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

    void Renderer::windowResizeCallback(){
        
    }


    void Renderer::recreateSwapchain(){
        vk->waitIdle();
        delete swapchain;
        createSwapchain();
    }

    void Renderer::createSwapchain(){
        swapchain = new Swapchain({.format = options.swapchainFormat,
                                   .useValidationLayers = options.debug});
        swapchain->attach(vk, this, window);
        swapchain->createSwapchain(options.windowDimensions, false, false);
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

    void Renderer::renderToRenderTarget(SceneNode &scene, Handle<RenderTarget> &rendertarget)
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

        vk->waitForFrame(buffers);

        vk->resetCommandBuffer(buffers.drawBuffer.buffer);
        vk->resetCommandBuffer(buffers.transferBuffer.buffer);

        recordCommand({.drawBuffer = buffers.drawBuffer,
                       .renderTarget = target,
                       .renderPass = pass,
                       .shader = shader,
                       .dimensions = {
                           .x = static_cast<int>(image.dimensions.x),
                           .y = static_cast<int>(image.dimensions.y),
                       },
                       .vertexInfo = scene.vertexInfo,
                       .instanceInfo = scene.instanceInfo});

        vk->submitDrawCmdBuffer({.bufferData = buffers,
                                 .submitType = GRAPHICS});
    }

    void Renderer::render(SceneNode &scene)
    {
        auto backbuffer = backBufferManager->getNext();
        renderToRenderTarget(scene, backbuffer);
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
        SubmitCommand command{.bufferData = buffers, .submitType = PRESENT};
        auto swapchainImage = swapchain->getNext(buffers.schainAcqSem);
        vk->presentFrame(image,
                         swapchainImage.image,
                         swapchainImage.sc,
                         swapchainImage.index,
                         command);
    }

#pragma endregion Rendering

#pragma region CleanUp/Destructor
    void Renderer::cleanup()
    {
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
            .areaDims = {.x = static_cast<int>(options.windowDimensions.x),
                         .y = static_cast<int>(options.windowDimensions.y)},
            .areaOffset = {.x = 0, .y = 0},
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
        RenderTarget buffer;
        RenderPass pass;
        if (!renderTargetPool.get(data.framebuffer, buffer))
            throw std::runtime_error("failed to get framebuffer");
        if (!renderpassPool.get(buffer.renderpass, pass))
            throw std::runtime_error("failed to get renderpass");

        vk->buildShader(
            {.name = shader.name,
             .vert = shader.vert,
             .frag = shader.frag,
             .renderpass = pass.renderPass,
             .layout = layout.layout},
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

        Handle<RenderPass> passhandle = createRenderPass(data.renderpassDesc);
        RenderPass pass;
        if (!renderpassPool.get(passhandle, pass))
        {
            throw std::runtime_error("Failed to create renderpass.");
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

        auto bufferHandle = renderTargetPool.set(framebuffer);
        if (bufferHandle.gen == 0)
            throw std::runtime_error("failed to set a rendertarget");
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

    Handle<RTCmdBuffers> Renderer::createRenderTargetCmdData()
    {
        RTCmdBuffers sync{
            .drawBuffer = allocateCommandBuffer({.count = 1,
                                                 .level = PRIMARY,
                                                 .type = GRAPHICS}),
            .transferBuffer = allocateCommandBuffer({.count = 1,
                                                     .level = PRIMARY,
                                                     .type = TRANSFER}),
            .schainAcqSem = vk->createSemaphore(),
            .transferSem = vk->createSemaphore(),
            .inFlightFen = vk->createFence(true),
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
            if (rtCmdPool.get(framebuffer.cmdBuffers, data))
                vk->destroyRenderTargetCmdData(data);

            vk->destroyFramebuffer(framebuffer);
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
