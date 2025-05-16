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
                .backBufferDesc = BackBufferManager::BasicMultiWriteForwardPipeline(windowWidth,
                                                                                     windowHeight,
                                                                                     1),
                });


    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(
                                        std::string("./resources/UV_checker1k.png"),
                                        IMAGE_FORMAT::RGBA_8_SRGB,
                                        TextureMode::READ, SamplerData(),
                                        r.getResourceManager());

    std::cout << "creating material" << std::endl;
    auto material = r.getMaterials().createUnlitMaterial(0, 100, texture);

    Handle<Geometry> triangle = GeometryBuilder::Triangle(r.getResourceManager());
    Handle<Geometry> quad =     GeometryBuilder::Quad(r.getResourceManager());
    Handle<Geometry> circle =   GeometryBuilder::Circle(r.getResourceManager(), 0.5f, 32);
    Handle<Geometry> pipe =     GeometryBuilder::Pipe(r.getResourceManager(), 0.5, 2.0, 10, 32);

    std::cout << "creating scene node" << std::endl;
    auto triangleNode = RenderScene::create_node({
        .name = "triangle",
        .content = {.geometry = triangle,
                    .material = material},
        .position = glm::vec3(-3.0f, 0, 0),
    });
    auto quadNode = RenderScene::create_node({
        .name = "quad",
        .content = {.geometry = quad,
                    .material = material},
        .position = glm::vec3(-1.5f, 0, 0),
    });

    auto circleNode = RenderScene::create_node({
        .name = "circle",
        .content = {.geometry = circle,
                    .material = material},
        .position = glm::vec3(0.0f, 0, 0),
    });

    auto pipeNode = RenderScene::create_node({
        .name = "pipe",
        .content = {.geometry = pipe,
                    .material = material},
        .position = glm::vec3(1.5f, 0, 0),
    });

    // Scene Description.
    std::shared_ptr<RenderScene> scene = RenderScene::create_node({.name = "root scene"});
    scene->add(pipeNode);
    scene->add(triangleNode);
    scene->add(quadNode);
    scene->add(circleNode);

    BufferedCamera camera = r.create_camera({
                   .position = glm::float3(0,-2,-5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(1000);
    
    uint32_t count = 0;
    float frame_TimeScale = 0.001;
    float dist = 5;
    std::cout << "setup complete" << std::endl;

    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;
        camera.setPosition(glm::vec3(glm::cos(t) * dist, -2, -glm::sin(t) * dist));
        camera.lookAt(glm::vec3(0));
        r.render_tree(scene, camera);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();


    return EXIT_SUCCESS;
}