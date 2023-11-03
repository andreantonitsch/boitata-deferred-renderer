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

    // Handle<Framebuffer> framebuffer = r.createFramebuffer(
    //     {
    //         .renderpassDesc = {.format = RGBA_8_SRGB, .attachments = attachments},
    //         .attachments = attachments,
    //         .dimensions = {windowWidth, windowHeight},
    //     });

    // Handle<Shader> shader = r.createShader({
    //     .name = "test",
    //     .vert = {
    //         .byteCode = utils::readFile("./src/09_shader_base_vert.spv"),
    //         .entryFunction = "main"},
    //     .frag = {.byteCode = utils::readFile("./src/09_shader_base_frag.spv"), .entryFunction = "main"},
    //     .framebuffer = framebuffer,
    // });

    while (!r.isWindowClosed())
    {
        r.render();
        // r.render(scene, target);
        // r.present(target);
    }

    //r.destroyFramebuffer(framebuffer);
    //r.destroyShader(shader);

    return EXIT_SUCCESS;
}