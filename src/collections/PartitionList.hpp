#ifndef BOITATAH_FREELIST_HPP
#define BOITATAH_FREELIST_HPP

#include <cstdint>
#include <vector>
/// List for free space in a buffer.



namespace boitatah{

    struct PartitionHandle{
        uint32_t i;
        uint32_t gen;
    };

    struct FreeListDesc{
        uint32_t size;
        uint32_t minPartitionSize;
        uint32_t maxPartition;
        uint32_t maxPartitions;
    };
    
    struct PartitionNode {
        PartitionNode* next;
        PartitionNode* previous;
        Partition* partition;
    };

    struct Partition {
        uint32_t size;
        uint32_t address;
        bool free;
    };

    class PartitionList{
        public:
            PartitionList(FreeListDesc desc);
            ~PartitionList(void);

            Partition fetch(PartitionHandle handle);
            PartitionHandle createPartition(uint32_t partitionSize);
            bool freePartition(PartitionHandle handle);

        private:
            PartitionNode* root;
            FreeListDesc options;

            std::vector<Partition> partitions;
            std::vector<uint32_t> freeIds;
            std::vector<uint32_t> generations;

    };

}


#endif // BOITATAH_FREELIST_HPP