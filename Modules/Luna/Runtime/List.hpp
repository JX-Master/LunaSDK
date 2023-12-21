/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file List.hpp
* @author JXMaster
* @date 2021/4/27
*/
#pragma once
#include "Allocator.hpp"
#include "Algorithm.hpp"
#include "Functional.hpp"

namespace Luna
{
	template <typename _Ty, typename _Alloc>
	class List;

	namespace ListImpl
	{
		struct NodeBase
		{
			NodeBase* m_next;
			NodeBase* m_prev;

			NodeBase() = default;
			NodeBase(NodeBase* next, NodeBase* prev) :
				m_next(next),
				m_prev(prev) {}

			void insert_before(NodeBase* pos)
			{
				NodeBase* next_node = pos;
				NodeBase* prev_node = pos->m_prev;
				prev_node->m_next = this;
				next_node->m_prev = this;
				m_prev = prev_node;
				m_next = next_node;
			}
			void remove_this()
			{
				m_next->m_prev = m_prev;
				m_prev->m_next = m_next;
			}

			// [first, last]
			static void insert_range(NodeBase* pos, NodeBase* first, NodeBase* last)
			{
				NodeBase* next_node = pos;
				NodeBase* prev_node = pos->m_prev;
				prev_node->m_next = first;
				next_node->m_prev = last;
				first->m_prev = prev_node;
				last->m_next = next_node;
			}

			// [first, last]
			static void remove_range(NodeBase* first, NodeBase* last)
			{
				last->m_next->m_prev = first->m_prev;
				first->m_prev->m_next = last->m_next;
			}
		};

		template <typename _Ty>
		struct Node : NodeBase
		{
			_Ty m_elem;

			Node() :
				m_elem() {}

			Node(const _Ty& elem) :
				m_elem(elem) {}

			Node(_Ty&& elem) :
				m_elem(move(elem)) {}
		};

		template <typename _Ty, bool _Const>
		class Iterator
		{
		public:
			using value_type = _Ty;
			using pointer = conditional_t<_Const, const value_type*, value_type*>;
			using reference = conditional_t<_Const, const value_type&, value_type&>;
			using iterator_category = bidirectional_iterator_tag;

			NodeBase* m_cur;

			Iterator(const Iterator<_Ty, false>& rhs) :
				m_cur(rhs.m_cur) {}
			Iterator(NodeBase* pnode) :
				m_cur(pnode) {}
			pointer operator->() const
			{
				return &(((Node<_Ty>*)m_cur)->m_elem);
			}
			reference operator*() const
			{
				return ((Node<_Ty>*)m_cur)->m_elem;
			}
			Iterator& operator++()
			{
				m_cur = m_cur->m_next;
				return *this;
			}
			Iterator operator++(int)
			{
				Iterator tmp(*this);
				operator++();
				return tmp;
			}
			Iterator& operator--()
			{
				m_cur = m_cur->m_prev;
				return *this;
			}
			Iterator operator--(int)
			{
				Iterator tmp(*this);
				operator--();
				return tmp;
			}
		};

		template <typename _Ty, bool _Const1, bool _Const2>
		bool operator==(const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
		{
			return lhs.m_cur == rhs.m_cur;
		}

		template <typename _Ty, bool _Const1, bool _Const2>
		bool operator!=(const Iterator<_Ty, _Const1>& lhs, const Iterator<_Ty, _Const2>& rhs)
		{
			return lhs.m_cur != rhs.m_cur;
		}
	}

	//! @addtogroup RuntimeContainer
    //! @{
	
	//! A container that stores elements as double-linked lists (nodes connected by pointers).
	template <typename _Ty, typename _Alloc = Allocator>
	class List
	{
	public:
		using value_type = _Ty;
		using allocator_type = _Alloc;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = ListImpl::Iterator<value_type, false>;
		using const_iterator = ListImpl::Iterator<value_type, true>;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;
		using node_type = ListImpl::Node<_Ty>;

		//! Constructs an empty list.
		List();
		//! Constructs an empty list with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the list.
		List(const allocator_type& alloc);
		//! Constructs a list with the specified number of elements.
		//! @param[in] count The number of elements to insert.
		//! @param[in] value The value used to construct elements. Every element will be copy-constructed from this value.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the list.
		List(usize count, const_reference value, const allocator_type& alloc = allocator_type());
		//! Constructs a list with the specified number of elements. Elements will be default-initialized.
		//! @param[in] count The number of elements to insert.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the list.
		explicit List(usize count, const allocator_type& alloc = allocator_type());
		//! Constructs a list with elements specified by one range. Elements in the range will be copy-inserted into the list. 
		//! @param[in] first The iterator to the first element of the range.
		//! @param[in] last The iterator to the one-past-last element of the range.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the list.
 		template <typename _InputIt>
		List(enable_if_t<!is_integral_v<_InputIt>, _InputIt> first, _InputIt last, const allocator_type& alloc = allocator_type());
		//! Constructs a list by copying elements from another list.
		//! @param[in] rhs The list to copy elements from.
		List(const List& rhs);
		//! Constructs a list with an custom allocator and with elements copied from another list.
		//! @param[in] rhs The list to copy elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the list.
		List(const List& rhs, const allocator_type& alloc);
		//! Constructs a list by moving elements from another list.
		//! @param[in] rhs The list to move elements from.
		List(List&& rhs);
		//! Constructs a list with an custom allocator and with elements moved from another list.
		//! @param[in] rhs The list to move elements from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the list.
		List(List&& rhs, const allocator_type& alloc);
		//! Constructs a list with elements specified by one initializer list.
		//! @param[in] ilist The initializer list.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the list.
		List(InitializerList<value_type> ilist, const allocator_type& alloc = allocator_type());
		~List();
		//! Replaces elements of the list by coping elements from another list.
		//! @param[in] rhs The list to copy elements from.
		//! @return Returns `*this`.
		List& operator=(const List& rhs);
		//! Replaces elements of the list by moving elements from another list.
		//! @param[in] rhs The list to move elements from.
		//! @return Returns `*this`.
		List& operator=(List&& rhs);
		//! Replaces elements of the list by elements from one initializer list.
		//! @param[in] ilist The initializer list.
		//! @return Returns `*this`.
		List& operator=(InitializerList<value_type> ilist);
		//! Replaces elements of the list by several copies of the specified value.
		//! @param[in] count The number of copies to insert to the list.
		//! @param[in] value The value to copy.
		void assign(usize count, const value_type& value);
		//! Replaces elements of the list by elements specified by one range. Elements in the range will be copy-inserted into the list.
		//! @param[in] first The iterator to the first element of the range.
		//! @param[in] last The iterator to the one-past-last element of the range.
		template <typename _InputIt>
		auto assign(_InputIt first, _InputIt last)->enable_if_t<!is_integral_v<_InputIt>, void>;
		//! Replaces elements of the list by elements from one initializer list.
		//! @param[in] ilist The initializer list.
		void assign(InitializerList<value_type> ilist);
		//! Gets the first element in the list.
		//! @return Returns one reference to the first element in the list.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		reference front();
		//! Gets the first element in the list.
		//! @return Returns one constant reference to the first element in the list.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		const_reference front() const;
		//! Gets the last element in the list.
		//! @return Returns one reference to the last element in the list.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		reference back();
		//! Gets the last element in the list.
		//! @return Returns one constant reference to the last element in the list.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		const_reference back() const;
		//! Gets one iterator to the first element of the list.
		//! @return Returns one iterator to the first element of the list.
		iterator begin();
		//! Gets one constant iterator to the first element of the list.
		//! @return Returns one constant iterator to the first element of the list.
		const_iterator begin() const;
		//! Gets one constant iterator to the first element of the list.
		//! @return Returns one constant iterator to the first element of the list.
		const_iterator cbegin() const;
		//! Gets one iterator to the one past last element of the list.
		//! @return Returns one iterator to the one past last element of the list.
		iterator end();
		//! Gets one constant iterator to the one past last element of the list.
		//! @return Returns one constant iterator to the one past last element of the list.
		const_iterator end() const;
		//! Gets one constant iterator to the one past last element of the list.
		//! @return Returns one constant iterator to the one past last element of the list.
		const_iterator cend() const;
		//! Gets one reverse iterator to the last element of the list.
		//! @return Returns one reverse iterator to the last element of the list.
		reverse_iterator rbegin();
		//! Gets one constant reverse iterator to the last element of the list.
		//! @return Returns one constant reverse iterator to the last element of the list.
		const_reverse_iterator rbegin() const;
		//! Gets one constant reverse iterator to the last element of the list.
		//! @return Returns one constant reverse iterator to the last element of the list.
		const_reverse_iterator crbegin() const;
		//! Gets one reverse iterator to the one-before-first element of the list.
		//! @return Returns one reverse iterator to the one-before-first element of the list.
		reverse_iterator rend();
		//! Gets one constant reverse iterator to the one-before-first element of the list.
		//! @return Returns one constant reverse iterator to the one-before-first element of the list.
		const_reverse_iterator rend() const;
		//! Gets one constant reverse iterator to the one-before-first element of the list.
		//! @return Returns one constant reverse iterator to the one-before-first element of the list.
		const_reverse_iterator crend() const;
		//! Checks whether this list is empty, that is, the size of this list is `0`.
		//! @return Returns `true` if this list is empty, returns `false` otherwise.
		bool empty() const;
		//! Gets the size of the list, that is, the number of elements in the list.
		//! @return Returns the size of the list.
		usize size() const;
		//! Removes all elements in the list.
		void clear();
		//! Inserts the specified element to the list.
		//! @param[in] pos The iterator to the position to insert the element. The element will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the list.
		//! @param[in] value The element to insert. The element will be copy-inserted into the list.
		//! @return Returns one iterator to the inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the list.
		iterator insert(const_iterator pos, const value_type& value);
		//! Inserts the specified element to the list.
		//! @param[in] pos The iterator to the position to insert the element. The element will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the list.
		//! @param[in] value The element to insert. The element will be move-inserted into the list.
		//! @return Returns one iterator to the inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the list.
		iterator insert(const_iterator pos, value_type&& value);
		//! Inserts several copies of the element to the list.
		//! @param[in] pos The iterator to the position to insert elements. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the list.
		//! @param[in] count The number of elements to insert.
		//! @param[in] value The value to initialize the new elements with.
		//! @return Returns one iterator to the first inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the list.
		iterator insert(const_iterator pos, usize count, const value_type& value);
		//! Inserts one range of elements to the list.
		//! @param[in] pos The iterator to the position to insert elements. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the list.
		//! @param[in] first The iterator to the first element to be inserted.
		//! @param[in] last The iterator to the one-past-last element to be inserted.
		//! @return Returns one iterator to the first inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the list.
		template<typename _InputIt>
		auto insert(const_iterator pos, _InputIt first, _InputIt last)->enable_if_t<!is_integral_v<_InputIt>, List<_Ty, _Alloc>::iterator>;
		//! Inserts one range of elements specified by the initializer list to the list.
		//! @param[in] pos The iterator to the position to insert elements. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the list.
		//! @param[in] ilist The initializer list.
		//! @return Returns one iterator to the first inserted element.
		//! @par Valid Usage
		//! * If `pos != end()`, `pos` must points to a valid element in the list.
		iterator insert(const_iterator pos, InitializerList<value_type> ilist);
		//! Constructs one element directly on the specified position of the list using the provided arguments.
		//! @param[in] pos The iterator to the position to construct the element. The elements will be inserted before the element 
		//! pointed by this iterator. This can be `end()`, indicating that the element will be inserted at the end of the list.
		//! @param[in] args The arguments to construct the element. `_Ty(args...)` will be used to 
		//! construct the element.
		//! @return Returns one iterator to the constructed element.
		template<typename... _Args>
		iterator emplace(const_iterator pos, _Args&&... args);
		//! Removes one element from the list.
		//! @param[in] pos The iterator to the element to be removed.
		//! @return Returns one iterator to the next element after the removed element, 
		//! or `end()` if such element does not exist.
		//! @par Valid Usage
		//! * `pos` must points to a valid element in the list.
		iterator erase(const_iterator pos);
		//! Removes one range of elements from the list.
		//! @param[in] first The iterator to the first element to be removed.
		//! @param[in] last The iterator to the one-past-last element to be removed.
		//! @return Returns one iterator to the next element after the removed elements, 
		//! or `end()` if such element does not exist.
		//! @par Valid Usage
		//! * `first` must be either `end()` or one valid element in the list.
		//! * If `first != end()`, [`first`, `last`) must specifies either one empty range (`first == last`) or one valid element range of the list.
		//! * If `first == end()`, [`first`, `last`) must specifies one empty range (`first == last`).
		iterator erase(const_iterator first, const_iterator last);
		//! Inserts one element at the back of the list.
		//! @param[in] value The element to insert. The element will be copy-inserted to the list.
		void push_back(const value_type& value);
		//! Inserts one element at the back of the list.
		//! @param[in] value The element to insert. The element will be move-inserted to the list.
		void push_back(value_type&& value);
		//! Constructs one element directly on the back of the list using the provided arguments.
		//! @param[in] args The arguments to construct the element. `_Ty(args...)` will be used to 
		//! construct the element.
		//! @return Returns one reference to the constructed element.
		template<typename... _Args>
		reference emplace_back(_Args&&... args);
		//! Removes the last element of the list.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		void pop_back();
		//! Inserts one element at the front of the list.
		//! @param[in] value The element to insert. The element will be copy-inserted to the list.
		void push_front(const value_type& value);
		//! Inserts one element at the front of the list.
		//! @param[in] value The element to insert. The element will be move-inserted to the list.
		void push_front(value_type&& value);
		//! Constructs one element directly on the front of the list using the provided arguments.
		//! @param[in] args The arguments to construct the element. `_Ty(args...)` will be used to 
		//! construct the element.
		//! @return Returns one reference to the constructed element.
		template<typename... _Args>
		reference emplace_front(_Args&&... args);
		//! Removes the first element of the list.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		void pop_front();
		//! Changes the number of elements in the list.
		//! @param[in] count The new size of the list.
		//! If this is larger than `size()`, new elements will be default-inserted at the back of the list.
		//! If this is smaller than `size()`, elements in range [`count`, `size()`) will be removed from the list.
		//! If this is equal to `size()`, this function does nothing.
		void resize(usize count);
		//! Changes the number of elements in the list.
		//! @param[in] count The new size of the list.
		//! If this is larger than `size()`, new elements will be copy-inserted at the back of the list using the provided value.
		//! If this is smaller than `size()`, elements in range [`count`, `size()`) will be removed from the list.
		//! If this is equal to `size()`, this function does nothing.
		//! @param[in] value The value to initialize the new elements with.
		void resize(usize count, const value_type& value);
		//! Swaps elements of this list with the specified list.
		//! @param[in] rhs The list to swap elements with.
		void swap(List& rhs);
		//! Merges another list into this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list. Elements are compared using `less<_Ty>`.
		//! @param[in] other The list to merge. This list will be empty after this operation.
		//! If this is equal to `*this`, this function does nothing.
		//! @par Valid Usage
		//! * Elements in `*this` and `other` must be sorted in ascending order.
		void merge(List& other);
		//! Merges another list into this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list. Elements are compared using `less<_Ty>`.
		//! @param[in] other The list to merge. This list will be empty after this operation.
		//! If this is equal to `*this`, this function does nothing.
		//! @par Valid Usage
		//! * Elements in `*this` and `other` must be sorted in ascending order.
		void merge(List&& other);
		//! Merges another list into this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list. Elements are compared using the user-specified comparison function object.
		//! @param[in] other The list to merge. This list will be empty after this operation.
		//! If this is equal to `*this`, this function does nothing.
		//! @param[in] comp The comparison function object to use.
		//! @par Valid Usage
		//! * `comp` must provide the following function: `bool operator()(const _Ty& a, const _Ty& b)`, that returns `true`
		//! if `a` should appear in the list before `b`.
		//! * Elements in `*this` and `other` must be sorted in an order defined by the user-specified comparison function, that
		//! is, for any two elements `a` and `b` in the same list, `comp(b, a)` must return `false` if `a` appears before `b`.
		template <typename _Compare>
		void merge(List& other, _Compare comp);
		//! Merges another list into this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list. Elements are compared using the user-specified comparison function object.
		//! @param[in] other The list to merge. This list will be empty after this operation.
		//! If this is equal to `*this`, this function does nothing.
		//! @param[in] comp The comparison function object to use.
		//! @par Valid Usage
		//! * `comp` must provide the following function: `bool operator()(const _Ty& a, const _Ty& b)`, that returns `true`
		//! if `a` should appear in the list before `b`.
		//! * Elements in `*this` and `other` must be sorted in an order defined by the user-specified comparison function, that
		//! is, for any two elements `a` and `b` in the same list, `comp(b, a)` must return `false` if `a` appears before `b`.
		template <typename _Compare>
		void merge(List&& other, _Compare comp);
		//! Transfers all elements from another list to this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list.
		//! @param[in] pos The iterator to the position to insert the transferred elements.
		//! @param[in] other The list to transfer elements from. This list will be empty after this operation.
		void splice(const_iterator pos, List& other);
		//! Transfers all elements from another list to this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list.
		//! @param[in] pos The iterator to the position to insert the transferred elements.
		//! @param[in] other The list to transfer elements from. This list will be empty after this operation.
		void splice(const_iterator pos, List&& other);
		//! Transfers one element from another list to this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list.
		//! @param[in] pos The iterator to the position to insert the transferred element.
		//! @param[in] other The list to transfer the element from.
		//! @param[in] it The iterator to the element to be transferred.
		void splice(const_iterator pos, List& other, const_iterator it);
		//! Transfers one element from another list to this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list.
		//! @param[in] pos The iterator to the position to insert the transferred element.
		//! @param[in] other The list to transfer the element from.
		//! @param[in] it The iterator to the element to be transferred.
		void splice(const_iterator pos, List&& other, const_iterator it);
		//! Transfers elements from another list to this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list.
		//! @param[in] pos The iterator to the position to insert the transferred elements.
		//! @param[in] other The list to transfer elements from.
		//! @param[in] first The iterator to the first element to be transferred.
		//! @param[in] last The iterator to the one-past-last element to be transferred.
		void splice(const_iterator pos, List& other, const_iterator first, const_iterator last);
		//! Transfers elements from another list to this list.
		//! @details No memory allocation or element copy/move will be performed, this function transfers elements by changing their
		//! pointers directly so that they link to the new list.
		//! @param[in] pos The iterator to the position to insert the transferred elements.
		//! @param[in] other The list to transfer elements from.
		//! @param[in] first The iterator to the first element to be transferred.
		//! @param[in] last The iterator to the one-past-last element to be transferred.
		void splice(const_iterator pos, List&& other, const_iterator first, const_iterator last);
		//! Removes all elements that are equal to value.
		//! @details Elements are compared using `equal_to<_Ty>`.
		//! @param[in] value The value to test.
		//! @return Returns the number of elements removed.
		usize remove(const value_type& value);
		//! Removes all elements for which the specified predicate returns `true`.
		//! @param[in] p The unary predicate which returns `​true` if the element should be removed.
		//! @return Returns the number of elements removed.
		template<typename _UnaryPredicate>
		usize remove_if(_UnaryPredicate p);
		//! Reverses the order of the elements in the list.
		void reverse();
		//! Removes all consecutive duplicate elements from the container.
		//! @details Only the first element in each group of equal elements is left. Elements are compared using `equal_to<_Ty>`.
		//! @return Returns the number of elements removed.
		usize unique();
		//! Removes all consecutive duplicate elements from the container.
		//! @details Only the first element in each group of equal elements is left. Elements are compared using the user-provided binary predicate.
		//! @param[in] p The binary predicate which returns `​true` if two elements are equal.
		//! @return Returns the number of elements removed.
		//! @par Valid Usage
		//! * `p` must provide the following function: `bool operator()(const _Ty& a, const _Ty& b)`, that returns `true`
		//! if `a` and `b` is equal.
		template<typename _BinaryPredicate>
		usize unique(_BinaryPredicate p);
		//! Sorts elements in ascending order.
		void sort();
		//! Sorts elements in using the user-specified comparison function object.
		//! @param[in] comp The comparison function object to use.
		//! @par Valid Usage
		//! * `comp` must provide the following function: `bool operator()(const _Ty& a, const _Ty& b)`, that returns `true`
		//! if `a` should appear in the list before `b`.
		template<typename _Compare>
		void sort(_Compare comp);
		//! Gets the allocator of the list.
		//! @return Returns one copy of the allocator of the list.
		allocator_type get_allocator() const;

	private:
		// -------------------- Begin of ABI compatible part --------------------
		// The `m_prior` pointer of this node points to the last element in the list.
		// The `m_next` pointer of this node points to the first element in the list.
		// This node is used rather than writing `m_first` and `m_last` directly 
		// so that it can be used for `end()`.
		OptionalPair<allocator_type, ListImpl::NodeBase> m_allocator_and_nodebase;		
		usize m_size;				// Number of nodes in the list.
		// --------------------  End of ABI compatible part  --------------------

		void internal_cleanup();
		void internal_assign_nocleanup(usize count, const_reference value);

		template <typename _InputIt>
		void internal_assign_iterator_nocleanup(_InputIt first, _InputIt last);

		void internal_assign_nocleanup(usize count);
		void internal_assign_nocleanup(const List& rhs);
		void internal_assign_nocleanup(List&& rhs);
		void internal_element_wise_assign_nocleanup(List&& rhs);

		template<typename _BinaryPredicate>
		iterator internal_sort(iterator first, iterator last, usize sz, _BinaryPredicate comp);

		template <typename _ElemTy, typename... _Args>
        _ElemTy* new_object(_Args&&... args)
		{
            _ElemTy* o = m_allocator_and_nodebase.first().template allocate<_ElemTy>(1);
			new (o) _ElemTy(forward<_Args>(args)...);
			return o;
		}

		template <typename _ElemTy>
		void delete_object(_ElemTy * o)
		{
			o->~_ElemTy();
            m_allocator_and_nodebase.first().template deallocate<_ElemTy>(o, 1);
		}
	};

	//! @}
}

#include "Impl/List.inl"