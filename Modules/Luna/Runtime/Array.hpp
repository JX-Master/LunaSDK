/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Array.hpp
* @author JXMaster
* @date 2022/9/27
*/
#pragma once
#include "Iterator.hpp"
#include "Algorithm.hpp"
#include "Memory.hpp"
#include "MemoryUtils.hpp"

namespace Luna
{
	inline constexpr usize DYNAMIC_ARRAY_SIZE = USIZE_MAX;

	template <typename _Ty, usize _Size = DYNAMIC_ARRAY_SIZE>
	class Array
	{
	public:
		using value_type = _Ty;
		using pointer = _Ty*;
		using const_pointer = const _Ty*;
		using reference = _Ty&;
		using const_reference = const _Ty&;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;

		constexpr reference at(usize pos) { lucheck(pos < _Size); return m_elements[pos]; }
		constexpr const_reference at(usize pos) const { lucheck(pos < _Size); return m_elements[pos]; }
		constexpr reference operator[](usize pos) { lucheck(pos < _Size); return m_elements[pos]; }
		constexpr const_reference operator[](usize pos) const { lucheck(pos < _Size); return m_elements[pos]; }
		constexpr reference front() { return m_elements[0]; }
		constexpr const_reference front() const { return m_elements[0]; }
		constexpr reference back() { return m_elements[_Size - 1]; }
		constexpr const_reference back() const { return m_elements[_Size - 1]; }
		constexpr _Ty* data() { return m_elements; }
		constexpr const _Ty* data() const { return m_elements; }
		constexpr iterator begin() { return m_elements; }
		constexpr const_iterator begin() const { return m_elements; }
		constexpr const_iterator cbegin() const { return m_elements; }
		constexpr iterator end() { return m_elements + _Size; }
		constexpr const_iterator end() const { return m_elements + _Size; }
		constexpr const_iterator cend() const { return m_elements + _Size; }
		constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
		constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
		constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
		constexpr const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		constexpr const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
		constexpr bool empty() const { return false; }
		constexpr usize size() const { return _Size; }
		constexpr void fill(const _Ty& value)
		{
			fill_assign_range(m_elements, m_elements + _Size, value);
		}
		constexpr void swap(Array& rhs)
		{
			for (usize i = 0; i < _Size; ++i)
			{
				Luna::swap(m_elements[i], rhs.m_elements[i]);
			}
		}
	private:
		_Ty m_elements[_Size];
	};

	template <typename _Ty>
	class Array<_Ty, 0>
	{
	public:
		using value_type = _Ty;
		using pointer = _Ty*;
		using const_pointer = const _Ty*;
		using reference = _Ty&;
		using const_reference = const _Ty&;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;

		constexpr _Ty* data() { return nullptr; }
		constexpr const _Ty* data() const { return nullptr; }
		constexpr iterator begin() { return nullptr; }
		constexpr const_iterator begin() const { return nullptr; }
		constexpr const_iterator cbegin() const { return nullptr; }
		constexpr iterator end() { return nullptr; }
		constexpr const_iterator end() const { return nullptr; }
		constexpr const_iterator cend() const { return nullptr; }
		constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
		constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
		constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
		constexpr const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		constexpr const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
		constexpr bool empty() const { return true; }
		constexpr usize size() const { return 0; }
		constexpr void fill(const _Ty& value) {}
		constexpr void swap(Array& rhs) {}
	};

	template <typename _Ty>
	class Array<_Ty, DYNAMIC_ARRAY_SIZE>
	{
	public:
		using value_type = _Ty;
		using pointer = _Ty*;
		using const_pointer = const _Ty*;
		using reference = _Ty&;
		using const_reference = const _Ty&;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;

		Array() :
			m_elements(nullptr),
			m_size(0) {}
		~Array()
		{
			internal_free();
		}
		Array(const Array& rhs)
		{
			if (rhs.m_elements)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * rhs.m_size, alignof(_Ty));
				m_size = rhs.m_size;
				copy_construct_range(rhs.m_elements, rhs.m_elements + rhs.m_size, m_elements);
			}
			else
			{
				m_elements = nullptr;
				m_size = 0;
			}
		}
		Array(Array&& rhs) :
			m_elements(rhs.m_elements),
			m_size(rhs.m_size)
		{
			rhs.m_elements = nullptr;
			rhs.m_size = 0;
		}
		Array& operator=(const Array& rhs)
		{
			internal_free();
			if (rhs.m_elements)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * rhs.m_size, alignof(_Ty));
				m_size = rhs.m_size;
				copy_construct_range(rhs.m_elements, rhs.m_elements + rhs.m_size, m_elements);
			}
			return *this;
		}
		Array& operator=(Array&& rhs)
		{
			internal_free();
			m_elements = rhs.m_elements;
			m_size = rhs.m_size;
			rhs.m_elements = nullptr;
			rhs.m_size = 0;
			return *this;
		}
		Array(usize count, const _Ty& value = _Ty())
		{
			if(count)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * count, alignof(_Ty));
				m_size = count;
				fill_construct_range(m_elements, m_elements + count, value);
			}
			else
			{
				m_elements = nullptr;
				m_size = 0;
			}
		}
		template <typename _InputIt>
		Array(_InputIt first, usize count)
		{
			if (count)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * count, alignof(_Ty));
				m_size = count;
				copy_construct_range_n(first, count, m_elements);
			}
			else
			{
				m_elements = nullptr;
				m_size = 0;
			}
		}
		Array(InitializerList<_Ty> ilist)
		{
			usize count = ilist.size();
			if (count)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * count, alignof(_Ty));
				m_size = count;
				copy_construct_range(ilist.begin(), ilist.end(), m_elements);
			}
			else
			{
				m_elements = nullptr;
				m_size = 0;
			}
		}
		reference at(usize pos) { lucheck(pos < m_size); return m_elements[pos]; }
		const_reference at(usize pos) const { lucheck(pos < m_size); return m_elements[pos]; }
		reference operator[](usize pos) { lucheck(pos < m_size); return m_elements[pos]; }
		const_reference operator[](usize pos) const { lucheck(pos < m_size); return m_elements[pos]; }
		reference front() { lucheck(!empty()); return m_elements[0]; }
		const_reference front() const { lucheck(!empty()); return m_elements[0]; }
		reference back() { lucheck(!empty()); return m_elements[m_size - 1]; }
		const_reference back() const { lucheck(!empty()); return m_elements[m_size - 1]; }
		_Ty* data() { return m_elements; }
		const _Ty* data() const { return m_elements; }
		iterator begin() { return m_elements; }
		const_iterator begin() const { return m_elements; }
		const_iterator cbegin() const { return m_elements; }
		iterator end() { return m_elements + m_size; }
		const_iterator end() const { return m_elements + m_size; }
		const_iterator cend() const { return m_elements + m_size; }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
		bool empty() const { return m_size == 0; }
		usize size() const { return m_size; }
		void clear()
		{
			internal_free();
		}
		void assign(usize count, const _Ty& value = _Ty())
		{
			internal_free();
			if (count)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * count, alignof(_Ty));
				m_size = count;
				fill_construct_range(m_elements, m_elements + m_size, value);
			}
		}
		template <typename _InputIt>
		void assign_n(_InputIt first, usize count)
		{
			internal_free();
			if (count)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * count, alignof(_Ty));
				m_size = count;
				copy_construct_range_n(first, count, m_elements);
			}
		}
		void assign(InitializerList<_Ty> ilist)
		{
			internal_free();
			usize count = ilist.size();
			if (count)
			{
				m_elements = (_Ty*)memalloc(sizeof(_Ty) * count, alignof(_Ty));
				m_size = count;
				copy_construct_range(ilist.begin(), ilist.end(), m_elements);
			}
		}
		void swap(Array& rhs)
		{
			auto temp_buf = m_elements;
			auto temp_size = m_size;
			m_elements = rhs.m_elements;
			m_size = rhs.m_size;
			rhs.m_elements = temp_buf;
			rhs.m_size = temp_size;
		}
	private:
		void internal_free()
		{
			if (m_elements)
			{
				destruct_range(m_elements, m_elements + m_size);
				memfree(m_elements, alignof(_Ty));
				m_elements = nullptr;
				m_size = 0;
			}
		}
		_Ty* m_elements;
		usize m_size;
	};
}