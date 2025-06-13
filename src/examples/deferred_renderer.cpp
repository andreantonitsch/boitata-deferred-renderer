#include <memory>
#include <iostream>

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
    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                /// Wether to use debug validation layers. high performance cost.
                .debug = true,
                /// Format for the present swapchain.
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB,
                /// A default render graph backbuffer.
                /// Implements a basic deferred pipeline. 
                .backBufferDesc = BackBufferManager::BasicDeferredPipeline(windowWidth,
                                                                            windowHeight)});
    /// Loads a texture
    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(
                                        std::string("./resources/UV_checker1k.png"),
                                        IMAGE_FORMAT::RGBA_8_SRGB,
                                        TextureMode::READ, SamplerData(),
                                        r.getResourceManager());

    /// Creates a unlit material
    /// This material renders on the stage 0 of the pipeline.
    auto material = r.getMaterials().createUnlitMaterial(0, 100, texture);

    /// Creates some base geometries.
    Handle<Geometry> triangle = GeometryBuilder::Triangle(r.getResourceManager());
    Handle<Geometry> quad =     GeometryBuilder::Quad(r.getResourceManager());
    Handle<Geometry> circle =   GeometryBuilder::Circle(r.getResourceManager(), 0.5f, 32);
    Handle<Geometry> pipe =     GeometryBuilder::Pipe(r.getResourceManager(), 0.5, 2.0, 10, 32);

    auto triangleNode = RenderScene::create_node({
        .name = "triangle",
        .content = RenderObject{.geometry = triangle,
                    .material = material,},
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

    /// Create a root SceneTree and add nodes to it.
    auto scene = RenderScene::create_node({.name = "root scene"});
    scene->add(pipeNode);
    scene->add(triangleNode);
    scene->add(quadNode);
    scene->add(circleNode);

    /// Creates the deferred composer material, and add its node to the scene.
    auto composer_material = r.getMaterials().createUnlitDeferredComposeMaterial(1, 150u);

    auto composerNode = RenderScene::create_node({
        .name = "composer",
        .content{.geometry = quad,
        .material = composer_material},
    });
    scene->add(composerNode);

    /// create a camera and point it
    BufferedCamera camera = r.create_camera({
                   .position = glm::float3(0,-2,-5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));

    boitatah::utils::Timewatch timewatch(1000);
    uint32_t count = 0;
    float frame_TimeScale = 0.001;
    float dist = 5;

    /// While there is awindow to render to
    while (!r.isWindowClosed())
    {
        float t = count * frame_TimeScale;
        count++;
        /// Move the camera
        camera.setPosition(glm::vec3(glm::cos(t) * dist, -2, -glm::sin(t) * dist));
        camera.lookAt(glm::vec3(0));

        /// Render the Scene.
        r.render_tree(scene, camera);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    
    /// Waits for idle GPU and cleans up the loaded data.
    r.waitIdle();
    r.getResourceManager().destroy(texture);
    r.getResourceManager().destroy(quad);
    r.getResourceManager().destroy(pipe);
    r.getResourceManager().destroy(circle);
    r.getResourceManager().destroy(triangle);

    return EXIT_SUCCESS;
}