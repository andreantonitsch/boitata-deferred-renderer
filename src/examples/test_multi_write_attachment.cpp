#include "../renderer/Renderer.hpp"
#include <iostream>
// #include <unistd.h>
#include <types/BttEnums.hpp>
#include <types/Shader.hpp>
#include <utils/utils.hpp>
#include <collections/Pool.hpp>

#include <renderer/resources/builders/GeometryBuilder.hpp>
#include <utils/ImageLoader.hpp>
#include <memory>
using namespace boitatah;

int main()
{


    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB,
                // .backBufferDesc2 = BackBufferManager::BasicMultiWriteForwardPipeline(windowWidth,
                //                                                            windowHeight, 1),
                .backBufferDesc2 = BackBufferManager::BasicMultiWriteForwardPipeline(windowWidth,
                                                                                     windowHeight,
                                                                                     1),
                });


    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(
                                        std::string("./resources/UV_checker1k.png"),
                                        IMAGE_FORMAT::RGBA_8_SRGB,
                                        TextureMode::READ, SamplerData(),
                                        r.getResourceManager());

    std::cout << "creating material" << std::endl;
    auto material = r.getMaterials().createUnlitMaterial(1, 100, texture);

    Handle<Geometry> triangle = GeometryBuilder::Triangle(r.getResourceManager());
    Handle<Geometry> quad =     GeometryBuilder::Quad(r.getResourceManager());
    Handle<Geometry> circle =   GeometryBuilder::Circle(r.getResourceManager(), 0.5f, 32);
    Handle<Geometry> pipe =     GeometryBuilder::Pipe(r.getResourceManager(), 0.5, 2.0, 10, 32);

    std::cout << "creating scene node" << std::endl;
    SceneNode triangleNode({
        .name = "triangle",
        .geometry = triangle,
        .material = material,
        .position = glm::vec3(-3.0f, 0, 0),
    });
    SceneNode quadNode({
        .name = "quad",
        .geometry = quad,
        .material = material,
        .position = glm::vec3(-1.5f, 0, 0),
    });

    SceneNode circleNode({
        .name = "circle",
        .geometry = circle,
        .material = material,
        .position = glm::vec3(0.0f, 0, 0),
    });

    SceneNode pipeNode({
        .name = "pipe",
        .geometry = pipe,
        .material = material,
        .position = glm::vec3(1.5f, 0, 0),
    });

    // Scene Description.
    SceneNode scene({.name = "root scene"});
    scene.add(&pipeNode);
    scene.add(&triangleNode);
    scene.add(&quadNode);
    scene.add(&circleNode);

    BufferedCamera camera = r.createCamera({
                   .position = glm::float3(0,-2,-5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(1000);
    
    uint32_t count = 0;
    float frame_TimeScale = 0.001;
    float dist = 5;
    std::cout << "setup complete" << std::endl;

    r.waitIdle();
    while (!r.isWindowClosed())
    {
        //camera.orbit(glm::vec3(0), glm::vec3(0, 0.1f, 0));
        float t = count * frame_TimeScale;
        count++;
        camera.setPosition(glm::vec3(glm::cos(t) * dist, -2, -glm::sin(t) * dist));
        camera.lookAt(glm::vec3(0));
        r.render_graph(scene, camera);
        ///camera.rotate(glm::vec3(0.0, 0.01, 0.0));
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();


    return EXIT_SUCCESS;
}