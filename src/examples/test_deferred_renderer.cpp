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
                .backBufferDesc2 = BackBufferDesc2{
                                        .dimensions = {windowWidth, windowHeight},
                                        .render_stages = {
                                            RenderStageDesc{
                                                .type = StageType::OBJECT_LIST,
                                                .attachments = {ATTACHMENT_TYPE::COLOR,
                                                                ATTACHMENT_TYPE::POSITION,
                                                                ATTACHMENT_TYPE::NORMAL,
                                                                ATTACHMENT_TYPE::DEPTH_STENCIL},
                                                .attachmentFormats = {  IMAGE_FORMAT::BGRA_8_SRGB,
                                                                        IMAGE_FORMAT::RGBA_32_SFLOAT,
                                                                        IMAGE_FORMAT::RGBA_32_SFLOAT,
                                                                        IMAGE_FORMAT::DEPTH_32_SFLOAT},
                                                .links = {},
                                            },
                                            RenderStageDesc{
                                                .type = StageType::SCREEN_QUAD,
                                                .attachments = {ATTACHMENT_TYPE::COLOR,},
                                                .attachmentFormats = {  IMAGE_FORMAT::BGRA_8_SRGB},
                                                .links = {
                                                    .attachToTexture = { 
                                                            AttachToTextureLink{0, 0, 0},
                                                            AttachToTextureLink{0, 1, 1},
                                                            AttachToTextureLink{0, 2, 2},},
                                                },
                                            },
                                        },
                                        .present_link = {1, 0},
                                        }

                });

    Handle<RenderTexture> texture = utils::TextureLoader::loadRenderTexture(std::string("./resources/UV_checker1k.png"),
     IMAGE_FORMAT::RGBA_8_SRGB,
     TextureMode::READ, SamplerData(),
     r.getResourceManager());

    std::cout << "creating bindings" << std::endl;
    auto textureBinding = r.getMaterialManager().createUnlitMaterialBindings();
    r.getMaterialManager().getBinding(textureBinding[1]).bindings[0].binding_handle.renderTex = texture;
    
    std::cout << "creating material" << std::endl;
    auto material = r.getMaterialManager().createUnlitMaterial(textureBinding);




    Handle<Geometry> quad = GeometryBuilder::Quad(r.getResourceManager());
    Handle<Geometry> triangle = GeometryBuilder::Triangle(r.getResourceManager());
    Handle<Geometry> circle = GeometryBuilder::Circle(r.getResourceManager(), 0.5f, 32);
    Handle<Geometry> pipe = GeometryBuilder::Pipe(r.getResourceManager(), 0.5, 2.0, 10, 32);

    std::cout << "creating scene node" << std::endl;
    SceneNode triangleNode({
        .name = "triangle",
        .geometry = pipe,
        .material = material,
        .position = glm::vec3(-3.0f, 0, 0),
    });
    SceneNode quadNode({
        .name = "quad",
        .geometry = pipe,
        .material = material,
        .position = glm::vec3(-1.5f, 0, 0),
    });

    SceneNode circleNode({
        .name = "circle",
        .geometry = pipe,
        .material = material,
        .position = glm::vec3(0.0f, 0, 0),
    });

    SceneNode pipeNode({
        .name = "pipe",
        .geometry = pipe,
        .material = material,
        .position = glm::vec3(1.5f, 0, 0),
    });

    // Scene Description.
    SceneNode scene({.name = "root scene"});
    scene.add(&pipeNode);
    scene.add(&triangleNode);
    scene.add(&quadNode);
    scene.add(&circleNode);

    Camera camera({
                   .position = glm::float3(0,-2,-5),
                   .aspect = static_cast<float>(windowWidth) / windowHeight,
                   });
    
    camera.lookAt(glm::vec3(0));
    boitatah::utils::Timewatch timewatch(1000);
    
    uint32_t count = 0;
    float frame_TimeScale = 0.001;
    float dist = 5;
    std::cout << "setup complete" << std::endl;
    r.waitIdle();
    while (!r.isWindowClosed())
    {
        //camera.orbit(glm::vec3(0), glm::vec3(0, 0.1f, 0));
        float t = count * frame_TimeScale;
        count++;
        camera.setPosition(glm::vec3(glm::cos(t) * dist, -2, -glm::sin(t) * dist));
        camera.lookAt(glm::vec3(0));
        r.render(scene, camera);
        ///camera.rotate(glm::vec3(0.0, 0.01, 0.0));
        std::cout << "\rFrametime :: " << timewatch.Lap() << "     " << std::flush;
    }
    r.waitIdle();


    return EXIT_SUCCESS;
}