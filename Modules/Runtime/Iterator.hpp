/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Iterator.hpp
* @author JXMaster
* @date 2020/2/14
* @brief The iterator library for SDK. 
*/
#pragma once
#include "Base.hpp"
namespace Luna
{
	template <typename _Iter>
	struct iterator_traits
	{
		using value_type = typename _Iter::value_type;
		using pointer = typename _Iter::pointer;
		using reference = typename _Iter::reference;
		using iterator_category = typename _Iter::iterator_category;
	};

	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};

	//! template specialization for pointers.
	template <typename _Ty>
	struct iterator_traits<_Ty*>
	{
		using value_type = remove_cv_t<_Ty>;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = random_access_iterator_tag;
	};

	template <typename _Ty>
	struct iterator_traits<const _Ty*>
	{
		using value_type = remove_cv_t<_Ty>;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = random_access_iterator_tag;
	};

	template <typename _Iter>
	class ReverseIterator
	{
	public:
		using iterator_type = _Iter;
		using value_type = typename iterator_traits<iterator_type>::value_type;
		using pointer = typename iterator_traits<iterator_type>::pointer;
		using reference = typename iterator_traits<iterator_type>::reference;
		using iterator_category = typename iterator_traits<iterator_type>::iterator_category;

	protected:
		iterator_type m_base;

	public:
		constexpr ReverseIterator()
			: m_base() { }

		constexpr explicit ReverseIterator(iterator_type i)
			: m_base(i) { }

		constexpr ReverseIterator(const ReverseIterator& ri)
			: m_base(ri.m_base) { }

		template <typename _Uty>
		constexpr ReverseIterator(const ReverseIterator<_Uty>& ri)
			: m_base(ri.base()) { }

		template <typename _Uty>
		constexpr ReverseIterator<iterator_type>& operator=(const ReverseIterator<_Uty>& ri)
		{
			m_base = ri.base(); 
			return *this;
		}

		constexpr iterator_type base() const
		{
			return m_base;
		}

		constexpr reference operator*() const
		{
			iterator_type i(m_base);
			return *--i;
		}

		constexpr pointer operator->() const
		{
			return &(operator*());
		}

		constexpr ReverseIterator& operator++()
		{
			--m_base; 
			return *this;
		}

		constexpr ReverseIterator operator++(int)
		{
			ReverseIterator ri(*this);
			--m_base;
			return ri;
		}

		constexpr ReverseIterator& operator--()
		{
			++m_base; 
			return *this;
		}

		constexpr ReverseIterator operator--(int)
		{
			ReverseIterator ri(*this);
			++m_base;
			return ri;
		}

		constexpr ReverseIterator operator+(isize n) const
		{
			return ReverseIterator(m_base - n);
		}

		constexpr ReverseIterator& operator+=(isize n)
		{
			m_base -= n; return *this;
		}

		constexpr ReverseIterator operator-(isize n) const
		{
			return ReverseIterator(m_base + n);
		}

		constexpr ReverseIterator& operator-=(isize n)
		{
			m_base += n; 
			return *this;
		}

		// http://cplusplus.github.io/LWG/lwg-defects.html#386, 
		// http://llvm.org/bugs/show_bug.cgi?id=17883 
		// random_access_iterator operator[] is merely required to return something convertible to reference. 
		// ReverseIterator operator[] can't necessarily know what to return as the underlying iterator 
		// operator[] may return something other than reference.
		constexpr reference operator[](isize n) const
		{
			return m_base[-n - 1];
		}
	};

	template <typename _Iter1, typename _Iter2>
	constexpr inline bool
		operator==(const ReverseIterator<_Iter1>& a, const ReverseIterator<_Iter2>& b)
	{
		return a.base() == b.base();
	}


	template <typename _Iter1, typename _Iter2>
	constexpr inline bool
		operator<(const ReverseIterator<_Iter1>& a, const ReverseIterator<_Iter2>& b)
	{
		return a.base() > b.base();
	}


	template <typename _Iter1, typename _Iter2>
	constexpr inline bool
		operator!=(const ReverseIterator<_Iter1>& a, const ReverseIterator<_Iter2>& b)
	{
		return a.base() != b.base();
	}


	template <typename _Iter1, typename _Iter2>
	constexpr inline bool
		operator>(const ReverseIterator<_Iter1>& a, const ReverseIterator<_Iter2>& b)
	{
		return a.base() < b.base();
	}


	template <typename _Iter1, typename _Iter2>
	constexpr inline bool
		operator<=(const ReverseIterator<_Iter1>& a, const ReverseIterator<_Iter2>& b)
	{
		return a.base() >= b.base();
	}


	template <typename _Iter1, typename _Iter2>
	constexpr inline bool
		operator>=(const ReverseIterator<_Iter1>& a, const ReverseIterator<_Iter2>& b)
	{
		return a.base() <= b.base();
	}


	template <typename _Iter1, typename _Iter2>
	constexpr inline isize
		operator-(const ReverseIterator<_Iter1>& a, const ReverseIterator<_Iter2>& b)
	{
		return b.base() - a.base();
	}


	template <typename _Iter>
	constexpr inline ReverseIterator<_Iter>
		operator+(isize n, const ReverseIterator<_Iter>& a)
	{
		return ReverseIterator<_Iter>(a.base() - n);
	}

	template <typename _Iter>
	constexpr ReverseIterator<_Iter> make_reverse_iterator(_Iter i)
	{
		return ReverseIterator<_Iter>(i);
	}

	namespace IterImpl
	{
		template<typename _Iter>
#ifdef LUNA_COMPILER_CPP17
		constexpr // required since C++17
#endif
		void do_advance(_Iter& it, isize n, input_iterator_tag)
		{
			while (n > 0) {
				--n;
				++it;
			}
		}

		template<typename _Iter>
#ifdef LUNA_COMPILER_CPP17
		constexpr // required since C++17
#endif
		void do_advance(_Iter& it, isize n, bidirectional_iterator_tag)
		{
			while (n > 0) {
				--n;
				++it;
			}
			while (n < 0) {
				++n;
				--it;
			}
		}

		template<typename _Iter>
#ifdef LUNA_COMPILER_CPP17
		constexpr // required since C++17
#endif
		void do_advance(_Iter& it, isize n, random_access_iterator_tag)
		{
			it += n;
		}

		template<typename _It>
#ifdef LUNA_COMPILER_CPP17
		constexpr // required since C++17
#endif
		isize do_distance(_It first, _It last, input_iterator_tag)
		{
			isize result = 0;
			while (first != last) {
				++first;
				++result;
			}
			return result;
		}

		template<typename _It>
#ifdef LUNA_COMPILER_CPP17
		constexpr // required since C++17
#endif
		isize do_distance(_It first, _It last, random_access_iterator_tag)
		{
			return last - first;
		}
	}

	template<typename _Iter, typename _Distance>
#ifdef LUNA_COMPILER_CPP17
	constexpr // required since C++17
#endif
	void advance(_Iter& it, _Distance n)
	{
		IterImpl::do_advance(it, isize(n),
			typename iterator_traits<_Iter>::iterator_category());
	}

	template<typename _It>
#ifdef LUNA_COMPILER_CPP17
	constexpr // required since C++17
#endif
	isize distance(_It first, _It last)
	{
		return IterImpl::do_distance(first, last,
			typename iterator_traits<_It>::iterator_category());
	}

	template<typename _Iter>
#ifdef LUNA_COMPILER_CPP17
	constexpr // required since C++17
#endif
	_Iter next(_Iter it, isize n = 1)
	{
		advance(it, n);
		return it;
	}

	template<typename _BidirIt>
#ifdef LUNA_COMPILER_CPP17
	constexpr // required since C++17
#endif
	_BidirIt prev(_BidirIt it, isize n = 1)
	{
		advance(it, -n);
		return it;
	}


}