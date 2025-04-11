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
                .backBufferDesc2 = BackBufferManager::BasicDeferredPipeline(windowWidth,
                                                                            windowHeight)
                });

    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(
                                        std::string("./resources/UV_checker1k.png"),
                                        IMAGE_FORMAT::RGBA_8_SRGB,
                                        TextureMode::READ, SamplerData(),
                                        r.getResourceManager());

    std::cout << "creating material" << std::endl;
    auto material = r.getMaterials().createLambertMaterial(0, 100, texture);

    Handle<Geometry> quad =     GeometryBuilder::Quad(r.getResourceManager());
    Handle<Geometry> pipe =     GeometryBuilder::Cylinder(r.getResourceManager(), 0.5, 2.0, 10, 32);

    SceneNode scene({.name = "root scene"});
    std::vector<SceneNode> nodes;

    SceneNode pipeNode({
        .name = "pipe",
        .geometry = pipe,
        .material = material,
        .position = glm::vec3(0, 0, 0),
    });

    nodes.reserve(100);
    for(int i = 0; i < 5; i++)
        for(int j = 0; j < 5; j++){
            nodes.push_back(pipeNode);
            nodes.back().translate(glm::vec3(-5 + 2*i, 0.5, - 5 + 2*j));
            scene.add(&nodes.back());
        }
    //scene.add(&pipeNode);

    // Scene Description.
    std::cout << "creating deferred composer material" << std::endl;
    auto composer_material = r.getMaterials().createLambertDeferredComposeMaterial(1, 150u);
    
    Handle<LightArray> light_handle = r.createLightArray(100);
    r.setLightArray(light_handle);
    auto& lights = r.getLightArray(light_handle);
    lights.update();
    
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.metadata(), 1, 0);
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.light_array(), 1, 1);
    
    r.getMaterialManager().printMaterial(composer_material);
    
    SceneNode composerNode({
        .name = "composer",
        .geometry = quad,
        .material = composer_material,
    });
    scene.add(&composerNode);

    BufferedCamera camera = r.createCamera({
                   .position = glm::float3(0,-2,-5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(1000);
    
    uint32_t count = 0;
    float frame_TimeScale = 0.001;
    float dist = 5;
    std::cout << " setup done" << std::endl;
    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;
        camera.setPosition(glm::vec3(glm::cos(t) * dist, -2, -glm::sin(t) * dist));
        camera.lookAt(glm::vec3(0));
        r.render_graph(scene, camera);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();


    return EXIT_SUCCESS;
}