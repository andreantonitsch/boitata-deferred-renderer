#include "../renderer/Renderer.hpp"
#include <iostream>
#include <memory>

// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"

using namespace boitatah;
using Boitatah = Renderer;
int main()
{

    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    Boitatah r(RendererOptions{
                .windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = IMAGE_FORMAT::BGRA_8_UNORM,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {IMAGE_FORMAT::BGRA_8_UNORM},
                                   .dimensions = {windowWidth, windowHeight}}});


    auto& objManager = r.getResourceManager();

    uint32_t currentFrame = 0u;

    auto exclusiveBufferHandle = objManager.create(GPUBufferCreateDescription{
        .size = 1024u,
        .usage = BUFFER_USAGE::VERTEX,
        .sharing_mode = SHARING_MODE::EXCLUSIVE,
    });

    std::cout << "created exclusive gpu buffer" << std::endl;

    auto sharedBufferHandle = objManager.create(GPUBufferCreateDescription{
        .size = 1024u,
        .usage = BUFFER_USAGE::VERTEX,
        .sharing_mode = SHARING_MODE::CONCURRENT,
    });

    std::cout << "created inclusive gpu buffer" << std::endl;

    auto& exclusiveBuffer = objManager.getResource(exclusiveBufferHandle);
    auto& sharedBuffer = objManager.getResource(sharedBufferHandle);
    
    //uint32_t a[] = {1,2,3,4,5,6,7,8,9,10};
    auto a = std::array{1, 2, 4, 8, 16, 32};

    exclusiveBuffer.copyData(a.data(), sizeof(uint32_t) * a.size());
    sharedBuffer.copyData(a.data(), sizeof(uint32_t) * a.size());

    
    auto& content = exclusiveBuffer.get_content(0);
    Buffer* buffer;
    auto gotBuffer = objManager.getBufferManager()->getAddressBuffer(content.buffer, buffer);

    std::cout << "got buffer : " << gotBuffer << " " << buffer->getID();

    objManager.forceCommitResource(exclusiveBufferHandle, currentFrame);
    objManager.waitForTransfers();

    //a[0], a[1], a[2] = 4, 5, 7;

    exclusiveBuffer.copyData(a.data(), sizeof(uint32_t) * a.size());

    //objManager.commitAll(currentFrame);
    //objManager.waitForTransfers();
    //a[0], a[1], a[2] = 8, 9, 10;



    objManager.beginCommitCommands();
    objManager.commitResourceCommand(exclusiveBufferHandle, currentFrame);
    objManager.commitResourceCommand(exclusiveBufferHandle, currentFrame+1);
    objManager.submitCommitCommands();
    //objManager.waitForTransfers();

    SceneNode triangle({
        .name = "triangle"
    });
    r.render(triangle);
    objManager.destroy(exclusiveBufferHandle);
    objManager.destroy(sharedBufferHandle);



    return EXIT_SUCCESS;
}