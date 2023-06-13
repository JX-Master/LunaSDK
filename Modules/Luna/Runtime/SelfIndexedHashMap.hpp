/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SelfIndexedHashMap.hpp
* @author JXMaster
* @date 2022/5/1
*/
#pragma once
#include "Source/RobinHoodHashTable.hpp"

namespace Luna
{
	//! @class SelfIndexedHashMap
	//! The self indexed hash map is for values whose key is one data member of the value, or 
	//! can be computed from the value. For every value type that the user want to use for self indexed
	//! hash map, the user must define one special structure called "key extractor", and passes the type
	//! as the `_ExtractKey` template argument for the map. In this structure, one operator function 
	//! `const _Kty& operator()(const _Ty& p) const` (or `_Kty operator()(const _Ty& p) const` if the key is 
	//! computed from value) must be defined to fetch the key of the value.
	//! 
	//! The user must ensure that the key data member is not changed after the element is inserted to the 
	//! map and before the element is removed from the map, or the behavior is undefined.
	template <
		typename _Kty,
		typename _Ty,
		typename _ExtractKey,
		typename _Hash = hash<_Kty>,		// Used to hash the key value.
		typename _KeyEqual = equal_to<_Kty>,
		typename _Alloc = Allocator>	// Used to compare the element.
		class SelfIndexedHashMap
	{
	public:
		using key_type = _Kty;
		using mapped_type = _Ty;
		using value_type = _Ty;
		using allocator_type = _Alloc;
		using hasher = _Hash;
		using key_equal = _KeyEqual;
		using extract_key = _ExtractKey;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = RobinHoodHashing::Iterator<value_type, false>;
		using const_iterator = RobinHoodHashing::Iterator<value_type, true>;

	private:
		using table_type = RobinHoodHashing::HashTable<key_type, value_type, extract_key, hasher, key_equal, allocator_type>;
		table_type m_base;

		SelfIndexedHashMap(table_type&& base) :
			m_base(move(base)) {}

	public:
		SelfIndexedHashMap() :
			m_base() {}
		SelfIndexedHashMap(const allocator_type& alloc) :
			m_base(alloc) {}
		SelfIndexedHashMap(const SelfIndexedHashMap& rhs) :
			m_base(rhs.m_base) {}
		SelfIndexedHashMap(const SelfIndexedHashMap& rhs, const allocator_type& alloc) :
			m_base(rhs.m_base, alloc) {}
		SelfIndexedHashMap(SelfIndexedHashMap&& rhs) :
			m_base(move(rhs.m_base)) {}
		SelfIndexedHashMap(SelfIndexedHashMap&& rhs, const allocator_type& alloc) :
			m_base(move(rhs.m_base), alloc) {}
		SelfIndexedHashMap& operator=(const SelfIndexedHashMap& rhs)
		{
			m_base = rhs.m_base;
			return *this;
		}
		SelfIndexedHashMap& operator=(SelfIndexedHashMap&& rhs)
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
		bool empty() const
		{
			return m_base.empty();
		}
		usize size() const
		{
			return m_base.size();
		}
		usize capacity() const
		{
			return m_base.capacity();
		}
		usize buffer_size() const
		{
			return m_base.buffer_size();
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
		void shrink_to_fit()
		{
			m_base.shrink_to_fit();
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
		bool contains(const key_type& key) const
		{
			return m_base.contains(key);
		}
		Pair<iterator, bool> insert(const value_type& value)
		{
			return m_base.insert(value);
		}
		Pair<iterator, bool> insert(value_type&& value)
		{
			return m_base.insert(move(value));
		}
		Pair<iterator, bool> insert_or_assign(const value_type& value)
		{
			return m_base.insert_or_assign(value);
		}
		Pair<iterator, bool> insert_or_assign(value_type&& value)
		{
			return m_base.insert_or_assign(move(value));
		}
		template <typename... _Args>
		Pair<iterator, bool> emplace(_Args&&... args)
		{
			return m_base.emplace(forward<_Args>(args)...);
		}
		iterator erase(const_iterator pos)
		{
			return m_base.erase(pos);
		}
		usize erase(const key_type& key)
		{
			return m_base.erase(key);
		}
		void swap(SelfIndexedHashMap& rhs)
		{
			SelfIndexedHashMap tmp(move(rhs));
			rhs = move(*this);
			*this = move(tmp);
		}
		allocator_type get_allocator() const
		{
			return m_base.get_allocator();
		}
	};
}