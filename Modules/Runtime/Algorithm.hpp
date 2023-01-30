/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Algorithm.hpp
* @author JXMaster
* @date 2020/2/17
* @brief Defines most commonly used algorithms.
*/
#pragma once
#include "Base.hpp"
#include "Iterator.hpp"

/*
	Functions in Algorithm.hpp:
	
	min
	max
	swap
	equal
*/

namespace Luna
{
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

	template <typename _Ty>
	inline constexpr const _Ty& min(const _Ty& a, const _Ty& b)
	{
		return (b < a) ? b : a;
	}

	template <typename _Ty, typename _LessComp>
	inline constexpr const _Ty& min(const _Ty& a, const _Ty& b, _LessComp less_comp)
	{
		return (less_comp(b, a)) ? b : a;
	}

	template <typename _Ty>
	inline constexpr const _Ty& max(const _Ty& a, const _Ty& b)
	{
		return (a < b) ? b : a;
	}

	template <typename _Ty, typename _LessComp>
	inline constexpr const _Ty& max(const _Ty& a, const _Ty& b, _LessComp less_comp)
	{
		return (less_comp(a, b)) ? b : a;
	}

	template <typename _Ty>
	inline void swap(_Ty& a, _Ty& b)
	{
		_Ty temp(move(a));
		a = move(b);
		b = move(temp);
	}

	template <typename _Iter1, typename _Iter2>
	inline bool equal(_Iter1 first1, _Iter1 last1, _Iter2 first2)
	{
		for (; first1 != last1; ++first1, ++first2) 
		{
			if (!(*first1 == *first2)) 
			{
				return false;
			}
		}
		return true;
	}

	template <typename _Iter1, typename _Iter2, typename _EqualComp>
	inline bool equal(_Iter1 first1, _Iter1 last1, _Iter2 first2, _EqualComp equal_compare)
	{
		for (; first1 != last1; ++first1, ++first2) 
		{
			if (!equal_compare(*first1, *first2)) 
			{
				return false;
			}
		}
		return true;
	}

	template<typename _InputIt, typename _Ty>
	inline constexpr _InputIt find(_InputIt first, _InputIt last, const _Ty& value)
	{
		for (; first != last; ++first) {
			if (*first == value) {
				return first;
			}
		}
		return last;
	}

	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr _InputIt find_if(_InputIt first, _InputIt last, _UnaryPredicate p)
	{
		for (; first != last; ++first) {
			if (p(*first)) {
				return first;
			}
		}
		return last;
	}

	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr _InputIt find_if_not(_InputIt first, _InputIt last, _UnaryPredicate q)
	{
		for (; first != last; ++first) {
			if (!q(*first)) {
				return first;
			}
		}
		return last;
	}

	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr bool all_of(_InputIt first, _InputIt last, _UnaryPredicate p)
	{
		return find_if_not(first, last, p) == last;
	}

	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr bool any_of(_InputIt first, _InputIt last, _UnaryPredicate p)
	{
		return find_if(first, last, p) != last;
	}

	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr bool none_of(_InputIt first, _InputIt last, _UnaryPredicate p)
	{
		return find_if(first, last, p) == last;
	}

	template<typename _InputIt, typename _UnaryFunction>
	inline constexpr _UnaryFunction for_each(_InputIt first, _InputIt last, _UnaryFunction f)
	{
		for (; first != last; ++first) {
			f(*first);
		}
		return f; // implicit move since C++11
	}

	template <typename _RandomIt>
	void sort(_RandomIt first, _RandomIt last)
	{
		if (first + 1 >= last) return;
		_RandomIt i = first;
		_RandomIt j = last - 1;
		_RandomIt flag = first;
		while (i < j)
		{
			while (*flag < *j && i < j) --j;
			while (!(*flag < *i) && i < j) ++i;
			if (i < j)
			{
				swap(*i, *j);
			}
		}
		swap(*i, *flag);
		sort(first, i);
		sort(i + 1, last);
	}

	template <typename _RandomIt, typename _Compare>
	void sort(_RandomIt first, _RandomIt last, _Compare comp)
	{
		if (first + 1 >= last) return;
		_RandomIt i = first;
		_RandomIt j = last - 1;
		_RandomIt flag = first;
		while (i < j)
		{
			while (comp(*flag, *j) && i < j) --j;
			while (!comp(*flag, *i) && i < j) ++i;
			if (i < j)
			{
				swap(*i, *j);
			}
		}
		swap(*i, *flag);
		sort(first, i, comp);
		sort(i + 1, last, comp);
	}

	template<typename _ForwardIt, typename _Ty>
	_ForwardIt upper_bound(_ForwardIt first, _ForwardIt last, const _Ty& value)
	{
		_ForwardIt it;
		typename iterator_traits<_ForwardIt>::difference_type count, step;
		count = distance(first, last);

		while (count > 0) {
			it = first;
			step = count / 2;
			advance(it, step);
			if (!(value < *it)) {
				first = ++it;
				count -= step + 1;
			}
			else
				count = step;
		}
		return first;
	}

	template<typename _ForwardIt, typename _Ty, typename _Compare>
	_ForwardIt upper_bound(_ForwardIt first, _ForwardIt last, const _Ty& value, _Compare comp)
	{
		_ForwardIt it;
		usize count, step;
		count = distance(first, last);

		while (count > 0) {
			it = first;
			step = count / 2;
			advance(it, step);
			if (!comp(value, *it)) {
				first = ++it;
				count -= step + 1;
			}
			else
				count = step;
		}
		return first;
	}

	template<typename _ForwardIt, typename _Ty>
	_ForwardIt lower_bound(_ForwardIt first, _ForwardIt last, const _Ty& value)
	{
		_ForwardIt it;
		usize count, step;
		count = distance(first, last);

		while (count > 0) {
			it = first;
			step = count / 2;
			advance(it, step);
			if (*it < value) {
				first = ++it;
				count -= step + 1;
			}
			else
				count = step;
		}
		return first;
	}

	template<typename _ForwardIt, typename _Ty, typename _Compare>
	_ForwardIt lower_bound(_ForwardIt first, _ForwardIt last, const _Ty& value, _Compare comp)
	{
		_ForwardIt it;
		usize count, step;
		count = distance(first, last);

		while (count > 0) {
			it = first;
			step = count / 2;
			advance(it, step);
			if (comp(*it, value)) {
				first = ++it;
				count -= step + 1;
			}
			else
				count = step;
		}
		return first;
	}

	template<typename _ForwardIt, typename _Ty>
	bool binary_search(_ForwardIt first, _ForwardIt last, const _Ty& value)
	{
		first = lower_bound(first, last, value);
		return (!(first == last) && !(value < *first));
	}

	template<typename _ForwardIt, typename _Ty, typename _Compare>
	bool binary_search(_ForwardIt first, _ForwardIt last, const _Ty& value, _Compare comp)
	{
		first = lower_bound(first, last, value, comp);
		return (!(first == last) && !(comp(value, *first)));
	}

	template<typename _ForwardIt, typename _Ty>
	_ForwardIt binary_search_iter(_ForwardIt first, _ForwardIt last, const _Ty& value)
	{
		first = lower_bound(first, last, value);
		return (!(first == last) && !(value < *first)) ? first : last;
	}

	template<typename _ForwardIt, typename _Ty, typename _Compare>
	_ForwardIt binary_search_iter(_ForwardIt first, _ForwardIt last, const _Ty& value, _Compare comp)
	{
		first = lower_bound(first, last, value, comp);
		return (!(first == last) && !(comp(value, *first))) ? first : lsat;
	}

	template<typename _ForwardIt, typename _Ty>
	Pair<_ForwardIt, _ForwardIt>
		equal_range(_ForwardIt first, _ForwardIt last,
			const _Ty& value)
	{
		return make_pair(lower_bound(first, last, value),
			upper_bound(first, last, value));
	}

	template<typename _ForwardIt, typename _Ty, typename _Compare>
	Pair<_ForwardIt, _ForwardIt>
		equal_range(_ForwardIt first, _ForwardIt last,
			const _Ty& value, _Compare comp)
	{
		return make_pair(lower_bound(first, last, value, comp),
			upper_bound(first, last, value, comp));
	}

	//! Returns true if the sorted range [first2, last2) is a subsequence of the sorted range [first1, last1). 
	//! (A subsequence need not be contiguous.)
	template<typename _InputIt1, typename _InputIt2>
	bool includes(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2)
	{
		for (; first2 != last2; ++first1)
		{
			if (first1 == last1 || *first2 < *first1)
				return false;
			if (!(*first1 < *first2))
				++first2;
		}
		return true;
	}
	template<typename _InputIt1, typename _InputIt2, typename _Compare>
	bool includes(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _Compare comp)
	{
		for (; first2 != last2; ++first1)
		{
			if (first1 == last1 || comp(*first2, *first1))
				return false;
			if (!comp(*first1, *first2))
				++first2;
		}
		return true;
	}
	
	template<typename _InputIt, typename _OutputIt>
	_OutputIt copy(_InputIt first, _InputIt last,
		_OutputIt d_first)
	{
		for (; first != last; (void)++first, (void)++d_first) {
			*d_first = *first;
		}
		return d_first;
	}

	template<typename _InputIt, typename _OutputIt, typename UnaryPredicate>
	_OutputIt copy_if(_InputIt first, _InputIt last,
		_OutputIt d_first, UnaryPredicate pred)
	{
		for (; first != last; ++first) {
			if (pred(*first)) {
				*d_first = *first;
				++d_first;
			}
		}
		return d_first;
	}

	//! Copies the elements from the sorted range `[first1, last1)` 
	//! which are not found in the sorted range `[first2, last2)` to the range beginning at `d_first`. 
	//! The output range is also sorted.
	//! 
	//! If `[first1, last1)` contains `m` elements that are equivalent to each otherand 
	//! `[first2, last2)` contains `n` elements that are equivalent to them, 
	//! the final `max(m - n, 0)` elements will be copied from `[first1, last1)` to the output range, 
	//! preserving order.
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt>
	_OutputIt set_difference(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _OutputIt d_first)
	{
		while (first1 != last1)
		{
			if (first2 == last2)
				return copy(first1, last1, d_first);

			if (*first1 < *first2)
				*d_first++ = *first1++;
			else
			{
				if (!(*first2 < *first1))
					++first1;
				++first2;
			}
		}
		return d_first;
	}
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt, typename _Compare>
	_OutputIt set_difference(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _OutputIt d_first, _Compare comp)
	{
		while (first1 != last1)
		{
			if (first2 == last2)
				return copy(first1, last1, d_first);

			if (comp(*first1, *first2))
				*d_first++ = *first1++;
			else
			{
				if (!comp(*first2, *first1))
					++first1;
				++first2;
			}
		}
		return d_first;
	}

	//! Constructs a sorted range beginning at `d_first` consisting of elements that are found in both 
	//! sorted ranges `[first1, last1)` and `[first2, last2)`.
	//! 
	//! If `[first1, last1)` contains `m` elements that are equivalent to each otherand `[first2, last2)`
	//! contains `n` elements that are equivalent to them, the first `min(m, n)` elements will be copied from 
	//! `[first1, last1)` to the output range, preserving order.
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt>
	_OutputIt set_intersection(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _OutputIt d_first)
	{
		while (first1 != last1 && first2 != last2)
		{
			if (*first1 < *first2)
				++first1;
			else
			{
				if (!(*first2 < *first1))
					*d_first++ = *first1++; // *first1 and *first2 are equivalent.
				++first2;
			}
		}
		return d_first;
	}
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt, typename _Compare>
	_OutputIt set_intersection(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _OutputIt d_first, _Compare comp)
	{
		while (first1 != last1 && first2 != last2)
		{
			if (comp(*first1, *first2))
				++first1;
			else
			{
				if (!comp(*first2, *first1))
					*d_first++ = *first1++; // *first1 and *first2 are equivalent.
				++first2;
			}
		}
		return d_first;
	}

	//! Computes symmetric difference of two sorted ranges: the elements that are found in either of the ranges, 
	//! but not in both of them are copied to the range beginning at `d_first`. The output range is also sorted.
	//! 
	//! If `[first1, last1)` contains `m` elements that are equivalent to each otherand `[first2, last2)` contains 
	//! `n` elements that are equivalent to them, then `abs(m - n)` of those elements will be copied to the output range, preserving order :
	//! 
	//! * if `m > n`, the final `m - n` of these elements from `[first1, last1)`.
	//! * if `m < n`, the final `n - m` of these elements from `[first2, last2)`.
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt>
	_OutputIt set_symmetric_difference(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _OutputIt d_first)
	{
		while (first1 != last1)
		{
			if (first2 == last2)
				return copy(first1, last1, d_first);

			if (*first1 < *first2)
				*d_first++ = *first1++;
			else
			{
				if (*first2 < *first1)
					*d_first++ = *first2;
				else
					++first1;
				++first2;
			}
		}
		return copy(first2, last2, d_first);
	}
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt, typename _Compare>
	_OutputIt set_symmetric_difference(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2,
		_OutputIt d_first, _Compare comp)
	{
		while (first1 != last1)
		{
			if (first2 == last2)
				return copy(first1, last1, d_first);

			if (comp(*first1, *first2))
				*d_first++ = *first1++;
			else
			{
				if (comp(*first2, *first1))
					*d_first++ = *first2;
				else
					++first1;
				++first2;
			}
		}
		return copy(first2, last2, d_first);
	}

	//! Constructs a sorted union beginning at `d_first` consisting of the set of elements present in 
	//! one or both sorted ranges `[first1, last1)` and `[first2, last2)`.
	//! 
	//! If `[first1, last1)` contains `m` elements that are equivalent to each otherand 
	//! `[first2, last2)` contains `n` elements that are equivalent to them, 
	//! then all `m` elements will be copied from `[first1, last1)` to the output range, preserving order, 
	//! and then the final `max(n - m, 0)` elements will be copied from `[first2, last2)` to the output range, 
	//! also preserving order.
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt>
	_OutputIt set_union(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _OutputIt d_first)
	{
		for (; first1 != last1; ++d_first)
		{
			if (first2 == last2)
				return copy(first1, last1, d_first);

			if (*first2 < *first1)
				*d_first = *first2++;
			else
			{
				*d_first = *first1;
				if (!(*first1 < *first2))
					++first2;
				++first1;
			}
		}
		return copy(first2, last2, d_first);
	}
	template<typename _InputIt1, typename _InputIt2, typename _OutputIt, typename _Compare>
	_OutputIt set_union(_InputIt1 first1, _InputIt1 last1,
		_InputIt2 first2, _InputIt2 last2, _OutputIt d_first, _Compare comp)
	{
		for (; first1 != last1; ++d_first)
		{
			if (first2 == last2)
				// Finished range 2, include the rest of range 1:
				return copy(first1, last1, d_first);

			if (comp(*first2, *first1))
				*d_first = *first2++;
			else
			{
				*d_first = *first1;
				if (!comp(*first1, *first2)) // Equivalent => don't need to include *first2.
					++first2;
				++first1;
			}
		}
		// Finished range 1, include the rest of range 2:
		return copy(first2, last2, d_first);
	}
}