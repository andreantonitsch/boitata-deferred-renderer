#include "../renderer/Renderer.hpp"
// #pragma once
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"



using namespace boitatah;

int main()
{

    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    Renderer r({
        .windowDimensions = {windowWidth, windowHeight},
        .appName = "Test Frame Buffer",
        .debug = true,
        .swapchainFormat = FORMAT::BGRA_8_SRGB,
    });

    std::vector<AttachmentDesc> attachments{
        {.index = 0,
         .format = RGBA_8_SRGB,
         .layout = COLOR_ATT_OPTIMAL,
         .initialLayout = UNDEFINED,
         .finalLayout = COLOR_ATT_OPTIMAL},
    };

    std::vector<ImageDesc> fbAttImageDesc = {
        {
            .format = RGBA_8_SRGB,
            .dimensions = {windowWidth, windowHeight},
            .initialLayout = UNDEFINED,
            .usage = USAGE::COLOR_ATT_TRANSFER_SRC,
        }
    };

    Handle<RenderTarget> rendertarget = r.createRenderTarget(
        {
            .renderpassDesc = {.format = RGBA_8_SRGB, .attachments = attachments},
            .attachments = attachments,
            .imageDesc = fbAttImageDesc,
            .dimensions = {windowWidth, windowHeight},
        });


    Handle<PipelineLayout> layout = r.createPipelineLayout({});

    Handle<Shader> shader = r.createShader({
        .name = "test",
        .vert = {
            .byteCode = utils::readFile("./src/09_shader_base_vert.spv"),
            .entryFunction = "main"},
        .frag = {.byteCode = utils::readFile("./src/09_shader_base_frag.spv"), .entryFunction = "main"},
        .framebuffer = rendertarget,
        .layout = layout,
    });

    SceneNode scene{.children = {}, .shader = shader};

    boitatah::utils::Timewatch timewatch(240);

    while (!r.isWindowClosed())
    {
        
        //wait for frame to finish
        // record command buffer to render scene into image
        // submit command buffer
        r.renderRenderTarget(scene, rendertarget);
        //std::cout << "rendered scene" << std::endl;
        // present the rendered frame to swapchain
        //      acquire image from swapchain.
        //      transfer rendertarget to swapchain.
        //      present image to screen, return to swapchain
        r.present(rendertarget);

        auto lap = timewatch.Lap();
        std::cout << "\rFrametime :: " << lap << std::endl;
    
    }

    r.waitIdle();

    r.destroyLayout(layout);
    r.destroyFramebuffer(rendertarget);
    r.destroyShader(shader);

    return EXIT_SUCCESS;
}