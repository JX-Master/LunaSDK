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
	//! @addtogroup RuntimeContainer
    //! @{
	
	///! An container that contains a set of unique objects using closed-addressing hashing algorithm. This container
	//! allows multiple copies of the same value exist in the container.
	//! @remark See remarks of @ref HashMap for details.
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

		//! Constructs an empty set.
		UnorderedMultiSet() :
			m_base() {}
		//! Constructs an empty set with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the set.
		UnorderedMultiSet(const allocator_type& alloc) :
			m_base(alloc) {}
		//! Constructs a set by coping elements from another set.
		//! @param[in] rhs The set to copy elements from.
		UnorderedMultiSet(const UnorderedMultiSet& rhs) :
			m_base(rhs.m_base) {}
		//! Constructs a set with an custom allocator and with elements copied from another set.
		//! @param[in] rhs The set to copy elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the set.
		UnorderedMultiSet(const UnorderedMultiSet& rhs, const allocator_type& alloc) :
			m_base(rhs.m_base, alloc) {}
		//! Constructs a set by moving elements from another set.
		//! @param[in] rhs The set to move elements from.
		UnorderedMultiSet(UnorderedMultiSet&& rhs) :
			m_base(move(rhs.m_base)) {}
		//! Constructs a set with an custom allocator and with elements moved from another set.
		//! @param[in] rhs The set to move elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the set.
		UnorderedMultiSet(UnorderedMultiSet&& rhs, const allocator_type& alloc) :
			m_base(move(rhs.m_base), alloc) {}
		//! Replaces elements of the set by coping elements from another set.
		//! @param[in] rhs The set to copy elements from.
		//! @return Returns `*this`.
		UnorderedMultiSet& operator=(const UnorderedMultiSet& rhs)
		{
			m_base = rhs.m_base;
			return *this;
		}
		//! Replaces elements of the set by moving elements from another set.
		//! @param[in] rhs The set to move elements from. This set will be empty after this operation.
		//! @return Returns `*this`.
		UnorderedMultiSet& operator=(UnorderedMultiSet&& rhs)
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
		//! buckets can hold before expanding the bucket buffer.
		//! @return Returns the capacity of the set.
		usize capacity() const
		{
			return m_base.capacity();
		}
		//! Gets the number of buckets of the set.
		//! @return Returns the number of buckets of the set.
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
		//! Gets the load factor of the set, which can be computed by `(f32)size() / (f32)bucket_count()`.
		//! @return Returns the load factor of the set. Returns `0.0f` if @ref bucket_count is `0`.
		f32 load_factor() const
		{
			return m_base.load_factor();
		}
		//! Gets the maximum load factor allowed for the set. 
		//! @details If `load_factor() > max_load_factor()` is `true` after one element is inserted, the set
		//! will expand bucket buffer to bring more buckets.
		//! @return Returns the maximum load factor allowed for the set.
		f32 max_load_factor() const
		{
			return m_base.max_load_factor();
		}
		//! Sets the maximum load factor allowed for the set.
		//! @details If the new load factor is smaller than `load_factor()`, the set
		//! will expand bucket buffer to bring more buckets.
		//! @param[in] ml The new load factor to set.
		//! @par Valid Usage
		//! * `m` must be greater than `0.0f`.
		void max_load_factor(f32 ml)
		{
			m_base.max_load_factor(ml);
		}
		//! Removes all elements in the set.
		void clear()
		{
			m_base.clear();
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
		//! Changes the bucket count and rehashes all elements to insert them to the new buckets.
		//! @param[in] new_buckets_count The new bucket count to set.
		//! @remark If the new bucket count is too small or makes load factor exceed load factor limits, 
		//! the new bucket count will be expanded to a minimum value that satisfies requirements. You can specify
		//! `new_buckets_count` to `0` to shrink the set.
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
		usize count(const key_type& key) const
		{
			return m_base.count(key);
		}
		//! Gets one pair of iterators specifying one range of elements whose keys are equal to the specified key.
		//! @param[in] key The key to test.
		//! @return Returns the element range.
		Pair<iterator, iterator> equal_range(const key_type& key)
		{
			return m_base.equal_range(key);
		}
		//! Gets one pair of iterators specifying one range of elements whose keys are equal to the specified key.
		//! @param[in] key The key to test.
		//! @return Returns the element range.
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
		//! Inserts the specified value to the set. The key is extracted from the value.
		//! @param[in] value The value to insert. The element is copy-constructed into the set.
		//! @return Returns one iterator to the inserted element.
		iterator insert(const value_type& value)
		{
			return m_base.multi_insert(value);
		}
		//! Inserts the specified value to the set. The key is extracted from the value.
		//! @param[in] value The value to insert. The element is move-constructed into the set.
		//! @return Returns one iterator to the inserted element.
		iterator insert(value_type&& value)
		{
			return m_base.multi_insert(move(value));
		}
		//! Inserts one node to the set if the node is not empty.
		//! @param[in] node The node to insert. The node must be extracted from one unordered set of the same type using
		//! @ref extract.
		//! @return Returns one iterator to the inserted element. Returns `end()` if `node` is empty.
		iterator insert(node_type&& node)
		{
			return m_base.template multi_insert<node_type>(move(node));
		}
		//! Constructs one element directly in the set using the provided arguments.
		//! @param[in] args The arguments to construct the element. `_Ty(args...)` will be used to 
		//! construct the element.
		//! @return Returns one iterator to the inserted element.
		template <typename... _Args>
		iterator emplace(_Args&&... args)
		{
			return m_base.multi_emplace(forward<_Args>(args)...);
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
		usize erase(const key_type& key)
		{
			return m_base.multi_erase(key);
		}
		//! Swaps elements of this set with the specified set.
		//! @param[in] rhs The set to swap elements with.
		void swap(UnorderedMultiSet& rhs)
		{
			UnorderedMultiSet tmp(move(rhs));
			rhs = move(*this);
			*this = move(tmp);
		}
		//! Extracts one node from the set, so that it can be inserted to another set without any 
		//! element copy or move operation.
		//! @param[in] pos The iterator to the element to be extracted.
		//! @return Returns the extracted node.
		//! @par Valid Usage
		//! * `pos` must points to a valid element in the set.
		node_type extract(const_iterator pos)
		{
			return m_base.template extract<node_type>(pos);
		}
		//! Gets the allocator used by this set.
		//! @return Returns one copy of the allocator used by this set.
		allocator_type get_allocator() const
		{
			return m_base.get_allocator();
		}
	};

	//! @}
}
