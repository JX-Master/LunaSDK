/*!
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
	template <typename _Ty>
	struct DefaultDelete
	{
		constexpr DefaultDelete() = default;
		void operator()(_Ty* ptr) const
		{
			memdelete(ptr);
		}
	};

	template <typename _Ty, typename _Deleter = DefaultDelete<_Ty>>
	class UniquePtr
	{
		OptionalPair<_Deleter, _Ty*> deleter_and_ptr;
	public:
		using pointer = _Ty*;
		using element_type = _Ty;
		using deleter_type = _Deleter;

		constexpr UniquePtr() :
			deleter_and_ptr(_Deleter(), nullptr) {}
		constexpr UniquePtr(nullptr_t) :
			deleter_and_ptr(_Deleter(), nullptr) {}
		explicit UniquePtr(pointer p) :
			deleter_and_ptr(_Deleter(), p) {}
		template<typename _D2 = deleter_type, enable_if_t<negation_v<is_reference<_D2>>, int> = 0>
		UniquePtr(pointer p, const deleter_type& d) :
			deleter_and_ptr(d, p) {}
		template<typename _D2 = deleter_type, enable_if_t<negation_v<is_reference<_D2>>, int> = 0>
		UniquePtr(pointer p, deleter_type&& d) :
			deleter_and_ptr(move(d), p) {}
		template<typename _D2 = deleter_type, enable_if_t<is_lvalue_reference_v<_D2>, int> = 0>
		UniquePtr(pointer p, remove_reference_t<deleter_type>& d) :
			deleter_and_ptr(d, p) {}
		template<typename _D2 = deleter_type, enable_if_t<is_lvalue_reference_v<_D2>, int> = 0>
		UniquePtr(pointer p, remove_reference_t<deleter_type>&& d) = delete;
		pointer get() const
		{
			return deleter_and_ptr.second();
		}
		deleter_type& get_deleter()
		{
			return deleter_and_ptr.first();
		}
		const deleter_type& get_deleter() const
		{
			return deleter_and_ptr.first();
		}
		explicit operator bool() const
		{
			return get() != nullptr;
		}
		typename add_lvalue_reference<_Ty>::type operator*() const
		{
			return *deleter_and_ptr.second();
		}
		pointer operator->() const
		{
			return deleter_and_ptr.second();
		}
		pointer release()
		{
			pointer p = deleter_and_ptr.second();
			deleter_and_ptr.second() = nullptr;
			return p;
		}
		void reset(pointer ptr = pointer())
		{
			pointer old_ptr = deleter_and_ptr.second();
			deleter_and_ptr.second() = ptr;
			if (old_ptr) get_deleter()(old_ptr);
		}
		void swap(UniquePtr& rhs)
		{
			swap(deleter_and_ptr.first(), rhs.deleter_and_ptr.first());
			swap(deleter_and_ptr.second(), rhs.deleter_and_ptr.second());
		}
		UniquePtr(const UniquePtr&) = delete;
		UniquePtr(UniquePtr&& rhs) :
			deleter_and_ptr(forward<deleter_type>(rhs.deleter_and_ptr.first()), rhs.release()) {}
		~UniquePtr()
		{
			if (deleter_and_ptr.second()) deleter_and_ptr.first()(deleter_and_ptr.second());
		}
		UniquePtr& operator=(const UniquePtr&) = delete;
		UniquePtr& operator=(UniquePtr&& rhs)
		{
			if (this != addressof(rhs))
			{
				reset(rhs.release());
				deleter_and_ptr.first() = forward<deleter_type>(rhs.deleter_and_ptr.first());
			}
			return *this;
		}
		UniquePtr& operator=(nullptr_t)
		{
			reset();
			return *this;
		}
		bool operator==(const UniquePtr& rhs) const
		{
			return deleter_and_ptr.second() == rhs.deleter_and_ptr.second();
		}
		bool operator!=(const UniquePtr& rhs) const
		{
			return !(*this == rhs);
		}
	};

	template <typename _Ty> struct hash<UniquePtr<_Ty>>
	{
		usize operator()(const UniquePtr<_Ty>& val) const { return (usize)val.get(); }
	};
}