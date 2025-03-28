#include "../renderer/Renderer.hpp"
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include "../renderer/modules/Camera.hpp"


#include <vector>

#include <chrono>
#include <thread>
#include <cstdlib>

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


    auto& bufferManager = r.getBufferManager();
    
    auto bufferSpace = bufferManager.reserveBuffer({
          .request = 1u << 4,
          .usage = BUFFER_USAGE::TRANSFER_DST,
          .sharing = SHARING_MODE::EXCLUSIVE,
      });

    int quantity = 10;
    std::vector<Handle<BufferAddress>> buffers(quantity);

    for (int i = 0; i < quantity; i++)
    {
        buffers[i] = bufferManager.reserveBuffer({
          .request = 1u << 10u ,
          .usage = BUFFER_USAGE::TRANSFER_DST,
          .sharing = SHARING_MODE::EXCLUSIVE,
      });
    }
    int a[1] = {100};

    bufferManager.copyToBuffer({.address = bufferSpace,
                                  .dataSize = static_cast<uint32_t>(sizeof(a)),
                                  .data = a, 
                                  });
    bufferManager.startBufferUpdates();
    bufferManager.queueingBufferUpdates();
    bufferManager.endBufferUpdates();

    //std::cout << "finished buffer updates" << std::endl;

    return EXIT_SUCCESS;
}