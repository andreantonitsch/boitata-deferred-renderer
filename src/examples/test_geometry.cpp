#include "../renderer/Renderer.hpp"
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
                                   .dimensions = {windowWidth, windowHeight}}});

    // Pipeline Layout for the Shader.
    Handle<PipelineLayout> layout = r.createPipelineLayout({});

    // Shader Description
    Handle<Shader> shader = r.createShader({
        .name = "test",
        .vert = {
            .byteCode = utils::readFile("./src/18_vert.spv"),
            .entryFunction = "main"},
        .frag = {.byteCode = utils::readFile("./src/18_frag.spv"), .entryFunction = "main"},
        .layout = layout,
        .bindings={{
            .stride = 20,
            .attributes = {{.format = FORMAT::RG_32_SFLOAT,
                           .offset = 0},
                           {.format = FORMAT::RGB_32_SFLOAT,
                            .offset = 8}}}}
    });

    //GeometryData geometry = squareVertices();
    GeometryData geometryData = planeVertices(1.0, 1.0, 500, 500);

    Handle<Geometry> geometry = r.createGeometry({
        .vertexInfo = {geometryData.vertices.size(), 0},
        .vertexSize = static_cast<uint32_t>(sizeof(Vertex)),
        .vertexDataSize = static_cast<uint32_t>(sizeof(Vertex)) * geometryData.vertices.size(),
        .vertexData = geometryData.vertices.data(),
        .indexCount = geometryData.indices.size(),
        .indexData = geometryData.indices.data(),
    });

    SceneNode triangle = {
        .name = "triangle",
        .shader = shader,
        .geometry = geometry,
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