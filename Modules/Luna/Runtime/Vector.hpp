/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Vector.hpp
* @author JXMaster
* @date 2018/12/9
*/
#pragma once
#include "Allocator.hpp"
#include "Span.hpp"
#include "Algorithm.hpp"
#include "TypeInfo.hpp"
#include "MemoryUtils.hpp"

namespace Luna
{
	//! @addtogroup RuntimeContainer
    //! @{

	//! A container that stores a continuous array of elements. Elements can be added to or removed from the container dynamically.
	//! @tparam _Ty The element type of the container.
	//! @tparam _Alloc The memory allocator used by the container. If not specified, @ref Allocator will be used.
	template <typename _Ty, typename _Alloc = Allocator>
	class Vector
	{
	public:

		using value_type = _Ty;
		using allocator_type = _Alloc;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;

		//! Constructs an empty vector.
		Vector();
		//! Constructs an empty vector with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the vector.
		Vector(const allocator_type& alloc);
		//! Constructs one vector with `count` elements, with their values initialized by `value`.
		//! @param[in] count The number of elements in the vector.
		//! @param[in] value The value for each element in the vector. The value is copy constructed into each element.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(usize count, const value_type& value, const allocator_type& alloc = allocator_type());
		//! Constructs one vector with `count` elements, which their values being default-initialized.
		//! @param[in] count The number of elements in the vector.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(usize count, const allocator_type& alloc = allocator_type());
		//! Constructs one vector with elements copied from the specified range.
		//! @param[in] first The iterator that points to the first element of the copy range.
		//! @param[in] last The iterator that points to the last element of the copy range.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		//! @details This function creates a vector whose elements are copied from range `[first, last)`, the iterator parameters should support be input iterators.
		template <typename _InputIt>
		Vector(enable_if_t<!is_integral_v<_InputIt>, _InputIt> first, _InputIt last, const allocator_type& alloc = allocator_type());
		//! Constructs a vector by copying elements from another vector.
		//! @param[in] rhs The vector to copy elements from.
		Vector(const Vector& rhs);
		//! Constructs a vector with an custom allocator and with elements copied from another vector.
		//! @param[in] rhs The vector to copy elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the vector.
		Vector(const Vector& rhs, const allocator_type& alloc);
		//! Constructs a vector by moving elements from another vector.
		//! @param[in] rhs The vector to move elements from.
		Vector(Vector&& rhs);
		//! Constructs a vector with an custom allocator and with elements moved from another vector.
		//! @param[in] rhs The vector to move elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the vector.
		Vector(Vector&& rhs, const allocator_type& alloc);
		//! Constructs one vector by coping all elements from an initializer list.
		//! @param[in] init The initializer list that contains the initial data of the vector. Every element of the new vector is copy-constructed from the corresponding elements in the initializer list.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(InitializerList<value_type> init, const allocator_type& alloc = allocator_type());
		
		//! Replaces elements of the vector by coping elements from another vector.
		//! @param[in] rhs The vector to copy elements from.
		//! @return Returns `*this`.
		Vector& operator=(const Vector& rhs);
		//! Replaces elements of the vector by moving elements from another vector.
		//! @param[in] rhs The vector to move elements from. This vector will be empty after this operation.
		//! @return Returns `*this`.
		Vector& operator=(Vector&& rhs);
		//! Replaces elements of the vector from one initializer list.
		//! @param[in] ilist The initializer list.
		//! @return Returns `*this`.
		Vector& operator=(InitializerList<value_type> ilist);

		~Vector();
		
		//! Gets one iterator to the first element of the vector.
		//! @return Returns one iterator to the first element of the vector.
		iterator begin();
		//! Gets one iterator to the one past last element of the vector.
		//! @return Returns one iterator to the one past last element of the vector.
		iterator end();
		//! Gets one constant iterator to the first element of the vector.
		//! @return Returns one constant iterator to the first element of the vector.
		const_iterator begin() const;
		//! Gets one constant iterator to the one past last element of the vector.
		//! @return Returns one constant iterator to the one past last element of the vector.
		const_iterator end() const;
		//! Gets one constant iterator to the first element of the vector.
		//! @return Returns one constant iterator to the first element of the vector.
		const_iterator cbegin() const;
		//! Gets one constant iterator to the one past last element of the vector.
		//! @return Returns one constant iterator to the one past last element of the vector.
		const_iterator cend() const;
		//! Gets one reverse iterator to the last element of the vector.
		//! @return Returns one reverse iterator to the last element of the vector.
		reverse_iterator rbegin();
		//! Gets one reverse iterator to the one-before-first element of the vector.
		//! @return Returns one reverse iterator to the one-before-first element of the vector.
		reverse_iterator rend();
		//! Gets one constant reverse iterator to the last element of the vector.
		//! @return Returns one constant reverse iterator to the last element of the vector.
		const_reverse_iterator rbegin() const;
		//! Gets one constant reverse iterator to the one-before-first element of the vector.
		//! @return Returns one constant reverse iterator to the one-before-first element of the vector.
		const_reverse_iterator rend() const;
		//! Gets one constant reverse iterator to the last element of the vector.
		//! @return Returns one constant reverse iterator to the last element of the vector.
		const_reverse_iterator crbegin() const;
		//! Gets one constant reverse iterator to the one-before-first element of the vector.
		//! @return Returns one constant reverse iterator to the one-before-first element of the vector.
		const_reverse_iterator crend() const;
		//! Gets the size of the vector, that is, the number of elements in the vector.
		//! @return Returns the size of the vector.
		usize size() const;
		//! Gets the capacity of the vector, that is, the maximum number of elements this vector can hold
		//! before next expansion.
		//! @return Returns the capacity of the vector.
		usize capacity() const;
		//! Checks whether this vector is empty, that is, the size of this vector is `0`.
		//! @return Returns `true` if this vector is empty, returns `false` otherwise.
		bool empty() const;
		//! Increases the capacity of the vector to a value greater than or equal to `new_cap`, so that it can 
		//! hold at least `new_cap` elements without reallocating the internal buffer.
		//! @details If `new_cap` is smaller than or equal to @ref capacity, this function does nothing.
		//! @param[in] new_cap The new capacity value to reserve.
		void reserve(usize new_cap);
		//! Resizes the vector.
		//! @param[in] n The new size of the vector.
		//! 
		//! If `n` is greater than @ref size, `n - size()` new elements will be default-inserted at the back of 
		//! the vector.
		//! 
		//! If `n` is smaller than @ref size, `size() - n` elements will be removed from the back of the vector.
		//! 
		//! If `n` is equal to @ref size, this function does nothing.
		void resize(usize n);
		//! Resizes the vector.
		//! @details If the new size is greater than @ref size, new elements will be copy-inserted at the back of 
		//! the vector using the provided value.
		//! 
		//! If the new size is smaller than @ref size, `size - n` elements will be removed from the back of the vector.
		//! 
		//! If the new size is equal to @ref size, this function does nothing.
		//! @param[in] n The new size of the vector.
		//! @param[in] v The initial value to copy for new elements.
		void resize(usize n, const value_type& v);
		//! Reduces the capacity of the vector so that @ref capacity == @ref size.
		//! @details If @ref size is `0`, this function releases the internal storage buffer. This can be
		//! used to clean up all dynamic memory allocated by this container.
		void shrink_to_fit();
		//! Gets the element at the specified index.
		//! @param[in] n The index of the element.
		//! @return Returns one reference to the element at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		reference operator[] (usize n);
		//! Gets the element at the specified index.
		//! @param[in] n The index of the element.
		//! @return Returns one constant reference to the element at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		const_reference operator[] (usize n) const;
		//! Gets the element at the specified index.
		//! @param[in] n The index of the element.
		//! @return Returns one reference to the element at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		reference at(usize n);
		//! Gets the element at the specified index.
		//! @param[in] n The index of the element.
		//! @return Returns one reference to the element at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		const_reference at(usize n) const;
		//! Gets the element at the front of the vector.
		//! @details The front element is the element with index `0`.
		//! @return Returns one reference to the front element of the vector.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		reference front();
		//! Gets the element at the front of the vector.
		//! @details The front element is the element with index `0`.
		//! @return Returns one constant reference to the front element of the vector.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		const_reference front() const;
		//! Gets the element at the back of the vector.
		//! @details The back element is the element with index `size() - 1`.
		//! @return Returns one reference to the back element of the vector.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		reference back();
		//! Gets the element at the back of the vector.
		//! @details The back element is the element with index `size() - 1`.
		//! @return Returns one constant reference to the back element of the vector.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		const_reference back() const;
		//! Gets one pointer to the data buffer of this vector.
		//! @return Returns one pointer to the data buffer of this vector.
		//! The returned pointer may be `nullptr` if `size()` is `0`, which indicates that the data buffer is not 
		//! allocated yet.
		pointer data();
		//! Gets one pointer to the data buffer of this vector.
		//! @return Returns one pointer to the data buffer of this vector.
		//! The returned pointer may be `nullptr` if `size()` is `0`, which indicates that the data buffer is not 
		//! allocated yet.
		const_pointer data() const;
		//! Removes all elements from the vector, but keeps the vector storage.
		//! @details The user can call @ref shrink_to_fit after this to free the storage.
		void clear();
		//! Pushes one element to the back of the vector.
		//! @param[in] val The element to push. The element will be copy-inserted to the vector.
		void push_back(const value_type& val);
		//! Pushes one element to the back of the vector.
		//! @param[in] val The element to push. The element will be move-inserted to the vector.
		void push_back(value_type&& val);
		//! Removes the element from the back of the vector.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		void pop_back();
		//! Replaces elements of the vector by several copies of the specified value.
		//! @param[in] count The number of copies to insert to the vector.
		//! @param[in] value The value to copy.
		void assign(usize count, const value_type& value);
		//! Replaces elements of the vector by elements specified by one range. Elements in the range will be copy-inserted into the vector.
		//! @param[in] first The iterator to the first element of the range.
		//! @param[in] last The iterator to the one-past-last element of the range.
		template <typename _InputIter>
		auto assign(_InputIter first, _InputIter last) -> enable_if_t<!is_integral_v<_InputIter>, void>;
		//! Replaces elements of the vector by elements from one initializer vector.
		//! @param[in] ivector The initializer vector.
		void assign(InitializerList<value_type> il);
		//! Replaces elements of the vector by elements specified by one span. Elements in the span will be copy-inserted into the vector.
		//! @param[in] data The span that specifies elements to copy from.
		template <typename _Rty>
		void assign(Span<_Rty> data);
		//! Inserts the specified element to the vector.
		//! @param[in] pos The iterator to the position to insert the element. The element will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the vector.
		//! @param[in] value The element to insert. The element will be copy-inserted into the vector.
		//! @return Returns one iterator to the inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the vector.
		iterator insert(const_iterator pos, const value_type& value);
		//! Inserts the specified element to the vector.
		//! @param[in] pos The iterator to the position to insert the element. The element will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the vector.
		//! @param[in] value The element to insert. The element will be move-inserted into the vector.
		//! @return Returns one iterator to the inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the vector.
		iterator insert(const_iterator pos, value_type&& value);
		//! Inserts several copies of the element to the vector.
		//! @param[in] pos The iterator to the position to insert elements. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the vector.
		//! @param[in] count The number of elements to insert.
		//! @param[in] value The value to initialize the new elements with.
		//! @return Returns one iterator to the first inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the vector.
		iterator insert(const_iterator pos, usize count, const value_type& value);
		//! Inserts one range of elements to the vector.
		//! @param[in] pos The iterator to the position to insert elements. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the vector.
		//! @param[in] first The iterator to the first element to be inserted.
		//! @param[in] last The iterator to the one-past-last element to be inserted.
		//! @return Returns one iterator to the first inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the vector.
		template <typename _InputIt>
		auto insert(const_iterator pos, _InputIt first, _InputIt last) -> enable_if_t<!is_integral_v<_InputIt>, iterator>;
		//! Inserts one range of elements specified by the initializer list to the vector.
		//! @param[in] pos The iterator to the position to insert elements. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the vector.
		//! @param[in] ilist The initializer list.
		//! @return Returns one iterator to the first inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the vector.
		iterator insert(const_iterator pos, InitializerList<value_type> ilist);
		//! Inserts elements specified by the span to the vector. Elements in the span will be copy-inserted into the vector.
		//! @param[in] pos The iterator to the position to insert elements. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the vector.
		//! @param[in] data The span that specifies elements to copy from.
		//! @return Returns one iterator to the inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the vector.
		template <typename _Rty>
		iterator insert(const_iterator pos, Span<_Rty> data);
		//! Removes one element from the vector.
		//! @param[in] pos The iterator to the element to be removed.
		//! @return Returns one iterator to the next element after the removed element, 
		//! or `end()` if such element does not exist.
		//! @par Valid Usage
		//! * `pos` must points to a valid element in the vector.
		iterator erase(const_iterator pos);
		//! Removes one range of elements from the vector.
		//! @param[in] first The iterator to the first element to be removed.
		//! @param[in] last The iterator to the one-past-last element to be removed.
		//! @return Returns one iterator to the next element after the removed elements, 
		//! or `end()` if such element does not exist.
		//! @par Valid Usage
		//! * `first` must be either `end()` or one valid element in the vector.
		//! * If `first != end()`, [`first`, `last`) must specifies either one empty range (`first == last`) or one valid element range of the vector.
		//! * If `first == end()`, [`first`, `last`) must specifies one empty range (`first == last`).
		iterator erase(const_iterator first, const_iterator last);
		//! Destructs the element at specified posiiton, then relocates the last element of the vector to the specified position.
		//! @details This can be used to prevent moving elements when the element order is not significant.
		//! @param[in] pos The iterator to the element to be removed.
		//! @return Returns one iterator to the next element after the removed element, 
		//! or `end()` if such element does not exist.
		//! @par Valid Usage
		//! * `pos` must points to a valid element in the vector.
		iterator swap_erase(const_iterator pos);
		//! Swaps elements of this vector with the specified vector.
		//! @param[in] rhs The vector to swap elements with.
		void swap(Vector& rhs);
		//! Constructs one element directly on the specified position of the vector using the provided arguments.
		//! @param[in] pos The iterator to the position to construct the element. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the vector.
		//! @param[in] args The arguments to construct the element. `_Ty(args...)` will be used to 
		//! construct the element.
		//! @return Returns one iterator to the constructed element.
		template <typename... _Args>
		iterator emplace(const_iterator pos, _Args&&... args);
		//! Constructs one element directly on the back of the vector using the provided arguments.
		//! @param[in] args The arguments to construct the element. `_Ty(args...)` will be used to 
		//! construct the element.
		//! @return Returns one reference to the constructed element.
		template <typename... _Args>
		iterator emplace_back(_Args&&... args);
		//! Gets the allocator of the vector.
		//! @return Returns one copy of the allocator of the vector.
		allocator_type get_allocator() const;
		//! Creates one span that specifies the element buffer of this vector.
		//! @return Returns the span that specifies the element buffer of this vector.
		Span<value_type> span();
		//! Creates one constant span that specifies the element buffer of this vector.
		//! @return Returns the constant span that specifies the element buffer of this vector.
		//! The return value of `data()` function of the returned span may be 
		Span<const value_type> span() const;
		//! Creates one constant span that specifies the element buffer of this vector.
		//! @return Returns the constant span that specifies the element buffer of this vector.
		Span<const value_type> cspan() const;

	private:
		// ---------------------------------------- Begin of ABI compatible part ----------------------------------------
		OptionalPair<allocator_type, _Ty*> m_allocator_buffer;	// The memory buffer and its allocator.
		usize m_size;		// Number of elements in the vector.
		usize m_capacity;	// Number of elements that can be included in the buffer before a reallocation is needed.
		// ----------------------------------------  End of ABI compatible part  ----------------------------------------

		value_type* internal_allocate(usize n);
		void internal_free(value_type* ptr, usize n);

		void free_buffer();
		void internal_expand_reserve(usize new_least_cap);

		template <typename _Iter>
		void internal_construct(enable_if_t<is_same_v<remove_cv_t<_Iter>, value_type*>, _Iter> first, _Iter last);

		template <typename _Iter>
		void internal_construct(enable_if_t<!is_same_v<remove_cv_t<_Iter>, value_type*>, _Iter> first, _Iter last);

		template <typename _InputIter>
		auto internal_assign(_InputIter first, _InputIter last)->enable_if_t<is_same_v<remove_cv_t<_InputIter>, value_type*>, void>;
		template <typename _InputIter>
		auto internal_assign(_InputIter first, _InputIter last)->enable_if_t<!is_same_v<remove_cv_t<_InputIter>, value_type*>, void>;
	
		template <typename _InputIt>
		auto internal_insert(const_iterator pos, _InputIt first, _InputIt last)->enable_if_t<is_same_v<remove_cv_t<_InputIt>, value_type*>, iterator>;
		template <typename _InputIt>
		auto internal_insert(const_iterator pos, _InputIt first, _InputIt last)->enable_if_t<!is_same_v<remove_cv_t<_InputIt>, value_type*>, iterator>;
	};

	//! Gets the type object of @ref Vector.
	//! @return Returns the type object of @ref Vector. The returned type is a generic type that can be
	//! instantiated by providing the element type.
	LUNA_RUNTIME_API typeinfo_t vector_type();
	template <typename _Ty> struct typeof_t<Vector<_Ty>>
	{
		typeinfo_t operator()() const { return get_generic_instanced_type(vector_type(), { typeof<_Ty>() }); }
	};

	//! @}
}

#include "Impl/Vector.inl"