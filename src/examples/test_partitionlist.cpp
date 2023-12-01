#include "../collections/PartitionList.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"
#include <vector>
#include <random>
int main()
{
    using namespace boitatah;

    uint32_t size = 1000000;

    FreeListDesc desc{.size = size,
                      .minPartitionSize = 64,
                      .maxPartitions = 1000000 / 64,
                      .dynamic = false};

    PartitionList list(desc);

    int adds = 100000000;
    int quant = 0;

    int maxSize = 4096;
    int minSize = 64;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distribution(minSize, maxSize);

    std::uniform_int_distribution<std::mt19937::result_type> yesno(0, 2);
    std::uniform_int_distribution<std::mt19937::result_type> removeThing(0, (adds / 64) - 1);

    std::vector<Handle<Partition>> partitions;

    utils::Timewatch watch(100);

    auto test1 = list.allocate(1024);
    list.release(test1);

    float occupations = 0.0f;

    for (int i = 0; i < adds; i++)
    {
        float avg = static_cast<float>(list.getOccupiedSpace()) / static_cast<float>(size);
        occupations += avg;
        if (yesno(rng) > 0)
        {
            auto handle = list.allocate(distribution(rng));
            if (!handle.isNull())
                partitions.push_back(handle);
        }
        else
        {
            if (partitions.size() > 0)
            {

                auto removeIndex = removeThing(rng) % partitions.size();
                auto handle = partitions[removeIndex];

                bool result = list.release(handle);

                if (result)
                    partitions.erase(partitions.begin() + removeIndex);
            }
        }

        if (i % 10000 == 0)
            //std::cout << "\r" << list.coolPrint() << "  " << avg << "   " << occupations / i << " lap  " << watch.Lap() / 10000 <<  std::flush;
            std::cout << "\r "  << avg << "   " << occupations / i << " lap  " << watch.Lap() / 10000 <<  std::flush;
    }

    // std::cout << "Loop Ended" << std::endl;
}