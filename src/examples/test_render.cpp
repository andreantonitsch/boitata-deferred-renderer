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
        }};
    
    RenderTargetDesc targetDesc{
                    .renderpassDesc = {.format = RGBA_8_SRGB, .attachments = attachments},
                    .attachments = attachments,
                    .imageDesc = fbAttImageDesc,
                    .dimensions = {windowWidth, windowHeight},
                };

    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = FORMAT::BGRA_8_SRGB,
                .backBufferDesc = targetDesc});

    Handle<RenderTarget> rendertarget = r.createRenderTarget(targetDesc);

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

    SceneNode scene{.children = {},
                    .shader = shader,
                    .vertexInfo = {3, 0},
                    .instanceInfo = {1, 0}};

    boitatah::utils::Timewatch timewatch(1000);
    int i = 0;
    bool renderToTarget = false;

    if (renderToTarget)
        while (!r.isWindowClosed())
        {

            // wait for frame to finish
            //  record command buffer to render scene into image
            //  submit command buffer
            r.renderToRenderTarget(scene, rendertarget);
            // std::cout << "rendered scene" << std::endl;
            //  present the rendered frame to swapchain
            //       acquire image from swapchain.
            //       transfer rendertarget to swapchain.
            //       present image to screen, return to swapchain
            r.presentRenderTarget(rendertarget);

            std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
            // i++;
            // if(i > 1) break;
        }
    else
        while (!r.isWindowClosed())
        {

            // wait for frame to finish
            //  record command buffer to render scene into image
            //  submit command buffer
            r.render(scene);


            std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
            // i++;
            // if(i > 1) break;
        }
    r.waitIdle();

    r.destroyLayout(layout);
    r.destroyRenderTarget(rendertarget);
    r.destroyShader(shader);

    return EXIT_SUCCESS;
}