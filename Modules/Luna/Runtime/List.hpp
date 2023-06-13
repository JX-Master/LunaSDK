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

		List();
		List(const allocator_type& alloc);
		List(usize count, const_reference value, const allocator_type& alloc = allocator_type());
		explicit List(usize count, const allocator_type& alloc = allocator_type());
		template <typename _InputIt>
		List(enable_if_t<!is_integral_v<_InputIt>, _InputIt> first, _InputIt last, const allocator_type& alloc = allocator_type());
		List(const List& rhs);
		List(const List& rhs, const allocator_type& alloc);
		List(List&& rhs);
		List(List&& rhs, const allocator_type& alloc);
		List(InitializerList<value_type> ilist, const allocator_type& alloc = allocator_type());
		~List();
		List& operator=(const List& rhs);
		List& operator=(List&& rhs);
		List& operator=(InitializerList<value_type> ilist);
		void assign(usize count, const value_type& value);
		template <typename _InputIt>
		auto assign(_InputIt first, _InputIt last)->enable_if_t<!is_integral_v<_InputIt>, void>;
		void assign(InitializerList<value_type> ilist);
		reference front();
		const_reference front() const;
		reference back();
		const_reference back() const;
		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;
		reverse_iterator rbegin();
		const_reverse_iterator rbegin() const;
		const_reverse_iterator crbegin() const;
		reverse_iterator rend();
		const_reverse_iterator rend() const;
		const_reverse_iterator crend() const;
		bool empty() const;
		usize size() const;
		void clear();
		iterator insert(const_iterator pos, const value_type& value);
		iterator insert(const_iterator pos, value_type&& value);
		iterator insert(const_iterator pos, usize count, const value_type& value);
		template<typename _InputIt>
		auto insert(const_iterator pos, _InputIt first, _InputIt last)->enable_if_t<!is_integral_v<_InputIt>, List<_Ty, _Alloc>::iterator>;
		iterator insert(const_iterator pos, InitializerList<value_type> ilist);
		template<typename... _Args>
		iterator emplace(const_iterator pos, _Args&&... args);
		iterator erase(const_iterator pos);
		iterator erase(const_iterator first, const_iterator last);
		void push_back(const value_type& value);
		void push_back(value_type&& value);
		template<typename... _Args>
		reference emplace_back(_Args&&... args);
		void pop_back();
		void push_front(const value_type& value);
		void push_front(value_type&& value);
		template<typename... _Args>
		reference emplace_front(_Args&&... args);
		void pop_front();
		void resize(usize count);
		void resize(usize count, const value_type& value);
		void swap(List& other);
		void merge(List& other);
		void merge(List&& other);
		template <typename _Compare>
		void merge(List& other, _Compare comp);
		template <typename _Compare>
		void merge(List&& other, _Compare comp);
		void splice(const_iterator pos, List& other);
		void splice(const_iterator pos, List&& other);
		void splice(const_iterator pos, List& other, const_iterator it);
		void splice(const_iterator pos, List&& other, const_iterator it);
		void splice(const_iterator pos, List& other, const_iterator first, const_iterator last);
		void splice(const_iterator pos, List&& other, const_iterator first, const_iterator last);
		usize remove(const value_type& value);
		template<typename _UnaryPredicate>
		usize remove_if(_UnaryPredicate p);
		void reverse();
		usize unique();
		template<typename _BinaryPredicate>
		usize unique(_BinaryPredicate p);
		void sort();
		template<typename _Compare>
		void sort(_Compare comp);
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

	// ABI assert.
	static_assert(sizeof(List<usize, Allocator>) == sizeof(usize) * 3, "List size does not match.");

	template <typename _Ty, typename _Alloc>
	bool operator==(const List<_Ty, _Alloc>& lhs, const List<_Ty, _Alloc>& rhs);
	template <typename _Ty, typename _Alloc>
	bool operator!=(const List<_Ty, _Alloc>& lhs, const List<_Ty, _Alloc>& rhs);
	template <typename _Ty, typename _Alloc>
	void swap(List<_Ty, _Alloc>& lhs, List<_Ty, _Alloc>& rhs);

	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List() :
		m_allocator_and_nodebase(allocator_type(), ListImpl::NodeBase()),
		m_size(0)
	{
		m_allocator_and_nodebase.second().m_next = &m_allocator_and_nodebase.second();
		m_allocator_and_nodebase.second().m_prev = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(const allocator_type& alloc) :
		m_allocator_and_nodebase(alloc, ListImpl::NodeBase()),
		m_size(0) 
	{
		m_allocator_and_nodebase.second().m_next = &m_allocator_and_nodebase.second();
		m_allocator_and_nodebase.second().m_prev = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(usize count, const_reference value, const allocator_type& alloc) :
		m_allocator_and_nodebase(alloc, ListImpl::NodeBase())
	{
		internal_assign_nocleanup(count, value);
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(usize count, const allocator_type& alloc) :
		m_allocator_and_nodebase(alloc, ListImpl::NodeBase())
	{
		internal_assign_nocleanup(count);
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIt>
	inline List<_Ty, _Alloc>::List(enable_if_t<!is_integral_v<_InputIt>, _InputIt> first, _InputIt last, const allocator_type& alloc) :
		m_allocator_and_nodebase(alloc, ListImpl::NodeBase())
	{
		internal_assign_iterator_nocleanup<_InputIt>(first, last);
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(const List& rhs) :
		m_allocator_and_nodebase(rhs.m_allocator_and_nodebase.first(), ListImpl::NodeBase())
	{
		internal_assign_nocleanup(rhs);
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(const List& rhs, const allocator_type& alloc) :
		m_allocator_and_nodebase(alloc, ListImpl::NodeBase())
	{
		internal_assign_nocleanup(rhs);
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(List&& rhs) :
		m_allocator_and_nodebase(move(rhs.m_allocator_and_nodebase.first()), ListImpl::NodeBase())
	{
		internal_assign_nocleanup(move(rhs));
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(List&& rhs, const allocator_type& alloc) :
		m_allocator_and_nodebase(alloc, ListImpl::NodeBase())
	{
		if (m_allocator_and_nodebase.first() == rhs.m_allocator_and_nodebase.first())
		{
			internal_assign_nocleanup(move(rhs));
		}
		else
		{
			internal_element_wise_assign_nocleanup(move(rhs));
		}
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::List(InitializerList<value_type> ilist, const allocator_type& alloc) :
		m_allocator_and_nodebase(alloc, ListImpl::NodeBase())
	{
		auto iter = ilist.begin();
		ListImpl::NodeBase* last_node = &m_allocator_and_nodebase.second();
		m_size = ilist.size();
		while (iter != ilist.end())
		{
			ListImpl::NodeBase* node = new_object<node_type>(move(*iter));
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>::~List()
	{
		internal_cleanup();
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>& List<_Ty, _Alloc>::operator=(const List& rhs)
	{
		internal_cleanup();
		internal_assign_nocleanup(rhs);
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>& List<_Ty, _Alloc>::operator=(List&& rhs)
	{
		internal_cleanup();
		internal_assign_nocleanup(move(rhs));
	}
	template <typename _Ty, typename _Alloc>
	inline List<_Ty, _Alloc>& List<_Ty, _Alloc>::operator=(InitializerList<value_type> ilist)
	{
		internal_cleanup();
		iterator iter = ilist.begin();
		node_type* last_node = &m_allocator_and_nodebase.second();
		m_size = ilist.size();
		while (iter != ilist.end())
		{
			node_type* node = new_object<node_type>(move(*iter));
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::assign(usize count, const value_type& value)
	{
		internal_cleanup();
		internal_assign_nocleanup(count, value);
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIt>
	inline auto List<_Ty, _Alloc>::assign(_InputIt first, _InputIt last) -> enable_if_t<!is_integral_v<_InputIt>, void>
	{
		internal_cleanup();
		internal_assign_iterator_nocleanup<_InputIt>(first, last);
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::assign(InitializerList<value_type> ilist)
	{
		internal_cleanup();
		iterator iter = ilist.begin();
		node_type* last_node = &m_allocator_and_nodebase.second();
		m_size = ilist.size();
		while (iter != ilist.end())
		{
			node_type* node = new_object<node_type>(move(*iter));
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::reference List<_Ty, _Alloc>::front()
	{
		lucheck(m_size);
		return ((node_type*)m_allocator_and_nodebase.second().m_next)->m_elem;
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_reference List<_Ty, _Alloc>::front() const
	{
		lucheck(m_size);
		return ((node_type*)m_allocator_and_nodebase.second().m_next)->m_elem;
	}
	template <typename _Ty, typename _Alloc>
	inline 	typename List<_Ty, _Alloc>::reference List<_Ty, _Alloc>::back()
	{
		lucheck(m_size);
		return ((node_type*)m_allocator_and_nodebase.second().m_prev)->m_elem;
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_reference List<_Ty, _Alloc>::back() const
	{
		lucheck(m_size);
		return ((node_type*)m_allocator_and_nodebase.second().m_prev)->m_elem;
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::begin()
	{
		return iterator(m_allocator_and_nodebase.second().m_next);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_iterator List<_Ty, _Alloc>::begin() const
	{
		return const_iterator(m_allocator_and_nodebase.second().m_next);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_iterator List<_Ty, _Alloc>::cbegin() const
	{
		return const_iterator(m_allocator_and_nodebase.second().m_next);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::end()
	{
		return iterator(&m_allocator_and_nodebase.second());
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_iterator List<_Ty, _Alloc>::end() const
	{
		return const_iterator(const_cast<ListImpl::NodeBase*>(&m_allocator_and_nodebase.second()));
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_iterator List<_Ty, _Alloc>::cend() const
	{
		return const_iterator(const_cast<ListImpl::NodeBase*>(&m_allocator_and_nodebase.second()));
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::reverse_iterator List<_Ty, _Alloc>::rbegin()
	{
		return reverse_iterator(end());
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_reverse_iterator List<_Ty, _Alloc>::rbegin() const
	{
		return const_reverse_iterator(end());
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_reverse_iterator List<_Ty, _Alloc>::crbegin() const
	{
		return const_reverse_iterator(cend());
	}
	template <typename _Ty, typename _Alloc>
	inline 	typename List<_Ty, _Alloc>::reverse_iterator List<_Ty, _Alloc>::rend()
	{
		return reverse_iterator(begin());
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_reverse_iterator List<_Ty, _Alloc>::rend() const
	{
		return const_reverse_iterator(begin());
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::const_reverse_iterator List<_Ty, _Alloc>::crend() const
	{
		return const_reverse_iterator(begin());
	}
	template <typename _Ty, typename _Alloc>
	inline bool List<_Ty, _Alloc>::empty() const
	{
		return m_size == 0;
	}
	template <typename _Ty, typename _Alloc>
	inline usize List<_Ty, _Alloc>::size() const
	{
		return m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::clear()
	{
		internal_cleanup();
		m_allocator_and_nodebase.second().m_next = &m_allocator_and_nodebase.second();
		m_allocator_and_nodebase.second().m_prev = &m_allocator_and_nodebase.second();
		m_size = 0;
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::insert(const_iterator pos, const value_type& value)
	{
		node_type* node = new_object<node_type>(value);
		node->insert_before(pos.m_cur);
		++m_size;
		return iterator(node);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::insert(const_iterator pos, value_type&& value)
	{
		node_type* node = new_object<node_type>(move(value));
		node->insert_before(pos.m_cur);
		++m_size;
		return iterator(node);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::insert(const_iterator pos, usize count, const value_type& value)
	{
		if (!count) return iterator(pos.m_cur);
		ListImpl::NodeBase* prev_node = pos.m_cur->m_prev;
		ListImpl::NodeBase* next_node = prev_node->m_next;
		ListImpl::NodeBase* last_node = prev_node;
		m_size += count;
		while (count)
		{
			node_type* node = new_object<node_type>(value);
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			--count;
		}
		next_node->m_prev = last_node;
		last_node->m_next = next_node;
		return iterator(prev_node->m_next);
	}
	template <typename _Ty, typename _Alloc>
	template<typename _InputIt>
	inline auto List<_Ty, _Alloc>::insert(const_iterator pos, _InputIt first, _InputIt last) -> enable_if_t<!is_integral_v<_InputIt>, List<_Ty, _Alloc>::iterator>
	{
		ListImpl::NodeBase* prev_node = pos.m_cur->m_prev;
		ListImpl::NodeBase* next_node = prev_node->m_next;
		ListImpl::NodeBase* last_node = prev_node;
		auto iter = first;
		while (iter != last)
		{
			ListImpl::NodeBase* node = new_object<node_type>(*iter);
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
			++m_size;
		}
		next_node->m_prev = last_node;
		last_node->m_next = next_node;
		return first == last ? iterator(prev_node) : iterator(prev_node->m_next);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::insert(const_iterator pos, InitializerList<value_type> ilist)
	{
		ListImpl::NodeBase* prev_node = pos.m_cur->m_prev;
		ListImpl::NodeBase* next_node = prev_node->m_next;
		ListImpl::NodeBase* last_node = prev_node;
		auto iter = ilist.begin();
		while (iter != ilist.end())
		{
			node_type* node = new_object<node_type>(move(*iter));
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
		}
		next_node->m_prev = last_node;
		last_node->m_next = next_node;
		m_size += ilist.size();
		return ilist.size() ? iterator(prev_node->m_next) : iterator(prev_node);
	}
	template <typename _Ty, typename _Alloc>
	template<typename... _Args>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::emplace(const_iterator pos, _Args&&... args)
	{
		node_type* node = new_object<node_type>(forward<_Args>(args)...);
		node->insert_before(pos.m_cur);
		++m_size;
		return iterator(node);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::erase(const_iterator pos)
	{
		lucheck(pos != end());
		ListImpl::NodeBase* cur = pos.m_cur;
		ListImpl::NodeBase* next_node = cur->m_next;
		cur->remove_this();
		delete_object((node_type*)cur);
		--m_size;
		return iterator(next_node);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::erase(const_iterator first, const_iterator last)
	{
		ListImpl::NodeBase* first_remove = first.m_cur;
		ListImpl::NodeBase* last_remove = last.m_cur->m_prev;
		ListImpl::NodeBase::remove_range(first_remove, last_remove);

		ListImpl::NodeBase* node = first_remove;
		while (node != last_remove)
		{
			ListImpl::NodeBase* next_remove = node->m_next;
			delete_object((node_type*)node);
			node = next_remove;
			--m_size;
		}
		delete_object((node_type*)node);
		--m_size;
		return iterator(last.m_cur);
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::push_back(const value_type& value)
	{
		node_type* node = new_object<node_type>(value);
		node->insert_before(&m_allocator_and_nodebase.second());
		++m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::push_back(value_type&& value)
	{
		node_type* node = new_object<node_type>(move(value));
		node->insert_before(&m_allocator_and_nodebase.second());
		++m_size;
	}
	template <typename _Ty, typename _Alloc>
	template<typename... _Args>
	inline typename List<_Ty, _Alloc>::reference List<_Ty, _Alloc>::emplace_back(_Args&&... args)
	{
		node_type* node = new_object<node_type>(forward<_Args>(args)...);
		node->insert_before(&m_allocator_and_nodebase.second());
		++m_size;
		return node->m_elem;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::pop_back()
	{
		lucheck(m_size);
		ListImpl::NodeBase* node = m_allocator_and_nodebase.second().m_prev;
		node->remove_this();
		delete_object((node_type*)node);
		--m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::push_front(const value_type& value)
	{
		node_type* node = new_object<node_type>(value);
		node->insert_before(m_allocator_and_nodebase.second().m_next);
		++m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::push_front(value_type&& value)
	{
		node_type* node = new_object<node_type>(move(value));
		node->insert_before(m_allocator_and_nodebase.second().m_next);
		++m_size;
	}
	template <typename _Ty, typename _Alloc>
	template<typename... _Args>
	inline typename List<_Ty, _Alloc>::reference List<_Ty, _Alloc>::emplace_front(_Args&&... args)
	{
		node_type* node = new_object<node_type>(forward<_Args>(args)...);
		node->insert_before(m_allocator_and_nodebase.second().m_next);
		++m_size;
		return node->m_elem;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::pop_front()
	{
		lucheck(m_size);
		ListImpl::NodeBase* node = m_allocator_and_nodebase.second().m_next;
		node->remove_this();
		delete_object((node_type*)node);
		--m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::resize(usize count)
	{
		if (count > m_size)
		{
			usize add_count = count - m_size;
			ListImpl::NodeBase* last_node = m_allocator_and_nodebase.second().m_prev;
			while (add_count)
			{
				node_type* node = new_object<node_type>();
				node->m_prev = last_node;
				last_node->m_next = node;
				last_node = node;
				--add_count;
			}
			m_allocator_and_nodebase.second().m_prev = last_node;
			last_node->m_next = &m_allocator_and_nodebase.second();
		}
		else if (count < m_size)
		{
			usize erase_count = m_size - count;
			// erase from trail to head.
			ListImpl::NodeBase* node = m_allocator_and_nodebase.second().m_prev;
			while (erase_count)
			{
				ListImpl::NodeBase* next_erase_node = node->m_prev;
				delete_object((node_type*)node);
				node = next_erase_node;
				--erase_count;
			}
			m_allocator_and_nodebase.second().m_prev = node;
			node->m_next = &m_allocator_and_nodebase.second();
		}
		m_size = count;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::resize(usize count, const value_type& value)
	{
		if (count > m_size)
		{
			usize add_count = count - m_size;
			ListImpl::NodeBase* last_node = m_allocator_and_nodebase.second().m_prev;
			while (add_count)
			{
				node_type* node = new_object<node_type>(value);
				node->m_prev = last_node;
				last_node->m_next = node;
				last_node = node;
				--add_count;
			}
			m_allocator_and_nodebase.second().m_prev = last_node;
			last_node->m_next = &m_allocator_and_nodebase.second();
		}
		else if (count < m_size)
		{
			usize erase_count = m_size - count;
			// erase from trail to head.
			ListImpl::NodeBase* node = m_allocator_and_nodebase.second().m_prev;
			while (erase_count)
			{
				ListImpl::NodeBase* next_erase_node = node->m_prev;
				delete_object((node_type*)node);
				node = next_erase_node;
				--erase_count;
			}
			m_allocator_and_nodebase.second().m_prev = node;
			node->m_next = &m_allocator_and_nodebase.second();
		}
		m_size = count;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::swap(List& other)
	{
		ListImpl::NodeBase tmp = m_allocator_and_nodebase.second();
		m_allocator_and_nodebase.second() = other.m_allocator_and_nodebase.second();
		other.m_allocator_and_nodebase.second() = tmp;
		if (m_allocator_and_nodebase.second().m_next == &other.m_allocator_and_nodebase.second())
		{
			m_allocator_and_nodebase.second().m_next = m_allocator_and_nodebase.second().m_prev = &m_allocator_and_nodebase.second();
		}
		else
		{
			m_allocator_and_nodebase.second().m_next->m_prev = m_allocator_and_nodebase.second().m_prev->m_next = &m_allocator_and_nodebase.second();
		}
		if (other.m_allocator_and_nodebase.second().m_next == &m_allocator_and_nodebase.second())
		{
			other.m_allocator_and_nodebase.second().m_next = other.m_allocator_and_nodebase.second().m_prev = &other.m_allocator_and_nodebase.second();
		}
		else
		{
			other.m_allocator_and_nodebase.second().m_next->m_prev = other.m_allocator_and_nodebase.second().m_prev->m_next = &other.m_allocator_and_nodebase.second();
		}
		usize sz = m_size;
		m_size = other.m_size;
		other.m_size = sz;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::merge(List& other)
	{
		merge(move(other), less<_Ty>());
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::merge(List&& other)
	{
		merge(move(other), less<_Ty>());
	}
	template <typename _Ty, typename _Alloc>
	template <typename _Compare>
	inline void List<_Ty, _Alloc>::merge(List& other, _Compare comp)
	{
		merge(move(other), comp);
	}
	template <typename _Ty, typename _Alloc>
	template <typename _Compare>
	inline void List<_Ty, _Alloc>::merge(List&& other, _Compare comp)
	{
		if (this == &other) return;
		ListImpl::NodeBase* cur_l1 = m_allocator_and_nodebase.second().m_next;
		ListImpl::NodeBase* cur_l2 = other.m_allocator_and_nodebase.second().m_next;
		ListImpl::NodeBase* last_node = &m_allocator_and_nodebase.second();
		while ((cur_l1 != &m_allocator_and_nodebase.second()) && (cur_l2 != &other.m_allocator_and_nodebase.second()))
		{
			if (comp(((node_type*)cur_l2)->m_elem, ((node_type*)cur_l1)->m_elem))
			{
				// Use cur_l2.
				last_node->m_next = cur_l2;
				cur_l2->m_prev = last_node;
				last_node = cur_l2;
				cur_l2 = cur_l2->m_next;
			}
			else
			{
				// Use cur_l1.
				last_node->m_next = cur_l1;
				cur_l1->m_prev = last_node;
				last_node = cur_l1;
				cur_l1 = cur_l1->m_next;
			}
		}
		// Advance cur_l1.
		while (cur_l1 != &m_allocator_and_nodebase.second())
		{
			last_node->m_next = cur_l1;
			cur_l1->m_prev = last_node;
			last_node = cur_l1;
			cur_l1 = cur_l1->m_next;
		}
		// Advance cur_l2.
		while (cur_l2 != &other.m_allocator_and_nodebase.second())
		{
			last_node->m_next = cur_l2;
			cur_l2->m_prev = last_node;
			last_node = cur_l2;
			cur_l2 = cur_l2->m_next;
		}
		last_node->m_next = &m_allocator_and_nodebase.second();
		m_allocator_and_nodebase.second().m_prev = last_node;
		other.m_allocator_and_nodebase.second().m_next = &other.m_allocator_and_nodebase.second();
		other.m_allocator_and_nodebase.second().m_prev = &other.m_allocator_and_nodebase.second();
		m_size += other.m_size;
		other.m_size = 0;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::splice(const_iterator pos, List& other)
	{
		splice(pos, move(other), other.begin(), other.end());
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::splice(const_iterator pos, List&& other)
	{
		splice(pos, move(other), other.begin(), other.end());
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::splice(const_iterator pos, List& other, const_iterator it)
	{
		splice(pos, move(other), it);
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::splice(const_iterator pos, List&& other, const_iterator it)
	{
		ListImpl::NodeBase* node = it.m_cur;
		node->remove_this();
		node->insert_before(pos.m_cur);
		--other.m_size;
		++m_size;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::splice(const_iterator pos, List& other,
		const_iterator first, const_iterator last)
	{
		splice(pos, move(other), first, last);
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::splice(const_iterator pos, List&& other,
		const_iterator first, const_iterator last)
	{
		usize count = distance(first, last);
		if (!count) return;
		ListImpl::NodeBase* first_splice = first.m_cur;
		ListImpl::NodeBase* last_splice = last.m_cur->m_prev;
		ListImpl::NodeBase::remove_range(first_splice, last_splice);
		ListImpl::NodeBase::insert_range(pos.m_cur, first_splice, last_splice);
		other.m_size -= count;
		m_size += count;
	}
	template <typename _Ty, typename _Alloc>
	inline usize List<_Ty, _Alloc>::remove(const value_type& value)
	{
		usize removed = 0;
		auto iter = begin();
		while (iter != end())
		{
			if (*iter == value)
			{
				iter = erase(iter);
				++removed;
			}
			else
			{
				++iter;
			}
		}
		return removed;
	}
	template <typename _Ty, typename _Alloc>
	template<typename _UnaryPredicate>
	inline usize List<_Ty, _Alloc>::remove_if(_UnaryPredicate p)
	{
		usize removed = 0;
		auto iter = begin();
		while (iter != end())
		{
			if (p(*iter))
			{
				iter = erase(iter);
				++removed;
			}
			else
			{
				++iter;
			}
		}
		return removed;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::reverse()
	{
		ListImpl::NodeBase* node = &m_allocator_and_nodebase.second();
		do
		{
			ListImpl::NodeBase* next_node = node->m_next;
			node->m_next = node->m_prev;
			node->m_prev = next_node;
			node = next_node;
		} while (node != &m_allocator_and_nodebase.second());
	}
	template <typename _Ty, typename _Alloc>
	inline usize List<_Ty, _Alloc>::unique()
	{
		return unique(equal_to<_Ty>());
	}
	template <typename _Ty, typename _Alloc>
	template<typename _BinaryPredicate>
	inline usize List<_Ty, _Alloc>::unique(_BinaryPredicate p)
	{
		auto iter = begin();
		usize removed = 0;
		while (iter != end())
		{
			auto comp_iter = iter;
			++comp_iter;
			while (comp_iter != end() && p(*iter, *comp_iter))
			{
				comp_iter = erase(comp_iter);
				++removed;
			}
			iter = comp_iter;
		}
		return removed;
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::sort()
	{
		internal_sort(begin(), end(), size(), less<_Ty>());
	}
	template <typename _Ty, typename _Alloc>
	template<typename _Compare>
	inline void List<_Ty, _Alloc>::sort(_Compare comp)
	{
		internal_sort(begin(), end(), size(), comp);
	}
	template <typename _Ty, typename _Alloc>
	inline typename List<_Ty, _Alloc>::allocator_type List<_Ty, _Alloc>::get_allocator() const
	{
		return m_allocator_and_nodebase.first();
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::internal_cleanup()
	{
		auto node = m_allocator_and_nodebase.second().m_next;
		while (node != &m_allocator_and_nodebase.second())
		{
			auto next_node = node->m_next;
			delete_object((node_type*)node);
			node = next_node;
		}
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::internal_assign_nocleanup(usize count, const_reference value)
	{
		ListImpl::NodeBase* last_node = &m_allocator_and_nodebase.second();
		m_size = count;
		while (count)
		{
			ListImpl::NodeBase* node = new_object<node_type>(value);
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			--count;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	template <typename _InputIt>
	inline void List<_Ty, _Alloc>::internal_assign_iterator_nocleanup(_InputIt first, _InputIt last)
	{
		_InputIt iter = first;
		ListImpl::NodeBase* last_node = &m_allocator_and_nodebase.second();
		m_size = 0;
		while (iter != last)
		{
			ListImpl::NodeBase* node = new_object<node_type>(*iter);
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
			++m_size;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::internal_assign_nocleanup(usize count)
	{
		ListImpl::NodeBase* last_node = &m_allocator_and_nodebase.second();
		m_size = count;
		while (count)
		{
			ListImpl::NodeBase* node = new_object<node_type>();
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			--count;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::internal_assign_nocleanup(const List& rhs)
	{
		const_iterator iter = rhs.begin();
		ListImpl::NodeBase* last_node = &m_allocator_and_nodebase.second();
		m_size = rhs.size();
		while (iter != rhs.end())
		{
			ListImpl::NodeBase* node = new_object<node_type>(*iter);
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::internal_assign_nocleanup(List&& rhs)
	{
		m_size = rhs.m_size;
		if (m_size)
		{
			m_allocator_and_nodebase.second().m_next = rhs.m_allocator_and_nodebase.second().m_next;
			m_allocator_and_nodebase.second().m_prev = rhs.m_allocator_and_nodebase.second().m_prev;
			m_allocator_and_nodebase.second().m_next->m_prev = &m_allocator_and_nodebase.second();
			m_allocator_and_nodebase.second().m_prev->m_next = &m_allocator_and_nodebase.second();
			rhs.m_allocator_and_nodebase.second().m_next = &rhs.m_allocator_and_nodebase.second();
			rhs.m_allocator_and_nodebase.second().m_prev = &rhs.m_allocator_and_nodebase.second();
			rhs.m_size = 0;
		}
		else
		{
			m_allocator_and_nodebase.second().m_next = &m_allocator_and_nodebase.second();
			m_allocator_and_nodebase.second().m_prev = &m_allocator_and_nodebase.second();
		}
	}
	template <typename _Ty, typename _Alloc>
	inline void List<_Ty, _Alloc>::internal_element_wise_assign_nocleanup(List&& rhs)
	{
		const_iterator iter = rhs.begin();
		ListImpl::NodeBase* last_node = &m_allocator_and_nodebase.second();
		m_size = rhs.size();
		while (iter != rhs.end())
		{
			ListImpl::NodeBase* node = new_object<node_type>(move(*iter));
			node->m_prev = last_node;
			last_node->m_next = node;
			last_node = node;
			++iter;
		}
		m_allocator_and_nodebase.second().m_prev = last_node;
		last_node->m_next = &m_allocator_and_nodebase.second();
		rhs.clear();
	}
	template <typename _Ty, typename _Alloc>
	template<typename _BinaryPredicate>
	inline typename List<_Ty, _Alloc>::iterator List<_Ty, _Alloc>::internal_sort(iterator first, iterator last, usize sz, _BinaryPredicate comp)
	{
		iterator begin1 = first;
		iterator end2 = last;
		switch (sz)
		{
		case 0:
		case 1:
			return begin1;
		case 2:
			if (comp(*--end2, *begin1))
			{
				end2.m_cur->remove_this();
				end2.m_cur->insert_before(begin1.m_cur);
				return end2;
			}
			return begin1;
		case 3:
		{
			iterator lowest = begin1;
			for (iterator iter = next(begin1); iter != end2; ++iter)
			{
				if (comp(*iter, *lowest))
				{
					lowest = iter;
				}
			}
			if (lowest == begin1)
			{
				++begin1;
			}
			else
			{
				lowest.m_cur->remove_this();
				lowest.m_cur->insert_before(begin1.m_cur);
			}
			if (comp(*--end2, *begin1))
			{
				end2.m_cur->remove_this();
				end2.m_cur->insert_before(begin1.m_cur);
			}

			return lowest;
		}
		default:break;
		}

		iterator result(nullptr);
		usize mid = (sz / 2);
		iterator end1 = next(begin1, (isize)mid);
		begin1 = internal_sort(begin1, end1, mid, comp);
		iterator begin2 = internal_sort(end1, end2, (isize)(sz - mid), comp);
		// If the start of the second list is before the start of the first list, insert the first list 
		// into the second at an appropriate starting place. 
		if (comp(*begin2, *begin1))
		{
			// Find the position to insert the first list into the second list. 
			iterator ix = next(begin2);
			while ((ix != end2) && comp(*ix, *begin1))
			{
				++ix;
			}
			// Cut out the initial segment of the second list and move it to be in front of the first list. 
			ListImpl::NodeBase* i2_cut = begin2.m_cur;
			ListImpl::NodeBase* i2_cut_last = ix.m_cur->m_prev;
			result = begin2;
			end1 = begin2 = ix;
			ListImpl::NodeBase::remove_range(i2_cut, i2_cut_last);
			ListImpl::NodeBase::insert_range(begin1.m_cur, i2_cut, i2_cut_last);
		}
		else
		{
			result = begin1;
			end1 = begin2;
		}

		// Merge the two segments. We do this by merging the second sub-segment into the first, by walking forward in each of the two sub-segments.
		for (++begin1; (begin1 != end1) && (begin2 != end2); ++begin1)
		{
			if (comp(*begin2, *begin1))
			{
				// Find the position to insert the i2 list into the i1 list. 
				iterator ix = next(begin2);
				while ((ix != end2) && comp(*ix, *begin1))
				{
					++ix;
				}
				// Cut this section of the i2 sub-segment out and merge into the appropriate place in the i1 list.
				ListImpl::NodeBase* i2_cut = begin2.m_cur;
				ListImpl::NodeBase* i2_cut_last = ix.m_cur->m_prev;
				if (end1 == begin2)
				{
					end1 = ix;
				}
				begin2 = ix;
				ListImpl::NodeBase::remove_range(i2_cut, i2_cut_last);
				ListImpl::NodeBase::insert_range(begin1.m_cur, i2_cut, i2_cut_last);
			}
		}
		return result;
	}
	template <typename _Ty, typename _Alloc>
	inline bool operator==(const List<_Ty, _Alloc>& lhs, const List<_Ty, _Alloc>& rhs)
	{
		auto i1 = lhs.begin();
		auto i2 = rhs.begin();
		while ((i1 != lhs.end()) && (i2 != rhs.end()))
		{
			if (!(*i1 == *i2))
			{
				return false;
			}
			++i1;
			++i2;
		}
		if ((i1 != lhs.end()) || (i2 != rhs.end()))
		{
			return false;
		}
		return true;
	}
	template <typename _Ty, typename _Alloc>
	inline bool operator!=(const List<_Ty, _Alloc>& lhs, const List<_Ty, _Alloc>& rhs)
	{
		return !(lhs == rhs);
	}
	template <typename _Ty, typename _Alloc>
	inline void swap(List<_Ty>& lhs, List<_Ty>& rhs)
	{
		lhs.swap(rhs);
	}

	template <typename _Ty, typename _Alloc>
	inline void gc_track(const List<_Ty, _Alloc>& obj)
	{
		for (auto& i : obj)
		{
			gc_track(i);
		}
	}
}
