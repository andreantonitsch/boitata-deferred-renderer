#include "../renderer/Renderer.hpp"
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include "../renderer/Camera.hpp"

using namespace boitatah;

int main()
{

    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = FORMAT::BGRA_8_UNORM,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {FORMAT::BGRA_8_UNORM},
                                   .dimensions = {windowWidth, windowHeight}}});

    // Pipeline Layout for the Shader.
    Handle<ShaderLayout> layout = r.createShaderLayout({});

    // Shader Description
    Handle<Shader> shader = r.createShader({.name = "test",
                                            .vert = {
                                                .byteCode = utils::readFile("./src/18_vert.spv"),
                                                .entryFunction = "main"},
                                            .frag = {.byteCode = utils::readFile("./src/18_frag.spv"), .entryFunction = "main"},
                                            .layout = layout,
                                            .bindings = {{.stride = 20, .attributes = {{.format = FORMAT::RG_32_SFLOAT, .offset = 0}, {.format = FORMAT::RGB_32_SFLOAT, .offset = formatSize(FORMAT::RG_32_SFLOAT)}}}}});

    GeometryData geometryData = triangleVertices();

    Handle<Geometry> geometry = r.createGeometry({
        .vertexInfo = {geometryData.vertices.size(), 0},
        .vertexSize = static_cast<uint32_t>(sizeof(Vertex)),
        .vertexDataSize = static_cast<uint32_t>(sizeof(Vertex) * geometryData.vertices.size()),
        .vertexData = geometryData.vertices.data(),
        .indexCount = static_cast<uint32_t>(geometryData.indices.size()),
        .indexData = geometryData.indices.data(),
    });

    SceneNode triangle({
        .name = "triangle",
        .geometry = geometry,
        .shader = shader,
    });

    // Scene Description.
    SceneNode scene({.name = "root scene"});
    scene.add(&triangle);

    Camera camera({.aspect = static_cast<float>(windowWidth) / windowHeight});
    
    boitatah::utils::Timewatch timewatch(1000);

    while (!r.isWindowClosed())
    {
        r.render(scene, camera);

        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();

    r.destroyLayout(layout);
    r.destroyShader(shader);

    return EXIT_SUCCESS;
}