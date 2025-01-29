#include "../renderer/Renderer.hpp"
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include <random>

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
    Handle<ShaderLayout> layout = r.createShaderLayout({
        .customPushConstants ={
            PushConstantDesc{
                .offset = sizeof(glm::mat4), //<-- must be larger or equal than sizeof(glm::mat4)
                .size = sizeof(glm::mat4) * 2, //<- V P matrices
                .stages = STAGE_FLAG::ALL_GRAPHICS}
        }
    });

    Handle<Shader> shader = r.createShader({.name = "test",
                                            .vert = {
                                                .byteCode = utils::readFile("./src/push_constants_vert.spv"),
                                                .entryFunction = "main"},
                                            .frag = {.byteCode = utils::readFile("./src/push_constants_frag.spv"), .entryFunction = "main"},
                                            .layout = layout,
                                            .bindings = {{.stride = 24, .attributes = {{.format = FORMAT::RGB_32_SFLOAT, .offset = 0}, {.format = FORMAT::RGB_32_SFLOAT, .offset = formatSize(FORMAT::RG_32_SFLOAT)}}}}});
    
    
    GeometryData geometryData = triangleVertices();
    //GeometryData geometryData = squareVertices();
    //GeometryData geometryData = planeVertices(1.0, 1.0, 10, 20);

    Handle<Geometry> geometry = r.getResourceManager().create(GeometryCreateDescription{
        .vertexInfo = { static_cast<uint32_t>(geometryData.vertices.size()), 0},
        .bufferData = { {   .vertexCount = static_cast<uint32_t>(geometryData.vertices.size()),
                            .vertexSize = static_cast<uint32_t>(sizeof(Vertex)),
                            .vertexDataPtr = geometryData.vertices.data()
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

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<> pos_dist(-1.0f, 1.0f);
    std::uniform_real_distribution<> scale_dist(0.25f, 0.5f);
    
    int count = 1000;
    for (int i = 0; i < count; i++)
    {
        SceneNode* other = new SceneNode({
        .name = "triangle",
        .geometry = geometry,
        .shader = shader,
        });
        // other->translate({pos_dist(rng),pos_dist(rng),pos_dist(rng)});
        // other->translate({scale_dist(rng),scale_dist(rng),scale_dist(rng)});
        scene.add(other);
    }
    
    scene.add(&triangle);
    boitatah::utils::Timewatch timewatch(10);

    r.waitIdle();
    
    float rotateSpeed = (2 * glm::pi<float>() / 9000 );

    while (!r.isWindowClosed())
    {
        r.render(scene);

        //triangle.rotate({0.0f, 0.0f, 1.0}, rotateSpeed); //<-- is a push constant update

        std::cout << "\rFrametime :: " << timewatch.Lap() << "     "   << std::flush;
    }
    r.waitIdle();

    r.destroyLayout(layout);
    r.destroyShader(shader);

    return EXIT_SUCCESS;
}