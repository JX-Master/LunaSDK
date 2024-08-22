/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StackAllocator.hpp
* @author JXMaster
* @date 2020/8/22
*/
#pragma once
#include "Memory.hpp"
namespace Luna
{
    struct StackAllocator
    {
        struct AllocationHeader
        {
            AllocationHeader* next;
        };

        static_assert((sizeof(AllocationHeader) % MAX_ALIGN) == 0, "Wrong AllocationHeader alignment!");

        AllocationHeader* m_allocation;
    public:
        StackAllocator() :
            m_allocation(nullptr) {}
        ~StackAllocator()
        {
            AllocationHeader* next = m_allocation;
            while(next)
            {
                AllocationHeader* cur = next;
                next = next->next;
                memfree(cur);
            }
        }
        void* allocate(usize size)
        {
            usize allocated_size = size + sizeof(AllocationHeader);
            AllocationHeader* header = (AllocationHeader*)memalloc(allocated_size);
            header->next = m_allocation;
            m_allocation = header;
            return (void*)(header + 1);
        }
    };
}