#include "../buffers/BufferAllocator.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include <vector>
#include <random>

int main()
{
    using namespace boitatah;
    using namespace boitatah::buffer;
    BufferAllocatorDesc desc{};
    desc.alignment = 16u;
    desc.height = 9u;
    desc.partitionSize = desc.alignment * 128u;

    BufferAllocator allocator(desc);
    int adds = 100000000;
    int quant = 0;
    
    std::cout << "Initialized buffer with : " << allocator.getSize() << std::endl;

    int maxSize = 4096;
    int minSize = 64;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distribution(minSize, maxSize);

    std::uniform_int_distribution<std::mt19937::result_type> yesno(0, 2);
    std::uniform_int_distribution<std::mt19937::result_type> removeThing(0, (adds / 64) - 1);

    std::vector<Handle<Block>> partitions;

    utils::Timewatch watch(100);
    
    double occupations = 0.0f;
    float size = static_cast<float>(allocator.getSize());
    int failed = 0;

    for (int i = 0; i < adds; i++)
    {
        occupations += allocator.getOccupiedSpace();
        if (yesno(rng) > 0)
        {
            auto handle = allocator.allocate(distribution(rng));
            if (!handle.isNull()){
                partitions.push_back(handle);
            }
            else{
                failed ++ ;
            }
        }
        else
        {
            if (partitions.size() > 0)
            {

                auto removeIndex = removeThing(rng) % partitions.size();
                auto handle = partitions[removeIndex];

                bool result = allocator.release(handle);

                if (result)
                    partitions.erase(partitions.begin() + removeIndex);
            }
        }
        if (i % 10000 == 0)
            std::cout << "\r" << allocator.coolPrint()  << "   "
             << i << " " << (occupations / i) / size << " "
             << watch.Lap() /10000 << std::flush;
    }

    // std::cout << "Loop Ended" << std::endl;
}