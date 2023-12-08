/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RobinHoodHashTable.hpp
* @author JXMaster
* @date 2022/4/30
* @brief A hash table implementation that uses Robin Hood hashing.
*/
#pragma once
#include "../Base.hpp"
#include "../Functional.hpp"
#include "../Algorithm.hpp"
#include "../Allocator.hpp"
#include "HashTableBase.hpp"
#include <cmath> // for ceilf

namespace Luna
{
	namespace RobinHoodHashing
	{
		constexpr usize EMPTY_SLOT = 0;
#ifdef LUNA_PLATFORM_32BIT
		constexpr usize TOMBSTONE_BIT = 0x80000000;
#else
		constexpr usize TOMBSTONE_BIT = 0x8000000000000000;
#endif

		struct ControlBlock
		{
			usize m_hash;
			//usize m_dist;
		};
    
        inline bool is_tombstone(usize h)
        {
            return (h & TOMBSTONE_BIT) != 0;
        }

		template <typename _Ty, bool _Const>
		struct Iterator
		{
			using value_type = _Ty;
			using pointer = conditional_t<_Const, const value_type*, value_type*>;
			using reference = conditional_t<_Const, const value_type&, value_type&>;
			using iterator_category = forward_iterator_tag;

			pointer m_value;
			ControlBlock* m_cb;
			ControlBlock* m_end;	// to present shifting above end.

			Iterator(pointer value, ControlBlock* cb, ControlBlock* end) :
				m_value(value),
				m_cb(cb),
				m_end(end) {}
			Iterator(const Iterator<_Ty, false>& rhs)
			{
				m_value = rhs.m_value;
				m_cb = rhs.m_cb;
				m_end = rhs.m_end;
			}
			reference operator*() const
			{
				return *m_value;
			}
			pointer operator->() const
			{
				return m_value;
			}
			Iterator& operator++()
			{
				do
				{
					++m_value;
					++m_cb;
				} while ((m_cb != m_end) && (m_cb->m_hash == EMPTY_SLOT || is_tombstone(m_cb->m_hash)));
				return *this;
			}
			Iterator operator++(int)
			{
				Iterator temp(*this);
				do
				{
					++m_value;
					++m_cb;
				} while ((m_cb != m_end) && (m_cb->m_hash == EMPTY_SLOT || is_tombstone(m_cb->m_hash)));
				return temp;
			}
			bool operator==(const Iterator& rhs) const
			{
				return m_cb == rhs.m_cb;
			}
			bool operator!=(const Iterator& rhs) const
			{
				return m_cb != rhs.m_cb;
			}
		};

		inline usize probe_distance(usize h, usize pos, usize buffer_size)
		{
			usize desired_pos = (h & ~TOMBSTONE_BIT) % buffer_size;
			return (pos >= desired_pos) ? pos - desired_pos : buffer_size + pos - desired_pos;
		}

		//! h must not be EMPTY_SLOT, and must not have TOMBSTONE_BIT set.
		template <typename _Vty>
		inline usize robinhood_insert(usize h, _Vty* src_buf, _Vty* value_buf, ControlBlock* cb_buf, usize buffer_size)
		{
			luassert(h != EMPTY_SLOT && !is_tombstone(h));
			// Extract current data.
			usize pos = h % buffer_size;
			usize dist = 0;
			usize ret_pos = USIZE_MAX;
			while (true)
			{
				if (cb_buf[pos].m_hash == EMPTY_SLOT)
				{
					cb_buf[pos].m_hash = h;
					//cb_buf[pos].m_dist = dist;
					copy_relocate(value_buf + pos, src_buf);
					if (ret_pos == USIZE_MAX) ret_pos = pos;
					break;
				}
				usize existing_dist = probe_distance(cb_buf[pos].m_hash, pos, buffer_size);
				//usize existing_dist = cb_buf[pos].m_dist;
				if ((existing_dist <= dist) && is_tombstone(cb_buf[pos].m_hash))
				{
					cb_buf[pos].m_hash = h;
					//cb_buf[pos].m_dist = dist;
					copy_relocate(value_buf + pos, src_buf);
					if (ret_pos == USIZE_MAX) ret_pos = pos;
					break;
				}
				if (existing_dist < dist)
				{
					usize temp_h = cb_buf[pos].m_hash;
					cb_buf[pos].m_hash = h;
					h = temp_h;
					Unconstructed<_Vty> temp_v;
					copy_relocate(&(temp_v.get()), value_buf + pos);
					copy_relocate(value_buf + pos, src_buf);
					copy_relocate(src_buf, &(temp_v.get()));
					//cb_buf[pos].m_dist = dist;
					dist = existing_dist;
					if (ret_pos == USIZE_MAX) ret_pos = pos;
				}
				++pos;
				++dist;
				if (pos == buffer_size) 
					pos = 0;
			}
			return ret_pos;
		}

		constexpr usize INITIAL_BUFFER_SIZE = 16;
		constexpr f32 INITIAL_LOAD_FACTOR = 0.9f;

		template <typename _Kty,
			typename _Vty,
			typename _ExtractKey,				// MapExtractKey for UnorderedMap, SetExtractKey for UnorderedSet.
			typename _Hash = hash<_Kty>,		// Used to hash the key value.
			typename _KeyEqual = equal_to<_Kty>,
			typename _Alloc = Allocator>	// Used to compare the element.
		class HashTable
		{
		public:
			using key_type = _Kty;
			using value_type = _Vty;
			using allocator_type = _Alloc;
			using hasher = _Hash;
			using key_equal = _KeyEqual;
			using reference = value_type&;
			using const_reference = const value_type&;
			using pointer = value_type*;
			using const_pointer = const value_type*;
			using iterator = Iterator<value_type, false>;
			using const_iterator = Iterator<value_type, true>;
			using extract_key = _ExtractKey;

			// -------------------- Begin of ABI compatible part --------------------

			//! A pointer to the hash table, which is an array of elements.
			OptionalPair<allocator_type, value_type*> m_allocator_and_value_buffer;
			//! A pointer to control block buffer.
			ControlBlock* m_cb_buffer;
			//! The the size of value buffer and control block buffer.
			usize m_buffer_size;
			//! The number of elements in the hash table.
			usize m_size;
			//! The maximum load factor of the table, which determines how often the rehashing 
			//! will occur. The load factor can be calculated by `m_size / m_buffer_size`, which is
			//! (0.0, 1.0]. This is different from chaining (or closed hashing) hash table
			//! implementation, in which the load factor is typically greater than one.
			f32 m_max_load_factor;

			// --------------------  End of ABI compatible part  --------------------

		private:
			template <typename _Ty>
			_Ty* allocate(usize n)
			{
				return m_allocator_and_value_buffer.first().template allocate<_Ty>(n);
			}
			template <typename _Ty>
			void deallocate(_Ty* ptr, usize n)
			{
				m_allocator_and_value_buffer.first().template deallocate<_Ty>(ptr, n);
			}
			value_type* internal_alloc_value_buffer(usize cap)
			{
				value_type* buf = allocate<value_type>(cap);
				return buf;
			}
			ControlBlock* internal_alloc_cb_buffer(usize cap)
			{
				ControlBlock* buf = allocate<ControlBlock>(cap);
				memzero(buf, sizeof(ControlBlock) * cap);
				return buf;
			}
			void internal_free_table()
			{
				if (m_allocator_and_value_buffer.second())
				{
					deallocate<value_type>(m_allocator_and_value_buffer.second(), m_buffer_size);
					deallocate<ControlBlock>(m_cb_buffer, m_buffer_size);
					m_allocator_and_value_buffer.second() = nullptr;
				}
			}
			void internal_clear()
			{
				for (usize i = 0; i < m_buffer_size; ++i)
				{
					usize h = m_cb_buffer[i].m_hash;
					if (h != EMPTY_SLOT && !is_tombstone(h))
					{
						(m_allocator_and_value_buffer.second() + i)->~value_type();
					}
					m_cb_buffer[i].m_hash = EMPTY_SLOT;
				}
				m_size = 0;
			}
			void internal_clear_and_free_table()
			{
				for (usize i = 0; i < m_buffer_size; ++i)
				{
					usize h = m_cb_buffer[i].m_hash;
					if (h == EMPTY_SLOT || is_tombstone(h)) continue;
					(m_allocator_and_value_buffer.second() + i)->~value_type();
				}
				internal_free_table();
				m_buffer_size = 0;
				m_size = 0;
			}
		public:
			bool empty() const
			{
				return m_size == 0;
			}
			usize size() const
			{
				return m_size;
			}
			usize hash_table_size() const
			{
				return m_buffer_size;
			}
			f32 load_factor() const
			{
				if (!m_buffer_size)
				{
					return 0.0f;
				}
				return (f32)m_size / (f32)m_buffer_size;
			}
			f32 max_load_factor() const
			{
				return m_max_load_factor;
			}
			void clear()
			{
				internal_clear();
			}
			void shrink_to_fit()
			{
				usize desired_size = (usize)ceilf((f32)m_size / m_max_load_factor);
				if (desired_size == 0)
				{
					internal_clear_and_free_table();
					return;
				}
				rehash(desired_size);
			}
			hasher hash_function() const
			{
				return hasher();
			}
			key_equal key_eq() const
			{
				return key_equal();
			}
			//! The number of elements this hash table can hold before next rehash.
			usize capacity() const
			{
				return (usize)floorf(m_max_load_factor * m_buffer_size);
			}
			void rehash(usize new_buffer_size)
			{
				new_buffer_size = max(max(new_buffer_size, (usize)(ceilf((f32)m_size / m_max_load_factor))), INITIAL_BUFFER_SIZE);
				if (new_buffer_size == m_buffer_size)
				{
					return;
				}
				value_type* value_buf = internal_alloc_value_buffer(new_buffer_size);
				ControlBlock* cb_buf = internal_alloc_cb_buffer(new_buffer_size);
				for (usize i = 0; i < m_buffer_size; ++i)
				{
					usize h = m_cb_buffer[i].m_hash;
					if (h == EMPTY_SLOT || is_tombstone(h)) continue;
					value_type* src = m_allocator_and_value_buffer.second() + i;
					robinhood_insert(h, src, value_buf, cb_buf, new_buffer_size);
				}
				internal_free_table();
				m_allocator_and_value_buffer.second() = value_buf;
				m_cb_buffer = cb_buf;
				m_buffer_size = new_buffer_size;
			}
			void reserve(usize new_cap)
			{
				usize current_cap = capacity();
				if (new_cap > current_cap)
				{
					rehash((usize)ceilf((f32)new_cap / m_max_load_factor));
				}
			}
			void max_load_factor(f32 ml)
			{
				lucheck(ml > 0.0f && ml <= 1.0f);
				m_max_load_factor = ml;
				if (load_factor() > max_load_factor())
				{
					rehash(0);
				}
			}
			HashTable() :
				m_allocator_and_value_buffer(allocator_type(), nullptr),
				m_buffer_size(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR) {}
			HashTable(const allocator_type& alloc) :
				m_allocator_and_value_buffer(alloc, nullptr),
				m_buffer_size(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR) {}
			HashTable(const HashTable& rhs) :
				m_allocator_and_value_buffer(allocator_type(), nullptr),
				m_buffer_size(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR)
			{
				max_load_factor(rhs.m_max_load_factor);
				if (!rhs.empty())
				{
					m_allocator_and_value_buffer.second() = internal_alloc_value_buffer(rhs.m_buffer_size);
					m_cb_buffer = internal_alloc_cb_buffer(rhs.m_buffer_size);
					m_buffer_size = rhs.m_buffer_size;
					for (usize i = 0; i < rhs.m_buffer_size; ++i)
					{
						usize h = rhs.m_cb_buffer[i].m_hash;
						m_cb_buffer[i].m_hash = h;
						if (h != EMPTY_SLOT && !is_tombstone(h))
						{
							copy_construct(m_allocator_and_value_buffer.second() + i, rhs.m_allocator_and_value_buffer.second() + i);
						}
					}
				}
				m_size = rhs.m_size;
			}
			HashTable(const HashTable& rhs, const allocator_type& alloc) :
				m_allocator_and_value_buffer(alloc, nullptr),
				m_buffer_size(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR)
			{
				max_load_factor(rhs.m_max_load_factor);
				if (!rhs.empty())
				{
					m_allocator_and_value_buffer.second() = internal_alloc_value_buffer(rhs.m_buffer_size);
					m_cb_buffer = internal_alloc_cb_buffer(rhs.m_buffer_size);
					m_buffer_size = rhs.m_buffer_size;
					for (usize i = 0; i < rhs.m_buffer_size; ++i)
					{
						usize h = rhs.m_cb_buffer[i].m_hash;
						m_cb_buffer[i].m_hash = h;
						if (h != EMPTY_SLOT && !is_tombstone(h))
						{
							copy_construct(m_allocator_and_value_buffer.second() + i, rhs.m_allocator_and_value_buffer.second() + i);
						}
					}
				}
				m_size = rhs.m_size;
			}
			HashTable(HashTable&& rhs) :
				m_allocator_and_value_buffer(move(rhs.m_allocator_and_value_buffer.first()), rhs.m_allocator_and_value_buffer.second()),
				m_cb_buffer(rhs.m_cb_buffer),
				m_buffer_size(rhs.m_buffer_size),
				m_size(rhs.m_size),
				m_max_load_factor(rhs.m_max_load_factor)
			{
				rhs.m_allocator_and_value_buffer.second() = nullptr;
				rhs.m_buffer_size = 0;
				rhs.m_size = 0;
			}
			HashTable(HashTable&& rhs, const allocator_type& alloc) :
				m_allocator_and_value_buffer(alloc, rhs.m_allocator_and_value_buffer.second())
			{
				if (m_allocator_and_value_buffer.first() == rhs.m_allocator_and_value_buffer.first())
				{
					m_cb_buffer = rhs.m_cb_buffer;
					m_buffer_size = rhs.m_buffer_size;
					m_size = rhs.m_size;
					m_max_load_factor = rhs.m_max_load_factor;
					rhs.m_allocator_and_value_buffer.second() = nullptr;
					rhs.m_buffer_size = 0;
					rhs.m_size = 0;
				}
				else
				{
					m_buffer_size = 0;
					m_size = 0;
					m_max_load_factor = INITIAL_LOAD_FACTOR;
					max_load_factor(rhs.m_max_load_factor);
					if (!rhs.empty())
					{
						m_allocator_and_value_buffer.second() = internal_alloc_value_buffer(rhs.m_buffer_size);
						m_cb_buffer = internal_alloc_cb_buffer(rhs.m_buffer_size);
						m_buffer_size = rhs.m_buffer_size;
						for (usize i = 0; i < rhs.m_buffer_size; ++i)
						{
							usize h = rhs.m_cb_buffer[i].m_hash;
							m_cb_buffer[i].m_hash = h;
							if (h != EMPTY_SLOT && !is_tombstone(h))
							{
								move_construct(m_allocator_and_value_buffer.second() + i, rhs.m_allocator_and_value_buffer.second() + i);
							}
						}
						rhs.clear();
					}
				}
			}
			HashTable& operator=(const HashTable& rhs)
			{
				internal_clear_and_free_table();
				max_load_factor(rhs.m_max_load_factor);
				if (!rhs.empty())
				{
					m_allocator_and_value_buffer.second() = internal_alloc_value_buffer(rhs.m_buffer_size);
					m_cb_buffer = internal_alloc_cb_buffer(rhs.m_buffer_size);
					m_buffer_size = rhs.m_buffer_size;
					for (usize i = 0; i < rhs.m_buffer_size; ++i)
					{
						usize h = rhs.m_cb_buffer[i].m_hash;
						m_cb_buffer[i].m_hash = h;
						if (h != EMPTY_SLOT && !is_tombstone(h))
						{
							copy_construct(m_allocator_and_value_buffer.second() + i, rhs.m_allocator_and_value_buffer.second() + i);
						}
					}
					m_size = rhs.m_size;
				}
				return *this;
			}
			HashTable& operator=(HashTable&& rhs)
			{
				internal_clear_and_free_table();
				if (m_allocator_and_value_buffer.first() == rhs.m_allocator_and_value_buffer.first())
				{
					m_allocator_and_value_buffer.second() = rhs.m_allocator_and_value_buffer.second();
					m_cb_buffer = rhs.m_cb_buffer;
					m_buffer_size = rhs.m_buffer_size;
					m_size = rhs.m_size;
					m_max_load_factor = rhs.m_max_load_factor;
					rhs.m_allocator_and_value_buffer.second() = nullptr;
					rhs.m_buffer_size = 0;
					rhs.m_size = 0;
				}
				else
				{
					m_allocator_and_value_buffer.second() = nullptr;
					m_buffer_size = 0;
					m_size = 0;
					m_max_load_factor = INITIAL_LOAD_FACTOR;
					max_load_factor(rhs.m_max_load_factor);
					if (!rhs.empty())
					{
						m_allocator_and_value_buffer.second() = internal_alloc_value_buffer(rhs.m_buffer_size);
						m_cb_buffer = internal_alloc_cb_buffer(rhs.m_buffer_size);
						m_buffer_size = rhs.m_buffer_size;
						for (usize i = 0; i < rhs.m_buffer_size; ++i)
						{
							usize h = rhs.m_cb_buffer[i].m_hash;
							m_cb_buffer[i].m_hash = h;
							if (h != EMPTY_SLOT && !is_tombstone(h))
							{
								move_construct(m_allocator_and_value_buffer.second() + i, rhs.m_allocator_and_value_buffer.second() + i);
							}
						}
						rhs.clear();
					}
				}
				return *this;
			}
			~HashTable()
			{
				internal_clear_and_free_table();
			}
			iterator begin()
			{
				if (!m_allocator_and_value_buffer.second())
				{
					return iterator(nullptr, nullptr, nullptr);
				}
				iterator i(m_allocator_and_value_buffer.second(), m_cb_buffer, m_cb_buffer + m_buffer_size);
				usize h = m_cb_buffer[0].m_hash;
				if (h == EMPTY_SLOT || is_tombstone(h)) ++i;
				return i;
			}
			const_iterator begin() const
			{
				if (!m_allocator_and_value_buffer.second())
				{
					return const_iterator(nullptr, nullptr, nullptr);
				}
				const_iterator i(m_allocator_and_value_buffer.second(), m_cb_buffer, m_cb_buffer + m_buffer_size);
				usize h = m_cb_buffer[0].m_hash;
				if (h == EMPTY_SLOT || is_tombstone(h)) ++i;
				return i;
			}
			const_iterator cbegin() const
			{
				return begin();
			}
			iterator end()
			{
				if (!m_allocator_and_value_buffer.second())
				{
					return iterator(nullptr, nullptr, nullptr);
				}
				return iterator(m_allocator_and_value_buffer.second() + m_buffer_size, m_cb_buffer + m_buffer_size, m_cb_buffer + m_buffer_size);
			}
			const_iterator end() const
			{
				if (!m_allocator_and_value_buffer.second())
				{
					return const_iterator(nullptr, nullptr, nullptr);
				}
				return const_iterator(m_allocator_and_value_buffer.second() + m_buffer_size, m_cb_buffer + m_buffer_size, m_cb_buffer + m_buffer_size);
			}
			const_iterator cend() const
			{
				return end();
			}
		private:
			//! Called in single insertion operations such as insert & emplace, 
			//! this call reserves enough spaces so rehash will not happen often.
			void increment_reserve(usize new_cap)
			{
				usize current_capacity = capacity();
				if (new_cap > current_capacity)
				{
					new_cap = max(new_cap, current_capacity * 2);
					rehash((usize)ceilf((f32)new_cap / m_max_load_factor));
				}
			}
			iterator internal_find(const key_type& key, usize h)
			{
				if (!m_buffer_size) return end();
				usize pos = h % m_buffer_size;
				usize dist = 0;
				while (true)
				{
					usize existing_hash = m_cb_buffer[pos].m_hash;
					if (existing_hash == h && key_equal()(key, extract_key()(m_allocator_and_value_buffer.second()[pos])))
						return iterator(m_allocator_and_value_buffer.second() + pos, m_cb_buffer + pos, m_cb_buffer + m_buffer_size);
					else if (existing_hash == EMPTY_SLOT) return end();
					//else if (dist > m_cb_buffer[pos].m_dist) return end();
					else if (dist > probe_distance(existing_hash, pos, m_buffer_size)) return end();
					++pos;
					++dist;
					if (pos == m_buffer_size) pos = 0;
				}
			}
			const_iterator internal_find(const key_type& key, usize h) const
			{
				if (!m_buffer_size) return end();
				usize pos = h % m_buffer_size;
				usize dist = 0;
				while (true)
				{
					usize existing_hash = m_cb_buffer[pos].m_hash;
					if (existing_hash == h && key_equal()(key, extract_key()(m_allocator_and_value_buffer.second()[pos])))
						return const_iterator(m_allocator_and_value_buffer.second() + pos, m_cb_buffer + pos, m_cb_buffer + m_buffer_size);
					else if (existing_hash == EMPTY_SLOT) return end();
					//else if (dist > m_cb_buffer[pos].m_dist) return end();
					else if (dist > probe_distance(existing_hash, pos, m_buffer_size)) return end();
					++pos;
					++dist;
					if (pos == m_buffer_size) pos = 0;
				}
			}
			usize hash_key(const key_type& key) const
			{
				usize h = hasher()(key);
				if (h == EMPTY_SLOT) ++h;
				return h & ~TOMBSTONE_BIT;
			}

			template <typename... _Args>
			iterator internal_insert(usize h, _Args&&... args)
			{
				increment_reserve(m_size + 1);
				Unconstructed<value_type> value;
				value.construct(forward<_Args>(args)...);
				usize pos = robinhood_insert(h, &(value.get()), m_allocator_and_value_buffer.second(), m_cb_buffer, m_buffer_size);
				++m_size;
				return iterator(m_allocator_and_value_buffer.second() + pos, m_cb_buffer + pos, m_cb_buffer + m_buffer_size);
			}

		public:
			iterator find(const key_type& key)
			{
				usize h = hash_key(key);
				return internal_find(key, h);
			}
			const_iterator find(const key_type& key) const
			{
				usize h = hash_key(key);
				return internal_find(key, h);
			}
			bool contains(const key_type& key) const
			{
				auto iter = find(key);
				return iter == end() ? false : true;
			}
			Pair<iterator, bool> insert(const value_type& value)
			{
				usize h = hash_key(extract_key()(value));
				auto iter = internal_find(extract_key()(value), h);
				if (iter != end())
				{
					return make_pair(iter, false);
				}
				return make_pair(internal_insert(h, value), true);
			}
			Pair<iterator, bool> insert(value_type&& value)
			{
				usize h = hash_key(extract_key()(value));
				auto iter = internal_find(extract_key()(value), h);
				if (iter != end())
				{
					return make_pair(iter, false);
				}
				return make_pair(internal_insert(h, move(value)), true);
			}
			Pair<iterator, bool> insert_or_assign(const value_type& value)
			{
				usize h = hash_key(extract_key()(value));
				auto iter = internal_find(extract_key()(value), h);
				if (iter != end())
				{
					(*iter) = value;
					return make_pair(iter, false);
				}
				return make_pair(internal_insert(h, value), true);
			}
			Pair<iterator, bool> insert_or_assign(value_type&& value)
			{
				usize h = hash_key(extract_key()(value));
				auto iter = internal_find(extract_key()(value), h);
				if (iter != end())
				{
					(*iter) = move(value);
					return make_pair(iter, false);
				}
				return make_pair(internal_insert(h, move(value)), true);
			}
			template <typename _M>
			Pair<iterator, bool> insert_or_assign(const key_type& key, _M&& value)
			{
				usize h = hash_key(key);
				auto iter = internal_find(key, h);
				if (iter != end())
				{
					iter->second = forward<_M>(value);
					return make_pair(iter, false);
				}
				return make_pair(internal_insert(h, value_type(key, forward<_M>(value))), true);
			}
			template <typename _M>
			Pair<iterator, bool> insert_or_assign(key_type&& key, _M&& value)
			{
				usize h = hash_key(key);
				auto iter = internal_find(key, h);
				if (iter != end())
				{
					iter->second = forward<_M>(value);
					return make_pair(iter, false);
				}
				return make_pair(internal_insert(h, value_type(move(key), forward<_M>(value))), true);
			}
			template <typename... _Args>
			Pair<iterator, bool> emplace(_Args&&... args)
			{
				Unconstructed<value_type> value;
				value.construct(forward<_Args>(args)...);
				usize h = hash_key(extract_key()(value.get()));
				iterator iter = internal_find(extract_key()(value.get()), h);
				if (iter != end())
				{
					value.destruct();
					return make_pair(iter, false);
				}
				increment_reserve(m_size + 1);
				usize pos = robinhood_insert(h, &(value.get()), m_allocator_and_value_buffer.second(), m_cb_buffer, m_buffer_size);
				++m_size;
				return make_pair(iterator(m_allocator_and_value_buffer.second() + pos, m_cb_buffer + pos, m_cb_buffer + m_buffer_size), true);
			}
			iterator erase(const_iterator pos)
			{
				value_type* value = const_cast<value_type*>(pos.m_value);
				ControlBlock* cb = const_cast<ControlBlock*>(pos.m_cb);
				destruct(value);
				cb->m_hash |= TOMBSTONE_BIT;
				--m_size;
				iterator i(value, cb, m_cb_buffer + m_buffer_size);
				++i;
				return i;
			}
			usize erase(const key_type& key)
			{
				auto iter = find(key);
				if (iter != end())
				{
					erase(iter);
					return 1;
				}
				return 0;
			}
			allocator_type get_allocator() const
			{
				return m_allocator_and_value_buffer.first();
			}
		};
	}
}
