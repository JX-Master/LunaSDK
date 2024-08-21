/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Alloca.hpp
* @author JXMaster
* @date 2024/8/20
*/
#pragma once
#include "Blob.hpp"

//! @addtogroup RuntimeMemoryUtils
//! @{

#ifndef LUNA_MAX_ALLOCA_SIZE

//! The maximun size of the memory block that is allowed to be allocated on stack using `alloca` in @ref lualloca.
//! @details If the real allocated size is greater than this, @ref lualloca will use heap allocation instead of `alloca`
//! to allocate the memory.
#define LUNA_MAX_ALLOCA_SIZE 256
#endif

//! Allocates one temporary memory that exists only on the current statement block scope.
//! @details This macro uses `alloca` to allocate memory when the allocation size is not greater than
//! @ref LUNA_MAX_ALLOCA_SIZE, or fallback to heap allocation if the size exceeds @ref LUNA_MAX_ALLOCA_SIZE.
//! In both cases, the allocated memory is valid only in the current scope, and will be released automatically when
//! the current scope is escaped.
//! 
//! In most cases, the user should use this macro instead of `alloca` to prevent allocating large memory on stack, which 
//! may cause stack overflow and crash the program. If the user do need to call `alloca` directly, she should check 
//! the allocation size manually, and fall back to heap allocation when the size is big.
//! 
//! The returned memory is not initialized, just like `alloca`. The user should initialize the memory data manually.
//! @param[out] _var The pointer variable that holds the allocated memory.
//! @param[in] _type The type of instances in the memory.
//! @param[in] _count The number of instances to allocate. The real allocated size is `sizeof(_type) * (_count)`.
#define lualloca(_var, _type, _count) \
_type* _var; \
Luna::Blob _var##_alloca_buffer; \
if ((sizeof(_type) * (_count)) <= LUNA_MAX_ALLOCA_SIZE) \
{ \
    _var = (_type*)alloca(sizeof(_type) * (_count)); \
} \
else \
{ \
    _var##_alloca_buffer.resize(sizeof(_type) * (_count)); \
    _var = (_type*)_var##_alloca_buffer.data(); \
}

//! @}