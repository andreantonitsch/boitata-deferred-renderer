#pragma once
/// PRIVATE BOITATAH HEADER

#include "../collections/Pool.hpp"
#include <vector>
#include <string>
#include <memory>
namespace boitatah::buffer {

    struct Block{
        uint32_t id;
        uint32_t address;
        uint32_t size;
    };

    enum BinaryTreeNodeOccupation{
        FREE = 0,
        PARTIAL = 1,
        FULL = 2
    };

    struct BinaryTreeNode {
        //index of block data,
        uint32_t index;

        // children nodes.
        uint32_t leftChildIndex;
        uint32_t rightChildIndex;

        uint32_t largestFreeBlockSize;

        //FREE nodes are leaves;
        //1 partial;
        //FULL
        BinaryTreeNodeOccupation occupation;
    };

    struct BufferAllocatorDesc{
        uint32_t alignment;
        //must be a multiple alignment * constant
        uint32_t partitionSize;
        //tree depth
        uint32_t height;
    };

    class BufferAllocator{
        public:
            BufferAllocator(const BufferAllocatorDesc &desc);
            //~BufferAllocator(void);

            bool getBlockData(Handle<Block> &handle, uint32_t &offset, uint32_t &size);
            Handle<Block> allocate(uint32_t request);
            bool release(Handle<Block> &handle);

            uint32_t freeSpace();
            uint32_t getOccupiedSpace();
            uint32_t getLargestFreeBlockSize();
            uint32_t getPartitionSize();
            std::string coolPrint();

            uint32_t getSize();

        private:
            uint32_t alignment;
            uint32_t partitionSize;
            uint32_t size;
            uint32_t height;
            uint32_t leafQuant;

            uint32_t occupiedSpace;

            std::vector<Block> blocks;
            std::vector<BinaryTreeNode> nodes;

            std::unique_ptr<Pool<Block>> blockPool;

            //uint32_t getNodeBuddy(uint32_t index);
            uint32_t getNodeLevel(uint32_t index);

            uint32_t findFreeNode(uint32_t request);

            void upstreamOccupationCorrect(uint32_t index);

            uint32_t getBlockIndexFromId(uint32_t id);

            uint32_t getMinimunFitSize(uint32_t request);

            std::vector<uint32_t> getLeaves(uint32_t maxDepth);

            BinaryTreeNodeOccupation occupationFromChildrenNodes(uint32_t index);


    };


}
