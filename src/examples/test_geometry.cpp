#include "../renderer/Renderer.hpp"
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include <renderer/resources/builders/GeometryBuilder.hpp>

using namespace boitatah;

int main()
{

    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_UNORM,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {IMAGE_FORMAT::BGRA_8_UNORM},
                                   .dimensions = {windowWidth, windowHeight}}});

    // Pipeline Layout for the Shader.
    Handle<ShaderLayout> layout = r.createShaderLayout({});
    Handle<Shader> shader = r.createShader({.name = "test",
                                            .vert = {
                                                .byteCode = utils::readFile("./src/camera_vert.spv"),
                                                .entryFunction = "main"},
                                            .frag = {.byteCode = utils::readFile("./src/camera_frag.spv"), .entryFunction = "main"},
                                            .layout = layout,
                                            .vertexBindings = {
                                                {.stride = 12, .attributes = {{.location = 0, .format = IMAGE_FORMAT::RGB_32_SFLOAT, .offset = 0}}},
                                                {.stride = 12, .attributes = {{.location = 1, .format = IMAGE_FORMAT::RGB_32_SFLOAT, .offset = 0}}},
                                                {.stride = 8, .attributes = {{.location = 2, .format = IMAGE_FORMAT::RG_32_SFLOAT, .offset = 0}}},
                                                }});
    
    Handle<Geometry> geometry = GeometryBuilder::Triangle(r.getResourceManager());

    std::cout << "Created Geometry" << std::endl;

    auto material = r.createMaterial({
        .shader = shader,
        .bindings = {},
        .vertexBufferBindings = {VERTEX_BUFFER_TYPE::POSITION, 
                                 VERTEX_BUFFER_TYPE::COLOR,
                                 VERTEX_BUFFER_TYPE::UV,
                                 },
        .name = "material test"
    });

    SceneNode triangle({
        .name = "triangle",
        .geometry = geometry,
        .material = material,
        });

    // Scene Description.
    SceneNode scene({.name = "root scene"});
    scene.add(&triangle);
    boitatah::utils::Timewatch timewatch(1000);

    r.waitIdle();

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