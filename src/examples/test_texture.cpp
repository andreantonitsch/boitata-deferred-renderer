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
                .debug = true,
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_UNORM,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {IMAGE_FORMAT::BGRA_8_UNORM},
                                   .dimensions = {windowWidth, windowHeight}}});

    auto& descManager = r.getDescriptorManager();
    auto setLayout = descManager.getLayout({
        .bindingDescriptors = {{
            .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
            .stages = STAGE_FLAG::ALL_GRAPHICS,
            .descriptorCount = 1,
        }}
    });

    // Pipeline Layout for the Shader.
    Handle<ShaderLayout> layout = r.getMaterialManager().getShaderManager().makeShaderLayout({
        .setLayouts = {setLayout},
        }
    );

    Handle<Shader> shader = r.createShader({.name = "test",
                                            .vert = {
                                                .byteCode = utils::readFile("./src/camera_vert.spv"),
                                                .entryFunction = "main"},
                                            .frag = {.byteCode = utils::readFile("./src/camera_frag.spv"), .entryFunction = "main"},
                                            .renderPass =     r.getBackBufferRenderPass(),
                                            .layout = layout,
                                            .vertexBindings = {
                                                {.stride = 12, .attributes = {{.location = 0, .format = IMAGE_FORMAT::RGB_32_SFLOAT, .offset = 0}}},
                                                {.stride = 12, .attributes = {{.location = 1, .format = IMAGE_FORMAT::RGB_32_SFLOAT, .offset = 0}}},
                                                {.stride = 8, .attributes = {{.location = 2, .format = IMAGE_FORMAT::RG_32_SFLOAT, .offset = 0}}},
                                                }});



    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(std::string("./resources/viking_room.png"),
     IMAGE_FORMAT::BGRA_8_SRGB,
     TextureMode::READ, SamplerData(),
     r.getResourceManager());

    auto material = r.createMaterial({
        .shader = shader,
        .bindings = {},
        .vertexBufferBindings = {VERTEX_BUFFER_TYPE::POSITION, 
                                 VERTEX_BUFFER_TYPE::COLOR,
                                 VERTEX_BUFFER_TYPE::UV,
                                 },
        .name = "material test"
    });
    Handle<Geometry> geometry = GeometryBuilder::Quad(r.getResourceManager());

    SceneNode triangle({
        .name = "triangle",
        .geometry = geometry,
        .material = material,
    });

    // Scene Description.
    SceneNode scene({.name = "root scene"});
    scene.add(&triangle);

    Camera camera({
                   .position = glm::float3(0,0,-10),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   
                   });

    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(1000);

    std::cout << "setup complete" << std::endl;

    //while (!r.isWindowClosed())
    //{
        r.render(scene, camera);
        ///camera.rotate(glm::vec3(0.0, 0.01, 0.0));
        camera.roll(0.01);
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    //}
    r.waitIdle();

    r.destroyLayout(layout);
    r.destroyShader(shader);

    return EXIT_SUCCESS;
}