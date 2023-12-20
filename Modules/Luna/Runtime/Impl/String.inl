/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file String.inl
* @author JXMaster
* @date 2023/12/18
*/
#pragma once
#include "../String.hpp"

namespace Luna
{
    template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString() :
		m_allocator_and_buffer(allocator_type(), nullptr),
		m_size(0),
		m_capacity(0) {}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0) {}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(usize count, value_type ch, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr),
		m_size(count),
		m_capacity(count)
	{
		if (count)
		{
			m_allocator_and_buffer.second() = allocate(count + 1);
			for (value_type* i = m_allocator_and_buffer.second(); i < m_allocator_and_buffer.second() + m_size; ++i)
			{
				*i = ch;
			}
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
		else
		{
			m_allocator_and_buffer.second() = nullptr;
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(const BasicString& rhs, usize pos, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr)
	{
		usize count = rhs.size() - pos;
		m_size = count;
		m_capacity = count;
		if (count)
		{
			m_allocator_and_buffer.second() = allocate(count + 1);
			memcpy(m_allocator_and_buffer.second(), rhs.c_str() + pos, sizeof(value_type) * count);
			m_allocator_and_buffer.second()[count] = (value_type)0;
		}
		else
		{
			m_allocator_and_buffer.second() = nullptr;
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(const BasicString& rhs, usize pos, usize count, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr)
	{
		count = (count == npos) ? rhs.size() - pos : count;
		m_size = count;
		m_capacity = count;
		if (count)
		{
			m_allocator_and_buffer.second() = allocate(count + 1);
			memcpy(m_allocator_and_buffer.second(), rhs.c_str() + pos, sizeof(value_type) * count);
			m_allocator_and_buffer.second()[count] = (value_type)0;
		}
		else
		{
			m_allocator_and_buffer.second() = nullptr;
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(const value_type* s, usize count, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr),
		m_size(count),
		m_capacity(count)
	{
		if (count)
		{
			m_allocator_and_buffer.second() = allocate(count + 1);
			memcpy(m_allocator_and_buffer.second(), s, sizeof(value_type) * count);
			m_allocator_and_buffer.second()[count] = (value_type)0;
		}
		else
		{
			m_allocator_and_buffer.second() = nullptr;
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(const value_type* s, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr)
	{
		usize count = strlength(s);
		m_size = count;
		m_capacity = count;
		if (count)
		{
			m_allocator_and_buffer.second() = allocate(count + 1);
			memcpy(m_allocator_and_buffer.second(), s, sizeof(value_type) * count);
			m_allocator_and_buffer.second()[count] = (value_type)0;
		}
		else
		{
			m_allocator_and_buffer.second() = nullptr;
		}
	}
	template <typename _Char, typename _Alloc>
	template <typename _InputIt>
	inline BasicString<_Char, _Alloc>::BasicString(_InputIt first, _InputIt last, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr),
		m_size(0),
		m_capacity(0)
	{
		for (; first != last; ++first)
		{
			push_back(*first);
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(const BasicString& rhs) :
		m_allocator_and_buffer(rhs.m_allocator_and_buffer.first(), nullptr)
	{
		if (!rhs.empty())
		{
			m_size = rhs.m_size;
			m_capacity = rhs.m_size;
			m_allocator_and_buffer.second() = allocate(m_size + 1);
			memcpy(m_allocator_and_buffer.second(), rhs.m_allocator_and_buffer.second(), sizeof(value_type) * (m_size + 1));
		}
		else
		{
			m_size = 0;
			m_capacity = 0;
			m_allocator_and_buffer.second() = nullptr;
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(const BasicString& rhs, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr)
	{
		if (!rhs.empty())
		{
			m_size = rhs.m_size;
			m_capacity = rhs.m_size;
			m_allocator_and_buffer.second() = allocate(m_size + 1);
			memcpy(m_allocator_and_buffer.second(), rhs.m_allocator_and_buffer.second(), sizeof(value_type) * (m_size + 1));
		}
		else
		{
			m_size = 0;
			m_capacity = 0;
			m_allocator_and_buffer.second() = nullptr;
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(BasicString&& rhs) :
		m_allocator_and_buffer(move(rhs.m_allocator_and_buffer.first()), rhs.m_allocator_and_buffer.second()),
		m_size(rhs.m_size),
		m_capacity(rhs.m_capacity)
	{
		rhs.m_allocator_and_buffer.second() = nullptr;
		rhs.m_size = 0;
		rhs.m_capacity = 0;
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(BasicString&& rhs, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr)
	{
		if (m_allocator_and_buffer.first() == rhs.m_allocator_and_buffer.first())
		{
			m_allocator_and_buffer.second() = rhs.m_allocator_and_buffer.second();
			m_size = rhs.m_size;
			m_capacity = rhs.m_capacity;
			rhs.m_allocator_and_buffer.second() = nullptr;
			rhs.m_size = 0;
			rhs.m_capacity = 0;
		}
		else
		{
			if (!rhs.empty())
			{
				m_size = rhs.m_size;
				m_capacity = rhs.m_size;
				m_allocator_and_buffer.second() = allocate(m_size + 1);
				memcpy(m_allocator_and_buffer.second(), rhs.m_allocator_and_buffer.second(), sizeof(value_type) * (m_size + 1));
				rhs.clear();
			}
			else
			{
				m_size = 0;
				m_capacity = 0;
				m_allocator_and_buffer.second() = nullptr;
			}
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::BasicString(InitializerList<value_type> ilist, const allocator_type& alloc) :
		m_allocator_and_buffer(alloc, nullptr),
		m_size(ilist.size()),
		m_capacity(ilist.size())
	{
		if (m_size)
		{
			m_allocator_and_buffer.second() = allocate(ilist.size() + 1);
			value_type* i = m_allocator_and_buffer.second();
			for (auto iter = ilist.begin(); iter != ilist.end(); ++iter)
			{
				*i = *iter;
				++i;
			}
			m_allocator_and_buffer.second()[m_size] = 0;
		}
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>& BasicString<_Char, _Alloc>::operator=(const BasicString& rhs)
	{
		free_buffer();
		if (!rhs.empty())
		{
			reserve(rhs.m_size);
			memcpy(m_allocator_and_buffer.second(), rhs.c_str(), (rhs.size() + 1) * sizeof(value_type));
			m_size = rhs.size();
		}
		return *this;
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>& BasicString<_Char, _Alloc>::operator=(BasicString&& rhs)
	{
		free_buffer();
		if (m_allocator_and_buffer.first() == rhs.m_allocator_and_buffer.first())
		{
			Luna::swap(m_allocator_and_buffer.second(), rhs.m_allocator_and_buffer.second());
			Luna::swap(m_size, rhs.m_size);
			Luna::swap(m_capacity, rhs.m_capacity);
		}
		else
		{
			if (!rhs.empty())
			{
				m_size = rhs.m_size;
				m_capacity = rhs.m_size;
				m_allocator_and_buffer.second() = allocate(m_size + 1);
				memcpy(m_allocator_and_buffer.second(), rhs.m_allocator_and_buffer.second(), sizeof(value_type) * (m_size + 1));
				rhs.clear();
			}
		}
		return *this;
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>& BasicString<_Char, _Alloc>::operator=(const value_type* s)
	{
		clear();
		usize count = strlen(s);
		reserve(count);
		if (count)
		{
			memcpy(m_allocator_and_buffer.second(), s, sizeof(value_type) * count);

		}
		m_size = count;
		if (m_allocator_and_buffer.second())
		{
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
		return *this;
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>& BasicString<_Char, _Alloc>::operator=(value_type ch)
	{
		clear();
		reserve(1);
		m_allocator_and_buffer.second()[0] = ch;
		m_size = 1;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
		return *this;
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>& BasicString<_Char, _Alloc>::operator=(InitializerList<value_type> ilist)
	{
		clear();
		reserve(ilist.size());
		if (ilist.size())
		{
			value_type* i = m_allocator_and_buffer.second();
			for (auto iter = ilist.begin(); iter != ilist.end(); ++iter)
			{
				*i = *iter;
				++i;
			}
		}
		m_size = ilist.size();
		if (m_allocator_and_buffer.second())
		{
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
		return *this;
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc>::~BasicString()
	{
		free_buffer();
	}
	//! May returns `nullptr` if the string is empty.
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::pointer BasicString<_Char, _Alloc>::data()
	{
		return m_allocator_and_buffer.second();
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_pointer BasicString<_Char, _Alloc>::data() const
	{
		return m_allocator_and_buffer.second();
	}
	//! Unlike `data`, this call always returns a valid string, not `nullptr`, even if the string is empty.
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_pointer BasicString<_Char, _Alloc>::c_str() const
	{
		return m_allocator_and_buffer.second() ? m_allocator_and_buffer.second() : Impl::StringTraits<value_type>::null_string;
	}
	// Returns a pointer to the first element. Can only be `nullptr` if `size` and `capacity` is both 0.
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::begin()
	{
		return m_allocator_and_buffer.second();
	}
	// Returns a pointer to the element next to the last element. Can only be `nullptr` if `size` and `capacity` is both 0.
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::end()
	{
		return m_allocator_and_buffer.second() + m_size;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_iterator BasicString<_Char, _Alloc>::begin() const
	{
		return m_allocator_and_buffer.second();
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_iterator BasicString<_Char, _Alloc>::end() const
	{
		return m_allocator_and_buffer.second() + m_size;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_iterator BasicString<_Char, _Alloc>::cbegin() const
	{
		return m_allocator_and_buffer.second();
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_iterator BasicString<_Char, _Alloc>::cend() const
	{
		return m_allocator_and_buffer.second() + m_size;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::reverse_iterator BasicString<_Char, _Alloc>::rbegin()
	{
		return reverse_iterator(end());
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::reverse_iterator BasicString<_Char, _Alloc>::rend()
	{
		return reverse_iterator(begin());
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reverse_iterator BasicString<_Char, _Alloc>::rbegin() const
	{
		return const_reverse_iterator(end());
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reverse_iterator BasicString<_Char, _Alloc>::rend() const
	{
		return const_reverse_iterator(begin());
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reverse_iterator BasicString<_Char, _Alloc>::crbegin() const
	{
		return const_reverse_iterator(cend());
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reverse_iterator BasicString<_Char, _Alloc>::crend() const
	{
		return const_reverse_iterator(cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::size() const
	{
		return m_size;
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::length() const
	{
		return m_size;
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::capacity() const
	{
		return m_capacity;
	}
	template <typename _Char, typename _Alloc>
	inline bool BasicString<_Char, _Alloc>::empty() const
	{
		return (m_size == 0);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::reserve(usize new_cap)
	{
		if (new_cap > m_capacity)
		{
			value_type* new_buf = allocate(new_cap + 1);
			if (m_allocator_and_buffer.second())
			{
				copy_relocate_range(begin(), end() + 1, new_buf);
				deallocate(m_allocator_and_buffer.second(), m_capacity);
			}
			m_allocator_and_buffer.second() = new_buf;
			m_capacity = new_cap;
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::resize(usize n, value_type v)
	{
		reserve(n);
		if (n > m_size)
		{
			fill_construct_range(m_allocator_and_buffer.second() + m_size, m_allocator_and_buffer.second() + n, v);
		}
		m_size = n;
		m_allocator_and_buffer.second()[n] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::shrink_to_fit()
	{
		if (m_capacity != m_size)
		{
			if (!m_size)
			{
				free_buffer();
			}
			else
			{
				value_type* new_buf = allocate(m_size + 1);
				if (m_allocator_and_buffer.second())
				{
					memcpy(new_buf, m_allocator_and_buffer.second(), sizeof(value_type) * (m_size + 1));
					deallocate(m_allocator_and_buffer.second(), m_capacity);
				}
				m_allocator_and_buffer.second() = new_buf;
				m_capacity = m_size;
			}
		}
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::reference BasicString<_Char, _Alloc>::operator[] (usize n)
	{
		luassert(n < m_size);
		return m_allocator_and_buffer.second()[n];
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reference BasicString<_Char, _Alloc>::operator[] (usize n) const
	{
		luassert(n < m_size);
		return m_allocator_and_buffer.second()[n];
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::reference BasicString<_Char, _Alloc>::at(usize n)
	{
		luassert(n < m_size);
		return m_allocator_and_buffer.second()[n];
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reference BasicString<_Char, _Alloc>::at(usize n) const
	{
		luassert(n < m_size);
		return m_allocator_and_buffer.second()[n];
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::reference BasicString<_Char, _Alloc>::front()
	{
		luassert(!empty());
		return m_allocator_and_buffer.second()[0];
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reference BasicString<_Char, _Alloc>::front() const
	{
		luassert(!empty());
		return m_allocator_and_buffer.second()[0];
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::reference BasicString<_Char, _Alloc>::back()
	{
		luassert(!empty());
		return m_allocator_and_buffer.second()[m_size - 1];
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::const_reference BasicString<_Char, _Alloc>::back() const
	{
		luassert(!empty());
		return m_allocator_and_buffer.second()[m_size - 1];
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::clear()
	{
		if (m_allocator_and_buffer.second())
		{
			m_allocator_and_buffer.second()[0] = (value_type)0;
		}
		m_size = 0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::push_back(value_type ch)
	{
		internal_expand_reserve(size() + 1);
		m_allocator_and_buffer.second()[m_size] = ch;
		++m_size;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::pop_back()
	{
		luassert(!empty());
		--m_size;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::assign(usize count, value_type ch)
	{
		clear();
		reserve(count);
		if (count)
		{
			fill_construct_range(m_allocator_and_buffer.second(), m_allocator_and_buffer.second() + count, ch);
			m_allocator_and_buffer.second()[count] = (value_type)0;
		}
		m_size = count;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::assign(const BasicString& str)
	{
		*this = str;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::assign(const BasicString& str, usize pos, usize count)
	{
		clear();
		count = (count == npos) ? str.size() - pos : count;
		reserve(count);
		if (count)
		{
			memcpy(m_allocator_and_buffer.second(), str.c_str() + pos, count * sizeof(value_type));
			m_allocator_and_buffer.second()[count] = (value_type)0;
		}
		m_size = count;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::assign(BasicString&& str)
	{
		free_buffer();
		m_allocator_and_buffer.second() = str.m_allocator_and_buffer.second();
		m_capacity = str.m_capacity;
		m_size = str.m_size;
		str.m_allocator_and_buffer.second() = nullptr;
		str.m_capacity = 0;
		str.m_size = 0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::assign(const value_type* s, usize count)
	{
		clear();
		reserve(count);
		if (count)
		{
			memcpy(m_allocator_and_buffer.second(), s, count * sizeof(value_type));
			m_allocator_and_buffer.second()[count] = (value_type)0;
			m_size = count;
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::assign(const value_type* s)
	{
		usize count = strlength(s);
		clear();
		reserve(count);
		if (count)
		{
			memcpy(m_allocator_and_buffer.second(), s, count * sizeof(value_type));
			m_allocator_and_buffer.second()[count] = (value_type)0;
			m_size = count;
		}
	}
	template <typename _Char, typename _Alloc>
	template <typename _InputIt>
	inline void BasicString<_Char, _Alloc>::assign(_InputIt first, _InputIt last)
	{
		clear();
		for (auto iter = first; iter != last; ++iter)
		{
			push_back(*iter);
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::assign(InitializerList<value_type> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::insert(usize index, usize count, value_type ch)
	{
		luassert(index <= m_size);
		internal_expand_reserve(m_size + count);
		if (index != m_size)
		{
			memmove(m_allocator_and_buffer.second() + index + count, m_allocator_and_buffer.second() + index, sizeof(value_type) * (m_size - index));
		}
		fill_construct_range(m_allocator_and_buffer.second() + index, m_allocator_and_buffer.second() + index + count, ch);
		m_size += count;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::insert(usize index, const value_type* s)
	{
		luassert(index <= m_size);
		usize count = strlength(s);
		internal_expand_reserve(m_size + count);
		if (index != m_size)
		{
			memmove(m_allocator_and_buffer.second() + index + count, m_allocator_and_buffer.second() + index, sizeof(value_type) * (m_size - index));
		}
		memcpy(m_allocator_and_buffer.second() + index, s, sizeof(value_type) * count);
		m_size += count;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::insert(usize index, const value_type* s, usize count)
	{
		luassert(index <= m_size);
		internal_expand_reserve(m_size + count);
		if (index != m_size)
		{
			memmove(m_allocator_and_buffer.second() + index + count, m_allocator_and_buffer.second() + index, sizeof(value_type) * (m_size - index));
		}
		memcpy(m_allocator_and_buffer.second() + index, s, sizeof(value_type) * count);
		m_size += count;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::insert(usize index, const BasicString& str)
	{
		luassert(index <= m_size);
		usize count = str.size();
		internal_expand_reserve(m_size + count);
		if (index != m_size)
		{
			memmove(m_allocator_and_buffer.second() + index + count, m_allocator_and_buffer.second() + index, sizeof(value_type) * (m_size - index));
		}
		memcpy(m_allocator_and_buffer.second() + index, str.c_str(), sizeof(value_type) * count);
		m_size += count;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::insert(usize index, const BasicString& str, usize index_str, usize count)
	{
		luassert(index <= m_size);
		count = min(count, str.size() - index_str);
		internal_expand_reserve(m_size + count);
		if (index != m_size)
		{
			memmove(m_allocator_and_buffer.second() + index + count, m_allocator_and_buffer.second() + index, sizeof(value_type) * (m_size - index));
		}
		memcpy(m_allocator_and_buffer.second() + index, str.c_str() + index_str, sizeof(value_type) * count);
		m_size += count;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::insert(const_iterator pos, value_type ch)
	{
		luassert(((usize)pos >= (usize)m_allocator_and_buffer.second()) && ((usize)pos <= (usize)(m_allocator_and_buffer.second() + m_size)));
		usize index = pos - cbegin();
		internal_expand_reserve(m_size + 1);
		auto mpos = begin() + index;
		if (mpos != end())
		{
			memmove(m_allocator_and_buffer.second() + index + 1, m_allocator_and_buffer.second() + index, sizeof(value_type) * (m_size - index));
		}
		m_allocator_and_buffer.second()[index] = ch;
		++m_size;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
		return mpos;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::insert(const_iterator pos, usize count, value_type ch)
	{
		luassert(((usize)pos >= (usize)m_allocator_and_buffer.second()) && ((usize)pos <= (usize)(m_allocator_and_buffer.second() + m_size)));
		usize index = pos - cbegin();
		internal_expand_reserve(m_size + count);
		auto mpos = begin() + index;
		if (mpos != end())
		{
			memmove(m_allocator_and_buffer.second() + index + count, m_allocator_and_buffer.second() + index, sizeof(value_type) * (m_size - index));
		}
		fill_construct_range(m_allocator_and_buffer.second() + index, m_allocator_and_buffer.second() + index + count, ch);
		m_size += count;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
		return mpos;
	}
	template <typename _Char, typename _Alloc>
	template <typename _InputIt>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::insert(const_iterator pos, _InputIt first, _InputIt last)
	{
		luassert(((usize)pos >= (usize)m_allocator_and_buffer.second()) && ((usize)pos <= (usize)(m_allocator_and_buffer.second() + m_size)));
		usize index = pos - cbegin();
		for (auto iter = first; iter != last; ++iter)
		{
			pos = insert(pos, *iter);
			++pos;
		}
		return begin() + index;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::insert(const_iterator pos, InitializerList<value_type> ilist)
	{
		return insert(pos, ilist.begin(), ilist.end());
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::erase(usize index, usize count)
	{
		count = min(m_size - index, count);
		luassert(index + count <= m_size);
		if ((index + count) != m_size)
		{
			memmove(m_allocator_and_buffer.second() + index, m_allocator_and_buffer.second() + index + count, sizeof(value_type) * (m_size - index - count));
		}
		m_size -= count;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::erase(const_iterator pos)
	{
		luassert(((usize)pos >= (usize)m_allocator_and_buffer.second()) && ((usize)pos < (usize)(m_allocator_and_buffer.second() + m_size)));
		if (pos != (end() - 1))
		{
			move_relocate_range((value_type*)pos + 1, (value_type*)end(), (value_type*)pos);
		}
		--m_size;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
		return const_cast<iterator>(pos);
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::iterator BasicString<_Char, _Alloc>::erase(const_iterator first, const_iterator last)
	{
		luassert(((usize)first >= (usize)m_allocator_and_buffer.second()) && ((usize)first < (usize)(m_allocator_and_buffer.second() + m_size)));
		luassert(((usize)last >= (usize)m_allocator_and_buffer.second()) && ((usize)last <= (usize)(m_allocator_and_buffer.second() + m_size)));
		if (last != end())
		{
			move_relocate_range((value_type*)last, (value_type*)end(), (value_type*)first);
		}
		m_size -= (last - first);
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
		return const_cast<iterator>(first);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::swap(BasicString& rhs)
	{
		BasicString tmp(move(*this));
		(*this) = move(rhs);
		rhs = move(tmp);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::append(usize count, value_type ch)
	{
		if (count)
		{
			internal_expand_reserve(m_size + count);
			fill_construct_range(m_allocator_and_buffer.second() + m_size, m_allocator_and_buffer.second() + m_size + count, ch);
			m_size += count;
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::append(const BasicString& str)
	{
		if (!str.empty())
		{
			internal_expand_reserve(m_size + str.size());
			memcpy(m_allocator_and_buffer.second() + m_size, str.c_str(), str.size() * sizeof(value_type));
			m_size += str.size();
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::append(const BasicString& str, usize pos, usize count)
	{
		count = min(count, str.size() - pos);
		if (count)
		{
			internal_expand_reserve(m_size + count);
			memcpy(m_allocator_and_buffer.second() + m_size, str.c_str() + pos, count * sizeof(value_type));
			m_size += count;
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::append(const value_type* s, usize count)
	{
		if (count)
		{
			internal_expand_reserve(m_size + count);
			memcpy(m_allocator_and_buffer.second() + m_size, s, count * sizeof(value_type));
			m_size += count;
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::append(const value_type* s)
	{
		usize count = strlength(s);
		if (count)
		{
			internal_expand_reserve(m_size + count);
			memcpy(m_allocator_and_buffer.second() + m_size, s, count * sizeof(value_type));
			m_size += count;
			m_allocator_and_buffer.second()[m_size] = (value_type)0;
		}
	}
	template <typename _Char, typename _Alloc>
	template <typename _InputIt>
	inline void BasicString<_Char, _Alloc>::append(_InputIt first, _InputIt last)
	{
		for (auto iter = first; iter != last; ++iter)
		{
			push_back(*iter);
		}
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::append(InitializerList<value_type> ilist)
	{
		append(ilist.begin(), ilist.end());
	}
	template <typename _Char, typename _Alloc>
	inline i32 BasicString<_Char, _Alloc>::compare(const BasicString& rhs) const
	{
		return strcmp(c_str(), rhs.c_str());
	}
	template <typename _Char, typename _Alloc>
	inline i32 BasicString<_Char, _Alloc>::compare(usize pos1, usize count1, const BasicString& rhs) const
	{
		count1 = min(count1, size() - pos1);
		return memcmp(c_str() + pos1, rhs.c_str(), count1 * sizeof(value_type));
	}
	template <typename _Char, typename _Alloc>
	inline i32 BasicString<_Char, _Alloc>::compare(usize pos1, usize count1, const BasicString& rhs, usize pos2, usize count2) const
	{
		count1 = min(count1, size() - pos1);
		count2 = min(count2, size() - pos2);
		return memcmp(c_str() + pos1, rhs.c_str() + pos2, min(count1, count2) * sizeof(value_type));
	}
	template <typename _Char, typename _Alloc>
	inline i32 BasicString<_Char, _Alloc>::compare(const value_type* s) const
	{
		return strcmp(c_str(), s);
	}
	template <typename _Char, typename _Alloc>
	inline i32 BasicString<_Char, _Alloc>::compare(usize pos1, usize count1, const value_type* s) const
	{
		count1 = min(count1, size() - pos1);
		return memcmp(c_str() + pos1, s, count1 * sizeof(value_type));
	}
	template <typename _Char, typename _Alloc>
	inline i32 BasicString<_Char, _Alloc>::compare(usize pos1, usize count1, const value_type* s, usize count2) const
	{
		count1 = min(count1, size() - pos1);
		return memcmp(c_str() + pos1, s, min(count1, count2) * sizeof(value_type));
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(usize pos, usize count, const BasicString& str)
	{
		count = min(count, m_size - pos);
		isize delta = str.size() - count;
		if (delta > 0)
		{
			internal_expand_reserve(m_size + delta);
		}
		memmove(m_allocator_and_buffer.second() + pos + str.size(), m_allocator_and_buffer.second() + pos + count, sizeof(value_type) * (m_size - pos - count));
		memcpy(m_allocator_and_buffer.second() + pos, str.c_str(), sizeof(value_type) * str.size());
		m_size += delta;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(const_iterator first, const_iterator last, const BasicString& str)
	{
		usize pos = first - m_allocator_and_buffer.second();
		usize count = last - first;
		replace(pos, count, str);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(usize pos, usize count, const BasicString& str, usize pos2, usize count2)
	{
		count = min(count, m_size - pos);
		count2 = min(count2, str.size() - pos2);
		isize delta = count2 - count;
		if (delta > 0)
		{
			internal_expand_reserve(m_size + delta);
		}
		memmove(m_allocator_and_buffer.second() + pos + count2, m_allocator_and_buffer.second() + pos + count, sizeof(value_type) * (m_size - pos - count));
		memcpy(m_allocator_and_buffer.second() + pos, str.c_str() + pos2, sizeof(value_type) * count2);
		m_size += delta;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	template <typename _InputIt>
	inline void BasicString<_Char, _Alloc>::replace(const_iterator first, const_iterator last, _InputIt first2, _InputIt last2)
	{
		usize pos = first - m_allocator_and_buffer.second();
		erase(first, last);
		insert(begin() + pos, first2, last2);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(usize pos, usize count, const value_type* cstr, usize count2)
	{
		count = min(count, m_size - pos);
		isize delta = count2 - count;
		if (delta > 0)
		{
			internal_expand_reserve(m_size + delta);
		}
		memmove(m_allocator_and_buffer.second() + pos + count2, m_allocator_and_buffer.second() + pos + count, sizeof(value_type) * (m_size - pos - count));
		memcpy(m_allocator_and_buffer.second() + pos, cstr, sizeof(value_type) * count2);
		m_size += delta;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(const_iterator first, const_iterator last, const value_type* cstr, usize count2)
	{
		usize pos = first - m_allocator_and_buffer.second();
		usize count = last - first;
		replace(pos, count, cstr, count2);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(usize pos, usize count, const value_type* cstr)
	{
		count = min(count, m_size - pos);
		usize count2 = strlength(cstr);
		replace(pos, count, cstr, count2);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(const_iterator first, const_iterator last, const value_type* cstr)
	{
		usize pos = first - m_allocator_and_buffer.second();
		usize count = last - first;
		replace(pos, count, cstr);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(usize pos, usize count, usize count2, value_type ch)
	{
		count = min(count, m_size - pos);
		isize delta = count2 - count;
		if (delta > 0)
		{
			internal_expand_reserve(m_size + delta);
		}
		memmove(m_allocator_and_buffer.second() + pos + count2, m_allocator_and_buffer.second() + pos + count, sizeof(value_type) * (m_size - pos - count));
		fill_construct_range(m_allocator_and_buffer.second() + pos, m_allocator_and_buffer.second() + pos + count2, ch);
		m_size += delta;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(const_iterator first, const_iterator last, usize count2, value_type ch)
	{
		usize pos = first - m_allocator_and_buffer.second();
		usize count = last - first;
		replace(pos, count, count2, ch);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::replace(const_iterator first, const_iterator last, InitializerList<value_type> ilist)
	{
		usize pos = first - m_allocator_and_buffer.second();
		usize count = last - first;
		isize delta = ilist.size() - count;
		if (delta > 0)
		{
			internal_expand_reserve(m_size + delta);
		}
		memmove(m_allocator_and_buffer.second() + pos + ilist.size(), m_allocator_and_buffer.second() + pos + count, sizeof(value_type) * (m_size - pos - count));
		auto iter = const_cast<iterator>(first);
		for (auto& i : ilist)
		{
			*iter = i;
			++iter;
		}
		m_size += delta;
		m_allocator_and_buffer.second()[m_size] = (value_type)0;
	}
	template <typename _Char, typename _Alloc>
	inline BasicString<_Char, _Alloc> BasicString<_Char, _Alloc>::substr(usize pos, usize count) const
	{
		luassert(pos <= m_size);
		count = min(count, m_size - pos);
		return BasicString(m_allocator_and_buffer.second() + pos, count, m_allocator_and_buffer.first());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::copy(value_type* dst, usize count, usize pos) const
	{
		luassert(pos <= m_size);
		count = min(count, m_size - pos);
		memcpy(dst, m_allocator_and_buffer.second() + pos, sizeof(value_type) * count);
		return count;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::allocator_type BasicString<_Char, _Alloc>::get_allocator() const
	{
		return m_allocator_and_buffer.first();
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::find(const BasicString& str, usize pos) const
	{
		if(pos >= size()) return npos;
		auto iter = search(cbegin() + pos, cend(), str.cbegin(), str.cend());
		return iter == cend() ? npos : (usize)(iter - cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::find(const value_type* s, usize pos, usize count) const
	{
		if(pos >= size()) return npos;
		auto iter = search(cbegin() + pos, cend(), s, s + count);
		return iter == cend() ? npos : (usize)(iter - cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::find(const value_type* s, usize pos) const
	{
		if(pos >= size()) return npos;
		auto count = strlen(s);
		auto iter = search(cbegin() + pos, cend(), s, s + count);
		return iter == cend() ? npos : (usize)(iter - cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::find(value_type ch, usize pos) const
	{
		if(pos >= size()) return npos;
		auto iter = Luna::find(cbegin(), cend(), ch);
		return iter == cend() ? npos : (usize)(iter - cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::rfind(const BasicString& str, usize pos) const
	{
		if(empty()) return npos;
		auto str_end = (pos >= size() - 1) ? cend() : cbegin() + pos + 1;
		auto iter = find_end(cbegin(), str_end, str.cbegin(), str.cend());
		return iter == str_end ? npos : (usize)(iter - cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::rfind(const value_type* s, usize pos, usize count) const
	{
		if(empty()) return npos;
		auto str_end = (pos >= size() - 1) ? cend() : cbegin() + pos + 1;
		auto iter = find_end(cbegin(), str_end, s, s + count);
		return iter == str_end ? npos : (usize)(iter - cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::rfind(const value_type* s, usize pos) const
	{
		if(empty()) return npos;
		auto count = strlen(s);
		auto str_end = (pos >= size() - 1) ? cend() : cbegin() + pos + 1;
		auto iter = find_end(cbegin(), str_end, s, s + count);
		return iter == str_end ? npos : (usize)(iter - cbegin());
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::rfind(value_type ch, usize pos) const
	{
		if(empty()) return npos;
		for(usize i = size() - 1; i > 0; --i)
		{
			if(at(i) == ch) return i;
		}
		return at(0) == ch ? 0 : npos;
	}
	template <typename _Char, typename _Alloc>
	inline typename BasicString<_Char, _Alloc>::value_type* BasicString<_Char, _Alloc>::allocate(usize n)
	{
        return m_allocator_and_buffer.first().template allocate<value_type>(n);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::deallocate(value_type* ptr, usize n)
	{
        m_allocator_and_buffer.first().template deallocate<value_type>(ptr, n);
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::free_buffer()
	{
		if (m_allocator_and_buffer.second())
		{
			deallocate(m_allocator_and_buffer.second(), m_capacity);
			m_allocator_and_buffer.second() = nullptr;
		}
		m_size = 0;
		m_capacity = 0;
	}
	template <typename _Char, typename _Alloc>
	inline usize BasicString<_Char, _Alloc>::strlength(const _Char* s)
	{
		usize count = 0;
		while (s[count] != (_Char)0) ++count;
		return count;
	}
	template <typename _Char, typename _Alloc>
	inline void BasicString<_Char, _Alloc>::internal_expand_reserve(usize new_least_cap)
	{
		if (new_least_cap > m_capacity)
		{
			reserve(max(max(new_least_cap, m_capacity * 2), (usize)4));	// Double the size by default.
		}
	}
}