/**
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file UniquePtr.hpp
* @author JXMaster
* @date 2021/9/20
*/
#pragma once
#include "Memory.hpp"
#include "Functional.hpp"

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	
	//! The default object deletion function used by @ref UniquePtr, which calls @ref memdelete to delete
	//! The object.
	//! @tparam _Ty The type of the object.
	template <typename _Ty>
	struct DefaultDelete
	{
		constexpr DefaultDelete() = default;
		void operator()(_Ty* ptr) const
		{
			memdelete(ptr);
		}
	};

	//! A smart pointer that wraps one dynamically created object, and deletes the object when 
	//! the pointer is destructed.
	//! 
	//! @tparam _Ty The type of the object to wrap.
	//! @tparam _Deleter The object deletion function to use.
	template <typename _Ty, typename _Deleter = DefaultDelete<_Ty>>
	class UniquePtr
	{
		OptionalPair<_Deleter, _Ty*> deleter_and_ptr;
	public:
		using pointer = _Ty*;
		using element_type = _Ty;
		using deleter_type = _Deleter;
		//! Constructs one null smart pointer.
		constexpr UniquePtr() :
			deleter_and_ptr(_Deleter(), nullptr) {}
		//! Constructs one null smart pointer with `nullptr_t`.
		constexpr UniquePtr(nullptr_t) :
			deleter_and_ptr(_Deleter(), nullptr) {}
		//! Constructs one smart pointer by wrapping pointer `p`.
		//! @details The smart pointer will have unique ownership to `p`, and the pointer should not be owned by another 
		//! smart pointer.
		//! @param p The pointer to wrap.
		explicit UniquePtr(pointer p) :
			deleter_and_ptr(_Deleter(), p) {}
		//! Constructs one smart pointer by wrapping pointer `p` with custom object deletion function.
		//! @param p The pointer to wrap.
		//! @param d The object deletion function used for the pointer. The function object will be copied into the smart pointer.
		template<typename _D2 = deleter_type, enable_if_t<negation_v<is_reference<_D2>>, int> = 0>
		UniquePtr(pointer p, const deleter_type& d) :
			deleter_and_ptr(d, p) {}
		//! Constructs one smart pointer by wrapping pointer `p` with custom object deletion function.
		//! @param p The pointer to wrap.
		//! @param d The object deletion function used for the pointer. The function object will be moved into the smart pointer.
		template<typename _D2 = deleter_type, enable_if_t<negation_v<is_reference<_D2>>, int> = 0>
		UniquePtr(pointer p, deleter_type&& d) :
			deleter_and_ptr(move(d), p) {}
		template<typename _D2 = deleter_type, enable_if_t<is_lvalue_reference_v<_D2>, int> = 0>
		UniquePtr(pointer p, remove_reference_t<deleter_type>& d) :
			deleter_and_ptr(d, p) {}
		template<typename _D2 = deleter_type, enable_if_t<is_lvalue_reference_v<_D2>, int> = 0>
		UniquePtr(pointer p, remove_reference_t<deleter_type>&& d) = delete;
		//! Gets the underlying native pointer.
		//! @return Returns the underlying native pointer. The pointer may be `nullptr` if the smart pointer is null.
		pointer get() const
		{
			return deleter_and_ptr.second();
		}
		//! Get the object deletion function.
		//! @return Returns one reference to the object deletion function.
		deleter_type& get_deleter()
		{
			return deleter_and_ptr.first();
		}
		//! Get the object deletion function.
		//! @return Returns one constant reference to the object deletion function.
		const deleter_type& get_deleter() const
		{
			return deleter_and_ptr.first();
		}
		//! Checks whether this smart pointer is not null.
		//! @return Returns `ture` if this smart pointer is not null. Returns `false` otherwise.
		explicit operator bool() const
		{
			return get() != nullptr;
		}
		//! Dereferences the native pointer.
		//! @return Returns one reference to the instance pointed by the pointer.
		//! @par Valid Usage
		//! * This pointer must not be null when calling this function.
		typename add_lvalue_reference<_Ty>::type operator*() const
		{
			return *deleter_and_ptr.second();
		}
		//! Accesses members of the instance pointed by the pointer.
		//! @return Returns one copy of the native pointer so that members of the referred instance can be accessed.
		//! @par Valid Usage
		//! * This pointer must not be null when calling this function.
		pointer operator->() const
		{
			return deleter_and_ptr.second();
		}
		//! Sets the native pointer to null, and gets the original pointer.
		//! @return Returns the original pointer wrapped by this smart pointer.
		//! The returned pointer may be `nullptr` if the smart pointer is null when this function is called.
		pointer release()
		{
			pointer p = deleter_and_ptr.second();
			deleter_and_ptr.second() = nullptr;
			return p;
		}
		//! Sets the native pointer of this smart pointer.
		//! If this smart pointer is not null, the original instance pointed by this smart pointer will be deleted.
		//! @param[in] ptr The new pointer to set. If this is not provided, the smart pointer will be cleared to null.
		void reset(pointer ptr = pointer())
		{
			pointer old_ptr = deleter_and_ptr.second();
			deleter_and_ptr.second() = ptr;
			if (old_ptr) get_deleter()(old_ptr);
		}
		//! Swaps native pointers of this smart pointer with the specified smart pointer.
		//! @param[in] rhs The smart pointer to swap with.
		void swap(UniquePtr& rhs)
		{
			swap(deleter_and_ptr.first(), rhs.deleter_and_ptr.first());
			swap(deleter_and_ptr.second(), rhs.deleter_and_ptr.second());
		}
		UniquePtr(const UniquePtr&) = delete;
		//! Constructs one smart pointer by moving native pointer from another smart pointer.
		//! @param[in] rhs The smart pointer to move pointer from. This smart pointer will be null after this operation.
		UniquePtr(UniquePtr&& rhs) :
			deleter_and_ptr(forward<deleter_type>(rhs.deleter_and_ptr.first()), rhs.release()) {}
		~UniquePtr()
		{
			if (deleter_and_ptr.second()) deleter_and_ptr.first()(deleter_and_ptr.second());
		}
		UniquePtr& operator=(const UniquePtr&) = delete;
		//! Assigns one smart pointer by moving native pointer from another smart pointer.
		//! If this smart pointer is not null, the original instance pointed by this smart pointer will be deleted.
		//! @param[in] rhs The smart pointer to move pointer from. This smart pointer will be null after this operation.
		UniquePtr& operator=(UniquePtr&& rhs)
		{
			if (this != addressof(rhs))
			{
				reset(rhs.release());
				deleter_and_ptr.first() = forward<deleter_type>(rhs.deleter_and_ptr.first());
			}
			return *this;
		}
		//! Sets one smart pointer to null.
		//! If this smart pointer is not null, the original instance pointed by this smart pointer will be deleted.
		UniquePtr& operator=(nullptr_t)
		{
			reset();
			return *this;
		}
		//! Compares two smart pointers for equality. Two smart pointers are equal if their native pointers are equal or 
		//! both null.
		//! @param[in] rhs The smart pointer to compare with.
		//! @return Returns `true` if two smart pointers are equal. Returns `false` otherwise.
		//! @remark Since one native pointer can never be hold by two unique smart pointers at the same time (which causes double deletion), 
		//! this is mainly used for comparing one smart pointer with `nullptr`.
		bool operator==(const UniquePtr& rhs) const
		{
			return deleter_and_ptr.second() == rhs.deleter_and_ptr.second();
		}
		//! Compares two smart pointers for non-equality.
		//! @param[in] rhs The smart pointer to compare with.
		//! @return Returns `true` if two smart pointers are not equal. Returns `false` otherwise.
		bool operator!=(const UniquePtr& rhs) const
		{
			return !(*this == rhs);
		}
	};

	//! @}

	template <typename _Ty> struct hash<UniquePtr<_Ty>>
	{
		usize operator()(const UniquePtr<_Ty>& val) const { return (usize)val.get(); }
	};
}