/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Atomic.hpp
* @author JXMaster
* @date 2018/12/7
 */
#pragma once
#include "PlatformDefines.hpp"

#if defined(LUNA_PLATFORM_WINDOWS)
#include "Impl/Platform/Windows/AtomicImpl.hpp"
#elif defined(LUNA_PLATFORM_POSIX)
#include "Impl/Platform/POSIX/AtomicImpl.hpp"
#endif

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeAtomic Atomic Operations
	//! @}

	//! @addtogroup RuntimeAtomic
    //! @{

	//! Atomically increase the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	i32 atom_inc_i32(i32 volatile* v);
	//! Atomically increase the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	u32 atom_inc_u32(u32 volatile* v);
	//! Atomically increase the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	i64 atom_inc_i64(i64 volatile* v);
	//! Atomically increase the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	u64 atom_inc_u64(u64 volatile* v);
	//! Atomically increase the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	usize atom_inc_usize(usize volatile* v);

	//! Atomically decrease the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	i32 atom_dec_i32(i32 volatile* v);
	//! Atomically decrease the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	u32 atom_dec_u32(u32 volatile* v);
	//! Atomically decrease the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	i64 atom_dec_i64(i64 volatile* v);
	//! Atomically decrease the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	u64 atom_dec_u64(u64 volatile* v);
	//! Atomically decrease the value of the variable by 1.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] v The pointer to the variable that needs to be changed.
	//! @return Returns the value of the variable after this operation.
	usize atom_dec_usize(usize volatile* v);

	//! Atomically increase the value of the variable by the the value provided. 
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] base The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be added to the variable.
	//! @return Returns the value of the variable before this operation.
	i32 atom_add_i32(i32 volatile* base, i32 v);
	//! Atomically increase the value of the variable by the the value provided. 
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] base The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be added to the variable.
	//! @return Returns the value of the variable before this operation.
	u32 atom_add_u32(u32 volatile* base, i32 v);
	//! Atomically increase the value of the variable by the the value provided. 
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] base The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be added to the variable.
	//! @return Returns the value of the variable before this operation.
	i64 atom_add_i64(i64 volatile* base, i64 v);
	//! Atomically increase the value of the variable by the the value provided. 
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] base The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be added to the variable.
	//! @return Returns the value of the variable before this operation.
	u64 atom_add_u64(u64 volatile* base, i64 v);
	//! Atomically increase the value of the variable by the the value provided. 
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] base The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be added to the variable.
	//! @return Returns the value of the variable before this operation.
	usize atom_add_usize(usize volatile* base, isize v);

	//! Atomically replace the value of the variable with the value provided.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] dst The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be set to the variable.
	//! @return Returns the value of the variable before this operation took place.
	i32 atom_exchange_i32(i32 volatile* dst, i32 v);
	//! Atomically replace the value of the variable with the value provided.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] dst The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be set to the variable.
	//! @return Returns the value of the variable before this operation took place.
	u32 atom_exchange_u32(u32 volatile* dst, u32 v);
	//! Atomically replace the value of the variable with the value provided.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] dst The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be set to the variable.
	//! @return Returns the value of the variable before this operation took place.
	i64 atom_exchange_i64(i64 volatile* dst, i64 v);
	//! Atomically replace the value of the variable with the value provided.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] dst The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be set to the variable.
	//! @return Returns the value of the variable before this operation took place.
	u64 atom_exchange_u64(u64 volatile* dst, u64 v);
	//! Atomically replace the value of the variable with the value provided.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] dst The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be set to the variable.
	//! @return Returns the value of the variable before this operation took place.
	template <typename _Ty>
	_Ty* atom_exchange_pointer(_Ty* volatile* target, void* value);
	//! Atomically replace the value of the variable with the value provided.
	//! @details This operation cannot be interrupted by system thread switching.
	//! @param[in] dst The pointer to the variable that needs to be changed.
	//! @param[in] v The value that needs to be set to the variable.
	//! @return Returns the value of the variable before this operation took place.
	usize atom_exchange_usize(usize volatile* dst, usize v);

	//! Atomically compares the value of the variable with the specified comperand, 
	//! and sets the variable to the specified value if equal.
	//! @param[in] dst The pointer to the variable that needs to be compared.
	//! @param[in] exchange The value to set to the variable if `*dst == comperand`.
	//! @param[in] comperand The value to compare with.
	//! @return Returns the value of the variable before this operation took place.
	//! @remark The following code demostrates the behavior of this function:
	//! ```c++
	//! vtype atom_compare_exchange_i32(vtype volatile* dst, vtype exchange, vtype comperand)
	//! {
	//! 	vtype r = *dst;
	//! 	if(*dst == comperand) *dst = exchange;
	//! 	return r;
	//! }
	//! ```
	i32 atom_compare_exchange_i32(i32 volatile* dst, i32 exchange, i32 comperand);
	//! Atomically compares the value of the variable with the specified comperand, 
	//! and sets the variable to the specified value if equal.
	//! @param[in] dst The pointer to the variable that needs to be compared.
	//! @param[in] exchange The value to set to the variable if `*dst == comperand`.
	//! @param[in] comperand The value to compare with.
	//! @return Returns the value of the variable before this operation took place.
	//! @remark See remarks of @ref atom_compare_exchange_i32 for details.
	u32 atom_compare_exchange_u32(u32 volatile* dst, u32 exchange, u32 comperand);
	//! Atomically compares the value of the variable with the specified comperand, 
	//! and sets the variable to the specified value if equal.
	//! @param[in] dst The pointer to the variable that needs to be compared.
	//! @param[in] exchange The value to set to the variable if `*dst == comperand`.
	//! @param[in] comperand The value to compare with.
	//! @return Returns the value of the variable before this operation took place.
	//! @remark See remarks of @ref atom_compare_exchange_i32 for details.
	template <typename _Ty>
	_Ty* atom_compare_exchange_pointer(_Ty* volatile* dst, void* exchange, void* comperand);
#ifdef LUNA_PLATFORM_64BIT
//! Atomically compares the value of the variable with the specified comperand, 
	//! and sets the variable to the specified value if equal.
	//! @param[in] dst The pointer to the variable that needs to be compared.
	//! @param[in] exchange The value to set to the variable if `*dst == comperand`.
	//! @param[in] comperand The value to compare with.
	//! @return Returns the value of the variable before this operation took place.
	//! @remark See remarks of @ref atom_compare_exchange_i32 for details.
	i64 atom_compare_exchange_i64(i64 volatile* dst, i64 exchange, i64 comperand);
	//! Atomically compares the value of the variable with the specified comperand, 
	//! and sets the variable to the specified value if equal.
	//! @param[in] dst The pointer to the variable that needs to be compared.
	//! @param[in] exchange The value to set to the variable if `*dst == comperand`.
	//! @param[in] comperand The value to compare with.
	//! @return Returns the value of the variable before this operation took place.
	//! @remark See remarks of @ref atom_compare_exchange_i32 for details.
	u64 atom_compare_exchange_u64(u64 volatile* dst, u64 exchange, u64 comperand);
#endif
	//! Atomically compares the value of the variable with the specified comperand, 
	//! and sets the variable to the specified value if equal.
	//! @param[in] dst The pointer to the variable that needs to be compared.
	//! @param[in] exchange The value to set to the variable if `*dst == comperand`.
	//! @param[in] comperand The value to compare with.
	//! @return Returns the value of the variable before this operation took place.
	//! @remark See remarks of @ref atom_compare_exchange_i32 for details.
	usize atom_compare_exchange_usize(usize volatile* dst, usize exchange, usize comperand);

	//! @}
}
