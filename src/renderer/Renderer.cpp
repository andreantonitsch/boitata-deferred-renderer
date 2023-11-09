#pragma once

#include <iostream>
#include <stdexcept>

#include "Renderer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>

namespace boitatah
{
#pragma region Initialization
    Renderer::Renderer(RendererOptions opts)
    {
        options = opts;
        initWindow();
        createVulkan();
        buildSwapchain();
        drawBuffer = allocateCommandBuffer({.count = 1,
                                            .level = PRIMARY});
    }

    void Renderer::createVulkan()
    {
        uint32_t extensionCount = 0;

        vk = new Vulkan({
            .appName = (char *)options.appName,
            .extensions = requiredWindowExtensions(),
            .useValidationLayers = options.debug,
            .debugMessages = options.debug,
            .window = window,
        });
    }
#pragma endregion Initialization

#pragma region Rendering
    void Renderer::render(SceneNode &scene, Handle<Framebuffer> &rendertarget)
    {
        vk->resetCommandBuffer(drawBuffer);

        writeCommandBuffer(scene, rendertarget);

        vk->submitCommandBuffer(drawBuffer);
    }

    void Renderer::writeCommandBuffer(SceneNode &scene, Handle<Framebuffer> &rendertarget)
    {
        Framebuffer buffer;
        if (!frameBufferPool.get(rendertarget, buffer))
        {
            throw std::runtime_error("Failed to write command buffer");
        }
        RenderPass pass;
        if (!renderpassPool.get(buffer.renderpass, pass))
        {
            throw std::runtime_error("Failed to write command buffer");
        }

        Image image;
        if (!imagePool.get(buffer.attachments[0], image))
        {
            throw std::runtime_error("Failed to write command buffer");
        }
        std::cout<<buffer.buffer;
        vk->recordCommand({
            .drawBuffer = drawBuffer.buffer,
            .pass = pass.renderPass,
            .frameBuffer = buffer.buffer,
            .areaDims = {static_cast<int>(image.dimensions.x),
                         static_cast<int>(image.dimensions.y)},
            .areaOffset = {0, 0},
            .vertexCount = 3,
            .instaceCount = 1,
            .firstVertex = 0,
            .firstInstance = 0,
        });
        // blah blah sets up draw commands
    }

    void Renderer::present(Handle<Framebuffer> &rendertarget)
    {
        windowEvents();
        vk->waitForFrame();
        // vk->copyRendertargetToSwapchain
        vk->presentFrame();
    }

#pragma endregion Rendering

#pragma region CleanUp/Destructor
    void Renderer::cleanup()
    {

        cleanupSwapchainBuffers();
        delete vk;
        cleanupWindow();
    }

    void Renderer::cleanupSwapchainBuffers()
    {
        for (auto &bufferhandle : swapchainBuffers)
        {
            destroyFramebuffer(bufferhandle);
        }
        swapchainBuffers.resize(0);
    }

    Renderer::~Renderer(void)
    {
        cleanup();
    }
#pragma endregion CleanUp / Destructor

#pragma region Command Buffers

    CommandBuffer Renderer::allocateCommandBuffer(const CommandBufferDesc &desc)
    {
        CommandBuffer buffer{
            .buffer = vk->allocateCommandBuffer(desc)};

        return buffer;
    }

    void Renderer::recordCommand(const DrawCommand &command)
    {
        Framebuffer framebuffer;
        if (!frameBufferPool.get(command.buffer, framebuffer))
        {
            throw std::runtime_error("failed to get the Framebuffer data");
        }

        RenderPass pass;
        if (!renderpassPool.get(framebuffer.renderpass, pass))
        {
            throw std::runtime_error("failed to get the Render Pass data");
        }

        vk->recordCommand({
            .drawBuffer = drawBuffer.buffer,
            .pass = pass.renderPass,
            .frameBuffer = framebuffer.buffer,
            .areaDims = {.x = static_cast<int>(options.windowDimensions.x),
                         .y = static_cast<int>(options.windowDimensions.y)},
            .areaOffset = {.x = 0, .y = 0},
            .vertexCount = 3,
            .instaceCount = 1,
            .firstVertex = 0,
            .firstInstance = 0,
        });
    }

#pragma endregion Command Buffers

#pragma region Window Functions

    void Renderer::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO temp
        window = glfwCreateWindow(options.windowDimensions.x,
                                  options.windowDimensions.y,
                                  options.appName,
                                  nullptr,
                                  nullptr);
    }
    void Renderer::cleanupWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Renderer::isWindowClosed()
    {
        return glfwWindowShouldClose(window);
    }

    void Renderer::buildSwapchain()
    {
        // Clear old swapchain and get new images.
        cleanupSwapchainBuffers();
        vk->buildSwapchain(options.swapchainFormat);
        std::vector<Image> swapchainImages = vk->getSwapchainImages();

        // Create new swapchain framebuffers
        std::vector<AttachmentDesc> attachments;
        attachments.push_back({.index = 0,
                               .format = BGRA_8_SRGB,
                               .layout = COLOR_ATT_OPTIMAL,
                               .samples = SAMPLES_1,
                               .initialLayout = UNDEFINED,
                               .finalLayout = PRESENT_SRC});

        for (const auto &image : swapchainImages)
        {
            std::vector<Handle<Image>> imageAttachments;
            imageAttachments.push_back(imagePool.set(image));
            FramebufferDesc desc{
                .renderpassDesc = {
                    .format = BGRA_8_SRGB,
                    .attachments = attachments,
                },
                .attachments = attachments,
                .attachmentImages = imageAttachments,
                .dimensions = image.dimensions,
            };

            Handle<Framebuffer> framebuffer = createFramebuffer(desc);

            swapchainBuffers.push_back(framebuffer);
        }
    }

    const std::vector<const char *> Renderer::requiredWindowExtensions()
    {
        std::vector<const char *> requiredExtensions;
        uint32_t extensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        for (uint32_t i = 0; i < extensionCount; i++)
        {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }
        return requiredExtensions;
    }

    void Renderer::windowEvents()
    {
        glfwPollEvents();
    }
#pragma endregion Window Functions

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
        Framebuffer buffer;
        RenderPass pass;
        if (!frameBufferPool.get(data.framebuffer, buffer))
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

    Handle<Framebuffer> Renderer::createFramebuffer(const FramebufferDesc &data)
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

        Framebuffer framebuffer{
            .buffer = vk->createFramebuffer(vkDesc),
            .attachments = images,
            .renderpass = passhandle,
        };

        return frameBufferPool.set(framebuffer);
    }

    Handle<RenderPass> Renderer::createRenderPass(const RenderPassDesc &data)
    {
        RenderPass pass{
            .renderPass = vk->createRenderPass(data)};

        return renderpassPool.set(pass);
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

    void Renderer::destroyFramebuffer(Handle<Framebuffer> bufferhandle)
    {
        Framebuffer framebuffer;
        if (frameBufferPool.clear(bufferhandle, framebuffer))
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
