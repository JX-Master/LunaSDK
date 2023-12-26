/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Vector.inl
* @author JXMaster
* @date 2023/12/5
*/
#pragma once
#include "../Vector.hpp"

namespace Luna
{
    // ABI assert.
	static_assert(sizeof(Vector<usize, Allocator>) == sizeof(usize) * 3, "Vector size does not match.");

	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector() :
		m_allocator_buffer(allocator_type(), nullptr),
		m_size(0),
		m_capacity(0) {}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(const allocator_type& alloc) :
		m_allocator_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0) {}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(usize count, const value_type& value, const allocator_type& alloc) :
		m_allocator_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0)
	{
		if (count)
		{
			reserve(count);
			fill_construct_range(m_allocator_buffer.second(), m_allocator_buffer.second() + count, value);
			m_size = count;
		}
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(usize count, const allocator_type& alloc) :
		m_allocator_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0)
	{
		if (count)
		{
			reserve(count);
			default_construct_range(m_allocator_buffer.second(), m_allocator_buffer.second() + count);
			m_size = count;
		}
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(const Vector& rhs) :
		m_allocator_buffer(rhs.m_allocator_buffer.first(), nullptr),
		m_size(0),
		m_capacity(0)
	{
		if (rhs.m_size)
		{
			reserve(rhs.m_size);
			// Copy the elements directly.
			copy_construct_range(rhs.begin(), rhs.end(), m_allocator_buffer.second());
			m_size = rhs.size();
		}
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(const Vector& rhs, const allocator_type& alloc) :
		m_allocator_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0)
	{
		if (rhs.m_size)
		{
			reserve(rhs.m_size);
			// Copy the elements directly.
			copy_construct_range(rhs.begin(), rhs.end(), m_allocator_buffer.second());
			m_size = rhs.size();
		}
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(Vector&& rhs) :
		m_allocator_buffer(move(rhs.m_allocator_buffer.first()), nullptr),
		m_size(0),
		m_capacity(0)
	{
		m_allocator_buffer.second() = rhs.m_allocator_buffer.second();
		m_size = rhs.m_size;
		m_capacity = rhs.m_capacity;
		rhs.m_allocator_buffer.second() = nullptr;
		rhs.m_size = 0;
		rhs.m_capacity = 0;
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(Vector&& rhs, const allocator_type& alloc) :
		m_allocator_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0)
	{
		if (m_allocator_buffer.first() == rhs.m_allocator_buffer.first())
		{
			m_allocator_buffer.second() = rhs.m_allocator_buffer.second();
			m_size = rhs.m_size;
			m_capacity = rhs.m_capacity;
			rhs.m_allocator_buffer.second() = nullptr;
			rhs.m_size = 0;
			rhs.m_capacity = 0;
		}
		else
		{
			reserve(rhs.m_size);
			// Copy the elements directly.
			move_construct_range(rhs.begin(), rhs.end(), m_allocator_buffer.second());
			m_size = rhs.size();
			rhs.clear();
		}
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::Vector(InitializerList<value_type> init, const allocator_type& alloc) :
		m_allocator_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0)
	{
		if (init.size())
		{
			reserve(init.size());
			move_construct_range(init.begin(), init.end(), m_allocator_buffer.second());
			m_size = init.size();
		}
	}
	template <typename _Ty, typename _Alloc>
	template <typename _Iter>
	inline void Vector<_Ty, _Alloc>::internal_construct(enable_if_t<is_same_v<remove_cv_t<_Iter>, value_type*>, _Iter> first, _Iter last)
	{
		usize count = last - first;
		if (count)
		{
			reserve(count);
			copy_construct_range(first, last, m_allocator_buffer.second());
			m_size = count;
		}
	}
	template <typename _Ty, typename _Alloc>
	template <typename _Iter>
	inline void Vector<_Ty, _Alloc>::internal_construct(enable_if_t<!is_same_v<remove_cv_t<_Iter>, value_type*>, _Iter> first, _Iter last)
	{
		for (; first != last; ++first)
		{
			push_back(*first);
		}
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIt>
	inline Vector<_Ty, _Alloc>::Vector(enable_if_t<!is_integral_v<_InputIt>, _InputIt> first, _InputIt last, const allocator_type& alloc) :
		m_allocator_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0)
	{
		internal_construct(first, last);
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>& Vector<_Ty, _Alloc>::operator=(const Vector& rhs)
	{
		clear();
		reserve(rhs.m_size);
		// Copy the elements directly.
		copy_construct_range(rhs.begin(), rhs.end(), m_allocator_buffer.second());
		m_size = rhs.size();
		return *this;
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>& Vector<_Ty, _Alloc>::operator=(Vector&& rhs)
	{
		clear();
		Luna::swap(m_allocator_buffer.second(), rhs.m_allocator_buffer.second());
		Luna::swap(m_size, rhs.m_size);
		Luna::swap(m_capacity, rhs.m_capacity);
		return *this;
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>& Vector<_Ty, _Alloc>::operator=(InitializerList<value_type> ilist)
	{
		clear();
		reserve(ilist.size());
		copy_construct_range(ilist.begin(), ilist.end(), m_allocator_buffer.second());
		m_size = ilist.size();
		return *this;
	}
	template <typename _Ty, typename _Alloc>
	inline Vector<_Ty, _Alloc>::~Vector()
	{
		free_buffer();
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::begin()
	{
		return m_allocator_buffer.second();
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::end()
	{
		return m_allocator_buffer.second() + m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_iterator Vector<_Ty, _Alloc>::begin() const
	{
		return m_allocator_buffer.second();
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_iterator Vector<_Ty, _Alloc>::end() const
	{
		return m_allocator_buffer.second() + m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_iterator Vector<_Ty, _Alloc>::cbegin() const
	{
		return m_allocator_buffer.second();
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_iterator Vector<_Ty, _Alloc>::cend() const
	{
		return m_allocator_buffer.second() + m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::reverse_iterator Vector<_Ty, _Alloc>::rbegin()
	{
		return reverse_iterator(end());
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::reverse_iterator Vector<_Ty, _Alloc>::rend()
	{
		return reverse_iterator(begin());
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reverse_iterator Vector<_Ty, _Alloc>::rbegin() const
	{
		return const_reverse_iterator(end());
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reverse_iterator Vector<_Ty, _Alloc>::rend() const
	{
		return const_reverse_iterator(begin());
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reverse_iterator Vector<_Ty, _Alloc>::crbegin() const
	{
		return const_reverse_iterator(cend());
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reverse_iterator Vector<_Ty, _Alloc>::crend() const
	{
		return const_reverse_iterator(cbegin());
	}
	template <typename _Ty, typename _Alloc>
	inline usize Vector<_Ty, _Alloc>::size() const
	{
		return m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline usize Vector<_Ty, _Alloc>::capacity() const
	{
		return m_capacity;
	}
	template <typename _Ty, typename _Alloc>
	inline bool Vector<_Ty, _Alloc>::empty() const
	{
		return (m_size == 0);
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::reserve(usize new_cap)
	{
		if (new_cap > m_capacity)
		{
			value_type* new_buf = internal_allocate(new_cap);
			if (m_allocator_buffer.second())
			{
				copy_relocate_range(begin(), end(), new_buf);
				internal_free(m_allocator_buffer.second(), m_capacity);
			}
			m_allocator_buffer.second() = new_buf;
			m_capacity = new_cap;
		}
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::resize(usize n)
	{
		reserve(n);
		if (n > m_size)
		{
			default_construct_range(m_allocator_buffer.second() + m_size, m_allocator_buffer.second() + n);
		}
		else if (n < m_size)
		{
			destruct_range(m_allocator_buffer.second() + n, m_allocator_buffer.second() + m_size);
		}
		m_size = n;
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::resize(usize n, const value_type& v)
	{
		reserve(n);
		if (n > m_size)
		{
			fill_construct_range(m_allocator_buffer.second() + m_size, m_allocator_buffer.second() + n, v);
		}
		else if (n < m_size)
		{
			destruct_range(m_allocator_buffer.second() + n, m_allocator_buffer.second() + m_size);
		}
		m_size = n;
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::shrink_to_fit()
	{
		if (m_capacity != m_size)
		{
			if (!m_size)
			{
				free_buffer();
			}
			else
			{
				value_type* new_buf = internal_allocate(m_size);
				if (m_allocator_buffer.second())
				{
					copy_relocate_range(m_allocator_buffer.second(), m_allocator_buffer.second() + m_size, new_buf);
					internal_free(m_allocator_buffer.second(), m_capacity);
				}
				m_allocator_buffer.second() = new_buf;
				m_capacity = m_size;
			}
		}
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::reference Vector<_Ty, _Alloc>::operator[] (usize n)
	{
		lucheck(n < m_size);
		return m_allocator_buffer.second()[n];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reference Vector<_Ty, _Alloc>::operator[] (usize n) const
	{
		lucheck(n < m_size);
		return m_allocator_buffer.second()[n];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::reference Vector<_Ty, _Alloc>::at(usize n)
	{
		lucheck(n < m_size);
		return m_allocator_buffer.second()[n];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reference Vector<_Ty, _Alloc>::at(usize n) const
	{
		lucheck(n < m_size);
		return m_allocator_buffer.second()[n];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::reference Vector<_Ty, _Alloc>::front()
	{
		lucheck(!empty());
		return m_allocator_buffer.second()[0];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reference Vector<_Ty, _Alloc>::front() const
	{
		lucheck(!empty());
		return m_allocator_buffer.second()[0];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::reference Vector<_Ty, _Alloc>::back()
	{
		lucheck(!empty());
		return m_allocator_buffer.second()[m_size - 1];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_reference Vector<_Ty, _Alloc>::back() const
	{
		lucheck(!empty());
		return m_allocator_buffer.second()[m_size - 1];
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::pointer Vector<_Ty, _Alloc>::data()
	{
		return m_allocator_buffer.second();
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::const_pointer Vector<_Ty, _Alloc>::data() const
	{
		return m_allocator_buffer.second();
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::clear()
	{
		destruct_range(begin(), end());
		m_size = 0;
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::push_back(const value_type& val)
	{
		internal_expand_reserve(size() + 1);
		new (m_allocator_buffer.second() + m_size) value_type(val);
		++m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::push_back(value_type&& val)
	{
		internal_expand_reserve(size() + 1);
		new (m_allocator_buffer.second() + m_size) value_type(move(val));
		++m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::pop_back()
	{
		lucheck(!empty());
		destruct(m_allocator_buffer.second() + m_size - 1);
		--m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::assign(usize count, const value_type& value)
	{
		clear();
		reserve(count);
		if (count)
		{
			fill_construct_range(m_allocator_buffer.second(), m_allocator_buffer.second() + count, value);
		}
		m_size = count;
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIter>
	inline auto Vector<_Ty, _Alloc>::internal_assign(_InputIter first, _InputIter last) -> enable_if_t<is_same_v<remove_cv_t<_InputIter>, value_type*>, void>
	{
		usize count = last - first;
		clear();
		reserve(count);
		copy_construct_range(first, last, m_allocator_buffer.second());
		m_size = count;
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIter>
	inline auto Vector<_Ty, _Alloc>::internal_assign(_InputIter first, _InputIter last) -> enable_if_t<!is_same_v<remove_cv_t<_InputIter>, value_type*>, void>
	{
		clear();
		for (; first != last; ++first)
		{
			push_back(*first);
		}
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIter>
	inline auto Vector<_Ty, _Alloc>::assign(_InputIter first, _InputIter last) -> enable_if_t<!is_integral_v<_InputIter>, void>
	{
		internal_assign(first, last);
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::assign(InitializerList<value_type> il)
	{
		assign(il.begin(), il.end());
	}
	template <typename _Ty, typename _Alloc>
	template <typename _Rty>
	inline void Vector<_Ty, _Alloc>::assign(Span<_Rty> data)
	{
		clear();
		reserve(data.size());
		copy_construct_range(data.begin(), data.end(), m_allocator_buffer.second());
		m_size = data.size();
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::insert(const_iterator pos, const value_type& val)
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos <= (usize)(m_allocator_buffer.second() + m_size)));
		usize index = pos - cbegin();
		internal_expand_reserve(m_size + 1);
		pos = begin() + index;
		if (pos != end())
		{
			move_relocate_range_backward((value_type*)pos, (value_type*)end(), (value_type*)end() + 1);
		}
		new ((void*)(m_allocator_buffer.second() + index)) value_type(val);
		++m_size;
		return const_cast<iterator>(pos);
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::insert(const_iterator pos, value_type&& val)
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos <= (usize)(m_allocator_buffer.second() + m_size)));
		usize index = pos - cbegin();
		internal_expand_reserve(m_size + 1);
		pos = begin() + index;
		if (pos != end())
		{
			move_relocate_range_backward((value_type*)pos, (value_type*)end(), (value_type*)end() + 1);
		}
		new ((void*)pos) value_type(move(val));
		++m_size;
		return const_cast<iterator>(pos);
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::insert(const_iterator pos, usize count, const value_type& val)
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos <= (usize)(m_allocator_buffer.second() + m_size)));
		usize index = pos - cbegin();
		internal_expand_reserve(m_size + count);
		pos = begin() + index;
		if (pos != end())
		{
			move_relocate_range_backward((value_type*)pos, (value_type*)end(), (value_type*)end() + count);
		}
		fill_construct_range(m_allocator_buffer.second() + index, m_allocator_buffer.second() + index + count, val);
		m_size += count;
		return const_cast<iterator>(pos);
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIt>
	inline auto Vector<_Ty, _Alloc>::internal_insert(const_iterator pos, _InputIt first, _InputIt last) -> enable_if_t<is_same_v<remove_cv_t<_InputIt>, value_type*>, Vector<_Ty, _Alloc>::iterator>
	{
		usize count = (last - first);
		return insert_n(pos, first, count);
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIt>
	inline auto Vector<_Ty, _Alloc>::internal_insert(const_iterator pos, _InputIt first, _InputIt last) -> enable_if_t<!is_same_v<remove_cv_t<_InputIt>, value_type*>, Vector<_Ty, _Alloc>::iterator>
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos <= (usize)(m_allocator_buffer.second() + m_size)));
		usize index = pos - cbegin();
		for (auto iter = first; iter != last; ++iter)
		{
			pos = insert(pos, *iter);
			++pos;
		}
		return begin() + index;
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIt>
	inline auto Vector<_Ty, _Alloc>::insert(const_iterator pos, _InputIt first, _InputIt last) -> enable_if_t<!is_integral_v<_InputIt>, iterator>
	{
		return internal_insert(pos, first, last);
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::insert(const_iterator pos, InitializerList<value_type> il)
	{
		return insert(pos, il.begin(), il.end());
	}
	template <typename _Ty, typename _Alloc>
	template <typename _Rty>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::insert(const_iterator pos,  Span<_Rty> data)
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos <= (usize)(m_allocator_buffer.second() + m_size)));
		usize index = pos - cbegin();
		internal_expand_reserve(data.size() + m_size);
		pos = begin() + index;
		if (pos != end())
		{
			move_relocate_range_backward((value_type*)pos, (value_type*)end(), (value_type*)end() + data.size());
		}
		copy_construct_range(data.begin(), data.end(), (value_type*)pos);
		m_size += data.size();
		return const_cast<iterator>(pos);
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::erase(const_iterator pos)
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos < (usize)(m_allocator_buffer.second() + m_size)));
		((value_type*)pos)->~value_type();
		if (pos != (end() - 1))
		{
			move_relocate_range((value_type*)pos + 1, (value_type*)end(), (value_type*)pos);
		}
		--m_size;
		return const_cast<iterator>(pos);
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::erase(const_iterator first, const_iterator last)
	{
		lucheck(((usize)first >= (usize)m_allocator_buffer.second()) && ((usize)first < (usize)(m_allocator_buffer.second() + m_size)));
		lucheck(((usize)last >= (usize)m_allocator_buffer.second()) && ((usize)last <= (usize)(m_allocator_buffer.second() + m_size)));
		destruct_range((value_type*)first, (value_type*)last);
		if (last != end())
		{
			move_relocate_range((value_type*)last, (value_type*)end(), (value_type*)first);
		}
		m_size -= (last - first);
		return const_cast<iterator>(first);
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::swap_erase(const_iterator pos)
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos < (usize)(m_allocator_buffer.second() + m_size)));
		((value_type*)pos)->~value_type();
		if ((pos + 1) != end())
		{
			copy_relocate((value_type*)pos, &back());
		}
		--m_size;
		return const_cast<iterator>(pos);
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::swap(Vector& rhs)
	{
		Vector tmp(move(*this));
		(*this) = move(rhs);
		rhs = move(tmp);
	}
	template <typename _Ty, typename _Alloc>
	template <typename... _Args>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::emplace(const_iterator pos, _Args&&... args)
	{
		lucheck(((usize)pos >= (usize)m_allocator_buffer.second()) && ((usize)pos <= (usize)(m_allocator_buffer.second() + m_size)));
		usize index = pos - cbegin();
		internal_expand_reserve(m_size + 1);
		pos = begin() + index;
		if (pos != end())
		{
			move_relocate_range_backward((value_type*)pos, (value_type*)end(), (value_type*)end() + 1);
		}
		new ((void*)pos) value_type(forward<_Args>(args)...);
		++m_size;
		return const_cast<iterator>(pos);
	}
	template <typename _Ty, typename _Alloc>
	template <typename... _Args>
	inline typename Vector<_Ty, _Alloc>::iterator Vector<_Ty, _Alloc>::emplace_back(_Args&&... args)
	{
		internal_expand_reserve(size() + 1);
		new (m_allocator_buffer.second() + m_size) value_type(forward<_Args>(args)...);
		++m_size;
		return (m_allocator_buffer.second() + m_size - 1);
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::allocator_type Vector<_Ty, _Alloc>::get_allocator() const
	{
		return m_allocator_buffer.first();
	}
	template <typename _Ty, typename _Alloc>
	inline Span<_Ty> Vector<_Ty, _Alloc>::span()
	{
		return Span<_Ty>(data(), size());
	}
	template <typename _Ty, typename _Alloc>
	inline Span<const _Ty> Vector<_Ty, _Alloc>::span() const
	{
		return Span<const _Ty>(data(), size());
	}
	template <typename _Ty, typename _Alloc>
	inline Span<const _Ty> Vector<_Ty, _Alloc>::cspan() const
	{
		return Span<const _Ty>(data(), size());
	}

	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::free_buffer()
	{
		destruct_range(begin(), end());
		if (m_allocator_buffer.second())
		{
			internal_free(m_allocator_buffer.second(), m_capacity);
			m_allocator_buffer.second() = nullptr;
		}
		m_size = 0;
		m_capacity = 0;
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::internal_expand_reserve(usize new_least_cap)
	{
		if (new_least_cap > m_capacity)
		{
			reserve(max(max(new_least_cap, m_capacity * 2), (usize)4));	// Double the size by default.
		}
	}
	template <typename _Ty, typename _Alloc>
	inline typename Vector<_Ty, _Alloc>::value_type* Vector<_Ty, _Alloc>::internal_allocate(usize n)
	{
        return m_allocator_buffer.first().template allocate<_Ty>(n);
	}
	template <typename _Ty, typename _Alloc>
	inline void Vector<_Ty, _Alloc>::internal_free(value_type* ptr, usize n)
	{
        m_allocator_buffer.first().template deallocate<_Ty>(ptr, n);
	}
}