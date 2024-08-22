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
    //! @addtogroup RuntimeMemory
    //! @{

    //! Opens one new stack allocation scope.
    //! @return opaque_t Returns one opaque handle that should be passed to @ref end_stack_alloc_scope
    //! when closing this scope.
    LUNA_RUNTIME_API opaque_t begin_stack_alloc_scope();

    //! Allocates memory from one thread-local stack attached to the current thread.
    //! @param[in] size The size, in bytes, of the memory to allocate.
    //! @param[in] alignment Optional. The alignment requirement specified when allocating the memory block. Default is 0.
    //! 
    //! If this is 0 (default), then the memory is allocated with no additional alignment requirement.In such case, the memory address is 
    //! aligned to @ref MAX_ALIGN.
    //! 
    //! @return Returns one pointer to the allocated memory block. Returns `nullptr` if `size` is 0.
    //! @remark The memory allocated from this function will be freed automatically when @ref end_stack_alloc_scope is called. Never
    //! call @ref memfree on memory allocated from this function.
    //! @par Valid Usage
    //! * @ref begin_stack_alloc_scope must be called at least one time before calling this function.
    //! * If `alignment` is not `0`, `alignment` **must** be powers of 2 (like 32, 64, 128, 256, etc).
    LUNA_RUNTIME_API void* stack_alloc(usize size, usize alignment = 0);

    //! Closes one stack allocation scope and frees all memory allocated in that scope.
    //! @param[in] handle The handle returned @ref begin_stack_alloc_scope, which identifies one scope to be closed.
    //! If the specified scope contains subscopes, these subscopes will also be closed, so the user does not need to (and should
    //! not) call `end_stack_alloc_scope` on such subscopes to close them again.
    //! @par Valid Usage
    //! * `handle` must specify one valid unclosed scope.
    LUNA_RUNTIME_API void end_stack_alloc_scope(opaque_t handle);

    //! The RAII wrapper for stack-based allocation.
    //! @details This type opens a new stack allocation scope upon constructing, and closes such 
    //! scope upon destructing. So instead of calling @ref begin_stack_alloc_scope and @ref end_stack_alloc_scope 
    //! manually, the user can simply declare one `StackAllocator` variable on the function body, then calls 
    //! @ref StackAllocator::allocate to allocate stack memory.
    struct StackAllocator
    {
    private:
        opaque_t m_allocation;
    public:
        StackAllocator() :
            m_allocation(begin_stack_alloc_scope()) {}
        ~StackAllocator()
        {
            end_stack_alloc_scope(m_allocation);
        }
        //! Allocates stack memory. See @ref stack_alloc for details.
        //! @param[in] size The size, in bytes, of the memory to allocate.
        //! @return Returns one pointer to the allocated memory block. Returns `nullptr` if `size` is 0.
        void* allocate(usize size, usize alignment = 0)
        {
            return stack_alloc(size, alignment);
        }
    };

    //! @}
}
