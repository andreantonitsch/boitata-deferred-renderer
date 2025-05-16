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

int main(){
    /// Window Dimensions
    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    /// Constructs the Renderer
    Renderer r({
        .windowDimensions = {windowWidth, windowHeight},
        .appName = "Test Frame Buffer",
        /// Wether to use debug validation layers. high performance cost.
        .debug = true,
        /// format for the present swapchain.
        .swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB,
        /// A default render graph backbuffer.
        /// Implements a basic deferred pipeline. 
        .backBufferDesc = BackBufferManager::BasicDeferredPipeline(windowWidth,
                                                                    windowHeight)});

    /// Loads a texture
    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(
                                        std::string("./resources/UV_checker2k.png"),
                                        IMAGE_FORMAT::RGBA_8_SRGB,
                                        TextureMode::READ, SamplerData(),
                                        r.getResourceManager());

    /// Creates a default Lambertian model lit material
    /// This material renders on the stage 0 of the pipeline.
    auto material = r.getMaterials().createLambertMaterial(0, 100, texture);

    /// Creates some base geometries.
    Handle<Geometry> quad =     GeometryBuilder::Quad(r.getResourceManager());
    //Handle<Geometry> geo =     GeometryBuilder::Cylinder(r.getResourceManager(), 0.5, 2.0, 10, 32);
    //Handle<Geometry> geo =     GeometryBuilder::Icosahedron(r.getResourceManager());
    Handle<Geometry> geo =     GeometryBuilder::Sphere(r.getResourceManager(), 2.0f, 20);
    
    /// Creates a geometry for the floor
    auto floor= RenderScene::create_node({
        .name = "floor",
        .content = {.geometry = quad,
        .material = material},
        .position = glm::vec3(0, 0.0, 0),
        .rotation = glm::vec3(glm::radians(-90.0), 0, 0),
        .scale = glm::vec3(100, 1.0, 100.0)
    });

    /// Creates an object for the object geometry
    auto obj = RenderScene::create_node({
        .name = "object",
        .content = {.geometry = geo,
        .material = material},
        .position = glm::vec3(0, -2, 0),
    });

    /// Creates a root SceneTree and adds the objects to it
    auto scene = RenderScene::create_node({.name = "root scene"});
    scene->add(floor);
    scene->add(obj);
    
    /// Adds a ring of objects to the root scene
    int ico_ring_count = 10;
    float ico_ring_dist = 8;
    std::vector<std::shared_ptr<RenderScene>> nodes(ico_ring_count);
    for(float i = 0.0f; i < glm::two_pi<float>(); i+=glm::two_pi<float>()/ico_ring_count){
        nodes.push_back(RenderScene::create_node({
        .name = "pipe",
        .content = {.geometry = geo,
        .material = material},
        .position = glm::vec3(glm::sin(i) * ico_ring_dist, -2, glm::cos(i) * ico_ring_dist),
        }));
        scene->add(nodes.back());
    }

    /// Creates a Light Array.
    Handle<LightArray> light_handle = r.createLightArray(9998);
    r.set_light_array(light_handle);
    auto& lights = r.getLightArray(light_handle);

    /// Add one light to it.
    lights.addLight({
                    .position = glm::vec4(1, -5, 0, 0),
                    .color = glm::vec4(1.0, 1.0, 1.0, 0),
                    .intensity = 10,
                    });
    /// Updates the LightArray
    lights.update();

    /// The deferred Renderer Material.
    /// Defered pipelines need a pass to compose the scene data into a final image
    /// This material renders on the stage 1 of the pipeline
    auto composer_material = r.getMaterials().createLambertDeferredComposeMaterial(1, 150u);
    
    /// Binds the lights to the composer material.
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.metadata(), 1, 0);
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.light_array(), 1, 1);
    
    r.getMaterialManager().printMaterial(composer_material);
    
    /// Finally create an SceneTree object for the composer and add it to the scene.
    auto composerNode = RenderScene::create_node({
        .name = "composer",
        .content = {.geometry = quad,
        .material = composer_material},
    });
    scene->add(composerNode);

    /// Create a camera and point it

    BufferedCamera camera = r.create_camera({
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

    /// While there is a window to display to
    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;
        
        /// Move the Light and update the array
        lights[0].position = glm::vec4(dist * sin(t),
                                        lights[0].position.y ,
                                       dist * cos(t),
                                        0);
        lights[0].intensity = 8 * (sin(t / 2) * 0.5f + 0.5f) + 2;
        lights.update();
        camera.setPosition(glm::float3(0,-10 * ((sin(t/20))*0.5f + 1.0f) -5, -5));
        camera.lookAt(glm::vec3(0));

        /// Renders the root SceneTree
        r.render_tree(scene, camera);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }

    /// Waits for idle GPU and cleans up the loaded data.
    r.waitIdle();
    r.getResourceManager().destroy(texture);
    r.getResourceManager().destroy(quad);
    r.getResourceManager().destroy(geo);

    return EXIT_SUCCESS;
}