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
	//! @class Vector
	//! @ingroup Runtime
	//! @brief A dynamic container type that stores a continuous array of elements.
	template <typename _Ty, typename _Alloc = Allocator>
	class Vector
	{
	public:

		//! @name Memory Types
		//! @{
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
		//! @}

		//! @name Constructors
		//! @{

		//! @brief Constructs one empty vector with default-initialized allocator.
		Vector();
		//! @brief Constructs one empty vector using the specified allocator instance. 
		//! @param[alloc] The allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(const allocator_type& alloc);
		//! @brief Constructs one vector with `count` elements, with their values initialized by `value`.
		//! @param[in] count The number of elements in the vector.
		//! @param[in] value The value for each element in the vector. The value is copy constructed into each element.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(usize count, const value_type& value, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one vector with `count` elements, which their values being default-initialized.
		//! @param[in] count The number of elements in the vector.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(usize count, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one vector with elements copied from the specified range.
		//! @param[in] first The iterator that points to the first element of the copy range.
		//! @param[in] last The iterator that points to the last element of the copy range.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		//! @details This function creates a vector whose elements are copied from range `[first, last)`, the iterator parameters should support be input iterators.
		template <typename _InputIt>
		Vector(enable_if_t<is_same_v<remove_cv_t<_InputIt>, value_type*>, _InputIt> first, _InputIt last, const allocator_type& alloc = allocator_type());
		template <typename _InputIt>
		Vector(enable_if_t<!is_integral_v<_InputIt> && !is_same_v<remove_cv_t<_InputIt>, value_type*>, _InputIt> first, _InputIt last, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one vector by coping all elements from another vector.
		//! @param[in] rhs The vector to copy from. Every element of the new vector is copy-constructed from the corresponding elements in `rhs` vector.
		Vector(const Vector& rhs);
		//! @brief Constructs one vector by coping all elements from another vector, but uses different allocator.
		//! @param[in] rhs The vector to copy from. Every element of the new vector is copy-constructed from the corresponding elements in `rhs` vector.
		//! @param[in] alloc The allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(const Vector& rhs, const allocator_type& alloc);
		//! @brief Constructs one vector by moving all elements from another vector.
		//! @param[in] rhs The vector to move elements from. Every element of the new vector is move-constructed from the corresponding elements in `rhs` vector. `rhs` vector is empty after this operation.
		Vector(Vector&& rhs);
		//! @brief Constructs one vector by moving all elements from another vector, but uses different allocator.
		//! @param[in] rhs The vector to move elements from. Every element of the new vector is move-constructed from the corresponding elements in `rhs` vector. `rhs` vector is empty after this operation.
		//! @param[in] alloc The allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(Vector&& rhs, const allocator_type& alloc);
		//! @brief Constructs one vector by coping all elements from an initializer list.
		//! @param[in] init The initializer list that contains the initial data of the vector. Every element of the new vector is copy-constructed from the corresponding elements in the initializer list.
		//! @param[in] alloc The optioanl allocator instance bound to this vector. The allocator instance is copied into the vector type.
		Vector(InitializerList<value_type> init, const allocator_type& alloc = allocator_type());
		//! @}

		//! @name Assign vector data
		//! @{
		
		//! @brief Assigning vector data 
		Vector& operator=(const Vector& rhs);
		Vector& operator=(Vector&& rhs);
		Vector& operator=(InitializerList<value_type> ilist);

		//! @}

		~Vector();

		//! @name Iterating vector elements
		//! @{
		
		//! @brief Fetches one iterator that points to the first element in the vector.
		//! @return Returns one iterator that points to the first element in the vector.
		iterator begin();
		//! @brief Fetches one iterator that points to the one-past-last element in the vector.
		//! @return Returns one iterator that points to the one-past-last element in the vector.
		iterator end();
		//! @brief Fetches one constant iterator that points to the first element in the vector.
		//! @return Returns one constant iterator that points to the first element in the vector.
		const_iterator begin() const;
		//! @brief Fetches one constant iterator that points to the one-past-last  element in the vector.
		//! @return Returns one constant iterator that points to the one-past-last  element in the vector.
		const_iterator end() const;
		//! @brief Fetches one constant iterator that points to the first element in the vector.
		//! @return Returns one constant iterator that points to the first element in the vector.
		const_iterator cbegin() const;
		//! @brief Fetches one constant iterator that points to the one-past-last  element in the vector.
		//! @return Returns one constant iterator that points to the one-past-last  element in the vector.
		const_iterator cend() const;
		reverse_iterator rbegin();
		reverse_iterator rend();
		const_reverse_iterator rbegin() const;
		const_reverse_iterator rend() const;
		const_reverse_iterator crbegin() const;
		const_reverse_iterator crend() const;

		//! @}

		usize size() const;
		usize capacity() const;
		bool empty() const;
		void reserve(usize new_cap);
		void resize(usize n);
		void resize(usize n, const value_type& v);
		void shrink_to_fit();
		reference operator[] (usize n);
		const_reference operator[] (usize n) const;
		reference at(usize n);
		const_reference at(usize n) const;
		reference front();
		const_reference front() const;
		reference back();
		const_reference back() const;
		pointer data();
		const_pointer data() const;
		void clear();
		void push_back(const value_type& val);
		void push_back(value_type&& val);
		void pop_back();
		void assign(usize count, const value_type& value);
		template <typename _InputIter>
		auto assign(_InputIter first, _InputIter last)->enable_if_t<is_same_v<remove_cv_t<_InputIter>, value_type*>, void>;
		template <typename _InputIter>
		auto assign(_InputIter first, _InputIter last)->enable_if_t<!is_integral_v<_InputIter> && !is_same_v<remove_cv_t<_InputIter>, value_type*>, void>;
		void assign(InitializerList<value_type> il);
		template <typename _InputIt>
		void assign_n(_InputIt first, usize count);
		iterator insert(const_iterator pos, const value_type& val);
		iterator insert(const_iterator pos, value_type&& val);
		iterator insert(const_iterator pos, usize count, const value_type& val);
		template <typename _InputIt>
		auto insert(const_iterator pos, _InputIt first, _InputIt last)->enable_if_t<is_same_v<remove_cv_t<_InputIt>, value_type*>, iterator>;
		template <typename _InputIt>
		auto insert(const_iterator pos, _InputIt first, _InputIt last)->enable_if_t<!is_integral_v<_InputIt> && !is_same_v<remove_cv_t<_InputIt>, value_type*>, iterator>;
		iterator insert(const_iterator pos, InitializerList<value_type> il);
		template <typename _InputIt>
		iterator insert_n(const_iterator pos, _InputIt first, usize count);
		iterator erase(const_iterator pos);
		iterator erase(const_iterator first, const_iterator last);
		// Destructs the element at specified posiiton, then relocates the last element of the vector to the specified position.
		// This can be used to prevent moving elements when the element order is not significant.
		iterator swap_erase(const_iterator pos);
		void swap(Vector& rhs);
		template <typename... _Args>
		iterator emplace(const_iterator pos, _Args&&... args);
		template <typename... _Args>
		iterator emplace_back(_Args&&... args);
		allocator_type get_allocator() const;

		Span<value_type> span();
		Span<const value_type> span() const;
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
	};

	LUNA_RUNTIME_API typeinfo_t vector_type();
	template <typename _Ty> struct typeof_t<Vector<_Ty>>
	{
		typeinfo_t operator()() const { return get_generic_instanced_type(vector_type(), { typeof<_Ty>() }); }
	};
}

#include "Impl/Vector.inl"