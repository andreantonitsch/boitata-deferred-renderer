#include "BufferAllocator.hpp"
#include "math.h"
#include <bit>
#include <algorithm>

namespace boitatah::buffer
{

    BufferAllocator::BufferAllocator(const BufferAllocatorDesc &desc) 
    {
        alignment = desc.alignment;
        
        partitionSize = desc.partitionSize % alignment == 0 ?
                        desc.partitionSize :
                        ((desc.partitionSize / alignment) + 1u)* alignment;

        height = desc.height;

        size = partitionSize * (1u << height);


        //while buffer is larger than 100mb
        while(size > 100000000){
            height--;
            size = desc.partitionSize * (1u << height);
        }

        leafQuant = size / partitionSize;

        blockPool = std::make_unique<Pool<Block>>(PoolOptions{.size = leafQuant * 2 });
        blocks.resize(leafQuant * 2 );
        nodes.resize(leafQuant * 2 );

        for (uint32_t i = 0; i < blocks.size(); i++)
        {
            uint32_t id = i + 1u;
            // Exact same thing as log2
            uint32_t depth = std::bit_width(id) - 1u;

            uint32_t pow_depth = 1 << depth;
            uint32_t id_depth = id - pow_depth;

            Block block{
                .id = id,
                .address = (size / pow_depth) * id_depth,
                .size = partitionSize << (height - depth)};

            BinaryTreeNode node{
                .index = i,
                .leftChildIndex = block.id * 2 - 1,
                .rightChildIndex = block.id * 2,
                .largestFreeBlockSize = block.size,
                .occupation = FREE,
            };

            blocks[i] = block;
            nodes[i] = node;
        }
    }

    bool BufferAllocator::getBlockData(Handle<Block> &handle, uint32_t &offset, uint32_t &size)
    {
        Block block;
        if (!blockPool->tryGet(handle, block))
            return false;

        offset = block.address;
        size = block.size;

        return true;
    }

    Handle<Block> BufferAllocator::allocate(uint32_t request)
    {
        // << "allocating in allocator" << std::endl;
        uint32_t available_index = findFreeNode(request);
        // std::cout << "available node " << available_index << std::endl;
        //  failure case. Full tree
        //std::cout << "allocated in allocator" << std::endl;
        if (available_index == UINT32_MAX)
            return Handle<Block>();

        nodes[available_index].occupation = FULL;
        nodes[available_index].largestFreeBlockSize = 0;
        upstreamOccupationCorrect(available_index);
        //std::cout << "allocator corrected" << std::endl;
        occupiedSpace += blocks[available_index].size;

        return blockPool->set(blocks[available_index]);
    }

    // Corrects tree from this node upwards.
    void BufferAllocator::upstreamOccupationCorrect(uint32_t index)
    {
        int n = static_cast<int>(index);

        // if not root. correct upwards;
        while (n > 0)
        {
            if (n % 2 == 0)
            {
                n = (n - 1) / 2;
            }
            else
            {
                n = n / 2;
            }

            uint32_t p = static_cast<uint32_t>(n);
            auto& node = nodes[p];
            node.occupation = occupationFromChildrenNodes(p);
            if(node.occupation == PARTIAL){
                node.largestFreeBlockSize = std::max(
                                nodes[node.leftChildIndex].largestFreeBlockSize,
                                nodes[node.rightChildIndex].largestFreeBlockSize);
            } else if(node.occupation == FULL){
                node.largestFreeBlockSize = static_cast<uint32_t>(0);
            } else if(node.occupation == FREE){
                node.largestFreeBlockSize = blocks[node.index].size;
            }
        }
    }

    inline uint32_t BufferAllocator::getBlockIndexFromId(uint32_t id)
    {
        return id - 1u;
    }

    // Minimun Partition Block that fits this request
    uint32_t BufferAllocator::getMinimunFitSize(uint32_t request)
    {
        uint32_t depth = height; // start at bottom to figure out correct depth
        // (2^level) * partitionSize  < request 2^(level-1) * partitionSize
        uint32_t fitSize = partitionSize << (height - depth);
        while (fitSize < request)
        {
            depth--;
            fitSize = partitionSize << (height - depth);
        }

        return fitSize;
    }

    std::vector<uint32_t> BufferAllocator::getLeaves(uint32_t maxDepth)
    {
        std::vector<uint32_t> leafNodes;

        uint32_t root = 0;
        std::vector<uint32_t> candidates;
        candidates.push_back(root);

        while (candidates.size() > 0)
        {
            uint32_t i = candidates.back();
            candidates.pop_back();

            if (nodes[i].occupation != PARTIAL || getNodeLevel(nodes[i].index) == maxDepth)
            {
                leafNodes.push_back(i);
                continue;
            }

            candidates.push_back(nodes[i].leftChildIndex);
            candidates.push_back(nodes[i].rightChildIndex);
        }
        return leafNodes;
    }

    //
    BinaryTreeNodeOccupation BufferAllocator::occupationFromChildrenNodes(uint32_t index)
    {

        uint32_t left = nodes[index].leftChildIndex;
        uint32_t right = nodes[index].rightChildIndex;

        if (nodes[left].occupation == FULL && nodes[right].occupation == FULL)
        {
            return FULL;
        }

        if (nodes[left].occupation == FREE && nodes[right].occupation == FREE)
        {
            return FREE;
        }

        return PARTIAL;
    }

    bool BufferAllocator::release(Handle<Block> &handle)
    {
        // fetch block
        Block block;
        if (!blockPool->clear(handle, block))
            return false;

        uint32_t index = getBlockIndexFromId(block.id);

        occupiedSpace -= block.size;

        // Free Node
        nodes[index].occupation = FREE;

        // Correct Upstream <-- buddy system.
        upstreamOccupationCorrect(index);

        return true;
    }

    uint32_t BufferAllocator::freeSpace()
    {
        return size - occupiedSpace;
    }

    uint32_t BufferAllocator::getOccupiedSpace()
    {
        return occupiedSpace;
    }

    uint32_t BufferAllocator::getLargestFreeBlockSize()
    {
        return nodes[0].largestFreeBlockSize;
    }

    uint32_t BufferAllocator::getPartitionSize()
    {
        return partitionSize;
    }

    std::string BufferAllocator::coolPrint()
    {
        uint32_t maxDepth = 6;
        std::vector<uint32_t> leaves = getLeaves(maxDepth);

        std::string s = "";

        uint32_t maxHeight = std::min(maxDepth, height);

        for (auto &leaf : leaves)
        {
            uint32_t leafLevel = getNodeLevel(leaf);
            uint32_t chars = 1 << (maxHeight - leafLevel);
            bool occupied = nodes[leaf].occupation == FULL;
            char c = 'a';
            if (nodes[leaf].occupation == FULL)
                c = '#';
            if (nodes[leaf].occupation == PARTIAL)
                c = '!';
            if (nodes[leaf].occupation == FREE)
                c = '.';

            std::string temp;
            // temp.assign(chars, occupied ? '!' : '.');
            temp.assign(chars, c);
            s.append(temp);
        }

        return s;
    }

    uint32_t BufferAllocator::getSize()
    {
        return size;
    }

    uint32_t BufferAllocator::getNodeLevel(uint32_t index)
    {
        return std::bit_width(index + 1) - 1;
    }

    uint32_t BufferAllocator::findFreeNode(uint32_t request)
    {
        //std::cout << "requested == "<< request << std::endl;
        uint32_t fitSize = getMinimunFitSize(request);
        //std:: cout << "minimum fit size == " << fitSize <<
        //" partition size == " << partitionSize << std::endl;

        std::vector<uint32_t> nodeCandidates{};
        nodeCandidates.push_back(0); // root

        uint32_t chosenIndex = UINT32_MAX;

        while (nodeCandidates.size() > 0)
        {

            uint32_t i = nodeCandidates.back();
            nodeCandidates.pop_back();
            //std::cout << "node : " << i << std::endl;
            // fits but block is full
            if (nodes[i].occupation == FULL)
            {
                continue;
            }

            // doesnt fit in this block;
            if (blocks[i].size < fitSize)
            {
                continue;
            }

            // found a FREE partition with correct size.
            if (nodes[i].occupation == FREE &&
                blocks[i].size == fitSize)
            {
            //    std::cout << "Found a valid index" << std::endl;
                chosenIndex = i;
                break;
            }

            if (nodes[nodes[i].rightChildIndex].occupation != FULL)
                nodeCandidates.push_back(nodes[i].rightChildIndex);
            if (nodes[nodes[i].leftChildIndex].occupation != FULL)
                nodeCandidates.push_back(nodes[i].leftChildIndex);
        }

        return chosenIndex;
    }

}
