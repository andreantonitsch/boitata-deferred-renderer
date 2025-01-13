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
                .swapchainFormat = FORMAT::BGRA_8_UNORM,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {FORMAT::BGRA_8_UNORM},
                                   .dimensions = {windowWidth, windowHeight}}});

    // Pipeline Layout for the Shader.
    Handle<ShaderLayout> layout = r.createShaderLayout({});

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
                            .offset = formatSize(FORMAT::RG_32_SFLOAT)}}}}
    });

    //GeometryData geometryData = triangleVertices();
    //GeometryData geometryData = squareVertices();
    GeometryData geometryData = planeVertices(1.0, 1.0, 100, 200);

    Handle<Geometry> geometry = r.getResourceManager().create(GeometryCreateDescription{
        .vertexInfo = {static_cast<uint32_t>(geometryData.vertices.size()), 0},
        .bufferData = {{
                        GeometryBufferDataDesc{
                            .vertexCount = static_cast<uint32_t>(geometryData.vertices.size()),
                            .vertexSize = static_cast<uint32_t>(sizeof(Vertex)),
                            .vertexDataPtr = geometryData.vertices.data()
                        }
                      }},
        .indexData = {
                      .count = static_cast<uint32_t>(geometryData.indices.size()),
                      .dataPtr = geometryData.indices.data(),
                      },
    });

    std::cout << "Created Geometry" << std::endl;
            

    SceneNode triangle({
        .name = "triangle",
        .geometry = geometry,
        .shader = shader,
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