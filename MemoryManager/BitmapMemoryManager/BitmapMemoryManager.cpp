#include "BitmapMemoryManager.hpp"
#include <iostream>
#include <stddef.h>
#include <cstdint>
#include <cstring>

size_t findIndexForBlockSequence(size_t size, bool *&bitmap, size_t bitmapSize);

namespace BitmapMemoryManager
{
    MemoryManager::MemoryManager()
    {
        this->size = NUM_BLOCKS;
        bitmap = new bool[this->size];
        for (size_t i = 0; i < this->size; ++i)
        {
            bitmap[i] = false;
        }
        arena = new char[this->size * BLOCK_SIZE];
    }

    void *MemoryManager::allocate(size_t size)
    {
        size_t i = 0;
        size_t totalSize = size + HEADER_SIZE;

        if (totalSize > BLOCK_SIZE)
        {
            size_t blocksNeeded = totalSize / BLOCK_SIZE + (totalSize % BLOCK_SIZE == 0 ? 0 : 1);
            size_t index = findIndexForBlockSequence(blocksNeeded, bitmap, this->get_size());
            if (index < this->get_size())
            {
                for (size_t j = index; j < index + blocksNeeded; ++j)
                {
                    bitmap[j] = true;
                }
                print_bitmap();
                size_t byteOffset = index * BLOCK_SIZE;
                std::memcpy(static_cast<char *>(arena) + byteOffset + BLOCK_SIZE_OFFSET, &totalSize, sizeof(size_t));
                *reinterpret_cast<size_t *>(static_cast<char *>(arena) + byteOffset + BLOCK_PTR_COUNT_OFFSET) = 0;
                return static_cast<char *>(arena) + byteOffset + HEADER_SIZE;
            }
            else
            {
                std::cerr << "Not enough memory" << std::endl;
                return nullptr;
            }
        }

        // Fits in 1 block
        while (i < this->get_size())
        {
            if (bitmap[i] == false)
            {
                bitmap[i] = true;
                print_bitmap();
                std::memcpy(static_cast<char *>(arena) + i * BLOCK_SIZE, &totalSize, sizeof(size_t));
                *reinterpret_cast<size_t *>(static_cast<char *>(arena) + i * BLOCK_SIZE + BLOCK_PTR_COUNT_OFFSET) = 0;
                return static_cast<char *>(arena) + i * BLOCK_SIZE + HEADER_SIZE;
            }
            ++i;
        }
        return nullptr;
    }

    void MemoryManager::deallocate(void *p)
    {
        size_t size = *reinterpret_cast<size_t *>(static_cast<char *>(p) - HEADER_SIZE + BLOCK_SIZE_OFFSET);
        char *start = static_cast<char *>(static_cast<char *>(p) - HEADER_SIZE);
        size_t index = (start - static_cast<char *>(arena)) / BLOCK_SIZE;

        size_t blocksNeeded = size / BLOCK_SIZE + (size % BLOCK_SIZE == 0 ? 0 : 1);

        for (size_t i = index; i < index + blocksNeeded; ++i)
        {
            bitmap[i] = false;
        }
        print_bitmap();
    }

    bool MemoryManager::isAddressValid(void *p)
    {
        return p >= arena && p < static_cast<char *>(arena) + this->get_size() * BLOCK_SIZE;
    }

    void MemoryManager::print_bitmap() const
    {
        for (size_t i = 0; i < this->get_size(); ++i)
        {
            std::cout << bitmap[i];
        }
        std::cout << std::endl;
    }

    MemoryManager::~MemoryManager()
    {
        delete[] static_cast<char *>(arena);
    }
}
size_t findIndexForBlockSequence(size_t size, bool *&bitmap, size_t bitmapSize)
{
    size_t i = 0;
    size_t consecutive = 0;
    while (i < bitmapSize)
    {
        if (bitmap[i] == false)
            consecutive++;
        else
            consecutive = 0;
        if (consecutive == size)
            return i - size + 1;
        i++;
    }
    return SIZE_MAX;
}