#include <boitatah/Renderer.hpp>
#include <iostream>
// #include <unistd.h>
#include <boitatah/BoitatahEnums.hpp>
#include <boitatah/types/Shader.hpp>
#include <boitatah/utils/utils.hpp>
#include <boitatah/collections/Pool.hpp>

#include <boitatah/resources/builders/GeometryBuilder.hpp>
#include <boitatah/utils/ImageLoader.hpp>
#include <memory>
#include <ctime>

using namespace boitatah;

int main(){
    /// Window Dimensions
    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    /// Constructs the Renderer
    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                /// Wether to use debug validation layers. high performance cost.
                .debug = false,
                /// Format for the present swapchain.
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB,
                /// A default render graph backbuffer.
                /// Implements a basic deferred pipeline. 
                .backBufferDesc = BackBufferManager::BasicDeferredPipeline(windowWidth,
                                                                            windowHeight)
                });

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
    //Handle<Geometry> obj =     GeometryBuilder::Cylinder(r.getResourceManager(), 0.5, 2.0, 10, 32);
    // Handle<Geometry> obj =     GeometryBuilder::Icosahedron(r.getResourceManager());
    Handle<Geometry> geo = GeometryBuilder::Sphere(r.getResourceManager(), 2.0f, 20);

    ///Creates a root SceneTree
    auto scene = RenderScene::create_node({.name = "root scene"});
    std::vector<std::shared_ptr<RenderScene>> nodes;

    auto floor = RenderScene::create_node({
        .name = "ground",
        .content = {.geometry = quad,
        .material = material},
        .position = glm::vec3(0, 0.0, 0),
        .rotation = glm::vec3(glm::radians(-90.0), 0, 0),
        .scale = glm::vec3(100, 1.0, 100.0)});
    scene->add(floor);

    /// Creates a template to base the objects on
    auto obj = RenderScene::create_node({
        .name = "obj",
        .content = {.geometry = geo,
        .material = material},
        .position = glm::vec3(0, 0, 0)});

    /// Add copies of obj spread around the scene.
    std::srand(std::time({}));
    nodes.reserve(125);
    for(int k = 0; k < 5; k++)
        for(int i = 0; i < k; i++)
            for(int j = 0; j < k; j++){
                nodes.push_back(obj);
                nodes.back()->translate(glm::vec3(std::sin(std::rand()) * 25, 
                                                 -k,
                                                 std::sin(std::rand()) * 25));
                scene->add(nodes.back());}
    
    /// The deferred Renderer Material.
    /// Defered pipelines need a pass to compose the scene data into a final image
    /// This material renders on the stage 1 of the pipeline
    auto composer_material = r.getMaterials().createLambertDeferredComposeMaterial(1, 150u);
    
    /// Creates a Light Array.
    Handle<LightArray> light_handle = r.createLightArray(9998);
    r.set_light_array(light_handle);
    auto& lights = r.getLightArray(light_handle);

    /// Add lights to then update the array.
    auto light_count = 100;
    auto light_count_over_two = light_count/2.0;
    for(int i = 0; i < light_count; i++)
        lights.addLight({
                        .position = glm::vec4(0, -(std::sin(std::rand())+2.0), 0, 0),
                        .color = glm::vec4(1.0, 1.0, 1.0, 0),
                        .intensity = 1,
                        });
    lights.update();
    
    /// binds the lights to the composer material.
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.metadata(), 1, 0);
    r.getMaterialManager().setBufferBindingAttribute(composer_material, lights.light_array(), 1, 1);
    
    r.getMaterialManager().printMaterial(composer_material);
    
    /// Adds the composer node object to the scene.
    auto composerNode = RenderScene::create_node({
        .name = "composer",
        .content = {.geometry = quad,
        .material = composer_material},
    });
    scene->add(composerNode);

    /// Creates the camera and points it
    BufferedCamera camera = r.create_camera({
                   .position = glm::float3(0,-50, -5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    camera.lookAt(glm::vec3(0));

    boitatah::utils::Timewatch timewatch(10);

    uint32_t count = 0;
    float frame_TimeScale = 0.01;
    float dist = 20;
    auto phi = glm::golden_ratio<float>();

    /// While the render window is open.
    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;

        // Moves the lights and update the array
        for(int i = -light_count_over_two; i < light_count-light_count_over_two; i++)
            lights[i+light_count_over_two].position = glm::vec4(
                                            dist * ((sin((t * i)/25+0.001) * i / 20)),
                                            lights[i+light_count_over_two].position.y ,
                                             dist * ((cos((t * i)/25+0.001) * i / 20)),
                                            0);
        lights.update();

        /// Moves the camera and repoints it.
        camera.setPosition(glm::float3(0,-25 * abs((sin(t/20))) -10, -5));
        camera.lookAt(glm::vec3(0));

        /// Renders the scene.
        r.render_tree(scene, camera);
    }
    
    /// Waits for idle GPU and cleans up the loaded data.
    r.waitIdle();
    r.getResourceManager().destroy(texture);
    r.getResourceManager().destroy(quad);
    r.getResourceManager().destroy(geo);
    
    return EXIT_SUCCESS;
}