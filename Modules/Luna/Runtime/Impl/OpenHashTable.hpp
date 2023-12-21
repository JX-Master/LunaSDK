/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file OpenHashTable.hpp
* @author JXMaster
* @date 2020/2/15
* @brief Implements a opening hashing (closed addressing) table similar to unordered map/set in STL.
*/
#pragma once
#include "../Base.hpp"
#include "../Functional.hpp"
#include "../Algorithm.hpp"
#include "../Allocator.hpp"
#include "HashTableBase.hpp"
#include <cmath>

namespace Luna
{
	namespace OpenHashTable
	{
		template <typename _Ty>
		struct Node
		{
			using value_type = _Ty;

			value_type m_value;
			Node* m_next;

			Node() = default;
			explicit Node(const value_type& value) :
				m_value(value) {}
			explicit Node(value_type&& value) :
				m_value(move(value)) {}

			Node(const Node&) = default;
			Node(Node&& rhs) :
				m_next(rhs.m_next),
				m_value(move(rhs.m_value)) {}
			Node& operator=(const Node&) = default;
			Node& operator=(Node&& rhs)
			{
				m_next = rhs.m_next;
				m_value = move(rhs.m_value);
				return *this;
			}
		};

		template <typename _Ty>
		struct BucketIteratorBase
		{
			Node<_Ty>* m_current_node;

			void increment()
			{
				m_current_node = m_current_node->m_next;
			}

			bool operator==(const BucketIteratorBase& rhs) const
			{
				return (m_current_node == rhs.m_current_node);
			}
			bool operator!=(const BucketIteratorBase& rhs) const
			{
				return !(*this == rhs);
			}
		};

		template <typename _Ty, bool _Const>
		struct BucketIterator
		{
			BucketIteratorBase<_Ty> m_base;

			using value_type = typename Node<_Ty>::value_type;
			using pointer = conditional_t<_Const, const value_type*, value_type*>;
			using reference = conditional_t<_Const, const value_type&, value_type&>;
			using iterator_category = forward_iterator_tag;

			explicit BucketIterator(Node<_Ty>* cur)
			{
				m_base.m_current_node = cur;
			}

			BucketIterator(const BucketIterator<_Ty, false>& rhs)
			{
				m_base = rhs.m_base;
			}

			reference operator*() const
			{
				return m_base.m_current_node->m_value;
			}

			pointer operator->() const
			{
				return &(m_base.m_current_node->m_value);
			}

			BucketIterator& operator++()
			{
				m_base.increment();
				return *this;
			}

			BucketIterator operator++(int)
			{
				BucketIterator temp(*this);
				m_base.increment();
				return temp;
			}

			bool operator==(const BucketIterator& rhs) const
			{
				return m_base == rhs.m_base;
			}

			bool operator!=(const BucketIterator& rhs) const
			{
				return m_base != rhs.m_base;
			}
		};

		template <typename _Ty>
		struct IteratorBase
		{
			Node<_Ty>* m_current_node;
			Node<_Ty>** m_current_bucket;

			void increment()
			{
				m_current_node = m_current_node->m_next;
				while (m_current_node == nullptr)
				{
					++m_current_bucket;
					m_current_node = *m_current_bucket;
				}
			}

			void increment_bucket()
			{
				++m_current_bucket;
				while (*m_current_bucket == nullptr)
				{
					++m_current_bucket;
				}
				m_current_node = *m_current_bucket;	// The last bucket is used as the end placeholder, which stores usize_max_v.
			}

			bool operator==(const IteratorBase& rhs) const
			{
				return (m_current_node == rhs.m_current_node) && (m_current_bucket == rhs.m_current_bucket);
			}
			bool operator!=(const IteratorBase& rhs) const
			{
				return !(*this == rhs);
			}
		};

		template <typename _Ty, bool _Const>
		struct Iterator
		{
			IteratorBase<_Ty> m_base;

			using value_type = typename Node<_Ty>::value_type;
			using pointer = conditional_t<_Const, const value_type*, value_type*>;
			using reference = conditional_t<_Const, const value_type&, value_type&>;
			using iterator_category = forward_iterator_tag;

			Iterator() = default;

			Iterator(Node<value_type>* node, Node<value_type>** bucket)
			{
				m_base.m_current_node = node;
				m_base.m_current_bucket = bucket;
			}

			Iterator(Node<value_type>** bucket)
			{
				m_base.m_current_bucket = bucket;
				m_base.m_current_node = *bucket;
			}

			Iterator(const Iterator<_Ty, false>& rhs)
			{
				m_base = rhs.m_base;
			}

			reference operator*() const
			{
				return m_base.m_current_node->m_value;
			}

			pointer operator->() const
			{
				return &(m_base.m_current_node->m_value);
			}

			Iterator& operator++()
			{
				m_base.increment();
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator temp(*this);
				m_base.increment();
				return temp;
			}

			bool operator==(const Iterator& rhs) const
			{
				return m_base == rhs.m_base;
			}

			bool operator!=(const Iterator& rhs) const
			{
				return m_base != rhs.m_base;
			}
		};

		template <typename _Vty, typename _Alloc>
		struct NodeHandleBase
		{
			using allocator_type = _Alloc;
			OptionalPair<allocator_type, Node<_Vty>*> allocator_and_node;
			NodeHandleBase() :
				allocator_and_node(allocator_type(), nullptr) {}
			NodeHandleBase(const allocator_type& alloc) :
				allocator_and_node(alloc, nullptr) {}
			NodeHandleBase(const allocator_type& alloc, Node<_Vty>* node) :
				allocator_and_node(alloc, node) {}
			NodeHandleBase(const NodeHandleBase&) = delete;
			NodeHandleBase(NodeHandleBase&& rhs) :
				allocator_and_node(rhs.allocator_and_node.first(), rhs.allocator_and_node.second())
			{
				rhs.allocator_and_node.second() = nullptr;
			}
			NodeHandleBase& operator=(const NodeHandleBase&) = delete;
			NodeHandleBase& operator=(NodeHandleBase&& rhs)
			{
				internal_free();
				allocator_and_node.first() = rhs.allocator_and_node.first();
				allocator_and_node.second() = rhs.allocator_and_node.second();
				rhs.allocator_and_node.second() = nullptr;
				return *this;
			}
			~NodeHandleBase()
			{
				internal_free();
			}
			void internal_free()
			{
				if(allocator_and_node.second())
				{
					allocator_and_node.second()->~Node<_Vty>();
					allocator_and_node.first().template deallocate<Node<_Vty>>(allocator_and_node.second(), 1);
				}
			}
			bool empty() const
			{
				return allocator_and_node.second() == nullptr;
			}
			explicit operator bool() const
			{
				return empty();
			}
			allocator_type get_allocator() const
			{
				return allocator_and_node.first();
			}
		};

		template <typename _Kty, typename _Vty, typename _Alloc>
		struct MapNodeHandle : public NodeHandleBase<Pair<_Kty, _Vty>, _Alloc>
		{
			MapNodeHandle() : NodeHandleBase<Pair<_Kty, _Vty>, _Alloc>() {}
			MapNodeHandle(const _Alloc& alloc) : NodeHandleBase<Pair<_Kty, _Vty>, _Alloc>(alloc) {}
			MapNodeHandle(const _Alloc& alloc, Node<Pair<_Kty, _Vty>>* node) : NodeHandleBase<Pair<_Kty, _Vty>, _Alloc>(alloc, node) {}
			MapNodeHandle(const MapNodeHandle&) = delete;
			MapNodeHandle(MapNodeHandle&& rhs) : NodeHandleBase<Pair<_Kty, _Vty>, _Alloc>(move(rhs)) {}
			MapNodeHandle& operator=(const MapNodeHandle&) = delete;
			MapNodeHandle& operator=(MapNodeHandle&& rhs)
			{
				NodeHandleBase<Pair<_Kty, _Vty>, _Alloc>::operator=(move(rhs));
				return *this;
			}
			void swap(MapNodeHandle& rhs)
			{
				MapNodeHandle t = move(*this);
				*this = move(rhs);
				rhs = move(t);
			}
			using key_type = _Kty;
			using mapped_type = _Vty;
			key_type& key() const
			{
				return this->allocator_and_node.second()->first;
			}
			mapped_type& mapped() const
			{
				return this->allocator_and_node.second()->second;
			}
		};

		template <typename _Vty, typename _Alloc>
		struct SetNodeHandle : public NodeHandleBase<_Vty, _Alloc>
		{
			SetNodeHandle() : NodeHandleBase<_Vty, _Alloc>() {}
			SetNodeHandle(const _Alloc& alloc) : NodeHandleBase<_Vty, _Alloc>(alloc) {}
			SetNodeHandle(const _Alloc& alloc, Node<_Vty>* node) : NodeHandleBase<_Vty, _Alloc>(alloc, node) {}
			SetNodeHandle(const SetNodeHandle&) = delete;
			SetNodeHandle(SetNodeHandle&& rhs) : NodeHandleBase<_Vty, _Alloc>(move(rhs)) {}
			SetNodeHandle& operator=(const SetNodeHandle&) = delete;
			SetNodeHandle& operator=(SetNodeHandle&& rhs)
			{
				NodeHandleBase<_Vty, _Alloc>::operator=(move(rhs));
				return *this;
			}
			void swap(SetNodeHandle& rhs)
			{
				SetNodeHandle t = move(*this);
				*this = move(rhs);
				rhs = move(t);
			}
			using value_type = _Vty;
			value_type& value() const
			{
				return *(this->allocator_and_node.second());
			}
		};

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
			using local_iterator = BucketIterator<value_type, false>;
			using const_local_iterator = BucketIterator<value_type, true>;

			using extract_key = _ExtractKey;

			// -------------------- Begin of ABI compatible part --------------------

			//! A pointer to the hash table, which is an array of linked lists.
			OptionalPair<allocator_type, Node<value_type>**> m_allocator_and_buckets;
			//! The number of buckets in total.
			usize m_bucket_count;
			//! The number of elements in the hash table.
			usize m_size;
			//! The maximum load factor of the table, which determines how often the rehashing 
			//! will occur. The load factor is how many elements are placed in one bucket in 
			//! average.
			f32 m_max_load_factor;

			// --------------------  End of ABI compatible part  --------------------

			static constexpr usize INITIAL_BUCKET = 16;
			static constexpr f32 INITIAL_LOAD_FACTOR = 16.0f;

			template <typename _Ty>
			_Ty* allocate(usize n)
			{
                return m_allocator_and_buckets.first().template allocate<_Ty>(n);
			}

			template <typename _Ty>
			void deallocate(_Ty* ptr, usize n)
			{
                m_allocator_and_buckets.first().template deallocate<_Ty>(ptr, n);
			}

			template <typename _Ty, typename... _Args>
			_Ty* new_object(_Args&&... args)
			{
				_Ty* o = allocate<_Ty>(1);
				new (o) _Ty(forward<_Args>(args)...);
				return o;
			}

			template <typename _Ty>
			void delete_object(_Ty* o)
			{
				o->~_Ty();
				deallocate<_Ty>(o, 1);
			}

			//! Clear all nodes in the specified bucket.
			void internal_clear_bucket(usize i)
			{
				auto cur = m_allocator_and_buckets.second()[i];
				while (cur)
				{
					auto next = cur->m_next;
					delete_object(cur);
					cur = next;
				}
				m_allocator_and_buckets.second()[i] = nullptr;
			}

			//! Frees the bucket table.
			void internal_free_table()
			{
				if (m_allocator_and_buckets.second())
				{
					deallocate<Node<value_type>*>(m_allocator_and_buckets.second(), m_bucket_count);
					m_allocator_and_buckets.second() = nullptr;
				}
			}

			//! Clears all buckets and then free table.
			void internal_clear()
			{
				for (usize i = 0; i < m_bucket_count; ++i)
				{
					internal_clear_bucket(i);
				}
				internal_free_table();
				m_bucket_count = 0;
				m_size = 0;
			}

			Node<value_type>** internal_alloc_table(usize cap)
			{
				Node<value_type>** buf = allocate<Node<value_type>*>(cap + 1);
				if (!buf)
				{
					return nullptr;
				}
				memzero(buf, sizeof(Node<value_type>*) * cap);
				buf[cap] = (Node<value_type>*)(USIZE_MAX);
				return buf;
			}

			HashTable() :
				m_allocator_and_buckets(allocator_type(), nullptr),
				m_bucket_count(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR) {}
			HashTable(const allocator_type& alloc) :
				m_allocator_and_buckets(alloc, nullptr),
				m_bucket_count(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR) {}
			HashTable(const HashTable& rhs) :
				m_allocator_and_buckets(allocator_type(), nullptr),
				m_bucket_count(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR)
			{
				max_load_factor(rhs.m_max_load_factor);
				if (!rhs.empty())
				{
					m_allocator_and_buckets.second() = internal_alloc_table(rhs.m_bucket_count);
					m_bucket_count = rhs.m_bucket_count;
					for (usize i = 0; i < rhs.m_bucket_count; ++i)
					{
						Node<value_type>* iter = rhs.m_allocator_and_buckets.second()[i];
						while (iter)
						{
							Node<value_type>* node = new_object<Node<value_type>>(*iter);
							// insert.
							node->m_next = m_allocator_and_buckets.second()[i];
							m_allocator_and_buckets.second()[i] = node;
							++m_size;
							iter = iter->m_next;
						}
					}
				}
			}
			HashTable(const HashTable& rhs, const allocator_type& alloc) :
				m_allocator_and_buckets(alloc, nullptr),
				m_bucket_count(0),
				m_size(0),
				m_max_load_factor(INITIAL_LOAD_FACTOR)
			{
				max_load_factor(rhs.m_max_load_factor);
				if (!rhs.empty())
				{
					m_allocator_and_buckets.second() = internal_alloc_table(rhs.m_bucket_count);
					m_bucket_count = rhs.m_bucket_count;
					for (usize i = 0; i < rhs.m_bucket_count; ++i)
					{
						Node<value_type>* iter = rhs.m_allocator_and_buckets.second()[i];
						while (iter)
						{
							Node<value_type>* node = new_object<Node<value_type>>(*iter);
							// insert.
							node->m_next = m_allocator_and_buckets.second()[i];
							m_allocator_and_buckets.second()[i] = node;
							++m_size;
							iter = iter->m_next;
						}
					}
				}
			}
			HashTable(HashTable&& rhs) :
				m_allocator_and_buckets(move(rhs.m_allocator_and_buckets.first()), rhs.m_allocator_and_buckets.second()),
				m_bucket_count(rhs.m_bucket_count),
				m_size(rhs.m_size),
				m_max_load_factor(rhs.m_max_load_factor)
			{
				rhs.m_allocator_and_buckets.second() = nullptr;
				rhs.m_bucket_count = 0;
				rhs.m_size = 0;
			}
			HashTable(HashTable&& rhs, const allocator_type& alloc) :
				m_allocator_and_buckets(alloc, rhs.m_allocator_and_buckets.second())
			{
				if (m_allocator_and_buckets.first() == rhs.m_allocator_and_buckets.first())
				{
					m_bucket_count = rhs.m_bucket_count;
					m_size = rhs.m_size;
					m_max_load_factor = rhs.m_max_load_factor;
					rhs.m_allocator_and_buckets.second() = nullptr;
					rhs.m_bucket_count = 0;
					rhs.m_size = 0;
				}
				else
				{
					m_bucket_count = 0;
					m_size = 0;
					m_max_load_factor = INITIAL_LOAD_FACTOR;
					max_load_factor(rhs.m_max_load_factor);
					if (!rhs.empty())
					{
						m_allocator_and_buckets.second() = internal_alloc_table(rhs.m_bucket_count);
						m_bucket_count = rhs.m_bucket_count;
						for (usize i = 0; i < rhs.m_bucket_count; ++i)
						{
							Node<value_type>* iter = rhs.m_allocator_and_buckets.second()[i];
							while (iter)
							{
								Node<value_type>* node = new_object<Node<value_type>>(move(*iter));
								// insert.
								node->m_next = m_allocator_and_buckets.second()[i];
								m_allocator_and_buckets.second()[i] = node;
								++m_size;
								iter = iter->m_next;
							}
						}
						rhs.clear();
					}
				}
			}
			HashTable& operator=(const HashTable& rhs)
			{
				internal_clear();
				max_load_factor(rhs.m_max_load_factor);
				if (!rhs.empty())
				{
					m_allocator_and_buckets.second() = internal_alloc_table(rhs.m_bucket_count);
					m_bucket_count = rhs.m_bucket_count;
					for (usize i = 0; i < rhs.m_bucket_count; ++i)
					{
						Node<value_type>* iter = rhs.m_allocator_and_buckets.second()[i];
						while (iter)
						{
							Node<value_type>* node = new_object<Node<value_type>>(*iter);
							// insert.
							node->m_next = m_allocator_and_buckets.second()[i];
							m_allocator_and_buckets.second()[i] = node;
							++m_size;
							iter = iter->m_next;
						}
					}
				}
				return *this;
			}
			HashTable& operator=(HashTable&& rhs)
			{
				internal_clear();
				if (m_allocator_and_buckets.first() == rhs.m_allocator_and_buckets.first())
				{
					m_allocator_and_buckets.second() = rhs.m_allocator_and_buckets.second();
					m_bucket_count = rhs.m_bucket_count;
					m_size = rhs.m_size;
					m_max_load_factor = rhs.m_max_load_factor;
					rhs.m_allocator_and_buckets.second() = nullptr;
					rhs.m_bucket_count = 0;
					rhs.m_size = 0;
				}
				else
				{
					m_allocator_and_buckets.second() = nullptr;
					m_bucket_count = 0;
					m_size = 0;
					m_max_load_factor = INITIAL_LOAD_FACTOR;
					max_load_factor(rhs.m_max_load_factor);
					if (!rhs.empty())
					{
						m_allocator_and_buckets.second() = internal_alloc_table(rhs.m_bucket_count);
						m_bucket_count = rhs.m_bucket_count;
						for (usize i = 0; i < rhs.m_bucket_count; ++i)
						{
							Node<value_type>* iter = rhs.m_allocator_and_buckets.second()[i];
							while (iter)
							{
								Node<value_type>* node = new_object<Node<value_type>>(move(*iter));
								// insert.
								node->m_next = m_allocator_and_buckets.second()[i];
								m_allocator_and_buckets.second()[i] = node;
								++m_size;
								iter = iter->m_next;
							}
						}
						rhs.clear();
					}
				}
				return *this;
			}
			~HashTable()
			{
				internal_clear();
			}
			iterator begin()
			{
				if (!m_allocator_and_buckets.second())
				{
					return iterator(nullptr, nullptr);	// The end() also returns this.
				}
				iterator i(m_allocator_and_buckets.second());
				if (!(i.m_base.m_current_node))
				{
					i.m_base.increment_bucket();
				}
				return i;
			}
			const_iterator begin() const
			{
				if (!m_allocator_and_buckets.second())
				{
					return const_iterator(nullptr, nullptr);	// The end() also returns this.
				}
				const_iterator i(m_allocator_and_buckets.second());
				if (!(i.m_base.m_current_node))
				{
					i.m_base.increment_bucket();
				}
				return i;
			}
			const_iterator cbegin() const
			{
				return begin();
			}
			iterator end()
			{
				if (!m_allocator_and_buckets.second())
				{
					return iterator(nullptr, nullptr);
				}
				// Returns the bucket after the last valid bucket, whose stored address is always `usize_max`.
				return iterator(m_allocator_and_buckets.second() + m_bucket_count);	
			}
			const_iterator end() const
			{
				if (!m_allocator_and_buckets.second())
				{
					return const_iterator(nullptr, nullptr);
				}
				return const_iterator(m_allocator_and_buckets.second() + m_bucket_count);
			}
			const_iterator cend() const
			{
				return end();
			}

			local_iterator begin(usize n)
			{
				luassert(n < m_bucket_count);
				return local_iterator(m_allocator_and_buckets.second()[n]);
			}
			const_local_iterator begin(usize n) const
			{
				luassert(n < m_bucket_count);
				return const_local_iterator(m_allocator_and_buckets.second()[n]);
			}
			const_local_iterator cbegin(usize n) const
			{
				luassert(n < m_bucket_count);
				return const_local_iterator(m_allocator_and_buckets.second()[n]);
			}
			local_iterator end(usize)
			{
				return local_iterator(nullptr);
			}
			const_local_iterator end(usize) const
			{
				return const_local_iterator(nullptr);
			}
			const_local_iterator cend(usize) const
			{
				return const_local_iterator(nullptr);
			}
			bool empty() const
			{
				return m_size == 0;
			}
			usize size() const
			{
				return m_size;
			}
			usize bucket_count() const
			{
				return m_bucket_count;
			}
			usize bucket_size(usize n) const
			{
				usize i = 0;
				auto iter = cbegin(n);
				while (iter != cend(n))
				{
					++i;
					++iter;
				}
				return i;
			}
		private:
			usize hash_code_to_bucket_index(usize hash_code) const
			{
				if (!m_bucket_count)
				{
					return 0;
				}
				return hash_code % m_bucket_count;
			}
		public:
			//! Returns the index of the bucket that the specified key is hashed to.
			usize bucket(const key_type& key) const
			{
				return hash_code_to_bucket_index(hasher()(key));
			}
			f32 load_factor() const
			{
				if (!m_bucket_count)
				{
					return 0.0f;
				}
				return (f32)m_size / (f32)m_bucket_count;
			}
			f32 max_load_factor() const
			{
				return m_max_load_factor;
			}
			void clear()
			{
				internal_clear();
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
				return (usize)ceilf(m_max_load_factor * m_bucket_count);
			}
			void rehash(usize new_buckets_count)
			{
				// The new buckets count is at least `INITIAL_BUCKET`(currently 16), if the new bucket count exceeds the maximum load
				// factor, it will be expanded to match the load factor.
				new_buckets_count = max(max(new_buckets_count, (usize)(ceilf((f32)size() / m_max_load_factor))), INITIAL_BUCKET);
				if (new_buckets_count == m_bucket_count)
				{
					return;
				}
				auto new_bkts = internal_alloc_table(new_buckets_count);
				// Do rehash. iterates the old hash buckets and moves all nodes to the new hash table.
				for (usize i = 0; i < m_bucket_count; ++i)
				{
					Node<value_type>* iter = m_allocator_and_buckets.second()[i];	// the iterator for the linked list.
					while (iter)
					{
						// rehash the element.
						usize bkt_index = (hasher()(extract_key()(iter->m_value))) % new_buckets_count;
						// borrow the node from old bucket to new.
						Node<value_type>* node = iter;
						iter = iter->m_next;
						node->m_next = new_bkts[bkt_index];	// insert into front.
						new_bkts[bkt_index] = node;
					}
				}
				internal_free_table();
				m_allocator_and_buckets.second() = new_bkts;
				m_bucket_count = new_buckets_count;
			}
			void reserve(usize new_cap)
			{
				usize current_capacity = capacity();
				if (new_cap > current_capacity)
				{
					rehash((usize)ceilf((f32)new_cap / m_max_load_factor));
				}
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
		public:
			void max_load_factor(f32 ml)
			{
				m_max_load_factor = ml;
				if (load_factor() > max_load_factor())
				{
					rehash(0);
				}
			}

			iterator internal_find(const _Kty& key, usize bucket_index)
			{
				if (!m_allocator_and_buckets.second())	// early out for empty case.
				{
					return end();
				}
				auto tar_bucket = m_allocator_and_buckets.second()[bucket_index];
				auto cur = tar_bucket;
				while (cur)
				{
					if (key_equal()(key, extract_key()(cur->m_value)))
					{
						return iterator(cur, m_allocator_and_buckets.second() + bucket_index);	// found.
					}
					cur = cur->m_next;
				}
				return end();	// not found.
			}

			const_iterator internal_find(const _Kty& key, usize bucket_index) const
			{
				if (!m_allocator_and_buckets.second())	// early out for empty case.
				{
					return end();
				}
				auto tar_bucket = m_allocator_and_buckets.second()[bucket_index];
				auto cur = tar_bucket;
				while (cur)
				{
					if (key_equal()(key, extract_key()(cur->m_value)))
					{
						return const_iterator(cur, m_allocator_and_buckets.second() + bucket_index);	// found.
					}
					cur = cur->m_next;
				}
				return end();	// not found.
			}

		private:

			iterator internal_insert_to_first_node(usize hash_code, Node<value_type>* new_node)
			{
				increment_reserve(m_size + 1);
				// In case that the table is rehashed, we need to defer the mediation until 
				// the table is rehashed. 
				usize bucket_index = hash_code_to_bucket_index(hash_code);
				new_node->m_next = m_allocator_and_buckets.second()[bucket_index];
				m_allocator_and_buckets.second()[bucket_index] = new_node;
				++m_size;
				return iterator(m_allocator_and_buckets.second() + bucket_index);
			}

			//! Emplaces and inserts the elements as the first node in the bucket.
			template <typename... _Args>
			iterator internal_insert_to_first(usize hash_code, _Args&&... args)
			{
				Node<value_type>* new_node = new_object<Node<value_type>>(forward<_Args>(args)...);
				return internal_insert_to_first_node(hash_code, new_node);
			}

		public:

			iterator find(const key_type& key)
			{
				usize hash_k = bucket(key);
				return internal_find(key, hash_k);
			}

			const_iterator find(const key_type& key) const
			{
				usize hash_k = bucket(key);
				return internal_find(key, hash_k);
			}

			usize count(const key_type& key) const
			{
				auto iter = find(key);
				if (iter == end())
				{
					return 0;
				}
				usize c = 1;
				++iter;
				while (iter != end() && (extract_key()(*iter) == key))
				{
					++c;
					++iter;
				}
				return c;
			}

			Pair<iterator, iterator> equal_range(const key_type& key)
			{
				auto iter = find(key);
				if (iter == end())
				{
					return Pair<iterator, iterator>(end(), end());
				}
				auto first = iter;
				++iter;
				while (iter != end() && (extract_key()(*iter) == key))
				{
					++iter;
				}
				auto second = iter;
				return Pair<iterator, iterator>(first, second);
			}

			Pair<const_iterator, const_iterator> equal_range(const key_type& key) const
			{
				auto iter = find(key);
				if (iter == end())
				{
					return Pair<const_iterator, const_iterator>(cend(), cend());
				}
				Pair<const_iterator, const_iterator> p;
				p.first = iter;
				++iter;
				while (iter != end() && (extract_key()(*iter) == key))
				{
					++iter;
				}
				p.second = iter;
				return p;
			}

			bool contains(const key_type& key) const
			{
				usize hash_k = bucket(key);
				return (internal_find(key, hash_k) == end()) ? false : true;
			}

			Pair<iterator, bool> insert(const value_type& value)
			{
				usize hash_code = hasher()(extract_key()(value));
				auto iter = internal_find(extract_key()(value), hash_code_to_bucket_index(hash_code));
				if (iter != end())
				{
					return make_pair(iter, false);
				}
				return make_pair(internal_insert_to_first(hash_code, value), true);
			}

			Pair<iterator, bool> insert(value_type&& value)
			{
				usize hash_code = hasher()(extract_key()(value));
				auto iter = internal_find(extract_key()(value), hash_code_to_bucket_index(hash_code));
				if (iter != end())
				{
					return make_pair(iter, false);
				}
				return make_pair(internal_insert_to_first(hash_code, move(value)), true);
			}
			template <typename _Ret, typename _Node>
			_Ret insert(_Node&& node)
			{
				if(!node.allocator_and_node.second())
				{
					_Ret ret;
					ret.inserted = false;
					ret.position = end();
					return ret;
				}
				auto& value = node.allocator_and_node.second()->m_value;
				usize hash_code = hasher()(extract_key()(value));
				auto iter = internal_find(extract_key()(value), hash_code_to_bucket_index(hash_code));
				_Ret ret;
				if (iter != end())
				{
					ret.inserted = false;
					ret.node = move(node);
					ret.position = iter;
				}
				else
				{
					ret.inserted = true;
					ret.position = internal_insert_to_first_node(hash_code, node.allocator_and_node.second());
					node.allocator_and_node.second() = nullptr;
				}
				return ret;
			}
			Pair<iterator, bool> insert_or_assign(const value_type& value)
			{
				usize hash_code = hasher()(extract_key()(value));
				auto iter = internal_find(extract_key()(value), hash_code_to_bucket_index(hash_code));
				if (iter != end())
				{
					(*iter) = value;
					return make_pair(iter, false);
				}
				return make_pair(internal_insert_to_first(hash_code, value), true);
			}
			Pair<iterator, bool> insert_or_assign(value_type&& value)
			{
				usize hash_code = hasher()(extract_key()(value));
				auto iter = internal_find(extract_key()(value), hash_code_to_bucket_index(hash_code));
				if (iter != end())
				{
					(*iter) = move(value);
					return make_pair(iter, false);
				}
				return make_pair(internal_insert_to_first(hash_code, move(value)), true);
			}
			template <typename _M>
			Pair<iterator, bool> insert_or_assign(const key_type& key, _M&& value)
			{
				usize hash_code = hasher()(key);
				auto iter = internal_find(key, hash_code_to_bucket_index(hash_code));
				if (iter != end())
				{
					iter->second = forward<_M>(value);
					return make_pair(iter, false);
				}
				return make_pair(internal_insert_to_first(hash_code, value_type(key, forward<_M>(value))), true);
			}

			template <typename _M>
			Pair<iterator, bool> insert_or_assign(key_type&& key, _M&& value)
			{
				usize hash_code = hasher()(key);
				auto iter = internal_find(key, hash_code_to_bucket_index(hash_code));
				if (iter != end())
				{
					iter->second = forward<_M>(value);
					return make_pair(iter, false);
				}
				return make_pair(internal_insert_to_first(hash_code, value_type(move(key), forward<_M>(value))), true);
			}

			template <typename... _Args>
			Pair<iterator, bool> emplace(_Args&&... args)
			{
				Node<value_type>* new_node = new_object<Node<value_type>>(value_type(forward<_Args>(args)...));
				usize hash_code = hasher()(extract_key()(new_node->m_value));
				auto iter = internal_find(extract_key()(new_node->m_value), hash_code_to_bucket_index(hash_code));
				if (iter != end())
				{
					delete_object(new_node);
					return Pair<iterator, bool>(iter, false);
				}
				increment_reserve(m_size + 1);
				// In case that the table is rehashed, we need to defer the mediation until 
				// the table is rehashed. 
				usize hash_k = hash_code_to_bucket_index(hash_code);
				new_node->m_next = m_allocator_and_buckets.second()[hash_k];
				m_allocator_and_buckets.second()[hash_k] = new_node;
				++m_size;
				return Pair<iterator, bool>(iterator(m_allocator_and_buckets.second() + hash_k), true);
			}

		private:
			iterator multi_insert_node(Node<value_type>* new_node)
			{
				increment_reserve(m_size + 1);
				usize hash_code = hasher()(extract_key()(new_node->m_value));
				usize bucket_index = hash_code_to_bucket_index(hash_code);
				auto iter = internal_find(extract_key()(new_node->m_value), bucket_index);
				++m_size;
				if (iter != end())
				{
					Node<value_type>* node = iter.m_base.m_current_node;
					new_node->m_next = node->m_next;
					node->m_next = new_node;
				}
				else
				{
					new_node->m_next = m_allocator_and_buckets.second()[bucket_index];
					m_allocator_and_buckets.second()[bucket_index] = new_node;
				}
				return iterator(new_node, m_allocator_and_buckets.second() + bucket_index);
			}
		public:

			//! Same as insert, but allows multi values being inserted using the same key.
			iterator multi_insert(const value_type& value)
			{
				Node<value_type>* new_node = new_object<Node<value_type>>(value);
				return multi_insert_node(new_node);
			}
			iterator multi_insert(value_type&& value)
			{
				Node<value_type>* new_node = new_object<Node<value_type>>(move(value));
				return multi_insert_node(new_node);
			}
			template <typename _Node>
			iterator multi_insert(_Node&& node)
			{
				if(!node.allocator_and_node.second())
				{
					return end();
				}
				iterator ret = multi_insert_node(node.allocator_and_node.second());
				node.allocator_and_node.second() = nullptr;
				return ret;
			}
			template <typename... _Args>
			iterator multi_emplace(_Args&&... args)
			{
				Node<value_type>* new_node = new_object<Node<value_type>>(value_type(forward<_Args>(args)...));
				return multi_insert_node(new_node);
			}
		private:
			Node<value_type>* internal_extract(const_iterator pos)
			{
				Node<value_type>* node = pos.m_base.m_current_node;
				Node<value_type>* node_cur = *(pos.m_base.m_current_bucket);
				if (node == node_cur)
				{
					// If the remove node is the first node in the bucket, modify the bucket pointer.
					*(pos.m_base.m_current_bucket) = node_cur->m_next; 
				}
				else
				{
					// Modify the prior node of the node to be deleted.
					Node<value_type>* node_next = node_cur->m_next;
					while (node_next != node)
					{
						node_cur = node_next;
						node_next = node_cur->m_next;
					}
					node_cur->m_next = node_next->m_next;
				}
				--m_size;
				return node;
			}
		public:
			iterator erase(const_iterator pos)
			{
				iterator inext(pos.m_base.m_current_node, pos.m_base.m_current_bucket);
				++inext;
				Node<value_type>* node = internal_extract(pos);
				delete_object(node);
				return inext;
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

			usize multi_erase(const key_type& key)
			{
				//! Checks the range to erase. All elements of the same key must be in the same bucket.
				auto range = equal_range(key);
				//! Finds the node before the first node to be erased.
				//! `nullptr` if the first node to be erased is the first node in the bucket.
				Node<value_type>* node_before = nullptr;
				if (*(range.first.m_base.m_current_bucket) != range.first.m_base.m_current_node)
				{
					node_before = *(range.first.m_base.m_current_bucket);
					while (node_before->m_next != range.first.m_base.m_current_node)
					{
						node_before = node_before->m_next;
					}
				}
				// Erase the node sequence.
				usize num_erase = 0;
				auto iter = range.first;
				// Record this so we know what to set after erasing.
				Node<value_type>* node_after = iter.m_base.m_current_node->m_next;
				while (iter != range.second)
				{
					auto erase_node = iter;
					++iter;
					++num_erase;
					node_after = erase_node.m_base.m_current_node->m_next;
					delete_object(erase_node.m_base.m_current_node);
				}
				// Modify the pointer.
				if (!node_before)
				{
					*(range.first.m_base.m_current_bucket) = node_after;
				}
				else
				{
					node_before->m_next = node_after;
				}
				m_size -= num_erase;
				return num_erase;
			}
			
			template <typename _Node>
			_Node extract(const_iterator pos)
			{
				Node<value_type>* node = internal_extract(pos);
				return _Node(get_allocator(), node);
			}

			allocator_type get_allocator() const
			{
				return m_allocator_and_buckets.first();
			}
		};

		//! Represents the insertion result of one node.
		template <typename _Iter, typename _Node>
		struct InsertResult
		{	
			//! The iterator identifing the insertion position if `inserted` is `true`, or
			//! 
			_Iter    position;
			//! `true` if the insertion is succeeded. `false` if the insertion is failed.
			bool     inserted;
			//! The original node passed in if the insertion is failed. 
			//! An empty node otherwise.
			_Node 	 node;
		};
	}
}
