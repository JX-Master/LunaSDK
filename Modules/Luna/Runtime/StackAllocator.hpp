/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StackAllocator.hpp
* @author JXMaster
* @date 2024/8/22
*/
#pragma once
#include "Memory.hpp"

namespace Luna
{
    LUNA_RUNTIME_API opaque_t begin_stack_alloc_scope();
    LUNA_RUNTIME_API void* stack_alloc(usize size, usize alignment = 0);
    LUNA_RUNTIME_API void end_stack_alloc_scope(opaque_t handle);

    struct StackAllocator
    {
        struct AllocationHeader
        {
            AllocationHeader* next;
        };

        static_assert((sizeof(AllocationHeader) % MAX_ALIGN) == 0, "Wrong AllocationHeader alignment!");

        opaque_t m_allocation;
    public:
        StackAllocator() :
            m_allocation(begin_stack_alloc_scope()) {}
        ~StackAllocator()
        {
            end_stack_alloc_scope(m_allocation);
        }
        void* allocate(usize size)
        {
            return stack_alloc(size);
        }
    };
}