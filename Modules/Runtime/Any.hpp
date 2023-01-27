/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Any.hpp
* @author JXMaster
* @date 2020/4/13
*/
#pragma once
#include "TypeInfo.hpp"
#include "Reflection.hpp"

namespace Luna
{
	//! @struct Any
	//! Represents one wrapper that may contain any value.
	struct Any
	{
	private:
		typeinfo_t m_type;
		void* m_data;
		void internal_clear()
		{
			if (m_data)
			{
				destruct_type(m_type, m_data);
				m_data = nullptr;
				m_type = nullptr;
			}
		}
	public:
		Any() :
			m_type(nullptr),
			m_data(nullptr) {}
		Any(const Any& rhs) :
			m_type(rhs.m_type)
		{
			if (rhs.m_data)
			{
				m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
				copy_construct_type(m_type, m_data, rhs.m_data);
			}
		}
		Any(Any&& rhs) :
			m_type(rhs.m_type),
			m_data(rhs.m_data)
		{
			// We move the value object directly rather than allocating another value object and move-constructing it.
			// This is ok for most cases, and it prevents memory allocation, althrough this behavior may not be same as `std::any` 
			// (actually both behaviors are allowed, see case (3) in https://en.cppreference.com/w/cpp/utility/any/any).
			rhs.m_type = nullptr;
			rhs.m_data = nullptr;
		}
		Any(typeinfo_t type) :
			m_type(type)
		{
			m_data = memalloc(get_type_size(type), get_type_alignment(type));
			construct_type(type, m_data);
		}
		Any(typeinfo_t type, void* data) :
			m_type(type),
			m_data(data) {}
		template <typename _Ty>
		Any(_Ty&& value) :
			m_type(typeof<_Ty>())
		{
			m_data = memalloc(sizeof(_Ty), alignof(_Ty));
			new (m_data) _Ty(forward<_Ty>(value));
		}
		Any& operator=(const Any& rhs)
		{
			internal_clear();
			m_type = rhs.m_type;
			if (rhs.m_data)
			{
				m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
				copy_construct_type(m_type, m_data, rhs.m_data);
			}
			return *this;
		}
		Any& operator=(Any&& rhs)
		{
			internal_clear();
			m_type = rhs.m_type;
			m_data = rhs.m_data;
			rhs.m_type = nullptr;
			rhs.m_data = nullptr;
			return *this;
		}
		template <typename _Ty>
		Any& operator=(_Ty&& rhs)
		{
			internal_clear();
			m_type = typeof<_Ty>();
			m_data = memalloc(sizeof(_Ty), alignof(_Ty));
			new (m_data) _Ty(forward<_Ty>(value));
			return *this;
		}
		~Any()
		{
			internal_clear();
		}
		void attach(typeinfo_t type, void* data)
		{
			internal_clear();
			m_type = type;
			m_data = data;
		}
		Pair<typeinfo_t, void*> detach()
		{
			Pair<typeinfo_t, void*> r(m_type, m_data);
			m_type = nullptr;
			m_data = nullptr;
			return r;
		}
		template <typename _Ty, typename... _Args>
		_Ty& emplace(_Args&&... args)
		{
			internal_clear();
			m_type = typeof<_Ty>();
			m_data = memalloc(sizeof(_Ty), alignof(_Ty));
			new (m_data) _Ty(forward<_Args>(args)...);
			return *((_Ty*)m_data);
		}
		void reset()
		{
			internal_clear();
		}
		void swap(Any& rhs)
		{
			typeinfo_t type = m_type;
			m_type = rhs.m_type;
			rhs.m_type = type;
			void* data = m_data;
			m_data = rhs.m_data;
			rhs.m_data = data;
		}
		bool has_value() const
		{
			return m_data != nullptr;
		}
		typeinfo_t type() const
		{
			return m_type;
		}
		const void* data() const
		{
			return m_data;
		}
		void* data()
		{
			return m_data;
		}
	};
}