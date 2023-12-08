/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RingDeque.hpp
* @author JXMaster
* @date 2020/2/16
*/
#pragma once
#include "Allocator.hpp"
#include "Algorithm.hpp"
#include "Iterator.hpp"
#include "MemoryUtils.hpp"

namespace Luna
{
	namespace RingDequeImpl
	{
		inline constexpr usize round_idx(isize index, usize buf_size)
		{
			return (index >= 0) ? (index % buf_size) : (usize)(((index + 1) % (isize)buf_size) + buf_size - 1);
		}

		template <typename _Ty, bool _Const>
		class Iterator
		{
		public:
			using value_type = _Ty;
			using pointer = conditional_t<_Const, const value_type*, value_type*>;
			using reference = conditional_t<_Const, const value_type&, value_type&>;
			using iterator_category = random_access_iterator_tag;

			value_type* m_buffer;		// The first element in the buffer.
			usize m_buffer_size;		// The size of the buffer.
			isize m_cur;			// The current index relative to `m_buffer` (may be negative or exceeds the range and will be rounded).

			Iterator() :
				m_buffer(nullptr),
				m_buffer_size(0),
				m_cur(0) {}
			Iterator(const Iterator<_Ty, false>& rhs) :
				m_buffer(rhs.m_buffer),
				m_buffer_size(rhs.m_buffer_size),
				m_cur(rhs.m_cur) {}
			Iterator(value_type* buf, usize buf_sz, isize cur) :
				m_buffer(buf),
				m_buffer_size(buf_sz),
				m_cur(cur) {}

			pointer operator->() const
			{
				return m_buffer + round_idx(m_cur, m_buffer_size);
			}
			reference operator*() const
			{
				return m_buffer[round_idx(m_cur, m_buffer_size)];
			}
			Iterator& operator++()
			{
				++m_cur;
				return *this;
			}
			Iterator  operator++(int)
			{
				Iterator tmp(*this);
				operator++();
				return tmp;
			}
			Iterator& operator--()
			{
				--m_cur;
				return *this;
			}
			Iterator  operator--(int)
			{
				Iterator tmp(*this);
				operator--();
				return tmp;
			}
			Iterator& operator+=(isize n)
			{
				m_cur += n;
				return *this;
			}
			Iterator& operator-=(isize n)
			{
				return (*this).operator+=(-n);
			}
			Iterator operator+(isize n) const
			{
				return Iterator(*this).operator+=(n);
			}
			Iterator operator-(isize n) const
			{
				return Iterator(*this).operator+=(-n);
			}
		};
    
        template <typename _Ty, bool _Const1, bool _Const2>
        bool operator==(const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
        {
            return lhs.m_cur == rhs.m_cur;
        }

        template <typename _Ty, bool _Const1, bool _Const2>
        bool operator!=(const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
        {
            return lhs.m_cur != rhs.m_cur;
        }

        template <typename _Ty, bool _Const1, bool _Const2>
        bool operator< (const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
        {
            return lhs.m_cur < rhs.m_cur;
        }

        template <typename _Ty, bool _Const1, bool _Const2>
        bool operator> (const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
        {
            return lhs.m_cur > rhs.m_cur;
        }
    
        template <typename _Ty, bool _Const1, bool _Const2>
        bool operator<=(const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
        {
            return lhs.m_cur <= rhs.m_cur;
        }

        template <typename _Ty, bool _Const1, bool _Const2>
        bool operator>=(const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
        {
            return lhs.m_cur >= rhs.m_cur;
        }

        template <typename _Ty, bool _Const>
        Iterator<_Ty, _Const> operator+(isize n, const Iterator<_Ty, _Const>& rhs)
        {
            return rhs + n;
        }

        template <typename _Ty, bool _Const1, bool _Const2>
        isize operator- (const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
        {
            return lhs.m_cur - rhs.m_cur;
        }
	}

	template <typename _Ty, typename _Alloc = Allocator>
	class RingDeque
	{
	public:
		using value_type = _Ty;
		using allocator_type = _Alloc;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = RingDequeImpl::Iterator<value_type, false>;
		using const_iterator = RingDequeImpl::Iterator<value_type, true>;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;

	private:
		// -------------------- Begin of ABI compatible part --------------------
		OptionalPair<allocator_type, _Ty*> m_allocator_and_buffer; // The memory buffer.
		usize m_capacity;		// The size of the memory buffer.
		isize m_begin;		// First element.
		isize m_end;		// Last element.
		// --------------------  End of ABI compatible part  --------------------

	public:
		iterator begin()
		{
			return iterator(m_allocator_and_buffer.second(), m_capacity, m_begin);
		}
		iterator end()
		{
			return iterator(m_allocator_and_buffer.second(), m_capacity, m_end);
		}
		const_iterator begin() const
		{
			return const_iterator(m_allocator_and_buffer.second(), m_capacity, m_begin);
		}
		const_iterator end() const
		{
			return const_iterator(m_allocator_and_buffer.second(), m_capacity, m_end);
		}
		const_iterator cbegin() const
		{
			return const_iterator(m_allocator_and_buffer.second(), m_capacity, m_begin);
		}
		const_iterator cend() const
		{
			return const_iterator(m_allocator_and_buffer.second(), m_capacity, m_end);
		}
		reverse_iterator rbegin()
		{
			return reverse_iterator(end());
		}
		reverse_iterator rend()
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator rbegin() const
		{
			return reverse_iterator(end());
		}
		const_reverse_iterator rend() const
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator crbegin() const
		{
			return reverse_iterator(cend());
		}
		const_reverse_iterator crend() const
		{
			return reverse_iterator(cbegin());
		}
		usize size() const
		{
			return (usize)(m_end - m_begin);
		}
		usize capacity() const
		{
			return m_capacity;
		}
		bool empty() const
		{
			return (size() == 0);
		}
	private:
		value_type* allocate(usize n)
		{
            return m_allocator_and_buffer.first().template allocate<value_type>(n);
		}

		void deallocate(value_type* ptr, usize n)
		{
            m_allocator_and_buffer.first().template deallocate<value_type>(ptr, n);
		}

		void free_buffer()
		{
			destruct_range(begin(), end());
			if (m_allocator_and_buffer.second())
			{
				deallocate(m_allocator_and_buffer.second(), m_capacity);
				m_allocator_and_buffer.second() = nullptr;
			}
			m_capacity = 0;
			m_begin = 0;
			m_end = 0;
		}
	public:
		RingDeque() :
			m_allocator_and_buffer(allocator_type(), nullptr),
			m_capacity(0),
			m_begin(0),
			m_end(0) {}
		RingDeque(const allocator_type& alloc) :
			m_allocator_and_buffer(alloc, nullptr),
			m_capacity(0),
			m_begin(0),
			m_end(0) {}
		RingDeque(const RingDeque& rhs) :
			m_allocator_and_buffer(rhs.m_allocator_and_buffer.first(), nullptr),
			m_capacity(0),
			m_begin(0),
			m_end(0)
		{
			reserve(rhs.m_capacity);
			usize begin_idx = RingDequeImpl::round_idx(rhs.m_begin, rhs.m_capacity);
			usize end_idx = RingDequeImpl::round_idx(rhs.m_end, rhs.m_capacity);
			end_idx = end_idx ? end_idx : rhs.m_capacity;
			value_type* buf = m_allocator_and_buffer.second();
			const value_type* rhs_buf = rhs.m_allocator_and_buffer.second();
			if (begin_idx < end_idx)
			{
				copy_construct_range(rhs_buf + begin_idx, rhs_buf + end_idx, buf + begin_idx);
			}
			else
			{
				copy_construct_range(rhs_buf + begin_idx, rhs_buf + m_capacity, buf + begin_idx);
				copy_construct_range(rhs_buf, rhs_buf + end_idx, buf);
			}
			m_begin = rhs.m_begin;
			m_end = rhs.m_end;
		}
		RingDeque(const RingDeque& rhs, const allocator_type& alloc) :
			m_allocator_and_buffer(alloc, nullptr),
			m_capacity(0),
			m_begin(0),
			m_end(0)
		{
			reserve(rhs.m_capacity);
			usize begin_idx = RingDequeImpl::round_idx(rhs.m_begin, rhs.m_capacity);
			usize end_idx = RingDequeImpl::round_idx(rhs.m_end, rhs.m_capacity);
			end_idx = end_idx ? end_idx : rhs.m_capacity;
			value_type* buf = m_allocator_and_buffer.second();
			const value_type* rhs_buf = rhs.m_allocator_and_buffer.second();
			if (begin_idx < end_idx)
			{
				copy_construct_range(rhs_buf + begin_idx, rhs_buf + end_idx, buf + begin_idx);
			}
			else
			{
				copy_construct_range(rhs_buf + begin_idx, rhs_buf + m_capacity, buf + begin_idx);
				copy_construct_range(rhs_buf, rhs_buf + end_idx, buf);
			}
			m_begin = rhs.m_begin;
			m_end = rhs.m_end;
		}
		RingDeque(RingDeque&& rhs) :
			m_allocator_and_buffer(move(rhs.m_allocator_and_buffer.first()), rhs.m_allocator_and_buffer.second()),
			m_capacity(rhs.m_capacity),
			m_begin(rhs.m_begin),
			m_end(rhs.m_end)
		{
			rhs.m_allocator_and_buffer.second() = nullptr;
			rhs.m_capacity = 0;
			rhs.m_begin = 0;
			rhs.m_end = 0;
		}
		RingDeque(RingDeque&& rhs, const allocator_type& alloc) :
			m_allocator_and_buffer(alloc, nullptr),
			m_capacity(0),
			m_begin(0),
			m_end(0)
		{
			if (m_allocator_and_buffer.first() == rhs.m_allocator_and_buffer.first())
			{
				m_allocator_and_buffer.second() = rhs.m_allocator_and_buffer.second();
				m_capacity = rhs.m_capacity;
				m_begin = rhs.m_begin;
				m_end = rhs.m_end;
				rhs.m_allocator_and_buffer.second() = nullptr;
				rhs.m_capacity = 0;
				rhs.m_begin = 0;
				rhs.m_end = 0;
			}
			else
			{
				reserve(rhs.m_capacity);
				usize begin_idx = RingDequeImpl::round_idx(rhs.m_begin, rhs.m_capacity);
				usize end_idx = RingDequeImpl::round_idx(rhs.m_end, rhs.m_capacity);
				end_idx = end_idx ? end_idx : rhs.m_capacity;
				value_type* buf = m_allocator_and_buffer.second();
				value_type* rhs_buf = rhs.m_allocator_and_buffer.second();
				if (begin_idx < end_idx)
				{
					move_construct_range(rhs_buf + begin_idx, rhs_buf + end_idx, buf + begin_idx);
				}
				else
				{
					move_construct_range(rhs_buf + begin_idx, rhs_buf + m_capacity, buf + begin_idx);
					move_construct_range(rhs_buf, rhs_buf + end_idx, buf);
				}
				m_begin = rhs.m_begin;
				m_end = rhs.m_end;
				rhs.clear();
			}
		}
		RingDeque& operator=(const RingDeque& rhs)
		{
			free_buffer();
			reserve(rhs.m_capacity);
			usize begin_idx = RingDequeImpl::round_idx(rhs.m_begin, rhs.m_capacity);
			usize end_idx = RingDequeImpl::round_idx(rhs.m_end, rhs.m_capacity);
			end_idx = end_idx ? end_idx : rhs.m_capacity;
			value_type* buf = m_allocator_and_buffer.second();
			const value_type* rhs_buf = rhs.m_allocator_and_buffer.second();
			if (begin_idx < end_idx)
			{
				copy_construct_range(rhs_buf + begin_idx, rhs_buf + end_idx, buf + begin_idx);
			}
			else
			{
				copy_construct_range(rhs_buf + begin_idx, rhs_buf + m_capacity, buf + begin_idx);
				copy_construct_range(rhs_buf, rhs_buf + end_idx, buf);
			}
			m_begin = rhs.m_begin;
			m_end = rhs.m_end;
		}
		RingDeque& operator=(RingDeque&& rhs)
		{
			free_buffer();
			if (m_allocator_and_buffer.first() == rhs.m_allocator_and_buffer.first())
			{
				Luna::swap(m_allocator_and_buffer.second(), rhs.m_allocator_and_buffer.second());
				Luna::swap(m_capacity, rhs.m_capacity);
				Luna::swap(m_begin, rhs.m_begin);
				Luna::swap(m_end, rhs.m_end);
			}
			else
			{
				reserve(rhs.m_capacity);
				usize begin_idx = RingDequeImpl::round_idx(rhs.m_begin, rhs.m_capacity);
				usize end_idx = RingDequeImpl::round_idx(rhs.m_end, rhs.m_capacity);
				end_idx = end_idx ? end_idx : rhs.m_capacity;
				value_type* buf = m_allocator_and_buffer.second();
				value_type* rhs_buf = rhs.m_allocator_and_buffer.second();
				if (begin_idx < end_idx)
				{
					move_construct_range(rhs_buf + begin_idx, rhs_buf + end_idx, buf + begin_idx);
				}
				else
				{
					move_construct_range(rhs_buf + begin_idx, rhs_buf + m_capacity, buf + begin_idx);
					move_construct_range(rhs_buf, rhs_buf + end_idx, buf);
				}
				m_begin = rhs.m_begin;
				m_end = rhs.m_end;
				rhs.clear();
			}
			return *this;
		}
		~RingDeque()
		{
			free_buffer();
		}
	private:
		// reallocates memory without touching the stored content. `new_cap` must be large enough to 
		// hold all elements.
		void internal_realloc(usize new_cap)
		{
			value_type* new_buf = allocate(new_cap);
			if (m_allocator_and_buffer.second())
			{
				value_type* buf = m_allocator_and_buffer.second();
				if (m_begin != m_end)
				{
					usize beg_idx = RingDequeImpl::round_idx(m_begin, m_capacity);
					usize end_idx = RingDequeImpl::round_idx(m_end, m_capacity);
					end_idx = end_idx ? end_idx : m_capacity;
					
					if (end_idx > beg_idx)
					{
						copy_relocate_range(buf + beg_idx, buf + end_idx, new_buf);
					}
					else
					{
						copy_relocate_range(buf + beg_idx, buf + m_capacity, new_buf);
						copy_relocate_range(buf, buf + end_idx, new_buf + m_capacity - beg_idx);
					}
				}
				deallocate(buf, m_capacity);
			}
			m_allocator_and_buffer.second() = new_buf;
			m_capacity = new_cap;
			usize sz = size();
			m_begin = 0;
			m_end = sz;
		}
	public:
		void reserve(usize new_cap)
		{
			if (new_cap > m_capacity)
			{
				internal_realloc(new_cap);
			}
		}
	private:
		void internal_expand_reserve(usize new_least_cap)
		{
			if (new_least_cap > m_capacity)
			{
				reserve(max(max(new_least_cap, m_capacity * 2), (usize)4));	// Double the size by default.
			}
		}
	public:
		void resize(usize n)
		{
			reserve(n);
			usize sz = size();
			value_type* buf = m_allocator_and_buffer.second();
			if (n > sz)
			{
				usize con_begin = RingDequeImpl::round_idx(m_end, m_capacity);
				usize con_end = RingDequeImpl::round_idx(m_begin + n, m_capacity);
				con_end = con_end ? con_end : m_capacity;
				if (con_begin <= con_end)
				{
					default_construct_range(buf + con_begin, buf + con_end);
				}
				else
				{
					default_construct_range(buf + con_begin, buf + m_capacity);
					default_construct_range(buf, buf + con_end);
				}
			}
			else if (n < sz)
			{
				usize con_begin = RingDequeImpl::round_idx(m_end, m_capacity);
				usize con_end = RingDequeImpl::round_idx(m_begin + n, m_capacity);
				con_end = con_end ? con_end : m_capacity;
				if (con_begin <= con_end)
				{
					destruct_range(buf + con_begin, buf + con_end);
				}
				else
				{
					destruct_range(buf + con_begin, buf + m_capacity);
					destruct_range(buf, buf + con_end);
				}
			}
			m_end = m_begin + n;
		}
		void resize(usize n, const value_type& v)
		{
			reserve(n);
			usize sz = size();
			value_type* buf = m_allocator_and_buffer.second();
			if (n > sz)
			{
				usize con_begin = RingDequeImpl::round_idx(m_end, m_capacity);
				usize con_end = RingDequeImpl::round_idx(m_begin + n, m_capacity);
				con_end = con_end ? con_end : m_capacity;
				if (con_begin <= con_end)
				{
					fill_construct_range(buf + con_begin, buf + con_end, v);
				}
				else
				{
					fill_construct_range(buf + con_begin, buf + m_capacity, v);
					fill_construct_range(buf, buf + con_end, v);
				}
			}
			else if (n < sz)
			{
				usize con_begin = RingDequeImpl::round_idx(m_end, m_capacity);
				usize con_end = RingDequeImpl::round_idx(m_begin + n, m_capacity);
				con_end = con_end ? con_end : m_capacity;
				if (con_begin <= con_end)
				{
					destruct_range(buf + con_begin, buf + con_end);
				}
				else
				{
					destruct_range(buf + con_begin, buf + m_capacity);
					destruct_range(buf, buf + con_end);
				}
			}
			m_end = m_begin + n;
		}
		void shrink_to_fit()
		{
			usize sz = size();
			if (m_capacity != sz)
			{
				if (!sz)
				{
					free_buffer();
					return;
				}
				else
				{
					internal_realloc(sz);
					return;
				}
			}
		}
		reference operator[] (usize n)
		{
			luassert(n < size());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_begin + n, m_capacity)];
		}
		const_reference operator[] (usize n) const
		{
			luassert(n < size());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_begin + n, m_capacity)];
		}
		reference at(usize n)
		{
			luassert(n < size());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_begin + n, m_capacity)];
		}
		const_reference at(usize n) const
		{
			luassert(n < size());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_begin + n, m_capacity)];
		}
		reference front()
		{
			luassert(!empty());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_begin, m_capacity)];
		}
		const_reference front() const
		{
			luassert(!empty());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_begin, m_capacity)];
		}
		reference back()
		{
			luassert(!empty());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_end - 1, m_capacity)];
		}
		const_reference back() const
		{
			luassert(!empty());
			return m_allocator_and_buffer.second()[RingDequeImpl::round_idx(m_end - 1, m_capacity)];
		}
		void clear()
		{
			destruct_range(begin(), end());
			m_end = m_begin;
		}
		void push_back(const value_type& val)
		{
			internal_expand_reserve(size() + 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_end, m_capacity)) value_type(val);
			++m_end;
		}
		void push_back(value_type&& val)
		{
			internal_expand_reserve(size() + 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_end, m_capacity)) value_type(move(val));
			++m_end;
		}
		void pop_back()
		{
			luassert(!empty());
			destruct(m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_end - 1, m_capacity));
			--m_end;
		}
		void push_front(const value_type& val)
		{
			internal_expand_reserve(size() + 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_begin - 1, m_capacity)) value_type(val);
			--m_begin;
		}
		void push_front(value_type&& val)
		{
			internal_expand_reserve(size() + 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_begin - 1, m_capacity)) value_type(move(val));
			--m_begin;
		}
		void pop_front()
		{
			luassert(!empty());
			destruct(m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_begin, m_capacity));
			++m_begin;
		}
		void assign(usize count, const value_type& value)
		{
			clear();
			reserve(count);
			fill_construct_range(m_allocator_and_buffer.second(), m_allocator_and_buffer.second() + count, value);
			m_begin = 0;
			m_end = count;
		}
		template <typename _InputIter>
		auto assign(_InputIter first, _InputIter last) -> enable_if_t<!is_integral_v<_InputIter>, void>
		{
			clear();
			for (auto iter = first; iter != last; ++iter)
			{
				push_back(*iter);
			}
		}
		void assign(InitializerList<value_type> il)
		{
			assign(il.begin(), il.end());
		}
	private:
		// first = begin() + pos
		void insert_move(isize first, usize count)
		{
			isize last = m_end;
			isize d_last = m_end + count;
			while (last != first)
			{
				// batch the call to move.
				usize last_idx = RingDequeImpl::round_idx(last, m_capacity);
				usize d_last_idx = RingDequeImpl::round_idx(d_last, m_capacity);
				last_idx = last_idx ? last_idx : m_capacity;
				d_last_idx = d_last_idx ? d_last_idx : m_capacity;
				usize batch_count = min(min(last_idx, d_last_idx), (usize)(last - first));

				value_type* buf = m_allocator_and_buffer.second();

				if (last_idx > d_last_idx)
				{
					// move forward.
					move_relocate_range(buf + last_idx - batch_count, buf + last_idx, buf + d_last_idx - batch_count);
				}
				else if(last_idx < d_last_idx)
				{
					// move backward.
					move_relocate_range_backward(buf + last_idx - batch_count, buf + last_idx, buf + d_last_idx);
				}

				last -= batch_count;
				d_last -= batch_count;
			}
		}
		// first = begin() + pos
		void erase_move(isize d_first, usize count)
		{
			isize first = d_first + count;
			isize last = m_end;
			while (first != last)
			{
				usize first_idx = RingDequeImpl::round_idx(first, m_capacity);
				usize d_first_idx = RingDequeImpl::round_idx(d_first, m_capacity);	// these two will never be m_buffer_size.
				usize batch_count = min(min(m_capacity - first_idx, m_capacity - d_first_idx), (usize)(last - first));

				value_type* buf = m_allocator_and_buffer.second();

				if (first_idx > d_first_idx)
				{
					// move forward.
					move_relocate_range(buf + first_idx, buf + first_idx + batch_count, buf + d_first_idx);
				}
				else if (first_idx < d_first_idx)
				{
					// move backward.
					move_relocate_range_backward(buf + first_idx, buf + first_idx + batch_count, buf + d_first_idx + batch_count);
				}
				first += batch_count;
				d_first += batch_count;
			}
		}


		// reserve `count` slots begin at index `pos`, the reserved part is uninitialized.
		// `pos` is the offsets relative to `begin()`.
		void insert_reserve(usize pos, usize count)
		{
			internal_expand_reserve(size() + count);
			insert_move(m_begin + pos, count);
			m_end += count;
		}

		//! Erases `count` elements start from `pos`.
		//! `pos` is the offsets relative to `begin()`.
		void internal_erase(usize pos, usize count)
		{
			erase_move(m_begin + pos, count);
			m_end -= count;
		}

	public:
		iterator insert(const_iterator pos, const value_type& val)
		{
			luassert((pos >= begin()) && (pos <= end()));
			usize idx = (usize)(pos - begin());	// the place to insert the element.
			insert_reserve(idx, 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_begin + idx, m_capacity)) value_type(val);
			return (begin() + idx);
		}
		iterator insert(const_iterator pos, value_type&& val)
		{
			luassert((pos >= begin()) && (pos <= end()));
			usize idx = (usize)(pos - begin());	// the place to insert the element.
			insert_reserve(idx, 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_begin + idx, m_capacity)) value_type(move(val));
			return (begin() + idx);
		}
		iterator insert(const_iterator pos, usize count, const value_type& val)
		{
			luassert((pos >= begin()) && (pos <= end()));
			usize idx = (usize)(pos - begin());	// the place to insert the element.
			insert_reserve(idx, count);
			auto pos2 = begin() + idx;
			fill_construct_range(pos2, pos2 + count, val);
			return (begin() + idx);
		}
		template <typename _InputIt>
		auto insert(const_iterator pos, _InputIt first, _InputIt last) -> enable_if_t<!is_integral_v<_InputIt>, iterator>
		{
			luassert((pos >= begin()) && (pos <= end()));
			usize idx = (usize)(pos - begin());	// the place to insert the element.
			for (auto iter = first; iter != last; ++iter)
			{
				pos = insert(pos, *iter);
				++pos;
			}
			return (begin() + idx);
		}
		iterator insert(const_iterator pos, InitializerList<value_type> il)
		{
			return insert(pos, il.begin(), il.end());
		}
		iterator erase(const_iterator pos)
		{
			luassert((pos >= begin()) && (pos <= end()));
			usize idx = (usize)(pos - begin());
			destruct(m_allocator_and_buffer.second() + RingDequeImpl::round_idx(pos.m_cur, m_capacity));
			internal_erase(idx, 1);
			return begin() + idx;
		}
		iterator erase(const_iterator first, const_iterator last)
		{
			luassert((first >= begin()) && (first <= end()));
			luassert((last >= begin()) && (last <= end()));
			luassert(first <= last);
			usize pos = (usize)(first - begin());
			usize sz = (usize)(last - first);
			if (!sz)
			{
				return begin() + pos;
			}
			destruct_range(m_allocator_and_buffer.second() + RingDequeImpl::round_idx(first.m_cur, m_capacity), 
				m_allocator_and_buffer.second() + RingDequeImpl::round_idx(last.m_cur, m_capacity));
			internal_erase(pos, sz);
			return begin() + pos;
		}
		void swap(RingDeque& rhs)
		{
			RingDeque tmp(move(*this));
			(*this) = move(rhs);
			rhs = move(tmp);
		}
		template <typename... _Args>
		iterator emplace(const_iterator pos, _Args&&... args)
		{
			luassert((pos >= begin()) && (pos <= end()));
			usize idx = (usize)(pos - begin());	// the place to insert the element.
			insert_reserve(idx, 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_begin + idx, m_capacity)) value_type(forward<_Args>(args)...);
			return begin() + idx;
		}
		template <typename... _Args>
		iterator emplace_back(_Args&&... args)
		{
			internal_expand_reserve(size() + 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_end, m_capacity)) value_type(forward<_Args>(args)...);
			++m_end;
			return end() - 1;
		}
		template <typename... _Args>
		iterator emplace_front(_Args&&... args)
		{
			internal_expand_reserve(size() + 1);
			new (m_allocator_and_buffer.second() + RingDequeImpl::round_idx(m_begin - 1, m_capacity)) value_type(forward<_Args>(args)...);
			--m_begin;
			return begin();
		}
		allocator_type get_allocator() const
		{
			return m_allocator_and_buffer.first();
		}
	};
}
