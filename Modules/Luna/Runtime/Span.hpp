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
	inline constexpr usize DYNAMIC_EXTENT = USIZE_MAX;

	//! Represents one reference to one continuous sequence of instances.
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

		constexpr Span() :
			m_buffer(nullptr) {}
		constexpr Span(element_type* arr) :
			m_buffer(arr) {}
		constexpr Span(const Span& rhs) = default;
		constexpr Span& operator=(const Span& rhs) = default;

		constexpr iterator begin() const { return m_buffer; }
		constexpr iterator end() const { return m_buffer + _Size; }
		constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
		constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

		constexpr reference front() const { return *m_buffer; }
		constexpr reference back() const { return *(m_buffer + _Size - 1); }
		constexpr reference operator[](usize index) const { lucheck(index < _Size); return m_buffer[index]; }
		constexpr pointer data() const { return m_buffer; }

		constexpr usize size() const { return _Size; }
		constexpr usize size_bytes() const { return _Size * sizeof(element_type); }
		constexpr bool empty() const { return _Size == 0; }

		template <usize _Count>
		constexpr Span<element_type, _Count> first() const { return Span<element_type, _Count>(m_buffer); }
		constexpr Span<element_type, DYNAMIC_EXTENT> first(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer, count); }

		template <usize _Count>
		constexpr Span<element_type, _Count> last() const { return Span<element_type, _Count>(m_buffer + _Size - _Count); }
		constexpr Span<element_type, DYNAMIC_EXTENT> last(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + (_Size - count), count); }

		template <usize _Count>
		constexpr Span<element_type, _Count> subspan(usize offset) const { return Span<element_type, _Count>(m_buffer + offset); }
		constexpr Span<element_type, DYNAMIC_EXTENT> subspan(usize offset, usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + offset, count); }

	private:
		_Ty* m_buffer;
	};

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

		constexpr Span() :
			m_buffer(nullptr),
			m_size(0) {}
		constexpr Span(element_type* arr, usize size) :
			m_buffer(arr),
			m_size(size) {}
		constexpr Span(InitializerList<remove_cv_t<_Ty>> ilist) :
			m_buffer((_Ty*)ilist.begin()),
			m_size(ilist.size()) {}
		constexpr Span(const Span& rhs) :
            m_buffer(rhs.m_buffer),
            m_size(rhs.m_size) {}
		constexpr Span& operator=(const Span& rhs)
        {
            m_buffer = rhs.m_buffer;
            m_size = rhs.m_size;
            return *this;
        }

		constexpr iterator begin() const { return m_buffer; }
		constexpr iterator end() const { return m_buffer + m_size; }
		constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
		constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

		constexpr reference front() const { return *m_buffer; }
		constexpr reference back() const { return *(m_buffer + m_size - 1); }
		constexpr reference operator[](usize index) const { lucheck(index < m_size); return m_buffer[index]; }
		constexpr pointer data() const { return m_buffer; }

		constexpr usize size() const { return m_size; }
		constexpr usize size_bytes() const { return m_size * sizeof(element_type); }
		constexpr bool empty() const { return m_size == 0; }

		template <usize _Count>
		constexpr Span<element_type, _Count> first() const { return Span<element_type, _Count>(m_buffer); }
		constexpr Span<element_type, DYNAMIC_EXTENT> first(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer, count); }

		template <usize _Count>
		constexpr Span<element_type, _Count> last() const { return Span<element_type, _Count>(m_buffer + m_size - _Count); }
		constexpr Span<element_type, DYNAMIC_EXTENT> last(usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + (m_size - count), count); }

		template <usize _Count>
		constexpr Span<element_type, _Count> subspan(usize offset) const { return Span<element_type, _Count>(m_buffer + offset); }
		constexpr Span<element_type, DYNAMIC_EXTENT> subspan(usize offset, usize count) const { return Span<element_type, DYNAMIC_EXTENT>(m_buffer + offset, count); }

	private:
		_Ty* m_buffer;
		usize m_size;
	};
}
