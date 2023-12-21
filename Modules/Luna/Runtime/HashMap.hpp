/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HashMap.hpp
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
	
	//! An container that contains key-value pairs with unique keys using open-addressing hashing algorithm.
	//! @remark LunaSDK provides two kinds of hashing-based containers: open-addressing containers and closed-addressing containers.
	//! The following containers are open-addressing containers, implemented using Robinhood hashing:
	//! 
	//! 1. @ref HashMap
	//! 2. @ref HashSet
	//! 3. @ref SelfIndexedHashmap
	//! The following containers are closed-addressing containers, implemented using buckets and per-bucket linked-lists:
	//! 1. @ref UnorderedMap
	//! 2. @ref UnorderedSet
	//! 3. @ref UnorderedMultiMap
	//! 4. @ref UnorderedMultiSet
	//! 5. @ref SelfIndexedUnorderedMap
	//! 6. @ref SelfIndexedUnorderedMultiMap
	//! 
	//! Open addressing (also known as closed hashing) algorithms store elements directly in hash table arrays, while closed addressing (also known as open hashing) algorithms allocate
	//! dedicated memory for every element, and stores pointers to such elements in hash table arrays. In open-addressing containers, one hash table slot can only store on element, the
	//! second element with the same hash value must be relocated to another empty slot; in closed-addressing containers, all elements with the same hash value can be stored in the same 
	//! hash table slot, usually stored as linked lists. See [Open vs Closed Addressing](https://programming.guide/hash-tables-open-vs-closed-addressing.html) for a detailed comparison 
	//! of open addressing and closed addressing.
	//! 
	//! Prefer @ref HashMap and @ref HashSet instead of @ref UnorderedMap and @ref UnorderedSet, since it performs better in memory fragmentation, memory locality and cache performance. 
	//! Use @ref UnorderedMap and @ref UnorderedSet if you have the following requirements:
	//! 
	//! 1. You want to insert multiple elements with the same key to the map, which is only supported by closed-addressing maps. Use @ref UnorderedMultiMap, @ref SelfIndexedUnorderedMultiMap
	//! and @ref UnorderedMultiSet in such case.
	//! 2. You element type has very big size, usually larger than 256, causing allocating element memory in data table become unacceptable because it will waste a lot of memory when
	//! the load factor is low. Closed-addressing maps only allocate memory for alive elements, making it consuming much less memory than open-addressing maps when the element size is big. 
	//! Closed-addressing maps also support extracting element nodes from one map and insert them to another maps without the need of allocating memory for elements, making it efficient to 
	//! transfer elements between maps.
	template <
		typename _Kty,
		typename _Ty,
		typename _Hash = hash<_Kty>,		// Used to hash the key value.
		typename _KeyEqual = equal_to<_Kty>,
		typename _Alloc = Allocator>	// Used to compare the element.
	class HashMap
	{
	public:
		using key_type = _Kty;
		using mapped_type = _Ty;
		using value_type = Pair<const _Kty, _Ty>;
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

		using table_type = RobinHoodHashing::HashTable<key_type, value_type, Impl::MapExtractKey<key_type, value_type>, hasher, key_equal, allocator_type>;

		table_type m_base;

		HashMap(table_type&& base) :
			m_base(move(base)) {}

	public:
		//! Constructs an empty map.
		HashMap() :
			m_base() {}
		//! Constructs an empty map with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		HashMap(const allocator_type& alloc) :
			m_base(alloc) {}
		//! Constructs a map by coping elements from another map.
		//! @param[in] rhs The map to copy elements from.
		HashMap(const HashMap& rhs) :
			m_base(rhs.m_base) {}
		//! Constructs a map with an custom allocator and with elements copied from another map.
		//! @param[in] rhs The map to copy elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		HashMap(const HashMap& rhs, const allocator_type& alloc) :
			m_base(rhs.m_base, alloc) {}
		//! Constructs a map by moving elements from another map.
		//! @param[in] rhs The map to move elements from.
		HashMap(HashMap&& rhs) :
			m_base(move(rhs.m_base)) {}
		//! Constructs a map with an custom allocator and with elements moved from another map.
		//! @param[in] rhs The map to move elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		HashMap(HashMap&& rhs, const allocator_type& alloc) :
			m_base(move(rhs.m_base), alloc) {}
		//! Replaces elements of the map by coping elements from another map.
		//! @param[in] rhs The map to copy elements from.
		//! @return Returns `*this`.
		HashMap& operator=(const HashMap& rhs)
		{
			m_base = rhs.m_base;
			return *this;
		}
		//! Replaces elements of the map by moving elements from another map.
		//! @param[in] rhs The map to move elements from. This map will be empty after this operation.
		//! @return Returns `*this`.
		HashMap& operator=(HashMap&& rhs)
		{
			m_base = move(rhs.m_base);
			return *this;
		}
		//! Gets one iterator to the first element of the map.
		//! @return Returns one iterator to the first element of the map.
		iterator begin()
		{
			return m_base.begin();
		}
		//! Gets one constant iterator to the first element of the map.
		//! @return Returns one constant iterator to the first element of the map.
		const_iterator begin() const
		{
			return m_base.begin();
		}
		//! Gets one constant iterator to the first element of the map.
		//! @return Returns one constant iterator to the first element of the map.
		const_iterator cbegin() const
		{
			return m_base.cbegin();
		}
		//! Gets one iterator to the one past last element of the map.
		//! @return Returns one iterator to the one past last element of the map.
		iterator end()
		{
			return m_base.end();
		}
		//! Gets one constant iterator to the one past last element of the map.
		//! @return Returns one constant iterator to the one past last element of the map.
		const_iterator end() const
		{
			return m_base.end();
		}
		//! Gets one constant iterator to the one past last element of the map.
		//! @return Returns one constant iterator to the one past last element of the map.
		const_iterator cend() const
		{
			return m_base.cend();
		}
		//! Checks whether this map is empty, that is, the size of this map is `0`.
		//! @return Returns `true` if this map is empty, returns `false` otherwise.
		bool empty() const
		{
			return m_base.empty();
		}
		//! Gets the size of the map, that is, the number of elements in the map.
		//! @return Returns the size of the map.
		usize size() const
		{
			return m_base.size();
		}
		//! Gets the capacity of the map, that is, the number of elements the 
		//! hash table can hold before expanding the hash table.
		//! @return Returns the capacity of the map.
		usize capacity() const
		{
			return m_base.capacity();
		}
		//! Gets the hash table size of the map, that is, the number of slots of the
		//! hash table array.
		//! @return Returns the hash table size of the map.
		usize hash_table_size() const
		{
			return m_base.hash_table_size();
		}
		//! Gets the load factor of the map, which can be computed by `(f32)size() / (f32)hash_table_size()`.
		//! @return Returns the load factor of the map.
		f32 load_factor() const
		{
			return m_base.load_factor();
		}
		//! Gets the maximum load factor allowed for the map. 
		//! @details If `load_factor() > max_load_factor()` is `true` after one element is inserted, the map
		//! will expand the hash table to bring more hash table slots.
		//! @return Returns the maximum load factor allowed for the map.
		f32 max_load_factor() const
		{
			return m_base.max_load_factor();
		}
		//! Sets the maximum load factor allowed for the map.
		//! @details If the new load factor is smaller than `load_factor()`, the map
		//! will expand the hash table to bring more hash table slots.
		//! @param[in] ml The new load factor to set.
		//! @par Valid Usage
		//! * `ml` must between [`0.0`, `1.0`].
		void max_load_factor(f32 ml)
		{
			m_base.max_load_factor(ml);
		}
		//! Removes all elements in the map.
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
		//! Gets the hash function used by this map.
		//! @return Returns the hash function used by this map.
		hasher hash_function() const
		{
			return m_base.hash_function();
		}
		//! Gets the equality comparison function used by this map.
		//! @return Returns the equality comparison function used by this map.
		key_equal key_eq() const
		{
			return m_base.key_eq();
		}
		//! Changes the data table size and rehashes all elements to insert them to the new data table.
		//! @param[in] new_data_table_size The new data table size to set.
		//! @remark If the new data table size is too small or makes load factor exceed load factor limits, 
		//! the new data table size will be expanded to a minimum value that satisfies requirements.
		void rehash(usize new_data_table_size)
		{
			m_base.rehash(new_data_table_size);
		}
		//! Expands the data table size to the specified value.
		//! @param[in] new_cap The new data table size to expand to.
		//! @remark This function does nothing if `new_cap` is smaller than or equal to `capacity()`.
		void reserve(usize new_cap)
		{
			m_base.reserve(new_cap);
		}
		//! Finds the specified element in the map.
		//! @param[in] key The key of the element to find.
		//! @return Returns one iterator to the element if the element is found. Returns `end()` otherwise.
		iterator find(const key_type& key)
		{
			return m_base.find(key);
		}
		//! Finds the specified element in the map.
		//! @param[in] key The key of the element to find.
		//! @return Returns one const iterator to the element if the element is found. Returns `end()` otherwise.
		const_iterator find(const key_type& key) const
		{
			return m_base.find(key);
		}
		//! Gets the number of elements whose key is equal to the specified key.
		//! @param[in] key The key of the element to count.
		//! @return Returns the number of elements whose key is equal to the specified key.
		//! @remark Since this map does not allow inserting multiple elements with the same key, the returned value 
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
		//! Inserts the specified key-value pair to the map.
		//! @param[in] value The key-value pair to insert. The element is copy-constructed into the map.
		//! @return Returns one iterator-bool pair indicating the insertion result:
		//! * If the returned Boolean value is `true`, then the element is successfully inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the insertion is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the map.
		Pair<iterator, bool> insert(const value_type& value)
		{
			return m_base.insert(value);
		}
		//! Inserts the specified key-value pair to the map.
		//! @param[in] value The key-value pair to insert. The element is move-constructed into the map.
		//! @return Returns one iterator-bool pair indicating the insertion result:
		//! * If the returned Boolean value is `true`, then the element is successfully inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the insertion is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the map.
		Pair<iterator, bool> insert(value_type&& value)
		{
			return m_base.insert(move(value));
		}
		//! Assigns the value to the element with the specified key, or inserts the key-value pair to the 
		//! map if such element is not found.
		//! @param[in] key The key of the element to assign or insert.
		//! @param[in] value The element value to assign or insert.
		//! @return Returns one iterator-bool pair indicating the result:
		//! * If the returned Boolean value is `true`, then the element is inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then one existing element is found and is assigned to the 
		//! specified value, and the returned iterator points to the existing element in the map.
		template <typename _M>
		Pair<iterator, bool> insert_or_assign(const key_type& key, _M&& value)
		{
			return m_base.template insert_or_assign<_M>(key, forward<_M>(value));
		}
		//! Assigns the value to the element with the specified key, or inserts the key-value pair to the 
		//! map if such element is not found.
		//! @param[in] key The key of the element to assign or insert.
		//! @param[in] value The element value to assign or insert.
		//! @return Returns one iterator-bool pair indicating the result:
		//! * If the returned Boolean value is `true`, then the element is inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then one existing element is found and is assigned to the 
		//! specified value, and the returned iterator points to the existing element in the map.
		template <typename _M>
		Pair<iterator, bool> insert_or_assign(key_type&& key, _M&& value)
		{
			return m_base.template insert_or_assign<_M>(move(key), forward<_M>(value));
		}
		//! Constructs one element directly in the map using the provided arguments.
		//! @param[in] args The arguments to construct the element. `Pair<const _Kty, _Ty>(args...)` will be used to 
		//! construct the element.
		//! @return Returns one iterator-bool pair indicating the result:
		//! * If the returned Boolean value is `true`, then the element is successfully constructed and inserted to 
		//! the map, and the returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the operation is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the map.
		template <typename... _Args>
		Pair<iterator, bool> emplace(_Args&&... args)
		{
			return m_base.emplace(forward<_Args>(args)...);
		}
		//! Removes one element from the map.
		//! @param[in] pos The iterator to the element to be removed.
		//! @return Returns one iterator to the next element after the removed element, 
		//! or `end()` if such element does not exist.
		//! @par Valid Usage
		//! * `pos` must points to a valid element in the map.
		iterator erase(const_iterator pos)
		{
			return m_base.erase(pos);
		}
		//! Removes elements with the specified key from the map.
		//! @param[in] key The key of the elements to remove.
		//! @return Returns the number of elements removed by this operation.
		//! @remark The returned value can only be `0` or `1` for this map type.
		usize erase(const key_type& key)
		{
			return m_base.erase(key);
		}
		//! Swaps elements of this map with the specified map.
		//! @param[in] rhs The map to swap elements with.
		void swap(HashMap& rhs)
		{
			HashMap tmp(move(rhs));
			rhs = move(*this);
			*this = move(tmp);
		}
		//! Gets the allocator used by this map.
		//! @return Returns one copy of the allocator used by this map.
		allocator_type get_allocator() const
		{
			return m_base.get_allocator();
		}
	};

	//! Gets the type object of @ref HashMap.
	//! @return Returns the type object of @ref HashMap. The returned type is a generic type that can be
	//! instantiated by providing the key and value type.
	LUNA_RUNTIME_API typeinfo_t hash_map_type();

	template <typename _Kty, typename _Ty> struct typeof_t<HashMap<_Kty, _Ty>>
	{
		typeinfo_t operator()() const { return get_generic_instanced_type(hash_map_type(), { typeof<_Kty>(), typeof<_Ty>() }); }
	};

	//! @}
}