#include "../renderer/Renderer.hpp"
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include "../renderer/modules/Camera.hpp"


#include <vector>

#include <chrono>
#include <thread>
#include <cstdlib>

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


    auto& bufferManager = r.getBufferManager();
    
    auto bufferSpace = bufferManager.reserveBuffer({
          .request = 1u << 4,
          .usage = BUFFER_USAGE::TRANSFER_DST,
          .sharing = SHARING_MODE::EXCLUSIVE,
      });

    int quantity = 10000;
    std::vector<Handle<BufferAddress>> buffers(quantity);

    for (int i = 0; i < quantity; i++)
    {
        buffers[i] = bufferManager.reserveBuffer({
          .request = 1u << 10u ,
          .usage = BUFFER_USAGE::TRANSFER_DST,
          .sharing = SHARING_MODE::EXCLUSIVE,
      });
    }
    
  

    // // Pipeline Layout for the Shader.
    // Handle<ShaderLayout> layout = r.createShaderLayout({});

    // // Shader Description
    // Handle<Shader> shader = r.createShader({.name = "test",
    //                                         .vert = {
    //                                             .byteCode = utils::readFile("./src/18_vert.spv"),
    //                                             .entryFunction = "main"},
    //                                         .frag = {.byteCode = utils::readFile("./src/18_frag.spv"), .entryFunction = "main"},
    //                                         .layout = layout,
    //                                         .bindings = {{.stride = 20, .attributes = {{.format = FORMAT::RG_32_SFLOAT, .offset = 0}, {.format = FORMAT::RGB_32_SFLOAT, .offset = formatSize(FORMAT::RG_32_SFLOAT)}}}}});

    // GeometryData geometryData = triangleVertices();

    // Handle<Geometry> geometry = r.createGeometry({
    //     .vertexInfo = {geometryData.vertices.size(), 0},
    //     .vertexSize = static_cast<uint32_t>(sizeof(Vertex)),
    //     .vertexDataSize = static_cast<uint32_t>(sizeof(Vertex) * geometryData.vertices.size()),
    //     .vertexData = geometryData.vertices.data(),
    //     .indexCount = static_cast<uint32_t>(geometryData.indices.size()),
    //     .indexData = geometryData.indices.data(),
    // });

    // SceneNode triangle({
    //     .name = "triangle",
    //     .geometry = geometry,
    //     .shader = shader,
    // });

    // // Scene Description.
    // SceneNode scene({.name = "root scene"});
    // scene.add(&triangle);

    // Camera camera({.aspect = static_cast<float>(windowWidth) / windowHeight});
    
    //boitatah::utils::Timewatch timewatch(10);

    // while (!r.isWindowClosed())
    // {
    //     //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //     //r.render(scene, camera);

    //     //std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    // }
    //r.waitIdle();

    //r.destroyLayout(layout);
    //r.destroyShader(shader);

    return EXIT_SUCCESS;
}