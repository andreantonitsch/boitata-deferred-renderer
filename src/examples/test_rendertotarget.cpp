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
         .format = IMAGE_FORMAT::RGBA_8_SRGB,
         .layout = IMAGE_LAYOUT::COLOR_ATT_OPTIMAL,
         .initialLayout = IMAGE_LAYOUT::UNDEFINED,
         .finalLayout = IMAGE_LAYOUT::COLOR_ATT_OPTIMAL},
    };

    std::vector<ImageDesc> fbAttImageDesc = {
        {
            .format = IMAGE_FORMAT::RGBA_8_SRGB,
            .dimensions = {windowWidth, windowHeight},
            .initialLayout = IMAGE_LAYOUT::UNDEFINED,
            .usage = IMAGE_USAGE::COLOR_ATT_TRANSFER_SRC,
        }};

    RenderTargetDesc targetDesc{
        .renderpassDesc = {.attachments = attachments},
        .attachments = attachments,
        .imageDesc = fbAttImageDesc,
        .dimensions = {windowWidth, windowHeight},
    };

    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {IMAGE_FORMAT::RGBA_8_SRGB},
                                   .dimensions = {windowWidth, windowHeight}}
                });

    Handle<RenderTarget> rendertarget = r.createRenderTarget(targetDesc);

    Handle<ShaderLayout> layout = r.createShaderLayout({});

    Handle<Shader> shader = r.createShader({
        .name = "test",
        .vert = {
            .byteCode = utils::readFile("./src/09_shader_base_vert.spv"),
            .entryFunction = "main"},
        .frag = {.byteCode = utils::readFile("./src/09_shader_base_frag.spv"), .entryFunction = "main"},
        .framebuffer = rendertarget,
        .layout = layout,
        .bindings={}
    });
    Handle<Geometry> geometry = r.getResourceManager().create({.vertexInfo = {3, 0}});

    SceneNode triangle({
        .name = "triangle",
        .geometry = geometry,
        .shader = shader,
        //.instanceInfo = {1, 0},
        });


    boitatah::utils::Timewatch timewatch(100);
    uint32_t frame = 0;
    while (!r.isWindowClosed())
    {
        // wait for frame to finish
        //  record command buffer to render scene into image
        //  submit command buffer
        r.renderToRenderTarget(triangle, rendertarget, frame);
        // std::cout << "rendered scene" << std::endl;
        //  present the rendered frame to swapchain
        //       acquire image from swapchain.
        //       transfer rendertarget to swapchain.
        //       present image to screen, return to swapchain
        r.presentRenderTarget(rendertarget);

        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
        frame++;
    }
    r.waitIdle();

    r.destroyLayout(layout);
    r.destroyRenderTarget(rendertarget);
    r.destroyShader(shader);

    return EXIT_SUCCESS;
}