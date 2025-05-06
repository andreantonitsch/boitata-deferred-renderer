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
#include <ctime>

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
                                        std::string("./resources/UV_checker2k.png"),
                                        IMAGE_FORMAT::RGBA_8_SRGB,
                                        TextureMode::READ, SamplerData(),
                                        r.getResourceManager());

    std::cout << "creating material" << std::endl;
    auto material = r.getMaterials().createLambertMaterial(0, 100, texture);

    Handle<Geometry> quad =     GeometryBuilder::Quad(r.getResourceManager());
    //Handle<Geometry> pipe =     GeometryBuilder::Cylinder(r.getResourceManager(), 0.5, 2.0, 10, 32);
    Handle<Geometry> pipe =     GeometryBuilder::Icosahedron(r.getResourceManager());

    RenderScene scene({.name = "root scene"});

    RenderScene floor({
        .name = "pipe",
        .content = {.geometry = quad,
        .material = material},
        .position = glm::vec3(0, 0.0, 0),
        .rotation = glm::vec3(glm::radians(-90.0), 0, 0),
        .scale = glm::vec3(100, 1.0, 100.0)
    });

    RenderScene ico({
        .name = "pipe",
        .content = {.geometry = pipe,
        .material = material},
        .position = glm::vec3(0, -2, 0),
    });


    scene.add(&floor);
    scene.add(&ico);
    
    int ico_ring_count = 10;
    float ico_ring_dist = 8;
    std::vector<RenderScene> nodes;
    nodes.reserve(ico_ring_count);
    for(float i = 0.0f; i < glm::two_pi<float>(); i+=glm::two_pi<float>()/ico_ring_count){
        RenderScene ico_ring({
        .name = "pipe",
        .content = {.geometry = pipe,
        .material = material},
        .position = glm::vec3(glm::sin(i) * ico_ring_dist, -2, glm::cos(i) * ico_ring_dist),
        });
        nodes.push_back(ico_ring);
        scene.add(&nodes.back());
    }

    // Scene Description.
    std::cout << "creating deferred composer material" << std::endl;
    auto composer_material = r.getMaterials().createLambertDeferredComposeMaterial(1, 150u);
    
    Handle<LightArray> light_handle = r.createLightArray(9998);
    r.setLightArray(light_handle);
    auto& lights = r.getLightArray(light_handle);

        lights.addLight({
                        .position = glm::vec4(1, -5, 0, 0),
                        .color = glm::vec4(1.0, 1.0, 1.0, 0),
                        .intensity = 10,
                        });
    lights.update();
    
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.metadata(), 1, 0);
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.light_array(), 1, 1);
    
    r.getMaterialManager().printMaterial(composer_material);
    
    RenderScene composerNode({
        .name = "composer",
        .content = {.geometry = quad,
        .material = composer_material},
    });
    scene.add(&composerNode);

    BufferedCamera camera = r.createCamera({
                   .position = glm::float3(0,-50, -5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(10);
    
    uint32_t count = 0;
    float frame_TimeScale = 0.01;
    float dist = 4;
    auto phi = glm::golden_ratio<float>();
    std::cout << " setup done" << std::endl;
    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;

        lights[0].position = glm::vec4(dist * sin(t),
                                        lights[0].position.y ,
                                       dist * cos(t),
                                        0);
        lights.update();
        camera.setPosition(glm::float3(0,-10 * abs((sin(t/20))) -5, -5));
        camera.lookAt(glm::vec3(0));

        r.render_graph(scene, camera);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();


    return EXIT_SUCCESS;
}