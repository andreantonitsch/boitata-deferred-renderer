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
                .swapchainFormat = FORMAT::BGRA_8_UNORM,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {FORMAT::BGRA_8_UNORM},
                                   .dimensions = {windowWidth, windowHeight}}});


    auto& objManager = r.getResourceManager();

    uint32_t currentFrame = 0u;

    auto exclusiveBufferHandle = objManager.create(GPUBufferCreateDescription{
        .size = 1024u,
        .usage = BUFFER_USAGE::TRANSFER_DST_VERTEX,
        .sharing_mode = SHARING_MODE::EXCLUSIVE,
    });

        std::cout << "created exclusive gpu buffer" << std::endl;

    auto sharedBufferHandle = objManager.create(GPUBufferCreateDescription{
        .size = 1024u,
        .usage = BUFFER_USAGE::TRANSFER_DST_VERTEX,
        .sharing_mode = SHARING_MODE::CONCURRENT,
    });

    std::cout << "created inclusive gpu buffer" << std::endl;

    auto& exclusiveBuffer = objManager.getResource(exclusiveBufferHandle);
    auto& sharedBuffer = objManager.getResource(sharedBufferHandle);
    
    int a[] = {1,2,3};

    exclusiveBuffer.copyData(a);
    sharedBuffer.copyData(a);


    objManager.forceCommitResource(exclusiveBufferHandle, currentFrame);

    a[0], a[1], a[2] = 4, 5, 7;

    exclusiveBuffer.copyData(a);

    objManager.commitAll(currentFrame);

    a[0], a[1], a[2] = 8, 9, 10;


    objManager.beginCommitCommands();
    objManager.commitResourceCommand(exclusiveBufferHandle, currentFrame);
    objManager.submitCommitCommands();


    objManager.destroy(exclusiveBufferHandle);
    objManager.destroy(sharedBufferHandle);

    return EXIT_SUCCESS;
}