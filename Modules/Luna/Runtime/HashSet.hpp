/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HashSet.hpp
* @author JXMaster
* @date 2022/5/1
*/
#pragma once
#include "Impl/RobinHoodHashTable.hpp"
#include "TypeInfo.hpp"

namespace Luna
{
	//! @addtogroup RuntimeContainer
    //! @{
	
	//! An container that contains a set of unique objects using open-addressing hashing algorithm.
	//! @remark See remarks of @ref HashMap for details.
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
		//! Constructs an empty set.
		HashSet() :
			m_base() {}
		//! Constructs an empty set with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the set.
		HashSet(const allocator_type& alloc) :
			m_base(alloc) {}
		//! Constructs a set by coping elements from another set.
		//! @param[in] rhs The set to copy elements from.
		HashSet(const HashSet& rhs) :
			m_base(rhs.m_base) {}
		//! Constructs a set with an custom allocator and with elements copied from another set.
		//! @param[in] rhs The set to copy elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the set.
		HashSet(const HashSet& rhs, const allocator_type& alloc) :
			m_base(rhs.m_base, alloc) {}
		//! Constructs a set by moving elements from another set.
		//! @param[in] rhs The set to move elements from.
		HashSet(HashSet&& rhs) :
			m_base(move(rhs.m_base)) {}
		//! Constructs a set with an custom allocator and with elements moved from another set.
		//! @param[in] rhs The set to move elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the set.
		HashSet(HashSet&& rhs, const allocator_type& alloc) :
			m_base(move(rhs.m_base), alloc) {}
		//! Replaces elements of the set by coping elements from another set.
		//! @param[in] rhs The set to copy elements from.
		//! @return Returns `*this`.
		HashSet& operator=(const HashSet& rhs)
		{
			m_base = rhs.m_base;
			return *this;
		}
		//! Replaces elements of the set by moving elements from another set.
		//! @param[in] rhs The set to move elements from. This set will be empty after this operation.
		//! @return Returns `*this`.
		HashSet& operator=(HashSet&& rhs)
		{
			m_base = move(rhs.m_base);
			return *this;
		}
		//! Gets one iterator to the first element of the set.
		//! @return Returns one iterator to the first element of the set.
		iterator begin()
		{
			return m_base.begin();
		}
		//! Gets one constant iterator to the first element of the set.
		//! @return Returns one constant iterator to the first element of the set.
		const_iterator begin() const
		{
			return m_base.begin();
		}
		//! Gets one constant iterator to the first element of the set.
		//! @return Returns one constant iterator to the first element of the set.
		const_iterator cbegin() const
		{
			return m_base.cbegin();
		}
		//! Gets one iterator to the one past last element of the set.
		//! @return Returns one iterator to the one past last element of the set.
		iterator end()
		{
			return m_base.end();
		}
		//! Gets one constant iterator to the one past last element of the set.
		//! @return Returns one constant iterator to the one past last element of the set.
		const_iterator end() const
		{
			return m_base.end();
		}
		//! Gets one constant iterator to the one past last element of the set.
		//! @return Returns one constant iterator to the one past last element of the set.
		const_iterator cend() const
		{
			return m_base.cend();
		}
		//! Checks whether this set is empty, that is, the size of this set is `0`.
		//! @return Returns `true` if this set is empty, returns `false` otherwise.
		bool empty() const
		{
			return m_base.empty();
		}
		//! Gets the size of the set, that is, the number of elements in the set.
		//! @return Returns the size of the set.
		usize size() const
		{
			return m_base.size();
		}
		//! Gets the capacity of the set, that is, the number of elements the 
		//! hash table can hold before expanding the hash table.
		//! @return Returns the capacity of the set.
		usize capacity() const
		{
			return m_base.capacity();
		}
		//! Gets the hash table size of the set, that is, the number of slots of the
		//! hash table array.
		//! @return Returns the hash table size of the set.
		usize hash_table_size() const
		{
			return m_base.hash_table_size();
		}
		//! Gets the load factor of the set, which can be computed by `(f32)size() / (f32)hash_table_size()`.
		//! @return Returns the load factor of the set.
		f32 load_factor() const
		{
			return m_base.load_factor();
		}
		//! Gets the maximum load factor allowed for the set. 
		//! @details If `load_factor() > max_load_factor()` is `true` after one element is inserted, the set
		//! will expand the hash table to bring more hash table slots.
		//! @return Returns the maximum load factor allowed for the set.
		f32 max_load_factor() const
		{
			return m_base.max_load_factor();
		}
		//! Sets the maximum load factor allowed for the set.
		//! @details If the new load factor is smaller than `load_factor()`, the set
		//! will expand the hash table to bring more hash table slots.
		//! @param[in] ml The new load factor to set.
		//! @par Valid Usage
		//! * `ml` must between [`0.0`, `1.0`].
		void max_load_factor(f32 ml)
		{
			m_base.max_load_factor(ml);
		}
		//! Removes all elements in the set.
		void clear()
		{
			m_base.clear();
		}
		//! Reduces the hash table size to a minimum value that satisfy the maximum load factor limitation.
		//! @details The hash table size can be computed as: `ceilf((f32)size() / max_load_factor())`.
		void shrink_to_fit()
		{
			m_base.shrink_to_fit();
		}
		//! Gets the hash function used by this set.
		//! @return Returns the hash function used by this set.
		hasher hash_function() const
		{
			return m_base.hash_function();
		}
		//! Gets the equality comparison function used by this set.
		//! @return Returns the equality comparison function used by this set.
		key_equal key_eq() const
		{
			return m_base.key_eq();
		}
		//! Changes the data table size and rehashes all elements to insert them to the new data table.
		//! @param[in] new_data_table_size The new data table size to set.
		//! @remark If the new data table size is too small or makes load factor exceed load factor limits, 
		//! the new data table size will be expanded to a minimum value that satisfies requirements.
		void rehash(usize new_buckets_count)
		{
			m_base.rehash(new_buckets_count);
		}
		//! Expands the data table size to the specified value.
		//! @param[in] new_cap The new data table size to expand to.
		//! @remark This function does nothing if `new_cap` is smaller than or equal to `capacity()`.
		void reserve(usize new_cap)
		{
			m_base.reserve(new_cap);
		}
		//! Finds the specified element in the set.
		//! @param[in] key The key of the element to find.
		//! @return Returns one iterator to the element if the element is found. Returns `end()` otherwise.
		iterator find(const key_type& key)
		{
			return m_base.find(key);
		}
		//! Finds the specified element in the set.
		//! @param[in] key The key of the element to find.
		//! @return Returns one const iterator to the element if the element is found. Returns `end()` otherwise.
		const_iterator find(const key_type& key) const
		{
			return m_base.find(key);
		}
		//! Gets the number of elements whose key is equal to the specified key.
		//! @param[in] key The key of the element to count.
		//! @return Returns the number of elements whose key is equal to the specified key.
		//! @remark Since this set does not allow inserting multiple elements with the same key, the returned value 
		//! will only be `1` if the key exists, or `0` if the key does not exist.
		usize count(const key_type& key) const
		{
			return m_base.count(key);
		}
		//! Checks whether at least one element with the specified key exists.
		//! @param[in] key The key of the element to check.
		//! @return Returns `ture` if at least one element with the specified key exists. Returns `false` otherwise.
		bool contains(const key_type& key) const
		{
			return m_base.contains(key);
		}
		//! Inserts the specified value to the set.
		//! @param[in] value The value to insert. The element is copy-constructed into the set.
		//! @return Returns one iterator-bool pair indicating the insertion result:
		//! * If the returned Boolean value is `true`, then the element is successfully inserted to the set, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the insertion is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the set.
		Pair<iterator, bool> insert(const value_type& value)
		{
			return m_base.insert(value);
		}
		//! Inserts the specified value to the set.
		//! @param[in] value The value to insert. The element is move-constructed into the set.
		//! @return Returns one iterator-bool pair indicating the insertion result:
		//! * If the returned Boolean value is `true`, then the element is successfully inserted to the set, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the insertion is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the set.
		Pair<iterator, bool> insert(value_type&& value)
		{
			return m_base.insert(move(value));
		}
		//! Constructs one element directly in the set using the provided arguments.
		//! @param[in] args The arguments to construct the element. `_Kty(args...)` will be used to 
		//! construct the element.
		//! @return Returns one iterator-bool pair indicating the result:
		//! * If the returned Boolean value is `true`, then the element is successfully constructed and inserted to 
		//! the set, and the returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the operation is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the set.
		template <typename... _Args>
		Pair<iterator, bool> emplace(_Args&&... args)
		{
			return m_base.emplace(forward<_Args>(args)...);
		}
		//! Removes one element from the set.
		//! @param[in] pos The iterator to the element to be removed.
		//! @return Returns one iterator to the next element after the removed element, 
		//! or `end()` if such element does not exist.
		//! @par Valid Usage
		//! * `pos` must points to a valid element in the set.
		iterator erase(const_iterator pos)
		{
			return m_base.erase(pos);
		}
		//! Removes elements with the specified key from the set.
		//! @param[in] key The key of the elements to remove.
		//! @return Returns the number of elements removed by this operation.
		//! @remark The returned value can only be `0` or `1` for this set type.
		usize erase(const key_type& key)
		{
			return m_base.erase(key);
		}
		//! Swaps elements of this set with the specified set.
		//! @param[in] rhs The set to swap elements with.
		void swap(HashSet& rhs)
		{
			HashSet tmp(move(rhs));
			rhs = move(*this);
			*this = move(tmp);
		}
		//! Gets the allocator used by this set.
		//! @return Returns one copy of the allocator used by this set.
		allocator_type get_allocator() const
		{
			return m_base.get_allocator();
		}
	};
	//! Gets the type object of @ref HashSet.
	//! @return Returns the type object of @ref HashSet. The returned type is a generic type that can be
	//! instantiated by providing the key and value type.
	LUNA_RUNTIME_API typeinfo_t hash_set_type();
	template <typename _Ty> struct typeof_t<HashSet<_Ty>>
	{
		typeinfo_t operator()() const { return get_generic_instanced_type(hash_set_type(), { typeof<_Ty>() }); }
	};

	//! @}
}