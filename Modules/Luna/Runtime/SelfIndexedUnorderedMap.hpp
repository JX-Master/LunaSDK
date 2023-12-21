/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SelfIndexedUnorderedMap.hpp
* @author JXMaster
* @date 2021/7/6
*/
#pragma once
#include "Impl/OpenHashTable.hpp"

namespace Luna
{
	//! @addtogroup RuntimeContainer
    //! @{
	
	//! Represents one self-indexed unordered map whose key can be extracted from the value, so that
	//! it does not need to be stored.
	//! @details For every value type that the user want to use for self indexed
	//! unordered map, the user must define one special structure called "key extractor", and passes the type
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
	class SelfIndexedUnorderedMap
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
		using iterator = OpenHashTable::Iterator<value_type, false>;
		using const_iterator = OpenHashTable::Iterator<value_type, true>;
		using local_iterator = OpenHashTable::BucketIterator<value_type, false>;
		using const_local_iterator = OpenHashTable::BucketIterator<value_type, true>;

	private:
		using table_type = OpenHashTable::HashTable<key_type, value_type, extract_key, hasher, key_equal, allocator_type>;
		table_type m_base;

		SelfIndexedUnorderedMap(table_type&& base) :
			m_base(move(base)) {}

	public:
		using node_type = OpenHashTable::SetNodeHandle<value_type, allocator_type>;
		using insert_return_type = OpenHashTable::InsertResult<iterator, node_type>;

		//! Constructs an empty map.
		SelfIndexedUnorderedMap() :
			m_base() {}
		//! Constructs an empty map with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		SelfIndexedUnorderedMap(const allocator_type& alloc) :
			m_base(alloc) {}
		//! Constructs a map by coping elements from another map.
		//! @param[in] rhs The map to copy elements from.
		SelfIndexedUnorderedMap(const SelfIndexedUnorderedMap& rhs) :
			m_base(rhs.m_base) {}
		//! Constructs a map with an custom allocator and with elements copied from another map.
		//! @param[in] rhs The map to copy elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		SelfIndexedUnorderedMap(const SelfIndexedUnorderedMap& rhs, const allocator_type& alloc) :
			m_base(rhs.m_base, alloc) {}
		//! Constructs a map by moving elements from another map.
		//! @param[in] rhs The map to move elements from.
		SelfIndexedUnorderedMap(SelfIndexedUnorderedMap&& rhs) :
			m_base(move(rhs.m_base)) {}
		//! Constructs a map with an custom allocator and with elements moved from another map.
		//! @param[in] rhs The map to move elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		SelfIndexedUnorderedMap(SelfIndexedUnorderedMap&& rhs, const allocator_type& alloc) :
			m_base(move(rhs.m_base), alloc) {}
		//! Replaces elements of the map by coping elements from another map.
		//! @param[in] rhs The map to copy elements from.
		//! @return Returns `*this`.
		SelfIndexedUnorderedMap& operator=(const SelfIndexedUnorderedMap& rhs)
		{
			m_base = rhs.m_base;
			return *this;
		}
		//! Replaces elements of the map by moving elements from another map.
		//! @param[in] rhs The map to move elements from. This map will be empty after this operation.
		//! @return Returns `*this`.
		SelfIndexedUnorderedMap& operator=(SelfIndexedUnorderedMap&& rhs)
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
		//! Gets an iterator to the first element of the bucket with specified index.
		//! @param[in] n The index of the bucket.
		//! @return Returns one iterator to the first element of the bucket.
		//! @par Valid Usage
		//! * `n` must be smaller than @ref bucket_count.
		local_iterator begin(usize n)
		{
			return m_base.begin(n);
		}
		//! Gets an constant iterator to the first element of the bucket with specified index.
		//! @param[in] n The index of the bucket.
		//! @return Returns one constant iterator to the first element of the bucket.
		//! @par Valid Usage
		//! * `n` must be smaller than @ref bucket_count.
		const_local_iterator begin(usize n) const
		{
			return m_base.begin(n);
		}
		//! Gets a constant iterator to the first element of the bucket with specified index.
		//! @param[in] n The index of the bucket.
		//! @return Returns one constant iterator to the first element of the bucket.
		//! @par Valid Usage
		//! * `n` must be smaller than @ref bucket_count.
		const_local_iterator cbegin(usize n) const
		{
			return m_base.cbegin(n);
		}
		//! Gets an iterator to the one-past-last element of the bucket with specified index.
		//! @param[in] n The index of the bucket.
		//! @return Returns one iterator to the one-past-last element of the bucket.
		//! @par Valid Usage
		//! * `n` must be smaller than @ref bucket_count.
		local_iterator end(usize n)
		{
			return m_base.end(n);
		}
		//! Gets a constant iterator to the one-past-last element of the bucket with specified index.
		//! @param[in] n The index of the bucket.
		//! @return Returns one iterator to the one-past-last element of the bucket.
		//! @par Valid Usage
		//! * `n` must be smaller than @ref bucket_count.
		const_local_iterator end(usize n) const
		{
			return m_base.end(n);
		}
		//! Gets a constant iterator to the one-past-last element of the bucket with specified index.
		//! @param[in] n The index of the bucket.
		//! @return Returns one iterator to the one-past-last element of the bucket.
		//! @par Valid Usage
		//! * `n` must be smaller than @ref bucket_count.
		const_local_iterator cend(usize n) const
		{
			return m_base.cend(n);
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
		//! buckets can hold before expanding the bucket buffer.
		//! @return Returns the capacity of the map.
		usize capacity() const
		{
			return m_base.capacity();
		}
		//! Gets the number of buckets of the map.
		//! @return Returns the number of buckets of the map.
		usize bucket_count() const
		{
			return m_base.bucket_count();
		}
		//! Gets the number of elements of the specified bucket.
		//! @param[in] n The index of the bucket.
		//! @return Returns the number of elements of the specified bucket.
		//! @par Valid Usage
		//! * `n` must be smaller than @ref bucket_count.
		usize bucket_size(usize n) const
		{
			return m_base.bucket_size(n);
		}
		//! Gets the index of the bucket that stores the specified key.
		//! @param[in] key The key to check.
		//! @return Returns the index of the bucket that stores the specified key.
		//! @par Valid Usage
		//! * @ref bucket_count must not be `0` when calling this function.
		usize bucket(const key_type& key) const
		{
			return m_base.bucket(key);
		}
		//! Gets the load factor of the map, which can be computed by `(f32)size() / (f32)bucket_count()`.
		//! @return Returns the load factor of the map. Returns `0.0f` if @ref bucket_count is `0`.
		f32 load_factor() const
		{
			return m_base.load_factor();
		}
		//! Gets the maximum load factor allowed for the map. 
		//! @details If `load_factor() > max_load_factor()` is `true` after one element is inserted, the map
		//! will expand bucket buffer to bring more buckets.
		//! @return Returns the maximum load factor allowed for the map.
		f32 max_load_factor() const
		{
			return m_base.max_load_factor();
		}
		//! Sets the maximum load factor allowed for the map.
		//! @details If the new load factor is smaller than `load_factor()`, the map
		//! will expand bucket buffer to bring more buckets.
		//! @param[in] ml The new load factor to set.
		//! @par Valid Usage
		//! * `m` must be greater than `0.0f`.
		void max_load_factor(f32 ml)
		{
			m_base.max_load_factor(ml);
		}
		//! Removes all elements in the map.
		void clear()
		{
			m_base.clear();
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
		//! Changes the bucket count and rehashes all elements to insert them to the new buckets.
		//! @param[in] new_buckets_count The new bucket count to set.
		//! @remark If the new bucket count is too small or makes load factor exceed load factor limits, 
		//! the new bucket count will be expanded to a minimum value that satisfies requirements. You can specify
		//! `new_buckets_count` to `0` to shrink the map.
		void rehash(usize new_buckets_count)
		{
			m_base.rehash(new_buckets_count);
		}
		//! Expands the bucket buffer so that it can store at least `new_cap` elements without enpanding the 
		//! bucket buffer again.
		//! @param[in] new_cap The number of element to reserve.
		//! @remark This function does nothing if `new_cap` is smaller than or equal to @ref capacity.
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
		//! Gets one pair of iterators specifying one range of elements whose keys are equal to the specified key.
		//! @param[in] key The key to test.
		//! @return Returns the element range.
		//! @remark Since this map does not allow inserting multiple elements with the same key, the returned range 
		//! will contain only one element if the key exists, or empty if the key does not exist.
		Pair<iterator, iterator> equal_range(const key_type& key)
		{
			return m_base.equal_range(key);
		}
		//! Gets one pair of iterators specifying one range of elements whose keys are equal to the specified key.
		//! @param[in] key The key to test.
		//! @return Returns the element range.
		//! @remark Since this map does not allow inserting multiple elements with the same key, the returned range 
		//! will contain only one element if the key exists, or empty if the key does not exist.
		Pair<const_iterator, const_iterator> equal_range(const key_type& key) const
		{
			return m_base.equal_range(key);
		}
		//! Checks whether at least one element with the specified key exists.
		//! @param[in] key The key of the element to check.
		//! @return Returns `ture` if at least one element with the specified key exists. Returns `false` otherwise.
		bool contains(const key_type& key) const
		{
			return m_base.contains(key);
		}
		//! Inserts the specified value to the map. The key is extracted from the value.
		//! @param[in] value The value to insert. The element is copy-constructed into the map.
		//! @return Returns one iterator-bool pair indicating the insertion result:
		//! * If the returned Boolean value is `true`, then the element is successfully inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the insertion is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the map.
		Pair<iterator, bool> insert(const value_type& value)
		{
			return m_base.insert(value);
		}
		//! Inserts the specified value to the map. The key is extracted from the value.
		//! @param[in] value The value to insert. The element is move-constructed into the map.
		//! @return Returns one iterator-bool pair indicating the insertion result:
		//! * If the returned Boolean value is `true`, then the element is successfully inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then the insertion is failed because another element with the 
		//! same key already exists, and the returned iterator points to the existing element in the map.
		Pair<iterator, bool> insert(value_type&& value)
		{
			return m_base.insert(move(value));
		}
		//! Inserts one node to the map.
		//! @param[in] node The node to insert. The node must be extracted from one unordered map of the same type using
		//! @ref extract.
		//! @return Returns one structure identifying the insertion result:
		//! 1. If `node` is empty, `inserted` is false, `position` is `end()`, and `node` is empty.
		//! 2. Otherwise if the insertion took place, `inserted` is `true`, `position` points to the inserted element, and `node` is empty.
		//! 3. If the insertion failed, `inserted` is false, `node` has the previous value of `node`, and `position` points to an element with a key equivalent to the key of the node.
		insert_return_type insert(node_type&& node)
		{
			return m_base.insert<insert_return_type, node_type>(move(node));
		}
		//! Assigns the value to the element with the specified key, or inserts the value pair to the 
		//! map if such element is not found. The key is extracted from the value.
		//! @param[in] value The element value to assign or insert.
		//! @return Returns one iterator-bool pair indicating the result:
		//! * If the returned Boolean value is `true`, then the element is inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then one existing element is found and is assigned to the 
		//! specified value, and the returned iterator points to the existing element in the map.
		Pair<iterator, bool> insert_or_assign(const value_type& value)
		{
			return m_base.insert_or_assign(value);
		}
		//! Assigns the value to the element with the specified key, or inserts the value pair to the 
		//! map if such element is not found. The key is extracted from the value.
		//! @param[in] value The element value to assign or insert.
		//! @return Returns one iterator-bool pair indicating the result:
		//! * If the returned Boolean value is `true`, then the element is inserted to the map, and the 
		//! returned iterator points to the inserted element.
		//! * If the returned Boolean value is `false`, then one existing element is found and is assigned to the 
		//! specified value, and the returned iterator points to the existing element in the map.
		Pair<iterator, bool> insert_or_assign(value_type&& value)
		{
			return m_base.insert_or_assign(move(value));
		}
		//! Constructs one element directly in the map using the provided arguments.
		//! @param[in] args The arguments to construct the element. `_Ty(args...)` will be used to 
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
		void swap(SelfIndexedUnorderedMap& rhs)
		{
			SelfIndexedUnorderedMap tmp(move(rhs));
			rhs = move(*this);
			*this = move(tmp);
		}
		//! Extracts one node from the map, so that it can be inserted to another map without any 
		//! element copy or move operation.
		//! @param[in] pos The iterator to the element to be extracted.
		//! @return Returns the extracted node.
		//! @par Valid Usage
		//! * `pos` must points to a valid element in the map.
		node_type extract(const_iterator pos)
		{
			return m_base.extract<node_type>(pos);
		}
		//! Gets the allocator used by this map.
		//! @return Returns one copy of the allocator used by this map.
		allocator_type get_allocator() const
		{
			return m_base.get_allocator();
		}
	};

	//! @}
}