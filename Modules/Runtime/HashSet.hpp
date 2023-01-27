/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HashSet.hpp
* @author JXMaster
* @date 2022/5/1
*/
#pragma once
#include "Source/RobinHoodHashTable.hpp"
#include "TypeInfo.hpp"

namespace Luna
{
	template <
		typename _Kty,
		typename _Hash = hash<_Kty>,		// Used to hash the key value.
		typename _KeyEqual = equal_to<_Kty>,
		typename _Alloc = Allocator>	// Used to compare the element.
		class HashSet
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
		using iterator = RobinHoodHashing::Iterator<value_type, false>;
		using const_iterator = RobinHoodHashing::Iterator<value_type, true>;
	private:
		using table_type = RobinHoodHashing::HashTable<key_type, value_type, Impl::SetExtractKey<key_type, value_type>, hasher, key_equal, allocator_type>;
		table_type m_base;
		HashSet(table_type&& base) :
			m_base(move(base)) {}
	public:
		HashSet() :
			m_base() {}
		HashSet(const allocator_type& alloc) :
			m_base(alloc) {}
		HashSet(const HashSet& rhs) :
			m_base(rhs.m_base) {}
		HashSet(const HashSet& rhs, const allocator_type& alloc) :
			m_base(rhs.m_base, alloc) {}
		HashSet(HashSet&& rhs) :
			m_base(move(rhs.m_base)) {}
		HashSet(HashSet&& rhs, const allocator_type& alloc) :
			m_base(move(rhs.m_base), alloc) {}
		HashSet& operator=(const HashSet& rhs)
		{
			m_base = rhs.m_base;
			return *this;
		}
		HashSet& operator=(HashSet&& rhs)
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
		template <typename _M>
		Pair<iterator, bool> insert_or_assign(const key_type& key, _M&& value)
		{
			return m_base.template insert_or_assign<_M>(key, forward<_M>(value));
		}
		template <typename _M>
		Pair<iterator, bool> insert_or_assign(key_type&& key, _M&& value)
		{
			return m_base.template insert_or_assign<_M>(move(key), forward<_M>(value));
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
		void swap(HashSet& rhs)
		{
			HashSet tmp(move(rhs));
			rhs = move(*this);
			*this = move(tmp);
		}
		allocator_type get_allocator() const
		{
			return m_base.get_allocator();
		}
	};

	LUNA_RUNTIME_API typeinfo_t hash_set_type();
	template <typename _Ty> struct typeof_t<HashSet<_Ty>>
	{
		typeinfo_t operator()() const { return get_generic_instanced_type(hash_set_type(), { typeof<_Ty>() }); }
	};
}