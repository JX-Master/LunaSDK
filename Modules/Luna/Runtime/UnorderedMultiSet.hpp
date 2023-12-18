/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file UnorderedMultiSet.hpp
* @author JXMaster
* @date 2020/12/26
*/
#pragma once
#include "Impl/OpenHashTable.hpp"

namespace Luna
{
	template <
		typename _Kty,
		typename _Hash = hash<_Kty>,		// Used to hash the key value.
		typename _KeyEqual = equal_to<_Kty>,
		typename _Alloc = Allocator>	// Used to compare the element.
	class UnorderedMultiSet
	{
	public:
		using key_type = _Kty;
		using value_type = _Kty;
		using allocator_type = _Alloc;
		using hasher = _Hash;
		using key_equal = _KeyEqual;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = OpenHashTable::Iterator<value_type, false>;
		using const_iterator = OpenHashTable::Iterator<value_type, true>;
		using local_iterator = OpenHashTable::BucketIterator<value_type, false>;
		using const_local_iterator = OpenHashTable::BucketIterator<value_type, true>;

	private:

		using table_type = OpenHashTable::HashTable<key_type, value_type, Impl::SetExtractKey<key_type, value_type>, hasher, key_equal, allocator_type>;

		table_type m_base;

		UnorderedMultiSet(table_type&& base) :
			m_base(move(base)) {}

	public:
		using node_type = OpenHashTable::SetNodeHandle<value_type, allocator_type>;
		using insert_return_type = OpenHashTable::InsertResult<iterator, node_type>;

		UnorderedMultiSet() :
			m_base() {}
		UnorderedMultiSet(const allocator_type& alloc) :
			m_base(alloc) {}
		UnorderedMultiSet(const UnorderedMultiSet& rhs) :
			m_base(rhs.m_base) {}
		UnorderedMultiSet(const UnorderedMultiSet& rhs, const allocator_type& alloc) :
			m_base(rhs.m_base, alloc) {}
		UnorderedMultiSet(UnorderedMultiSet&& rhs) :
			m_base(move(rhs.m_base)) {}
		UnorderedMultiSet(UnorderedMultiSet&& rhs, const allocator_type& alloc) :
			m_base(move(rhs.m_base), alloc) {}
		UnorderedMultiSet& operator=(const UnorderedMultiSet& rhs)
		{
			m_base = rhs.m_base;
			return *this;
		}
		UnorderedMultiSet& operator=(UnorderedMultiSet&& rhs)
		{
			m_base = move(rhs.m_base);
			return *this;
		}
	public:
		iterator begin()
		{
			return m_base.begin();
		}
		const_iterator begin() const
		{
			return m_base.begin();
		}
		const_iterator cbegin() const
		{
			return m_base.cbegin();
		}
		iterator end()
		{
			return m_base.end();
		}
		const_iterator end() const
		{
			return m_base.end();
		}
		const_iterator cend() const
		{
			return m_base.cend();
		}

		local_iterator begin(usize n)
		{
			return m_base.begin(n);
		}
		const_local_iterator begin(usize n) const
		{
			return m_base.begin(n);
		}
		const_local_iterator cbegin(usize n) const
		{
			return m_base.cbegin(n);
		}
		local_iterator end(usize n)
		{
			return m_base.end(n);
		}
		const_local_iterator end(usize n) const
		{
			return m_base.end(n);
		}
		const_local_iterator cend(usize n) const
		{
			return m_base.cend(n);
		}
		bool empty() const
		{
			return m_base.empty();
		}
		usize size() const
		{
			return m_base.size();
		}
		usize bucket_count() const
		{
			return m_base.bucket_count();
		}
		usize bucket_size(usize n) const
		{
			return m_base.bucket_size(n);
		}
		usize bucket(const key_type& key) const
		{
			return m_base.bucket(key);
		}
		f32 load_factor() const
		{
			return m_base.load_factor();
		}
		f32 max_load_factor() const
		{
			return m_base.max_load_factor();
		}
		void clear()
		{
			m_base.clear();
		}
		hasher hash_function() const
		{
			return m_base.hash_function();
		}
		key_equal key_eq() const
		{
			return m_base.key_eq();
		}

		void rehash(usize new_buckets_count)
		{
			m_base.rehash(new_buckets_count);
		}
		void reserve(usize new_cap)
		{
			m_base.reserve(new_cap);
		}
		void max_load_factor(f32 ml)
		{
			m_base.max_load_factor(ml);
		}
		iterator find(const key_type& key)
		{
			return m_base.find(key);
		}
		const_iterator find(const key_type& key) const
		{
			return m_base.find(key);
		}
		usize count(const key_type& key) const
		{
			return m_base.count(key);
		}
		Pair<iterator, iterator> equal_range(const key_type& key)
		{
			return m_base.equal_range(key);
		}
		Pair<const_iterator, const_iterator> equal_range(const key_type& key) const
		{
			return m_base.equal_range(key);
		}
		bool contains(const key_type& key) const
		{
			return m_base.contains(key);
		}
		iterator insert(const value_type& value)
		{
			return m_base.multi_insert(value);
		}
		iterator insert(value_type&& value)
		{
			return m_base.multi_insert(move(value));
		}
		iterator insert(node_type&& node)
		{
			return m_base.multi_insert<node_type>(move(node));
		}
		template <typename... _Args>
		iterator emplace(_Args&&... args)
		{
			return m_base.multi_emplace(forward<_Args>(args)...);
		}

		iterator erase(const_iterator pos)
		{
			return m_base.erase(pos);
		}
		usize erase(const key_type& key)
		{
			return m_base.multi_erase(key);
		}
		void swap(UnorderedMultiSet& rhs)
		{
			UnorderedMultiSet tmp(move(rhs));
			rhs = move(*this);
			*this = move(tmp);
		}
		node_type extract(const_iterator pos)
		{
			return m_base.extract<node_type>(pos);
		}
		allocator_type get_allocator() const
		{
			return m_base.get_allocator();
		}
	};
}
