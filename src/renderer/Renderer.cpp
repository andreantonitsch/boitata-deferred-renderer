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
        allocateCommandBuffer({
            .count =1,
            .level = PRIMARY
        });
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
    void Renderer::render()
    {
        windowEvents();
    }
#pragma end region Rendering

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
#pragma endregion CleanUp/Destructor

#pragma region Command Buffers

    CommandBuffer Renderer::allocateCommandBuffer(const CommandBufferDesc &desc)
    {
        CommandBuffer buffer {
            .buffer = vk->allocateCommandBuffer(desc)
        };

        return buffer;
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

            // Handle<RenderPass> passhandle = renderpassPool.set(
            //     {.renderPass = vk->createRenderPass(desc.renderpassDesc)});
            // RenderPass pass;
            // if (!renderpassPool.get(passhandle, pass))
            //     throw std::runtime_error("Failed to create render pass");

            // Pushes the Pool handle for the created framebuffer.
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
        return Handle<PipelineLayout>();
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

#pragma endregion Destroy Vulkan Objects

}
