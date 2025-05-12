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
                .debug = false,
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

    auto scene = RenderScene::create_node({.name = "root scene"});
    std::vector<std::shared_ptr<RenderScene>> nodes;

    auto floor = RenderScene::create_node({
        .name = "pipe",
        .content = {.geometry = quad,
        .material = material},
        .position = glm::vec3(0, 0.0, 0),
        .rotation = glm::vec3(glm::radians(-90.0), 0, 0),
        .scale = glm::vec3(100, 1.0, 100.0)
    });

    auto ico = RenderScene::create_node({
        .name = "pipe",
        .content = {.geometry = pipe,
        .material = material},
        .position = glm::vec3(0, 0, 0),
    });

    std::srand(std::time({}));
    nodes.reserve(125);
    for(int k = 0; k < 5; k++)
        for(int i = 0; i < k; i++)
            for(int j = 0; j < k; j++){
                nodes.push_back(ico);
                nodes.back()->translate(glm::vec3(std::sin(std::rand()) * 25, 
                                                 -k,
                                                 std::sin(std::rand()) * 25));
                scene->add(nodes.back());
        }
    scene->add(floor);

    // Scene Description.
    std::cout << "creating deferred composer material" << std::endl;
    auto composer_material = r.getMaterials().createLambertDeferredComposeMaterial(1, 150u);
    
    Handle<LightArray> light_handle = r.createLightArray(9998);
    r.setLightArray(light_handle);
    auto& lights = r.getLightArray(light_handle);

    auto light_count = 100;
    auto light_count_over_two = light_count/2.0;
    for(int i = 0; i < light_count; i++)
        lights.addLight({
                        .position = glm::vec4(0, -(std::sin(std::rand())+2.0), 0, 0),
                        .color = glm::vec4(1.0, 1.0, 1.0, 0),
                        .intensity = 1,
                        });
    lights.update();
    
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.metadata(), 1, 0);
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.light_array(), 1, 1);
    
    r.getMaterialManager().printMaterial(composer_material);
    
    auto composerNode = RenderScene::create_node({
        .name = "composer",
        .content = {.geometry = quad,
        .material = composer_material},
    });
    scene->add(composerNode);

    BufferedCamera camera = r.createCamera({
                   .position = glm::float3(0,-50, -5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(10);
    
    uint32_t count = 0;
    float frame_TimeScale = 0.01;
    float dist = 20;
    auto phi = glm::golden_ratio<float>();
    std::cout << " setup done" << std::endl;
    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;
        for(int i = -light_count_over_two; i < light_count-light_count_over_two; i++)
            lights[i+light_count_over_two].position = glm::vec4(
                                            dist * ((sin((t * i)/25+0.001) * i / 20)),
                                            lights[i+light_count_over_two].position.y ,
                                             dist * ((cos((t * i)/25+0.001) * i / 20)),
                                            0);
        lights.update();
        camera.setPosition(glm::float3(0,-25 * abs((sin(t/20))) -10, -5));
        camera.lookAt(glm::vec3(0));

        r.render_graph(scene, camera);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();


    return EXIT_SUCCESS;
}