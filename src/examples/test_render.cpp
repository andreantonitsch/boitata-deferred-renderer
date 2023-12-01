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

    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = FORMAT::BGRA_8_SRGB,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {FORMAT::RGBA_8_SRGB},
                                   .dimensions = {windowWidth, windowHeight}}
                });

    // Pipeline Layout for the Shader.
    Handle<PipelineLayout> layout = r.createPipelineLayout({});

    // Shader Description
    Handle<Shader> shader = r.createShader({
        .name = "test",
        .vert = {
            .byteCode = utils::readFile("./src/09_shader_base_vert.spv"),
            .entryFunction = "main"},
        .frag = {.byteCode = utils::readFile("./src/09_shader_base_frag.spv"), .entryFunction = "main"},
        .layout = layout,
        .bindings={}
    });
    Handle<Geometry> geometry = r.createGeometry({.vertexInfo = {3, 0}});

    SceneNode triangle = {
        .name = "triangle",
        .shader = shader,
        .geometry = geometry,
        //.instanceInfo = {1, 0},
        };

    
    // Scene Description.
    SceneNode scene{.name = "root scene", .children = {triangle}};

    boitatah::utils::Timewatch timewatch(1000);

    while (!r.isWindowClosed())
    {
        r.render(scene);

        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;

    }
    r.waitIdle();

    r.destroyLayout(layout);
    r.destroyShader(shader);

    return EXIT_SUCCESS;
}