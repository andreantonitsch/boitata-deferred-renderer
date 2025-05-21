#include <iostream>
#include <memory>

#include <boitatah/Renderer.hpp>
#include <boitatah/BoitatahEnums.hpp>
#include <boitatah/types/Shader.hpp>
#include <boitatah/utils/utils.hpp>
#include <boitatah/collections/Pool.hpp>

#include <boitatah/resources/builders/GeometryBuilder.hpp>
#include <boitatah/utils/ImageLoader.hpp>

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
        /// Format for the present swapchain.
        .swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB,
        /// A default render graph backbuffer.
        /// Writers  to 4 attachments (color, position, normal, depth/stencil)
        .backBufferDesc = BackBufferManager::BasicMultiWriteForwardPipeline(windowWidth,
                                                                            windowHeight,
                                                                            1)});

    /// Loads a texture
    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(
                                        std::string("./resources/UV_checker1k.png"),
                                        IMAGE_FORMAT::RGBA_8_SRGB,
                                        TextureMode::READ, SamplerData(),
                                        r.getResourceManager());

    /// Creates a default unlit material
    auto material = r.getMaterials().createUnlitMaterial(0, 100, texture);

    /// Creates some base geometries.
    Handle<Geometry> triangle = GeometryBuilder::Triangle(r.getResourceManager());
    Handle<Geometry> quad =     GeometryBuilder::Quad(r.getResourceManager());
    Handle<Geometry> circle =   GeometryBuilder::Circle(r.getResourceManager(), 0.5f, 32);
    Handle<Geometry> pipe =     GeometryBuilder::Pipe(r.getResourceManager(), 0.5, 2.0, 10, 32);

    /// Creates SceneTree nodes for the objects
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

    /// Creates a root scene
    std::shared_ptr<RenderScene> scene = RenderScene::create_node({.name = "root scene"});

    /// adds objects to the root scene.
    scene->add(pipeNode);
    scene->add(triangleNode);
    scene->add(quadNode);
    scene->add(circleNode);


    /// create a camera and points it to the system origin
    BufferedCamera camera = r.create_camera({
                   .position = glm::float3(0,-2,-5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));

    /// timewatch for frametime measuring
    boitatah::utils::Timewatch timewatch(1000);

    uint32_t count = 0;
    float frame_TimeScale = 0.001;
    float dist = 5;
    std::cout << "setup complete" << std::endl;

    /// While the renderer window is still being displayed
    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;
        /// Moves the camera a bit
        camera.setPosition(glm::vec3(glm::cos(t) * dist, -2, -glm::sin(t) * dist));
        camera.lookAt(glm::vec3(0));

        /// Renders the root scene SceneTree
        r.render_tree(scene, camera);

        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }

    /// Waits for the GPU to idle before destructors
    /// cleaning up everything and close the renderer
    r.waitIdle();

    r.getResourceManager().destroy(quad);
    r.getResourceManager().destroy(triangle);
    r.getResourceManager().destroy(circle);
    r.getResourceManager().destroy(pipe);
    r.getResourceManager().destroy(texture);
    return EXIT_SUCCESS;
}