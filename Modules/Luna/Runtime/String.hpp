/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file String.hpp
* @author JXMaster
* @date 2020/7/31
*/
#pragma once
#include "Allocator.hpp"
#include "Algorithm.hpp"
#include "Iterator.hpp"
#include "MemoryUtils.hpp"
#include "TypeInfo.hpp"

namespace Luna
{
	namespace Impl
	{
		template <typename _Char>
		struct StringTraits;

		template <>
		struct StringTraits<c8>
		{
			static constexpr const c8* null_string = u8"";
		};

		template <>
		struct StringTraits<c16>
		{
			static constexpr const c16* null_string = u"";
		};

		template <>
		struct StringTraits<c32>
		{
			static constexpr const c32* null_string = U"";
		};

		template <>
		struct StringTraits<wchar_t>
		{
			static constexpr const wchar_t* null_string = L"";
		};
	}

	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeString String library
	//! @}

	//! @addtogroup RuntimeString
	//! @{

	//! The basic string implementation that is suitable for any character types.
	template <typename _Char, typename _Alloc = Allocator>
	class BasicString
	{
	public:
		using value_type = _Char;
		using allocator_type = _Alloc;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;

		//! A special value that usually represents the end of the string. The exact meaning of this value
		//! is content specific.
		static constexpr usize npos = (usize)-1;

		//! Constructs one empty string.
		BasicString();
		//! Constructs one empty string with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(const allocator_type& alloc);
		//! Constructs one string using `count` copies of character `ch`.
		//! @param[in] count The number of characters to add to the string.
		//! @param[in] ch The character to copy.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(usize count, value_type ch, const allocator_type& alloc = allocator_type());
		//! Constructs one string by coping characters in range from another string.
		//! @param[in] rhs The string to copy from.
		//! @param[in] pos The index of the first character to copy. Characters in range [`pos`, `rhs.size()`) of 
		//! `rhs` will be copied.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * `pos` must be smaller than `rhs.size()`.
		BasicString(const BasicString& rhs, usize pos, const allocator_type& alloc = allocator_type());
		//! Constructs one string by coping characters from another string.
		//! @param[in] rhs The string to copy from.
		//! @param[in] pos The index of the first character to copy.
		//! @param[in] count The number of characters to copy. Characters in range [`pos`, `pos + count`) of 
		//! `rhs` will be copied.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * `pos + count` must be not greater than `rhs.size()`.
		BasicString(const BasicString& rhs, usize pos, usize count, const allocator_type& alloc = allocator_type());
		//! Constructs one string by coping characters from the provided character array.
		//! @param[in] s The pointer to the first character to copy.
		//! @param[in] count The number of characters to copy.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * If `count` is not `0`, `s` must points to a valid character array with at least `count` characters.
		BasicString(const value_type* s, usize count, const allocator_type& alloc = allocator_type());
		//! Constructs one string by coping characters from the provided null-terminated C string. 
		//! The length of the string is determined by the first null character.
		//! @param[in] s The pointer to one null-terminated string, where characters are copied from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		BasicString(const value_type* s, const allocator_type& alloc = allocator_type());
		//! Constructs one string by coping characters from string specified by the iterator range.
		//! @param[in] first The iterator to the first character to be copied.
		//! @param[in] last The iterator to the one-past-last character to be copied.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		template <typename _InputIt>
		BasicString(_InputIt first, _InputIt last, const allocator_type& alloc = allocator_type());
		//! Constructs one string by coping data from another string.
		//! @param[in] rhs The string to copy data from.
		BasicString(const BasicString& rhs);
		//! Constructs one string by coping data from another string.
		//! @param[in] rhs The string to copy data from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(const BasicString& rhs, const allocator_type& alloc);
		//! Constructs one string by moving data from another string.
		//! @param[in] rhs The string to move data from. This string will be empty after this operation.
		BasicString(BasicString&& rhs);
		//! Constructs one string by moving data from another string.
		//! @param[in] rhs The string to move data from. This string will be empty after this operation.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(BasicString&& rhs, const allocator_type& alloc);
		//! Constructs one string by coping characters from the initializer list.
		//! @param[in] ilist The initializer list to copy characters from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(InitializerList<value_type> ilist, const allocator_type& alloc = allocator_type());
		//! Assigns the string by coping data from another string.
		//! @param[in] rhs The string to copy data from.
		//! @return Returns `*this`.
		BasicString& operator=(const BasicString& rhs);
		//! Assigns the string by moving data from another string.
		//! @param[in] rhs The string to move data from.
		//! @return Returns `*this`.
		BasicString& operator=(BasicString&& rhs);
		//! Assigns the string by coping characters from the provided null-terminated C string. 
		//! The length of the string is determined by the first null character.
		//! @param[in] s The pointer to the first character to copy.
		//! @return Returns `*this`.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		BasicString& operator=(const value_type* s);
		//! Assigns the string with the specified character. This operation behaves the same as 
		//! `assign(1, ch)`.
		//! @param[in] ch The character to assign.
		//! @return Returns `*this`.
		BasicString& operator=(value_type ch);
		//! Assigns the string by coping characters from the initializer list.
		//! @param[in] ilist The initializer list to copy characters from.
		//! @return Returns `*this`.
		BasicString& operator=(InitializerList<value_type> ilist);
		~BasicString();
		//! Gets one pointer to the underlying character data.
		//! @return Returns one pointer to the underlying character data. The returned pointer may be `nullptr`
		//! if the string is empty, to ensure one non-null return value, use @ref c_str instead, which always 
		//! return one valid but empty string buffer ("") if the string is empty.
		pointer data();
		//! Gets one constant pointer to the underlying character data.
		//! @return Returns one constant pointer to the underlying character data. The returned pointer may be `nullptr`
		//! if the string is empty, to ensure one non-null return value, use @ref c_str instead, which always 
		//! return one valid but empty string buffer ("") if the string is empty.
		const_pointer data() const;
		//! Gets a non-modifiable C string pointer to the characters stored by this string.
		//! @return Returns the C string pointer to the characters stored by this string. 
		//! The returned pointer will never be `nullptr`. If this string is empty, one valid pointer to a null character
		//! is returned.
		const_pointer c_str() const;
		//! Gets one iterator to the first character of this string.
		//! @return Returns one iterator to the first character of this string.
		iterator begin();
		//! Gets one iterator to the one-past-last character of this string.
		//! @return Returns one iterator to the one-past-last character of this string.
		iterator end();
		//! Gets one constnat iterator to the first character of this string.
		//! @return Returns one constant iterator to the first character of this string.
		const_iterator begin() const;
		//! Gets one constant iterator to the one-past-last character of this string.
		//! @return Returns one constant iterator to the one-past-last character of this string.
		const_iterator end() const;
		//! Gets one constnat iterator to the first character of this string.
		//! @return Returns one constant iterator to the first character of this string.
		const_iterator cbegin() const;
		//! Gets one constant iterator to the one-past-last character of this string.
		//! @return Returns one constant iterator to the one-past-last character of this string.
		const_iterator cend() const;
		//! Gets one reverse iterator to the last character of this string.
		//! @return Returns one reverse iterator to the last character of this string.
		reverse_iterator rbegin();
		//! Gets one reverse iterator to the one-before-first character of this string.
		//! @return Returns one reverse iterator to the one-before-first character of this string.
		reverse_iterator rend();
		//! Gets one constant reverse iterator to the last character of this string.
		//! @return Returns one constant reverse iterator to the last character of this string.
		const_reverse_iterator rbegin() const;
		//! Gets one constant reverse iterator to the one-before-first character of this string.
		//! @return Returns one constant reverse iterator to the one-before-first character of this string.
		const_reverse_iterator rend() const;
		//! Gets one constant reverse iterator to the last character of this string.
		//! @return Returns one constant reverse iterator to the last character of this string.
		const_reverse_iterator crbegin() const;
		//! Gets one constant reverse iterator to the one-before-first character of this string.
		//! @return Returns one constant reverse iterator to the one-before-first character of this string.
		const_reverse_iterator crend() const;
		//! Gets the size of the string, that is, the number of characters in the string, excluding the 
		//! null terminator.
		//! @return Returns the size of the string.
		usize size() const;
		//! Gets the size of the string, that is, the number of characters in the string, excluding the 
		//! null terminator.
		//! @return Returns the size of the string.
		usize length() const;
		//! Gets the capacity of the string, that is, the maximum number of characters that can be stored 
		//! in this string without reallocating the string buffer.
		//! @return Returns the capacity of the string.
		usize capacity() const;
		//! Checks whether this string is empty, that is, the size of this string is `0`.
		//! @return Returns `true` if this string is empty, returns `false` otherwise.
		bool empty() const;
		//! Increases the capacity of the string to a value greater than or equal to `new_cap`, so that it can 
		//! hold at least `new_cap` characters without reallocating the string buffer.
		//! @details If `new_cap` is smaller than or equal to @ref capacity, this function does nothing.
		//! @param[in] new_cap The new capacity value to reserve.
		void reserve(usize new_cap);
		//! Resizes the string.
		//! @param[in] n The new size of the string.
		//! 
		//! If `n` is greater than @ref size, `n - size()` copied of `v` will be inserted to the back of the string.
		//! 
		//! If `n` is smaller than @ref size, `size() - n` characters will be removed from the back of the string.
		//! 
		//! If `n` is equal to @ref size, this function does nothing.
		//! @param[in] v The character to insert if `n` is greater than @ref size.
		void resize(usize n, value_type v);
		//! Reduces the capacity of the string so that @ref capacity == @ref size.
		//! @details If @ref size is `0`, this function releases the internal string buffer. This can be
		//! used to clean up all dynamic memory allocated by this string.
		void shrink_to_fit();
		//! Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		reference operator[] (usize n);
		//! Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one constant reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		const_reference operator[] (usize n) const;
		//! Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		reference at(usize n);
		//! Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one constant reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		const_reference at(usize n) const;
		//! Gets the first character of the string.
		//! @details The first character is the character with index `0`.
		//! @return Returns one reference to the first character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		reference front();
		//! Gets the first character of the string.
		//! @details The first character is the character with index `0`.
		//! @return Returns one constant reference to the first character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		const_reference front() const;
		//! Gets the last character of the string.
		//! @details The last character is the character with index `size() - 1`.
		//! @return Returns one reference to the last character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		reference back();
		//! Gets the last character of the string.
		//! @details The last character is the character with index `size() - 1`.
		//! @return Returns one constant reference to the last character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		const_reference back() const;
		//! Removes all characters from the string, but keeps the string buffer.
		//! @details The user can call @ref shrink_to_fit after this to free the string buffer.
		void clear();
		//! Pushes one character to the back of the string.
		//! @param[in] val The character to push.
		void push_back(value_type ch);
		//! Removes the element from the back of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		void pop_back();
		//! Assigns the string data by `count` copies of `ch`.
		//! @param[in] count The string size.
		//! @param[in] ch The character used to fill the string.
		void assign(usize count, value_type ch);
		//! Assigns the string data by coping characters from another string.
		//! @param[in] str The string to copy characters from.
		void assign(const BasicString& str);
		//! Assigns the string data by coping characters from one subrange of another string.
		//! @param[in] str The string to copy characters from.
		//! @param[in] pos The index of the first character to copy.
		//! @param[in] count The number of characters to copy. If this is @ref npos, strings in
		//! range [`pos`, `str.size()`) will be copied.
		//! @par Valid Usage
		//! * `pos + count` must be not greater than `str.size()`.
		void assign(const BasicString& str, usize pos, usize count = npos);
		//! Assigns the string data by moving characters from another string.
		//! @param[in] str The string to move data characters from. This string will be empty after this 
		//! operation.
		void assign(BasicString&& str);
		//! Assigns the string data by coping characters from the provided character array.
		//! @param[in] s The pointer to the first character to copy.
		//! @param[in] count The number of characters to copy.
		//! @par Valid Usage
		//! * If `count` is not `0`, `s` must points to a valid character array with at least `count` characters.
		void assign(const value_type* s, usize count);
		//! Assigns the string data by coping characters from the provided null-terminated C string. 
		//! The length of the string is determined by the first null character.
		//! @param[in] s The pointer to one null-terminated string, where characters are copied from.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		void assign(const value_type* s);
		//! Assigns the string data by coping characters from string specified by the iterator range.
		//! @param[in] first The iterator to the first character to be copied.
		//! @param[in] last The iterator to the one-past-last character to be copied.
		template <typename _InputIt>
		void assign(_InputIt first, _InputIt last);
		//! Assigns the string data by coping characters from the initializer list.
		//! @param[in] ilist The initializer list to copy characters from.
		void assign(InitializerList<value_type> ilist);
		//! Inserts `count` copies of characters in the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] count The number of characters to insert.
		//! @param[in] ch The character to insert.
		void insert(usize index, usize count, value_type ch);
		//! Inserts one null-terminated C string at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] s The pointer to one null-terminated string, where characters are copied from.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		void insert(usize index, const value_type* s);
		//! Inserts one character array at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] s The pointer to the first character to insert.
		//! @param[in] count The number of characters to insert.
		//! @par Valid Usage
		//! * If `count` is not `0`, `s` must points to a valid character array with at least `count` characters.
		void insert(usize index, const value_type* s, usize count);
		//! Inserts another string at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] str The string to insert.
		void insert(usize index, const BasicString& str);
		//! Inserts one subrange of another string at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] str The string to insert.
		//! @param[in] index_str The index of the first character in `str` to insert.
		//! @param[in] count The number of characters to insert.
		//! If `count` is greater than `str.size() - index_str`, only `str.size() - index_str` characters will be inserted.
		void insert(usize index, const BasicString& str, usize index_str, usize count = npos);
		//! Inserts one character at the specified position.
		//! @param[in] pos The iterator to the position to insert.
		//! @param[in] ch The character to insert.
		//! @return Returns one iterator to the inserted character.
		iterator insert(const_iterator pos, value_type ch);
		//! Inserts `count` copies of `ch` at the specified position.
		//! @param[in] pos The iterator to the position to insert.
		//! @param[in] count The number of characters to insert.
		//! @param[in] ch The character to insert.
		//! @return Returns one iterator to the first inserted character.
		iterator insert(const_iterator pos, usize count, value_type ch);
		//! Inserts characters from the iterator range at the specified position.
		//! @param[in] pos The iterator to the position to insert.
		//! @param[in] first The iterator to the first character to be inserted.
		//! @param[in] last The iterator to the one-past-last character to be inserted.
		template <typename _InputIt>
		iterator insert(const_iterator pos, _InputIt first, _InputIt last);
		//! Inserts characters from the specified initializer list at the specified position.
		//! @param[in] pos The iterator to the position to insert.
		//! @param[in] ilist The initializer list of characters to be inserted.
		iterator insert(const_iterator pos, InitializerList<value_type> ilist);
		//! Removes `count` characters from the specified position.
		//! @param[in] index The index of the first character to remove.
		//! @param[in] count The number of characters to remove.
		//! If `count` is greater than `size() - index`, only `size() - index` characters will be removed.
		void erase(usize index = 0, usize count = npos);
		//! Removes one character.
		//! @param[in] pos The iterator to the character to remove.
		//! @return Returns one iterator to the next character after the removed character,
		//! or `end()` if such character does not exist.
		//! @par Valid Usage
		//! * `pos` must specifies one valid character in the string.
		iterator erase(const_iterator pos);
		//! Removes characters in the specified range.
		//! @param[in] first The iterator to the first character to remove.
		//! @param[in] last The iterator to the one-past-last character to remove.
		//! @return Returns one iterator to the next character after the removed characters,
		//! or `end()` if such character does not exist.
		//! @par Valid Usage
		//! * `first` and `last` must specifies one valid character range [`first`, `last`) in the string.
		iterator erase(const_iterator first, const_iterator last);
		//! Swaps characters of this string with the specified string.
		//! @param[in] rhs The string to swap characters with.
		void swap(BasicString& rhs);
		//! Appends `count` copies of character `ch` to the back of the string.
		//! @param[in] count The number of character copies to append.
		//! @param[in] ch The character to append.
		void append(usize count, value_type ch);
		//! Appends another string to the back of the string.
		//! @param[in] str The string to append.
		void append(const BasicString& str);
		//! Appends one subrange of another string to the back of the string.
		//! @param[in] str The string to append.
		//! @param[in] index_str The index of the first character in `str` to append.
		//! @param[in] count The number of characters to append.
		//! If `count` is greater than `str.size() - pos`, only `str.size() - pos` characters will be appended.
		void append(const BasicString& str, usize pos, usize count = npos);
		//! Appends one character array to the back of the string.
		//! @param[in] s The pointer to the first character to append.
		//! @param[in] count The number of characters to append.
		//! @par Valid Usage
		//! * If `count` is not `0`, `s` must points to a valid character array with at least `count` characters.
		void append(const value_type* s, usize count);
		//! Appends one null-terminated C string to the back of the string.
		//! @param[in] s The pointer to one null-terminated string, where characters are copied from.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		void append(const value_type* s);
		//! Appends characters from the iterator range to the back of the string.
		//! @param[in] first The iterator to the first character to be appended.
		//! @param[in] last The iterator to the one-past-last character to be appended.
		template <typename _InputIt>
		void append(_InputIt first, _InputIt last);
		//! Appends characters from the specified initializer list to the back of the string.
		//! @param[in] ilist The initializer list of characters to be appended.
		void append(InitializerList<value_type> ilist);
		//! Compares this string with the specified string.
		//! @param[in] rhs The string to compare with.
		//! @return Returns `0` if both character sequences compare equivalent.
		//! 
		//! Returns negative value if `*this` appears before the character sequence specified by `rhs`, in lexicographical order.
		//! 
		//! Returns positive value if `*this` appears after the character sequence specified by `rhs`, in lexicographical order.
		i32 compare(const BasicString& rhs) const;
		//! Compares [`pos1`, `pos1 + count1`) substring of this string to `rhs`.
		//! @param[in] pos1 The index of the first character in `*this` to compare.
		//! @param[in] count1 The number of characters in `*this` to compare.
		//! If `pos1 + count1` is greater than `this->size()`, `count1` will be clamped to `this->size() - pos1`.
		//! @param[in] rhs The string to compare with.
		//! @return Returns `0` if both character sequences compare equivalent.
		//! 
		//! Returns negative value if `*this` appears before the character sequence specified by `rhs`, in lexicographical order.
		//! 
		//! Returns positive value if `*this` appears after the character sequence specified by `rhs`, in lexicographical order.
		//! @par Valid Usage
		//! * `pos1` must not be greater than `this->size()`.
		i32 compare(usize pos1, usize count1, const BasicString& rhs) const;
		//! Compares a [`pos1`, `pos1 + count1`) substring of this string to a substring [`pos2`, `pos2 + count2`) of `rhs`.
		//! @param[in] pos1 The index of the first character in `*this` to compare.
		//! @param[in] count1 The number of characters in `*this` to compare.
		//! If `pos1 + count1` is greater than `this->size()`, `count1` will be clamped to `this->size() - pos1`.
		//! @param[in] rhs The string to compare with.
		//! @param[in] pos2 The index of the first character in `rhs` to compare.
		//! @param[in] count2 The number of characters in `rhs` to compare.
		//! If `pos2 + count2` is greater than `rhs.size()`, `count2` will be clamped to `rhs.size() - pos2`.
		//! @return Returns `0` if both character sequences compare equivalent.
		//! 
		//! Returns negative value if `*this` appears before the character sequence specified by `rhs`, in lexicographical order.
		//! 
		//! Returns positive value if `*this` appears after the character sequence specified by `rhs`, in lexicographical order.
		//! @par Valid Usage
		//! * `pos1` must not be greater than `this->size()`.
		//! * `pos2` must not be greater than `rhs.size()`.
		i32 compare(usize pos1, usize count1, const BasicString& rhs, usize pos2, usize count2 = npos) const;
		//! Compares this string to the null-terminated C string.
		//! @param[in] s The null-terminated C string to compare with.
		//! @return Returns `0` if both character sequences compare equivalent.
		//! 
		//! Returns negative value if `*this` appears before the character sequence specified by `rhs`, in lexicographical order.
		//! 
		//! Returns positive value if `*this` appears after the character sequence specified by `rhs`, in lexicographical order.
		i32 compare(const value_type* s) const;
		//! Compares [`pos1`, `pos1 + count1`) substring of this string to the null-terminated C string.
		//! @param[in] pos1 The index of the first character in `*this` to compare.
		//! @param[in] count1 The number of characters in `*this` to compare.
		//! If `pos1 + count1` is greater than `this->size()`, `count1` will be clamped to `this->size() - pos1`.
		//! @param[in] s The null-terminated C string to compare with.
		//! @return Returns `0` if both character sequences compare equivalent.
		//! 
		//! Returns negative value if `*this` appears before the character sequence specified by `rhs`, in lexicographical order.
		//! 
		//! Returns positive value if `*this` appears after the character sequence specified by `rhs`, in lexicographical order.
		//! @par Valid Usage
		//! * `pos1` must not be greater than `this->size()`.
		//! * `s` must points to a valid null-terminated string.
		i32 compare(usize pos1, usize count1, const value_type* s) const;
		//! Compares [`pos1`, `pos1 + count1`) substring of this string to the characters in the range [`s`, `s + count2`).
		//! @param[in] pos1 The index of the first character in `*this` to compare.
		//! @param[in] count1 The number of characters in `*this` to compare.
		//! If `pos1 + count1` is greater than `this->size()`, `count1` will be clamped to `this->size() - pos1`.
		//! @param[in] s The string to compare with.
		//! @param[in] count2 The number of characters in `s` to compare.
		//! @par Valid Usage
		//! * `pos1` must not be greater than `this->size()`.
		//! * If `count2` is not `0`, `s` must points to a valid character array with at least `count2` characters.
		i32 compare(usize pos1, usize count1, const value_type* s, usize count2) const;
		//! Replaces characters in range [`pos`, `pos + count`) with characters of `str`.
		//! @param[in] pos The index of the first character to replace.
		//! @param[in] count The number of characters to replace.
		//! If `pos + count` is greater than `this->size()`, `count` will be clamped to `this->size() - pos`.
		//! @param[in] str The string to use for replacement.
		//! @par Valid Usage
		//! * `pos` must not be greater than `this->size()`.
		void replace(usize pos, usize count, const BasicString& str);
		//! Replaces characters in range [`first`, `last`) with characters of `str`.
		//! @param[in] first The iterator to the first character to replace.
		//! @param[in] last The iterator to the one-past-last character to replace.
		//! @param[in] str The string to use for replacement.
		//! @par Valid Usage
		//! * [`first`, `last`) must specify a valid range of this string.
		void replace(const_iterator first, const_iterator last, const BasicString& str);
		//! Replaces characters in range [`pos`, `pos + count`) with a substring [`pos2`, `pos2 + count2`) of `str`.
		//! @param[in] pos The index of the first character to replace.
		//! @param[in] count The number of characters to replace.
		//! If `pos + count` is greater than `this->size()`, `count` will be clamped to `this->size() - pos`.
		//! @param[in] str The string to use for replacement.
		//! @param[in] pos2 The index of the first character in `rhs` to use for replacement.
		//! @param[in] count2 The number of characters in `rhs` to use for replacement.
		//! If `pos2 + count2` is greater than `str.size()`, `count2` will be clamped to `str.size() - pos2`.
		//! @par Valid Usage
		//! * `pos` must not be greater than `this->size()`.
		//! * `pos2` must not be greater than `str.size()`.
		void replace(usize pos, usize count, const BasicString& str, usize pos2, usize count2 = npos);
		//! Replaces characters in range [`first`, `last`) with the characters in the range [`first2`, `last2`).
		//! @param[in] first The iterator to the first character to replace.
		//! @param[in] last The iterator to the one-past-last character to replace.
		//! @param[in] first2 The iterator to the first character to use for replacement.
		//! @param[in] last2 The iterator to the one-past-last character use for replacement.
		//! @par Valid Usage
		//! * [`first`, `last`) must specify a valid range of this string.
		template <typename _InputIt>
		void replace(const_iterator first, const_iterator last, _InputIt first2, _InputIt last2);
		//! Replaces characters in range [`pos`, `pos + count`) with a character array [`cstr`, `cstr + count2).
		//! @param[in] pos The index of the first character to replace.
		//! @param[in] count The number of characters to replace.
		//! If `pos + count` is greater than `this->size()`, `count` will be clamped to `this->size() - pos`.
		//! @param[in] cstr The pointer to the character array to use for replacement.
		//! @param[in] count2 The number of characters in `cstr` to use for replacement.
		//! @par Valid Usage
		//! * `pos` must not be greater than `this->size()`.
		void replace(usize pos, usize count, const value_type* cstr, usize count2);
		//! Replaces characters in range [`first`, `last`) with a character array [`cstr`, `cstr + count2).
		//! @param[in] first The iterator to the first character to replace.
		//! @param[in] last The iterator to the one-past-last character to replace.
		//! @param[in] cstr The pointer to the character array to use for replacement.
		//! @param[in] count2 The number of characters in `cstr` to use for replacement.
		//! @par Valid Usage
		//! * [`first`, `last`) must specify a valid range of this string.
		void replace(const_iterator first, const_iterator last, const value_type* cstr, usize count2);
		//! Replaces characters in range [`pos`, `pos + count`) with a null-terminated C string.
		//! @param[in] pos The index of the first character to replace.
		//! @param[in] count The number of characters to replace.
		//! If `pos + count` is greater than `this->size()`, `count` will be clamped to `this->size() - pos`.
		//! @param[in] cstr The pointer to the null-terminated C string to use for replacement.
		//! @par Valid Usage
		//! * `pos` must not be greater than `this->size()`.
		//! * `cstr` must points to a valid null-terminated string.
		void replace(usize pos, usize count, const value_type* cstr);
		//! Replaces characters in range [`first`, `last`) with a null-terminated C string.
		//! @param[in] first The iterator to the first character to replace.
		//! @param[in] last The iterator to the one-past-last character to replace.
		//! @param[in] cstr The pointer to the null-terminated C string to use for replacement.
		//! @par Valid Usage
		//! * [`first`, `last`) must specify a valid range of this string.
		//! * `cstr` must points to a valid null-terminated string.
		void replace(const_iterator first, const_iterator last, const value_type* cstr);
		//! Replaces characters in range [`pos`, `pos + count`) with `count2` copies of character `ch`.
		//! @param[in] pos The index of the first character to replace.
		//! @param[in] count The number of characters to replace.
		//! If `pos + count` is greater than `this->size()`, `count` will be clamped to `this->size() - pos`.
		//! @param[in] count2 The number of characters to use for replacement.
		//! @param[in] ch The character to use for replacement.
		//! @par Valid Usage
		//! * `pos` must not be greater than `this->size()`.
		void replace(usize pos, usize count, usize count2, value_type ch);
		//! Replaces characters in range [`first`, `last`) with `count2` copies of character `ch`.
		//! @param[in] first The iterator to the first character to replace.
		//! @param[in] last The iterator to the one-past-last character to replace.
		//! @param[in] count2 The number of characters to use for replacement.
		//! @param[in] ch The character to use for replacement.
		//! @par Valid Usage
		//! * [`first`, `last`) must specify a valid range of this string.
		void replace(const_iterator first, const_iterator last, usize count2, value_type ch);
		//! Replaces characters in range [`first`, `last`) with characters from the specified initializer list.
		//! @param[in] first The iterator to the first character to replace.
		//! @param[in] last The iterator to the one-past-last character to replace.
		//! @param[in] ilist The initializer list with the characters to use for replacement.
		void replace(const_iterator first, const_iterator last, InitializerList<value_type> ilist);
		//! Creates a substring of this string.
		//! @param[in] pos The index of the first character to include in the substring.
		//! @param[in] count The number of characters to include in the substring.
		//! If `pos + count` is greater than `this->size()`, `count` will be clamped to `this->size() - pos`.
		//! @return Returns the created substring.
		//! @par Valid Usage
		//! * `pos` must not be greater than `this->size()`.
		BasicString substr(usize pos = 0, usize count = npos) const;
		//! Copies a substring [`pos`, `pos + count`) to character string pointed to by `dst`.
		//! @param[in] dst The pointer to the destination character string.
		//! @param[in] count The number of characters to copy.
		//! If `pos + count` is greater than `this->size()`, `count` will be clamped to `this->size() - pos`.
		//! @param[in] pos The index of the first character to copy.
		//! @return Returns number of characters copied.
		//! @par Valid Usage
		//! * `pos` must not be greater than `this->size()`.
		usize copy(value_type* dst, usize count, usize pos = 0) const;
		//! Gets the allocator of the string.
		//! @return Returns one copy of the allocator of the string.
		allocator_type get_allocator() const;
		//! Finds the first occurrence of the specified character sequence in the string.
		//! @param[in] str The string that holds the character sequence to search.
		//! @param[in] pos The index at which to start the search.
		//! @return Returns the index of the first character of the found occurrence. Returns @ref npos if no
		//! such occurrence is found.
		usize find(const BasicString& str, usize pos = 0) const;
		//! Finds the first occurrence of the specified character sequence in the string.
		//! @param[in] s The string that holds the character sequence to search.
		//! @param[in] pos The index at which to start the search.
		//! @param[in] count The number of characters to search. Character in range [`s`, `s + count`)
		//! will be used for searching.
		//! @return Returns the index of the first character of the found occurrence. Returns @ref npos if no
		//! such occurrence is found.
		usize find(const value_type* s, usize pos, usize count) const;
		//! Finds the first occurrence of the specified character sequence in the string.
		//! @param[in] s The string that holds the character sequence to search. Characters in range [`s`, `s + strlen(s)`)
		//! will be used for searching.
		//! @param[in] pos The index at which to start the search.
		//! @return Returns the index of the first character of the found occurrence. Returns @ref npos if no
		//! such occurrence is found.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		usize find(const value_type* s, usize pos = 0) const;
		//! Finds the first occurrence of the specified character in the string.
		//! @param[in] ch The character to search for.
		//! @param[in] pos The index at which to start the search.
		//! @return Returns the index of the found character. Returns @ref npos if no
		//! such occurrence is found.
		usize find(value_type ch, usize pos = 0) const;
		//! Finds the last occurrence of the specified character sequence in the string.
		//! @param[in] str The string that holds the character sequence to search.
		//! @param[in] pos The index at which to begin searching. The character at `pos` is included in searching.
		//! If `pos >= size()` (including @ref npos), the search will start from the end of the string.
		//! @return Returns the index of the first character of the found occurrence. The index is an offset from the
		//! start of the string, not the end.
		//! Returns @ref npos if no such occurrence is found.
		//! 
		//! If `str.empty()` is `true`, returns `pos` if `pos < size()`, returns `size()` otherwise.
		//!
		//! @ref npos is always returned if `size() == 0`, even if `str.empty()` is `true`.
		usize rfind(const BasicString& str, usize pos = npos) const;
		//! Finds the last occurrence of the specified character sequence in the string.
		//! @param[in] s The string that holds the character sequence to search.
		//! @param[in] pos The index at which to begin searching. The character at `pos` is included in searching.
		//! If `pos >= size()` (including @ref npos), the search will start from the end of the string.
		//! @param[in] count The number of characters to search. Character in range [`s`, `s + count`)
		//! will be used for searching.
		//! @return Returns the index of the first character of the found occurrence. The index is an offset from the
		//! start of the string, not the end.
		//! Returns @ref npos if no such occurrence is found.
		//! 
		//! If `count` is `0`, returns `pos` if `pos < size()`, returns `size()` otherwise.
		//!
		//! @ref npos is always returned if `size() == 0`, even if `count` is `0`.
		usize rfind(const value_type* s, usize pos, usize count) const;
		//! Finds the last occurrence of the specified character sequence in the string.
		//! @param[in] s The string that holds the character sequence to search. Characters in range [`s`, `s + strlen(s)`)
		//! will be used for searching.
		//! @param[in] pos The index at which to begin searching. The character at `pos` is included in searching.
		//! If `pos >= size()` (including @ref npos), the search will start from the end of the string.
		//! @return Returns the index of the first character of the found occurrence. The index is an offset from the
		//! start of the string, not the end.
		//! Returns @ref npos if no such occurrence is found.
		//! 
		//! If `strlen(s)` is `0`, returns `pos` if `pos < size()`, returns `size()` otherwise.
		//!
		//! @ref npos is always returned if `size() == 0`, even if `strlen(s)` is `0`.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		usize rfind(const value_type* s, usize pos = 0) const;
		//! Finds the first occurrence of the specified character in the string.
		//! @param[in] ch The character to search for.
		//! @param[in] pos The index at which to begin searching. The character at `pos` is included in searching.
		//! If `pos >= size()` (including @ref npos), the search will start from the end of the string.
		//! @return Returns the index of the found character. Returns @ref npos if no
		//! such occurrence is found.
		usize rfind(value_type ch, usize pos = npos) const;

	private:
		// -------------------- Begin of ABI compatible part --------------------
		OptionalPair<allocator_type, _Char*> m_allocator_and_buffer;// The memory buffer.
		usize m_size;			// Number of elements in the vector.
		usize m_capacity;		// Number of elements that can be included in the buffer before a reallocation is needed.
		// --------------------  End of ABI compatible part  --------------------

		value_type* allocate(usize n);
		void deallocate(value_type* ptr, usize n);

		// Frees all dynamic memory.
		void free_buffer();
		usize strlength(const _Char* s);
		void internal_expand_reserve(usize new_least_cap);
	};

	//! The string that contains @ref c8 characters.
	//! @details See @ref BasicString for string documentation.
	using String = BasicString<c8>;
	//! The string that contains wchat_t characters.
	//! @details See @ref BasicString for string documentation.
	using WString = BasicString<wchar_t>;
	//! The string that contains @ref c16 characters.
	//! @details See @ref BasicString for string documentation.
	using String16 = BasicString<c16>;
	//! The string that contains @ref c32 characters.
	//! @details See @ref BasicString for string documentation.
	using String32 = BasicString<c32>;

	//! Gets the type object of @ref String.
	//! @return Returns the type object of @ref String.
	LUNA_RUNTIME_API typeinfo_t string_type();
	template <> struct typeof_t<String> { typeinfo_t operator()() const { return string_type(); } };

	//! @}
}

#include "Impl/String.inl"