/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Span.hpp
* @author JXMaster
* @date 2022/9/20
*/
#pragma once
#include "Iterator.hpp"
#include "Assert.hpp"

namespace Luna
{
	//! Represents one span whose size is determined at runtime.
	inline constexpr usize DYNAMIC_EXTENT = USIZE_MAX;

	//! Represents one reference to one continuous sequence of instances. The size of the span is fixed and specified as template argument.
	template <typename _Ty, usize _Size = DYNAMIC_EXTENT>
	class Span
	{
	public:
		using element_type = _Ty;
		using value_type = remove_cv_t<_Ty>;
		using pointer = _Ty*;
		using const_pointer = const _Ty*;
		using reference = _Ty&;
		using const_reference = const _Ty&;
		using iterator = pointer;
		using reverse_iterator = ReverseIterator<iterator>;

		//! Constructs one empty span.
		constexpr Span() :
			m_buffer(nullptr) {}
		//! Constructs one span by providing the referred range directly.
		//! @param[in] data The pointer to the first element in the range.
		constexpr Span(element_type* arr) :
			m_buffer(arr) {}
		//! Constructs one span using the data provided by the specified initializer list.
		//! @param[in] ilist The initializer list used.
		//! @remark This is only used to specify one initializer list for one Span parameter of one function. 
		//! Since the initializer list exists only in the expression evaluation scope, you can not refer
		//! one initializer list on one lvalue span (spans exist as local variables, member variables or global/static variables),
		//! or the behavior is undefined.
		//! @par Valid Usage
		//! * `ilist.size()` must be greater than or equal to `_Size`.
		constexpr Span(InitializerList<remove_cv_t<_Ty>> ilist) :
			m_buffer(ilist.begin())
		{
			static_assert(ilist.size() >= _Size, "The size of the initializer list must be not smaller than the size of the span.");
		}
		//! Constructs one span by coping data from another span.
		//! @param[in] rhs The span to copy data from.
		constexpr Span(const Span& rhs) = default;
		//! Assigns one span by coping data from another span.
		//! @param[in] rhs The span to copy data from.
		//! @return Returns `*this`.
		constexpr Span& operator=(const Span& rhs) = default;

		//! Gets one iterator to the first element of the span.
		//! @return Returns one iterator to the first element of the span.
		constexpr iterator begin() const { return m_buffer; }
		//! Gets one iterator to the one past last element of the span.
		//! @return Returns one iterator to the one past last element of the span.
		constexpr iterator end() const { return m_buffer + _Size; }
		//! Gets one reverse iterator to the last element of the span.
		//! @return Returns one reverse iterator to the last element of the span.
		constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
		//! Gets one reverse iterator to the one-before-first element of the span.
		//! @return Returns one reverse iterator to the one-before-first element of the span.
		constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

		//! Gets a reference to the first (index 0) element in the span.
		//! @return Returns a reference to the first element in the span.
		constexpr reference front() const { return *m_buffer; }
		//! Gets a reference to the last (index `size()` - 1) element in the span.
		//! @return Returns a reference to the first element in the span.
		constexpr reference back() const { return *(m_buffer + _Size - 1); }
		//! Gets a refernece of the element at the specified index.
		//! @param[in] pos The index of the element.
		//! @return Returns a reference of the specified element.
		constexpr reference operator[](usize index) const { lucheck(index < _Size); return m_buffer[index]; }
		//! Gets one pointer to the span data memory.
		//! @return Returns one pointer to the span data memory.
		constexpr pointer data() const { return m_buffer; }

		//! Gets the size (number of elements) of the span.
		//! @return Returns the size of the span.
		constexpr usize size() const { return _Size; }
		//! Gets the size of the span in bytes, which is `size() * sizeof(element_type)`.
		//! @return Returns the size of the span  in bytes.
		constexpr usize size_bytes() const { return _Size * sizeof(element_type); }
		//! Checks whether this span is empty, that is, the size of this span is `0`.
		//! @return Returns `true` if this span is empty, returns `false` otherwise.
		constexpr bool empty() const { return _Size == 0; }

		//! Creates a new fixed-sized span referring the first `_Count` elements of this span.
		//! @return Returns one span referring the first `_Count` elements of this span.
		template <usize _Count>
		constexpr Span<element_type, _Count> first() const { return Span<element_type, _Count>(m_buffer); }
		//! Creates a new dynamic-sized span referring the first `count` elements of this span.
		//! @param[in] count The size of the new span.
		//! @return Returns one span referring the first `count` elements of this span.
		constexpr Span<element_type, DYNAMIC_EXTENT> first(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer, count); }

		//! Creates a new fixed-sized span referring the last `_Count` elements of this span.
		//! @return Returns one span referring the last `_Count` elements of this span.
		template <usize _Count>
		constexpr Span<element_type, _Count> last() const { return Span<element_type, _Count>(m_buffer + _Size - _Count); }
		//! Creates a new dynamic-sized span referring the last `count` elements of this span.
		//! @param[in] count The size of the new span.
		//! @return Returns one span referring the last `count` elements of this span.
		constexpr Span<element_type, DYNAMIC_EXTENT> last(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + (_Size - count), count); }

		//! Creates a new fixed-sized span referring `_Count` elements beginning at `offset` of this span.
		//! @param[in] offset The index of the first element to refer for the new span.
		//! @return Returns one span referring `_Count` elements beginning at `offset` of this span.
		template <usize _Count>
		constexpr Span<element_type, _Count> subspan(usize offset) const { return Span<element_type, _Count>(m_buffer + offset); }
		//! Creates a new dynamic-sized span referring `count` elements beginning at `offset` of this span.
		//! @param[in] offset The index of the first element to refer for the new span.
		//! @param[in] count The size of the new span.
		//! @return Returns one span referring `count` elements beginning at `offset` of this span.
		constexpr Span<element_type, DYNAMIC_EXTENT> subspan(usize offset, usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + offset, count); }

	private:
		_Ty* m_buffer;
	};

	//! Represents one reference to one continuous sequence of instances. The size of the span is determined at runtime.
	template <typename _Ty>
	class Span<_Ty, DYNAMIC_EXTENT>
	{
	public:
		using element_type = _Ty;
		using value_type = remove_cv_t<_Ty>;
		using pointer = _Ty*;
		using const_pointer = const _Ty*;
		using reference = _Ty&;
		using const_reference = const _Ty&;
		using iterator = pointer;
		using reverse_iterator = ReverseIterator<iterator>;

		//! Constructs one empty span.
		constexpr Span() :
			m_buffer(nullptr),
			m_size(0) {}
		//! Constructs one span by providing the referred range directly.
		//! @param[in] data The pointer to the first element in the range.
		//! @param[in] size The size of the range.
		constexpr Span(element_type* data, usize size) :
			m_buffer(data),
			m_size(size) {}
		//! Constructs one span using the data provided by the specified initializer list.
		//! @param[in] ilist The initializer list used.
		//! @remark This is only used to specify one initializer list for one Span parameter of one function. 
		//! Since the initializer list exists only in the expression evaluation scope, you can not refer
		//! one initializer list on one lvalue span (spans exist as local variables, member variables or global/static variables),
		//! or the behavior is undefined.
		constexpr Span(InitializerList<remove_cv_t<_Ty>> ilist) :
			m_buffer((_Ty*)ilist.begin()),
			m_size(ilist.size()) {}
		//! Constructs one span by coping data from another span.
		//! @param[in] rhs The span to copy data from.
		constexpr Span(const Span& rhs) :
            m_buffer(rhs.m_buffer),
            m_size(rhs.m_size) {}
		//! Assigns one span by coping data from another span.
		//! @param[in] rhs The span to copy data from.
		//! @return Returns `*this`.
		constexpr Span& operator=(const Span& rhs)
        {
            m_buffer = rhs.m_buffer;
            m_size = rhs.m_size;
            return *this;
        }

		//! Gets one iterator to the first element of the span.
		//! @return Returns one iterator to the first element of the span.
		constexpr iterator begin() const { return m_buffer; }
		//! Gets one iterator to the one past last element of the span.
		//! @return Returns one iterator to the one past last element of the span.
		constexpr iterator end() const { return m_buffer + m_size; }
		//! Gets one reverse iterator to the last element of the span.
		//! @return Returns one reverse iterator to the last element of the span.
		constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
		//! Gets one reverse iterator to the one-before-first element of the span.
		//! @return Returns one reverse iterator to the one-before-first element of the span.
		constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

		//! Gets a reference to the first (index 0) element in the span.
		//! @return Returns a reference to the first element in the span.
		constexpr reference front() const { return *m_buffer; }
		//! Gets a reference to the last (index `size()` - 1) element in the span.
		//! @return Returns a reference to the first element in the span.
		constexpr reference back() const { return *(m_buffer + m_size - 1); }
		//! Gets a refernece of the element at the specified index.
		//! @param[in] pos The index of the element.
		//! @return Returns a reference of the specified element.
		constexpr reference operator[](usize index) const { lucheck(index < m_size); return m_buffer[index]; }
		//! Gets one pointer to the span data memory.
		//! @return Returns one pointer to the span data memory.
		constexpr pointer data() const { return m_buffer; }

		//! Gets the size (number of elements) of the span.
		//! @return Returns the size of the span.
		constexpr usize size() const { return m_size; }
		//! Gets the size of the span in bytes, which is `size() * sizeof(element_type)`.
		//! @return Returns the size of the span  in bytes.
		constexpr usize size_bytes() const { return m_size * sizeof(element_type); }
		//! Checks whether this span is empty, that is, the size of this span is `0`.
		//! @return Returns `true` if this span is empty, returns `false` otherwise.
		constexpr bool empty() const { return m_size == 0; }

		//! Creates a new fixed-sized span referring the first `_Count` elements of this span.
		//! @return Returns one span referring the first `_Count` elements of this span.
		template <usize _Count>
		constexpr Span<element_type, _Count> first() const { return Span<element_type, _Count>(m_buffer); }
		//! Creates a new dynamic-sized span referring the first `count` elements of this span.
		//! @param[in] count The size of the new span.
		//! @return Returns one span referring the first `count` elements of this span.
		constexpr Span<element_type, DYNAMIC_EXTENT> first(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer, count); }

		//! Creates a new fixed-sized span referring the last `_Count` elements of this span.
		//! @return Returns one span referring the last `_Count` elements of this span.
		template <usize _Count>
		constexpr Span<element_type, _Count> last() const { return Span<element_type, _Count>(m_buffer + m_size - _Count); }
		//! Creates a new dynamic-sized span referring the last `count` elements of this span.
		//! @param[in] count The size of the new span.
		//! @return Returns one span referring the last `count` elements of this span.
		constexpr Span<element_type, DYNAMIC_EXTENT> last(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + (m_size - count), count); }

		//! Creates a new fixed-sized span referring `_Count` elements beginning at `offset` of this span.
		//! @param[in] offset The index of the first element to refer for the new span.
		//! @return Returns one span referring `_Count` elements beginning at `offset` of this span.
		template <usize _Count>
		constexpr Span<element_type, _Count> subspan(usize offset) const { return Span<element_type, _Count>(m_buffer + offset); }
		//! Creates a new dynamic-sized span referring `count` elements beginning at `offset` of this span.
		//! @param[in] offset The index of the first element to refer for the new span.
		//! @param[in] count The size of the new span.
		//! @return Returns one span referring `count` elements beginning at `offset` of this span.
		constexpr Span<element_type, DYNAMIC_EXTENT> subspan(usize offset, usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + offset, count); }

	private:
		_Ty* m_buffer;
		usize m_size;
	};
}
