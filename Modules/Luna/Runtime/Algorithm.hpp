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
#include "Memory.hpp"

namespace Luna
{
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeAlgorithm Algorithms
	//! @}

	//! @addtogroup RuntimeAlgorithm
	//! @{
	
	//! Returns the smaller of the given values.
	//! @param[in] a The first value to compare.
	//! @param[in] b The second value to compare.
	//! @return Returns the smaller of the given values. Returns `a` if values are equivalent.
	template <typename _Ty>
	inline constexpr const _Ty& min(const _Ty& a, const _Ty& b)
	{
		return (b < a) ? b : a;
	}

	//! Returns the smaller of the given values.
	//! @param[in] a The first value to compare.
	//! @param[in] b The second value to compare.
	//! @param[in] less_comp The user-defined less (<) comparison function used for comparing.
	//! @return Returns the smaller of the given values. Returns `a` if values are equivalent.
	template <typename _Ty, typename _LessComp>
	inline constexpr const _Ty& min(const _Ty& a, const _Ty& b, _LessComp less_comp)
	{
		return (less_comp(b, a)) ? b : a;
	}

	//! Returns the greater of the given values.
	//! @param[in] a The first value to compare.
	//! @param[in] b The second value to compare.
	//! @return Returns the greater of the given values. Returns `a` if values are equivalent.
	template <typename _Ty>
	inline constexpr const _Ty& max(const _Ty& a, const _Ty& b)
	{
		return (a < b) ? b : a;
	}

	//! Returns the greater of the given values.
	//! @param[in] a The first value to compare.
	//! @param[in] b The second value to compare.
	//! @param[in] less_comp The user-defined less (<) comparison function used for comparing.
	//! @return Returns the greater of the given values. Returns `a` if values are equivalent.
	template <typename _Ty, typename _LessComp>
	inline constexpr const _Ty& max(const _Ty& a, const _Ty& b, _LessComp less_comp)
	{
		return (less_comp(a, b)) ? b : a;
	}

	//! Swaps two values.
	//! @param[in] a The first value to swap.
	//! @param[in] b The second value to swap.
	template <typename _Ty>
	inline void swap(_Ty& a, _Ty& b)
	{
		_Ty temp(move(a));
		a = move(b);
		b = move(temp);
	}

	//! Tests the equality of two ranges.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @return Returns `true` if two ranges are equal, returns `false` otherwise.
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

	//! Tests the equality of two ranges.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] equal_compare The user-defined equal (==) comparison function used for comparing.
	//! @return Returns `true` if two ranges are equal, returns `false` otherwise.
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

	namespace Impl
	{
		template <typename _RandomIt>
		inline void kmp_compute_lps(_RandomIt pattern, usize pattern_len, usize* lps)
		{
			lps[0] = 0;
			usize len = 0;
			usize i = 1;
			while(i < pattern_len)
			{
				if(pattern[i] == pattern[len])
				{
					++len;
					lps[i] = len;
					++i;
				}
				else
				{
					if(len != 0)
					{
						len = lps[len - 1];
					}
					else
					{
						lps[i] = 0;
						++i;
					}
				}
			}	
		}
		template <typename _RandomIt>
		inline void kmp_compute_lps_reverse(_RandomIt pattern, usize pattern_len, usize* lps)
		{
			lps[pattern_len - 1] = 0;
			usize len = 0;
			usize i = 1;
			while(i < pattern_len)
			{
				if(pattern[pattern_len - 1 - i] == pattern[pattern_len - 1 - len])
				{
					++len;
					lps[pattern_len - 1 - i] = len;
					++i;
				}
				else
				{
					if(len != 0)
					{
						len = lps[pattern_len - len];
					}
					else
					{
						lps[pattern_len - 1 - i] = 0;
						++i;
					}
				}
			}	
		}
		template <typename _RandomIt>
		inline _RandomIt kmp_search(_RandomIt str, usize str_size, _RandomIt pattern, usize pattern_size, 
			const usize* lps)
		{
			if(pattern_size > str_size) return str + str_size;
			usize pattern_i = 0;
			for(usize i = 0; i < str_size; ++i)
			{
				while(pattern_i && str[i] != pattern[pattern_i]) 
				{
					pattern_i = lps[pattern_i - 1];
				}
				if(str[i] == pattern[pattern_i])
				{
					if(pattern_i == pattern_size - 1) return str + i - pattern_i;
					++pattern_i;
				}
			}
			return str + str_size;
		}
		template <typename _RandomIt>
		inline _RandomIt kmp_search_reverse(_RandomIt str, usize str_size, _RandomIt pattern, usize pattern_size, 
			const usize* lps)
		{
			if(pattern_size > str_size) return str + str_size;
			usize pattern_i = 0;
			for(usize i = 0; i < str_size; ++i)
			{
				while(pattern_i && str[str_size - 1 - i] != pattern[pattern_size - 1 - pattern_i]) 
				{
					pattern_i = lps[pattern_size - pattern_i];
				}
				if(str[str_size - 1 - i] == pattern[pattern_size - 1 - pattern_i])
				{
					if(pattern_i == pattern_size - 1) return str + str_size - (i - pattern_i) - pattern_size;
					++pattern_i;
				}
			}
			return str + str_size;
		}

		constexpr usize KMP_STACK_SIZE_THRESHOLD = 256;
	}

	//! Searches for the first occurrence of the sequence of elements in the specified range.
	//! @param[in] first The iterator to the first element of the search range.
	//! @param[in] last The iterator to the one-past-last element of the search range.
	//! @param[in] pattern_first The iterator to the first element of the sequence to search.
	//! @param[in] pattern_last The iterator to the one-past-last element of the sequence to search.
	//! @return Returns one iterator to the beginning of first occurrence of the sequence[`pattern_first`, `pattern_last`).
	//! Returns `last` if no such occurrence is found.
	template <typename _ForwardIt>
	auto search(_ForwardIt first, _ForwardIt last,
		_ForwardIt pattern_first, _ForwardIt pattern_last) -> enable_if_t<is_pointer_v<_ForwardIt>, _ForwardIt>
	{
		usize str_size = (usize)distance(first, last);
		usize pattern_size = (usize)distance(pattern_first, pattern_last);
		if(pattern_size == 0) return first;
		usize* lps;
		usize lps_size_bytes = sizeof(usize) * pattern_size;
		if(lps_size_bytes > Impl::KMP_STACK_SIZE_THRESHOLD) lps = (usize*)memalloc(lps_size_bytes);
		else lps = (usize*)alloca(lps_size_bytes);
		Impl::kmp_compute_lps(pattern_first, pattern_size, lps);
		auto it = Impl::kmp_search(first, str_size, pattern_first, pattern_size, lps);
		if(lps_size_bytes > Impl::KMP_STACK_SIZE_THRESHOLD) memfree(lps);
		return it;
	}

	//! Searches for the last occurrence of the sequence of elements in the specified range.
	//! @param[in] first The iterator to the first element of the search range.
	//! @param[in] last The iterator to the one-past-last element of the search range.
	//! @param[in] pattern_first The iterator to the first element of the sequence to search.
	//! @param[in] pattern_last The iterator to the one-past-last element of the sequence to search.
	//! @return Returns one iterator to the beginning of last occurrence of the sequence[`pattern_first`, `pattern_last`).
	//! Returns `last` if no such occurrence is found.
	template <typename _ForwardIt>
	auto find_end(_ForwardIt first, _ForwardIt last,
		_ForwardIt pattern_first, _ForwardIt pattern_last) -> enable_if_t<is_pointer_v<_ForwardIt>, _ForwardIt>
	{
		usize str_size = (usize)distance(first, last);
		usize pattern_size = (usize)distance(pattern_first, pattern_last);
		if(pattern_size == 0) return last;
		usize* lps;
		usize lps_size_bytes = sizeof(usize) * pattern_size;
		if(lps_size_bytes > Impl::KMP_STACK_SIZE_THRESHOLD) lps = (usize*)memalloc(lps_size_bytes);
		else lps = (usize*)alloca(lps_size_bytes);
		Impl::kmp_compute_lps_reverse(pattern_first, pattern_size, lps);
		auto it = Impl::kmp_search_reverse(first, str_size, pattern_first, pattern_size, lps);
		if(lps_size_bytes > Impl::KMP_STACK_SIZE_THRESHOLD) memfree(lps);
		return it;
	}

	//! Searches for the first element in the range that is equal to (==) the specified value.
	//! @param[in] first The iterator to the first element of the search range.
	//! @param[in] last The iterator to the one-past-last element of the search range.
	//! @param[in] value The value to compare for equality.
	//! @return Returns one iterator to the first element that is equal to the specified value. Returns `last` if not found.
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

	//! Searches for the first element in the range that passes the user-provided unary predicate.
	//! @param[in] first The iterator to the first element of the search range.
	//! @param[in] last The iterator to the one-past-last element of the search range.
	//! @param[in] p The user-provided unary predicate which will be called to test elements.
	//! @return Returns one iterator to the first element that `p(v)` is `true`. Returns `last` if not found.
	//! @par Valid Usage
	//! * The expression `p(v)` must be convertible to `bool` for every argument `v` of type `VT`, where `VT` is the value
	//! type of `_InputIt`, and must not modify `v`.
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

	//! Searches for the first element in the range that fails the user-provided unary predicate.
	//! @param[in] first The iterator to the first element of the search range.
	//! @param[in] last The iterator to the one-past-last element of the search range.
	//! @param[in] q The user-provided unary predicate which will be called to test elements.
	//! @return Returns one iterator to the first element that `q(v)` is `false`. Returns `last` if not found.
	//! @par Valid Usage
	//! * The expression `q(v)` must be convertible to `bool` for every argument `v` of type `VT`, where `VT` is the value
	//! type of `_InputIt`, and must not modify `v`.
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

	//! Checks if the unary predicate returns `true` for all elements in the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] p The user-provided unary predicate which will be called to test elements.
	//! @return Returns `ture` if the unary predicate returns `true` for all elements in the range. Returns `false` otherwise.
	//! @par Valid Usage
	//! * The expression `p(v)` must be convertible to `bool` for every argument `v` of type `VT`, where `VT` is the value
	//! type of `_InputIt`, and must not modify `v`.
	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr bool all_of(_InputIt first, _InputIt last, _UnaryPredicate p)
	{
		return find_if_not(first, last, p) == last;
	}

	//! Checks if the unary predicate returns `true` for at least one element in the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] p The user-provided unary predicate which will be called to test elements.
	//! @return Returns `ture` if the unary predicate returns `true` for at least one element in the range. Returns `false` otherwise.
	//! @par Valid Usage
	//! * The expression `p(v)` must be convertible to `bool` for every argument `v` of type `VT`, where `VT` is the value
	//! type of `_InputIt`, and must not modify `v`.
	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr bool any_of(_InputIt first, _InputIt last, _UnaryPredicate p)
	{
		return find_if(first, last, p) != last;
	}

	//! Checks if the unary predicate returns `false` for all elements in the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] p The user-provided unary predicate which will be called to test elements.
	//! @return Returns `ture` if the unary predicate returns `false` for all elements in the range. Returns `false` otherwise.
	//! @par Valid Usage
	//! * The expression `p(v)` must be convertible to `bool` for every argument `v` of type `VT`, where `VT` is the value
	//! type of `_InputIt`, and must not modify `v`.
	template<typename _InputIt, typename _UnaryPredicate>
	inline constexpr bool none_of(_InputIt first, _InputIt last, _UnaryPredicate p)
	{
		return find_if(first, last, p) == last;
	}

	//! Applies the given function object to every element in the range, in order.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] f The user-defined unary function object to be applied.
	//! @return Returns `f`.
	//! @par Valid Usage
	//! * `f` must have the following function signature: `void f(const Type& v)`, where `Type` is the value type of `_InputIt`.
	template<typename _InputIt, typename _UnaryFunction>
	inline constexpr _UnaryFunction for_each(_InputIt first, _InputIt last, _UnaryFunction f)
	{
		for (; first != last; ++first) {
			f(*first);
		}
		return f; // implicit move since C++11
	}

	//! Sorts the elements in the range in non-descending order. The order of equal elements is not guaranteed to be preserved.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
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

	//! Sorts the elements in the range in non-descending order. The order of equal elements is not guaranteed to be preserved.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] comp The user-defined comparision function object, which returns `true` if the first argument is less than the second.
	//! @par Valid Usage
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of `_RandomIt`.
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

	//! Finds the first element in the range such that `value < element` is `true`.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @return Returns an iterator to the first element in the range such that `value < element` is `true`, 
	//! or `last` if no such element is found.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
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

	//! Finds the first element in the range such that `comp(value, element)` is `true`.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns an iterator to the first element in the range such that `comp(value, element)` is `true`,
	//! or `last` if no such element is found.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of `_ForwardIt`.
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

	//! Finds the first element in the range such that `element < value` is `false`.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @return Returns an iterator to the first element in the range such that `element < value` is `false`, 
	//! or `last` if no such element is found.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
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

	//! Finds the first element in the range such that `comp(element, value)` is `false`.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns an iterator to the first element in the range such that `comp(element, value)` is `false`, 
	//! or `last` if no such element is found.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of `_ForwardIt`.
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

	//! Checks if an element equivalent to the specified value appears within the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @return Returns `true` if an element equal to `value` is found, `false` otherwise.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	template<typename _ForwardIt, typename _Ty>
	bool binary_search(_ForwardIt first, _ForwardIt last, const _Ty& value)
	{
		first = lower_bound(first, last, value);
		return (!(first == last) && !(value < *first));
	}

	//! Checks if an element equivalent to the specified value appears within the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns `true` if an element equal to `value` is found, `false` otherwise.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of `_ForwardIt`.
	template<typename _ForwardIt, typename _Ty, typename _Compare>
	bool binary_search(_ForwardIt first, _ForwardIt last, const _Ty& value, _Compare comp)
	{
		first = lower_bound(first, last, value, comp);
		return (!(first == last) && !(comp(value, *first)));
	}

	//! Finds an element equivalent to the specified value in the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @return Returns an iterator to the found element. If multiple equivalent elements exist in the range, returns the first one.
	//! Returns `last` if not found.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	template<typename _ForwardIt, typename _Ty>
	_ForwardIt binary_search_iter(_ForwardIt first, _ForwardIt last, const _Ty& value)
	{
		first = lower_bound(first, last, value);
		return (!(first == last) && !(value < *first)) ? first : last;
	}

	//! Finds an element equivalent to the specified value in the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns an iterator to the found element. If multiple equivalent elements exist in the range, returns the first one.
	//! Returns `last` if not found.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of `_ForwardIt`.
	template<typename _ForwardIt, typename _Ty, typename _Compare>
	_ForwardIt binary_search_iter(_ForwardIt first, _ForwardIt last, const _Ty& value, _Compare comp)
	{
		first = lower_bound(first, last, value, comp);
		return (!(first == last) && !(comp(value, *first))) ? first : last;
	}

	//! Gets a range containing all elements equivalent to the specified value in the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @return Returns a pair of iterators pointing to the begin and end of the range. The first iterator points to the first element of the range,
	//! the second iterator points to the one-past-last element of the range. If the specified element is not found in the range, returns one pair of iterators
	//! that are equal to each other.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	template<typename _ForwardIt, typename _Ty>
	Pair<_ForwardIt, _ForwardIt>
		equal_range(_ForwardIt first, _ForwardIt last,
			const _Ty& value)
	{
		return make_pair(lower_bound(first, last, value),
			upper_bound(first, last, value));
	}

	//! Gets a range containing all elements equivalent to the specified value in the range.
	//! @param[in] first The iterator to the first element of the range.
	//! @param[in] last The iterator to the one-past-last element of the range.
	//! @param[in] value The value to compare elements to.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns a pair of iterators pointing to the begin and end of the range. The first iterator points to the first element of the range,
	//! the second iterator points to the one-past-last element of the range. If the specified element is not found in the range, returns one pair of iterators
	//! that are equal to each other.
	//! @par Valid Usage
	//! * Elements in the range specified by [`first`, `last`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of `_ForwardIt`.
	template<typename _ForwardIt, typename _Ty, typename _Compare>
	Pair<_ForwardIt, _ForwardIt>
		equal_range(_ForwardIt first, _ForwardIt last,
			const _Ty& value, _Compare comp)
	{
		return make_pair(lower_bound(first, last, value, comp),
			upper_bound(first, last, value, comp));
	}

	//! Checks if the sorted range [`first2`, `last2`) is a subsequence of the sorted range [`first1`, `last1`).
	//! (A subsequence need not be contiguous.)
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @return Returns `true` if the sorted range [`first2`, `last2`) is a subsequence of the sorted range [`first1`, `last1`).
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
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
	//! Checks if the sorted range [`first2`, `last2`) is a subsequence of the sorted range [`first1`, `last1`).
	//! (A subsequence need not be contiguous.)
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns `true` if the sorted range [`first2`, `last2`) is a subsequence of the sorted range [`first1`, `last1`).
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of both `_InputIt1` and `_InputIt2`.
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
	
	//! Copies elements from one range to another range.
	//! @param[in] first The iterator to the first element of the source range.
	//! @param[in] last The iterator to the one-past-last element of the source range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	template<typename _InputIt, typename _OutputIt>
	_OutputIt copy(_InputIt first, _InputIt last,
		_OutputIt d_first)
	{
		for (; first != last; (void)++first, (void)++d_first) {
			*d_first = *first;
		}
		return d_first;
	}

	//! Copies elements that pass user-defined function from one range to another range.
	//! The relative order of elements that are copied is preserved.
	//! @param[in] first The iterator to the first element of the source range.
	//! @param[in] last The iterator to the one-past-last element of the source range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @param[in] pred The user-defined unary predicate which returns `​true` for elements that should be copied.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	//! @par Valid Usage
	//! * The expression `pred(v)` must be convertible to `bool` for every argument `v` of type `VT`, where `VT` is the value
	//! type of `_InputIt`, and must not modify `v`.
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

	//! Copies elements that appear in the first sorted range and do not appear in the 
	//! second sorted range to the destination range. The destination range is also sorted.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each other and 
	//! [`first2`, `last2`) contains `n` elements that are equivalent to them, 
	//! the final `max(m - n, 0)` elements will be copied from [`first1`, `last1`) to the output range, 
	//! preserving order.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
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

	//! Copies elements that appear in the first sorted range and do not appear in the 
	//! second sorted range to the destination range. The destination range is also sorted.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each other and 
	//! [`first2`, `last2`) contains `n` elements that are equivalent to them, 
	//! the final `max(m - n, 0)` elements will be copied from [`first1`, `last1`) to the output range, 
	//! preserving order.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of both `_InputIt1` and `_InputIt2`.
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

	//! Constructs a sorted range consisting of elements that are found in both sorted ranges.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each otherand [`first2`, `last2`)
	//! contains `n` elements that are equivalent to them, the first `min(m, n)` elements will be copied from 
	//! [`first1`, `last1`) to the output range, preserving order.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
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

	//! Constructs a sorted range consisting of elements that are found in both sorted ranges.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each otherand [`first2`, `last2`)
	//! contains `n` elements that are equivalent to them, the first `min(m, n)` elements will be copied from 
	//! [`first1`, `last1`) to the output range, preserving order.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of both `_InputIt1` and `_InputIt2`.
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

	//! Computes symmetric difference of two sorted ranges: elements that are found in either of the ranges, 
	//! but not in both of them are copied to the destination range. The destination range is also sorted.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each otherand [`first2`, `last2`) contains 
	//! `n` elements that are equivalent to them, then `abs(m - n)` of those elements will be copied to the output range, preserving order:
	//! * if `m > n`, the final `m - n` of these elements from [`first1`, `last1`).
	//! * if `m < n`, the final `n - m` of these elements from [`first2`, `last2`).
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
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

	//! Computes symmetric difference of two sorted ranges: elements that are found in either of the ranges, 
	//! but not in both of them are copied to the destination range. The destination range is also sorted.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @return Returns one iterator to the one-past-last element of the destination range.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each otherand [`first2`, `last2`) contains 
	//! `n` elements that are equivalent to them, then `abs(m - n)` of those elements will be copied to the output range, preserving order:
	//! * if `m > n`, the final `m - n` of these elements from [`first1`, `last1`).
	//! * if `m < n`, the final `n - m` of these elements from [`first2`, `last2`).
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of both `_InputIt1` and `_InputIt2`.
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

	//! Constructs a sorted union beginning at the destination range consisting of the set of elements present in 
	//! one or both sorted ranges.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each otherand 
	//! [`first2`, `last2`) contains `n` elements that are equivalent to them, 
	//! then all `m` elements will be copied from [`first1`, `last1`) to the output range, preserving order, 
	//! and then the final `max(n - m, 0)` elements will be copied from [`first2`, `last2`) to the output range, 
	//! also preserving order.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
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

	//! Constructs a sorted union beginning at the destination range consisting of the set of elements present in 
	//! one or both sorted ranges.
	//! @param[in] first1 The iterator to the first element of the first range.
	//! @param[in] last1 The iterator to the one-past-last element of the first range.
	//! @param[in] first2 The iterator to the first element of the second range.
	//! @param[in] last2 The iterator to the one-past-last element of the second range.
	//! @param[in] d_first The iterator to the first element of the destination range.
	//! @param[in] comp The user-provided binary predicate which returns `​true` if the first argument is less than the second.
	//! @remark If [`first1`, `last1`) contains `m` elements that are equivalent to each otherand 
	//! [`first2`, `last2`) contains `n` elements that are equivalent to them, 
	//! then all `m` elements will be copied from [`first1`, `last1`) to the output range, preserving order, 
	//! and then the final `max(n - m, 0)` elements will be copied from [`first2`, `last2`) to the output range, 
	//! also preserving order.
	//! @par Valid Usage
	//! * Elements in the ranges specified by [`first1`, `last1`) and [`first2`, `last2`) must be sorted in non-descending order.
	//! * `comp` must have the following function signature: `bool comp(const Type& a, const Type& b)`, where `Type` is the value type of both `_InputIt1` and `_InputIt2`.
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

	//! @}
}