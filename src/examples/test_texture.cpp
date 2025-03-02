#include <renderer/Renderer.hpp>
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include "../renderer/modules/Camera.hpp"

#include <renderer/resources/builders/GeometryBuilder.hpp>

#include <utils/ImageLoader.hpp>

using namespace boitatah;

int main()
{

    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    Renderer r({.windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = false,
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {IMAGE_FORMAT::BGRA_8_SRGB},
                                   .dimensions = {windowWidth, windowHeight}}});

    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(std::string("./resources/UV_checker2k.png"),
     IMAGE_FORMAT::RGBA_8_SRGB,
     TextureMode::READ, SamplerData(),
     r.getResourceManager());

    std::cout << "creating bindings" << std::endl;
    auto textureBinding = r.getMaterialManager().createUnlitMaterialBindings();
    r.getMaterialManager().getBinding(textureBinding[1]).bindings[0].binding_handle.renderTex = texture;
    


    std::cout << "creating material" << std::endl;
    auto material = r.getMaterialManager().createUnlitMaterial(textureBinding);
    Handle<Geometry> geometry = GeometryBuilder::Quad(r.getResourceManager());

    std::cout << "creating scene node" << std::endl;
    SceneNode triangle({
        .name = "triangle",
        .geometry = geometry,
        .material = material,
    });

    // Scene Description.
    SceneNode scene({.name = "root scene"});
    scene.add(&triangle);

    Camera camera({
                   .position = glm::float3(0,0,-1),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   
                   });

    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(1000);

    std::cout << "setup complete" << std::endl;
    r.waitIdle();
    while (!r.isWindowClosed())
    {
        r.render(scene, camera);
        ///camera.rotate(glm::vec3(0.0, 0.01, 0.0));
        camera.roll(0.0001);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();

    return EXIT_SUCCESS;
}