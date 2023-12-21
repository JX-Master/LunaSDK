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
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeContainer Containers
	//! @}

	//! @addtogroup RuntimeContainer
    //! @{
	
	//! Specifies a dynamic-sized array for @ref Array.
	inline constexpr usize DYNAMIC_ARRAY_SIZE = USIZE_MAX;

	//! Represents one array of fixed or dynamic size.
	//! @details Array is one container that contains fixed number of elements. The size of one array can be set
	//! in compile time by specifying `_Size` of the array, or can be set when creating the array by setting `_Size`
	//! to @ref DYNAMIC_ARRAY_SIZE. Unlike @ref Vector, the size of one array cannot be changed after the array is created.
	//! If the size of the array is determined in compile time, the memory for elements will be allocated directly in the 
	//! array object; if the size of the array is determined in run time, the memory for elements will be allocated dynamically
	//! on heap.
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

		//! Gets a refernece of the element at the specified index.
		//! @param[in] pos The index of the element.
		//! @return Returns a reference of the specified element.
		constexpr reference at(usize pos) { lucheck(pos < _Size); return m_elements[pos]; }
		//! Gets a const refernece of the element at the specified index.
		//! @param[in] pos The index of the element.
		//! @return Returns a const reference of the specified element.
		constexpr const_reference at(usize pos) const { lucheck(pos < _Size); return m_elements[pos]; }
		//! Gets a refernece of the element at the specified index.
		//! @param[in] pos The index of the element.
		//! @return Returns a reference of the specified element.
		constexpr reference operator[](usize pos) { lucheck(pos < _Size); return m_elements[pos]; }
		//! Gets a constant refernece of the element at the specified index.
		//! @param[in] pos The index of the element.
		//! @return Returns a constant reference of the specified element.
		constexpr const_reference operator[](usize pos) const { lucheck(pos < _Size); return m_elements[pos]; }
		//! Gets a reference to the first (index 0) element in the array.
		//! @return Returns a reference to the first element in the array.
		constexpr reference front() { return m_elements[0]; }
		//! Gets a constant reference to the first (index 0) element in the array.
		//! @return Returns a constant reference to the first element in the array.
		constexpr const_reference front() const { return m_elements[0]; }
		//! Gets a reference to the last (index `size()` - 1) element in the array.
		//! @return Returns a reference to the first element in the array.
		constexpr reference back() { return m_elements[_Size - 1]; }
		//! Gets a reference to the last (index `size()` - 1) element in the array.
		//! @return Returns a reference to the first element in the array.
		constexpr const_reference back() const { return m_elements[_Size - 1]; }
		//! Gets one pointer to the array data memory.
		//! @return Returns one pointer to the array data memory.
		constexpr _Ty* data() { return m_elements; }
		//! Gets one constant pointer to the array data memory.
		//! @return Returns one constant pointer to the array data memory.
		constexpr const _Ty* data() const { return m_elements; }
		//! Gets one iterator to the first element of the array.
		//! @return Returns one iterator to the first element of the array.
		constexpr iterator begin() { return m_elements; }
		//! Gets one constant iterator to the first element of the array.
		//! @return Returns one constant iterator to the first element of the array.
		constexpr const_iterator begin() const { return m_elements; }
		//! Gets one constant iterator to the first element of the array.
		//! @return Returns one constant iterator to the first element of the array.
		constexpr const_iterator cbegin() const { return m_elements; }
		//! Gets one iterator to the one past last element of the array.
		//! @return Returns one iterator to the one past last element of the array.
		constexpr iterator end() { return m_elements + _Size; }
		//! Gets one constant iterator to the one past last element of the array.
		//! @return Returns one constant iterator to the one past last element of the array.
		constexpr const_iterator end() const { return m_elements + _Size; }
		//! Gets one constant iterator to the one past last element of the array.
		//! @return Returns one constant iterator to the one past last element of the array.
		constexpr const_iterator cend() const { return m_elements + _Size; }
		//! Gets one reverse iterator to the last element of the array.
		//! @return Returns one reverse iterator to the last element of the array.
		constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
		//! Gets one constant reverse iterator to the last element of the array.
		//! @return Returns one constant reverse iterator to the last element of the array.
		constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		//! Gets one constant reverse iterator to the last element of the array.
		//! @return Returns one constant reverse iterator to the last element of the array.
		constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
		//! Gets one reverse iterator to the one-before-first element of the array.
		//! @return Returns one reverse iterator to the one-before-first element of the array.
		constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
		//! Gets one constant reverse iterator to the one-before-first element of the array.
		//! @return Returns one constant reverse iterator to the one-before-first element of the array.
		constexpr const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		//! Gets one constant reverse iterator to the one-before-first element of the array.
		//! @return Returns one constant reverse iterator to the one-before-first element of the array.
		constexpr const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
		//! Checks whether this array is empty, that is, the size of this array is `0`.
		//! @return Returns `true` if this array is empty, returns `false` otherwise.
		constexpr bool empty() const { return false; }
		//! Gets the size of the array.
		//! @return Returns the size of the array.
		constexpr usize size() const { return _Size; }
		//! Assigns every element in the array with the specified value.
		//! @param[in] value The value to fill.
		constexpr void fill(const _Ty& value)
		{
			fill_assign_range(m_elements, m_elements + _Size, value);
		}
		//! Swaps content of this array with another array of the same element type and size.
		//! @param[in] rhs The array to swap content with.
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

	//! @}
}