#include "PartitionList.hpp"

namespace boitatah
{

    PartitionList::PartitionList(FreeListDesc &desc) : partitionPool({.size = desc.maxPartitions,
                                                                      .dynamic = desc.dynamic,
                                                                      .name = "partition list pool"})
    {
        root = new PartitionNode{.free = true};
        last = root;
        current = root;
        options = desc;

        Partition rootPartition{
            .size = desc.size,
            .address = 0,
        };

        Handle<Partition> partitionHandle = partitionPool.set(rootPartition);
        root->partition = partitionHandle;

        totalOccupation = 0;
    }

    PartitionList::~PartitionList(void)
    {
        current = last;
        while (current->previous != nullptr)
        {

            current = current->previous;
            delete current->next;
        }
        delete current;
    }

    Partition PartitionList::fetch(Handle<Partition> handle)
    {
        Partition p;

        if (partitionPool.get(handle, p))
            return p;

        return Partition{.size = UINT32_MAX, .address = UINT32_MAX, .m_Node = nullptr};
    }

    Handle<Partition> PartitionList::allocate(uint32_t requestedSize)
    {
        PartitionNode *n = findNodeWithSpace(requestedSize);

        if (n == nullptr)
        {
            // fail mode. return null handle;
            return Handle<Partition>{};
        }

        n = partitionize(n, requestedSize);

        totalOccupation += requestedSize;

        return n->partition;
    }
    bool PartitionList::release(Handle<Partition> handle)
    {
        Partition p;
        if (!partitionPool.get(handle, p))
            return false;

        totalOccupation -= p.size;
        current = agglutinateNodes(p.m_Node);

        return true;
    }
    uint32_t PartitionList::getOccupiedSpace()
    {
        return totalOccupation;
    }
    std::string PartitionList::coolPrint()
    {
        int print_chars = 100;
        int slices = options.size / print_chars;

        PartitionNode *n;

        n = root;

        int remainder = 0;
        bool filledOrEmpty = root->free;
        std::string ret = "";
        while (n != nullptr)
        {
            if (filledOrEmpty == n->free)
            {
                Partition p;
                partitionPool.get(n->partition, p);
                remainder += p.size;
                n = n->next;
            }
            else
            {
                int chars = remainder / slices;
                if (chars > 0)
                {
                    std::string temp;
                    temp.assign(chars, filledOrEmpty ? '.' : '!');
                    ret.append(temp);
                }
                remainder -= chars * slices;
                filledOrEmpty = n->free;
            }
        }
        int chars = remainder / slices;
        if (chars > 0)
        {
            std::string temp;
            temp.assign(chars, filledOrEmpty ? '.' : '!');
            ret.append(temp);
        }
        remainder -= chars * slices;
        return ret;
    }
    PartitionNode *PartitionList::findNodeWithSpace(uint32_t request)
    {
        PartitionNode *n;

        // find partition with enough free space.
        n = current;
        do
        {

            if (n->free)
            {
                Partition p;

                bool success = partitionPool.get(n->partition, p);
                // old reference. discard iteration
                if (!success)
                {
                    // advances pointer
                    // if next is end of list. Loop to start.
                    n = n->next != nullptr ? n->next : root;

                    // looped back around;
                    if (n == current)
                    {
                        // return fail mode
                        return nullptr;
                    }

                    // after advancing reference continue;
                    continue;
                }

                // current reference. proceed to check size
                if (p.size >= request)
                {
                    // success find.
                    return n;
                }
            }

            // advances pointer
            // if next is end of list. Loop to start.
            n = n->next != nullptr ? n->next : root;
            // looped back around;
            if (n == current)
            {
                // return fail mode
                return nullptr;
            }
        } while (n != current);

        // fail mode
        return nullptr;
    }

    PartitionNode *PartitionList::partitionize(PartitionNode *targetPartition, uint32_t size)
    {

        Partition hostPartition;

        // this was just checked;
        //  remove and fetch contents
        partitionPool.clear(targetPartition->partition, hostPartition);

        // create a new partition starting after the requested memory
        Partition remainingPartition = {
            .size = hostPartition.size - size,
            .address = (hostPartition.address + size),
        };

        // adjust size, address remains the same;
        hostPartition.size = size;

        PartitionNode *newNode = new PartitionNode{};
        hostPartition.m_Node = targetPartition;
        remainingPartition.m_Node = newNode;

        auto allocated = partitionPool.set(hostPartition);
        auto remaining = partitionPool.set(remainingPartition);

        // now we fix the nodes
        targetPartition->partition = allocated;
        newNode->free = true;
        newNode->next = targetPartition->next;
        newNode->previous = targetPartition;
        newNode->partition = remaining;

        targetPartition->next = newNode;
        targetPartition->free = false;

        // if tagetPartition wasnt the last node
        if (newNode->next != nullptr)
            newNode->next->previous = newNode;

        return targetPartition;
    }

    PartitionNode *PartitionList::agglutinateNodes(PartitionNode *targetNode)
    {
        uint32_t totalSize = 0;
        uint32_t address;

        Partition p;
        partitionPool.clear(targetNode->partition, p);

        totalSize += p.size;
        address = p.address;

        if (targetNode->previous != nullptr && targetNode->previous->free)
        {
            Partition p2;
            partitionPool.clear(targetNode->previous->partition, p2);
            totalSize += p2.size;
            address = p2.address;

            // cleanup
            PartitionNode *previous = targetNode->previous;
            previous->next = targetNode->next;

            if (previous->next != nullptr)
                previous->next->previous = previous;

            // correct if we removed the last element;
            if (targetNode == last)
                last = previous;

            delete targetNode;
            targetNode = previous;
        }

        if (targetNode->next != nullptr && targetNode->next->free)
        {
            Partition p2;
            partitionPool.clear(targetNode->next->partition, p2);
            totalSize += p2.size;

            // clean up
            PartitionNode *next = targetNode->next;
            targetNode->next = next->next;

            if (next->next != nullptr)
                next->next->previous = targetNode;

            // correct if we removed the last element;
            if (next == last)
                last = targetNode;

            delete next;
        }

        p.address = address;
        p.size = totalSize;

        targetNode->free = true;
        targetNode->partition = partitionPool.set(p);

        return targetNode;
    }
}