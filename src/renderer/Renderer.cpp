#pragma once

#include <iostream>
#include <stdexcept>

#include "Renderer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>

namespace boitatah
{
    /// CONSTRUCTORS
    Renderer::Renderer(RendererOptions opts)
    {
        options = opts;
        initWindow();
        createVulkan();
        buildSwapchain();
    }
    /// END CONSTRUCTORS

    void Renderer::render()
    {
        windowEvents();
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

    // Clean Up // Destructors
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

    /// WINDOWN FUNCTIONS

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
        std::vector<Handle<Image>> imageAttachments;
        attachments.push_back({.index = 0,
                               .format = BGRA_8_SRGB,
                               .samples = SAMPLES_1,
                               .initialLayout = UNDEFINED,
                               .finalLayout = PRESENT_SRC});

        for (const auto &image : swapchainImages)
        {
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
            Handle<RenderPass> passhandle = renderpassPool.set(
                {.renderPass = vk->createRenderPass(desc.renderpassDesc)});
            RenderPass pass;
            if (!renderpassPool.get(passhandle, pass))
                throw std::runtime_error("Failed to create render pass");

            const FramebufferDescVk vkDesc = {.views = {image.view},
                                              .pass = pass.renderPass,
                                              .dimensions = desc.dimensions};

            // Pushes the Pool handle for the created framebuffer.
            swapchainBuffers.push_back(
                frameBufferPool.set(
                    {
                        .buffer = vk->createFramebuffer(vkDesc),
                        .attachments = imageAttachments,
                        .renderpass = passhandle,
                        // vk->createFramebuffer
                    }));
        }
    }

    Handle<Shader> Renderer::createShader(const ShaderDesc &data)
    {
        Shader shader{
            .name = data.name,
            .vert = {.shaderModule = vk->createShaderModule(data.vert.byteCode),
                     .entryFunction = data.vert.entryFunction},
            .frag = {.shaderModule = vk->createShaderModule(data.frag.byteCode),
                     .entryFunction = data.vert.entryFunction}};

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
             .renderpass = pass.renderPass},
            shader);

        return shaderPool.set(shader);
    }

    Handle<Framebuffer> Renderer::createFramebuffer(const FramebufferDesc &data)
    {
        if (data.attachmentImages.empty())
        {
            // Create images.
        }

        Handle<RenderPass> passhandle = createRenderPass(data.renderpassDesc);
        RenderPass pass;
        if (!renderpassPool.get(passhandle, pass))
        {
            throw std::runtime_error("Failed to create renderpass.");
        }

        std::vector<VkImageView> imageViews;

        for (auto &imagehandle : data.attachmentImages)
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
            .renderpass = passhandle};

        return frameBufferPool.set(framebuffer);
    }

    Handle<RenderPass> Renderer::createRenderPass(const RenderPassDesc &data)
    {
        RenderPass pass{
            .renderPass = vk->createRenderPass(data)};

        return renderpassPool.set(pass);
    }

    Handle<PipelineLayout> Renderer::createPipelineLayout(const PipelineLayoutDesc &desc)
    {
        return Handle<PipelineLayout>();
    }

    void Renderer::destroyShader(Handle<Shader> handle)
    {
        Shader shader;
        if (shaderPool.clear(handle, shader))
        {
            vk->destroyShader(shader);
        }
        else
        {
            std::cout << "Shader Double Destruction" << std::endl;
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
                    if(!image.swapchain)
                        vk->destroyImage(image);
                }
            }

            vk->destroyFramebuffer(framebuffer);

            RenderPass pass;
            if (renderpassPool.clear(framebuffer.renderpass, pass))
            {
                vk->destroyRenderpass(pass);
            }
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

    /// END WINDOW FUNCTIONS
}
