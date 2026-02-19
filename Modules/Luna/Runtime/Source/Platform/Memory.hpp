/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Base.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Allocates memory blocks from system heap.
        //! @param[in] size The number of bytes to allocate. If this is 0, no memory will be allocated and the return value will be `nullptr`.
        //! @param[in] alignment Optional. The required alignment of the allocated memory block. 
        //! 
        //! If this is 0 (default), then the memory is allocated with no additional alignment requirement. In such case, the memory address is 
        //! aligned to times of 8 in 32-bit platform, and times of 16 in 64-bit platform. 
        //! 
        //! If this is not 0, then the memory is allocated with the specified alignment requirement satisfied. The alignment value must be powers
        //! of 2 (32, 64, 128, 256, etc).
        //! 
        //! Note that you shall use the same alignment value for the same memory block in `allocate`, `free`, `reallocate` and `size` function.
        //! 
        //! @return Returns a pointer to the first available memory address allocated, or `nullptr` if failed to allocate one from this allocator.
        void* memalloc(usize size, usize alignment = 0);

        //! Reallocates memory allocated by `Platform::memalloc` or `Platform::memrealloc`.
        //! @param[in] ptr The pointer returned by `Platform::memalloc` or `Platform::memrealloc` method. If this is `nullptr`, this function behaves the same as 
        //! @ref memalloc.
        //! @param[in] size The number of bytes to allocate. If this is 0, no memory will be allocated and the return value will be `nullptr`.
        //! @param[in] alignment Optional. The required alignment of the allocated memory block. This applies to both `ptr` and new allocated memory block.
        //! @return Returns a pointer to the first available memory address allocated, or `nullptr` if failed to allocate one from this allocator.
        //! @remarks Use `memrealloc` instead of `memalloc` + `memfree` gives the platform a hint that the new memory block will take the same role as the old
        //! memory block, so the platform can perform potectial optimization for allocating new blocks.
        void* memrealloc(void* ptr, usize size, usize alignment = 0);

        //! Frees memory blocks allocated by `Platform::memalloc` or `Platform::memrealloc`.
        //! @param[in] ptr The pointer returned by `Platform::memalloc` or `Platform::memrealloc` method. If this is `nullptr`, this function does nothing.
        //! @param[in] alignment Optional. The alignment requirement specified when allocating the memory block. Default is 0.
        void memfree(void* ptr, usize alignment = 0);

        //! Gets the allocated size of the memory block allocated by `Platform::memalloc` or `Platform::memrealloc`. 
        //! The returned size is the size that is available for the user to use. 
        //! Note that the allocated size may be bigger than the size required to specify alignment and padding requirements.
        //! @param[in] ptr The pointer returned by `Platform::memalloc` or `Platform::memrealloc`.
        //! @param[in] alignment Optional. The alignment requirement specified for the memory block during allocation. Default is 0. 
        //! @return The size of bytes of the memory block. If `ptr` is `nullptr`, the returned value is 0.
        usize memsize(void* ptr, usize alignment = 0);

        //! Global object creation function.
        template <typename _Ty, typename... _Args>
        _Ty* memnew(_Args&&... args)
        {
            _Ty* o = reinterpret_cast<_Ty*>(Platform::memalloc(sizeof(_Ty), alignof(_Ty)));
            if (o)
            {
                new (o) _Ty(forward<_Args>(args)...);
                return o;
            }
            return nullptr;
        }

        //! Global object deleting function.
        template <typename _Ty>
        void memdelete(_Ty* o)
        {
            o->~_Ty();
            Platform::memfree(o, alignof(_Ty));
        }

        //! The allocator that allocates memory from OS directly.
        class Allocator
        {
        public:
            template <typename _Ty>
            _Ty* allocate(usize n = 1)
            {
                return (_Ty*)Platform::memalloc(sizeof(_Ty) * n, alignof(_Ty));
            }
            template <typename _Ty>
            void deallocate(_Ty* ptr, usize n = 1)
            {
                Platform::memfree(ptr, alignof(_Ty));
            }
            bool operator==(const Allocator&)
            {
                return true;
            }
            bool operator!=(const Allocator&)
            {
                return false;
            }
        };
    }
}