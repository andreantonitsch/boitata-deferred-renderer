#ifndef BOITATAH_FREELIST_HPP
#define BOITATAH_FREELIST_HPP

#include <cstdint>
#include <vector>
#include <string>
#include "Pool.hpp"
/// List for free space in a buffer.



namespace boitatah{

    struct Partition;
    
    struct FreeListDesc{
        uint32_t size;
        uint32_t minPartitionSize;
        uint32_t maxPartitions;
        bool dynamic;
    };
    
    struct PartitionNode {
        PartitionNode* next;
        PartitionNode* previous;
        Handle<Partition> partition;
        bool free;
    };

    struct FreeNode{
        PartitionNode* node;
        FreeNode* next;
        FreeNode* previous;
    };

    struct Partition {
        uint32_t size; //in bytes
        uint32_t address; //in bytes

        // TODO Change this to Handle for safety
        PartitionNode* m_Node;

        // TODO add isNull() method
    };

    class PartitionList{
        public:
            PartitionList(FreeListDesc &desc);
            ~PartitionList(void);

            Partition fetch(Handle<Partition> handle);
            Handle<Partition> allocate(uint32_t partitionSize);
            bool release(Handle<Partition> handle);

            uint32_t getOccupiedSpace();
            std::string coolPrint();

        private:
            FreeListDesc options;

            PartitionNode* root;
            PartitionNode* last;
            PartitionNode* current;

            FreeNode* firstFree;


            //add arrays with max partitionnodes?
            //for faster caching - new instantiation
            
            uint32_t totalOccupation;

            Pool<Partition> partitionPool;

            PartitionNode* findNodeWithSpace(uint32_t request);

            PartitionNode* partitionize(PartitionNode* targetPartition, uint32_t size);
    
            PartitionNode* agglutinateNodes(PartitionNode* targetNode);

    };

}


#endif // BOITATAH_FREELIST_HPP