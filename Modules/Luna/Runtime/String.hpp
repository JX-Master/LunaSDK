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

	//! @brief The basic string implementation that is suitable for any character types.
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

		//! @brief A special value that usually represents the end of the string. The exact meaning of this value
		//! is content specific.
		static constexpr usize npos = (usize)-1;

		//! @brief Constructs one empty string.
		BasicString();
		//! @brief Constructs one empty string with an custom allocator.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(const allocator_type& alloc);
		//! @brief Constructs one string using `count` copies of character `ch`.
		//! @param[in] count The number of characters to add to the string.
		//! @param[in] ch The character to copy.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(usize count, value_type ch, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one string by coping characters in range from another string.
		//! @param[in] rhs The string to copy from.
		//! @param[in] pos The index of the first character to copy. Characters in range [`pos`, `rhs.size()`) of 
		//! `rhs` will be copied.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * `pos` must be smaller than `rhs.size()`.
		BasicString(const BasicString& rhs, usize pos, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one string by coping characters from another string.
		//! @param[in] rhs The string to copy from.
		//! @param[in] pos The index of the first character to copy.
		//! @param[in] count The number of characters to copy. Characters in range [`pos`, `pos + count`) of 
		//! `rhs` will be copied.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * `pos + count` must be not greater than `rhs.size()`.
		BasicString(const BasicString& rhs, usize pos, usize count, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one string by coping characters from the provided character array.
		//! @param[in] s The pointer to the first character to copy.
		//! @param[in] count The number of characters to copy.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * If `count` is not `0`, `s` must points to a valid character array with at least `count` characters.
		BasicString(const value_type* s, usize count, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one string by coping characters from the provided null-terminated C string. 
		//! The length of the string is determined by the first null character.
		//! @param[in] s The pointer to one null-terminated string, where characters are copied from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		BasicString(const value_type* s, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one string by coping characters from string specified by the iterator range.
		//! @param[in] first The iterator to the first character to be copied.
		//! @param[in] last The iterator to the one-past-last character to be copied.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		template <typename _InputIt>
		BasicString(_InputIt first, _InputIt last, const allocator_type& alloc = allocator_type());
		//! @brief Constructs one string by coping data from another string.
		//! @param[in] rhs The string to copy data from.
		BasicString(const BasicString& rhs);
		//! @brief Constructs one string by coping data from another string.
		//! @param[in] rhs The string to copy data from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(const BasicString& rhs, const allocator_type& alloc);
		//! @brief Constructs one string by moving data from another string.
		//! @param[in] rhs The string to move data from. This string will be empty after this operation.
		BasicString(BasicString&& rhs);
		//! @brief Constructs one string by moving data from another string.
		//! @param[in] rhs The string to move data from. This string will be empty after this operation.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(BasicString&& rhs, const allocator_type& alloc);
		//! @brief Constructs one string by coping characters from the initializer list.
		//! @param[in] ilist The initializer list to copy characters from.
		//! @param[in] alloc The allocator to use. The allocator object will be copy-constructed into the map.
		BasicString(InitializerList<value_type> ilist, const allocator_type& alloc = allocator_type());
		//! @brief Assigns the string by coping data from another string.
		//! @param[in] rhs The string to copy data from.
		//! @return Returns `*this`.
		BasicString& operator=(const BasicString& rhs);
		//! @brief Assigns the string by moving data from another string.
		//! @param[in] rhs The string to move data from.
		//! @return Returns `*this`.
		BasicString& operator=(BasicString&& rhs);
		//! @brief Assigns the string by coping characters from the provided null-terminated C string. 
		//! The length of the string is determined by the first null character.
		//! @param[in] s The pointer to the first character to copy.
		//! @return Returns `*this`.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		BasicString& operator=(const value_type* s);
		//! @brief Assigns the string with the specified character. This operation behaves the same as 
		//! `assign(1, ch)`.
		//! @param[in] ch The character to assign.
		//! @return Returns `*this`.
		BasicString& operator=(value_type ch);
		//! @brief Assigns the string by coping characters from the initializer list.
		//! @param[in] ilist The initializer list to copy characters from.
		//! @return Returns `*this`.
		BasicString& operator=(InitializerList<value_type> ilist);
		~BasicString();
		//! @brief Gets one pointer to the underlying character data.
		//! @return Returns one pointer to the underlying character data. The returned pointer may be `nullptr`
		//! if the string is empty, to ensure one non-null return value, use @ref c_str instead, which always 
		//! return one valid but empty string buffer ("") if the string is empty.
		pointer data();
		//! @brief Gets one constant pointer to the underlying character data.
		//! @return Returns one constant pointer to the underlying character data. The returned pointer may be `nullptr`
		//! if the string is empty, to ensure one non-null return value, use @ref c_str instead, which always 
		//! return one valid but empty string buffer ("") if the string is empty.
		const_pointer data() const;
		//! @brief Gets a non-modifiable C string pointer to the characters stored by this string.
		//! @return Returns the C string pointer to the characters stored by this string. 
		//! The returned pointer will never be `nullptr`. If this string is empty, one valid pointer to a null character
		//! is returned.
		const_pointer c_str() const;
		//! @brief Gets one iterator to the first character of this string.
		//! @return Returns one iterator to the first character of this string.
		iterator begin();
		//! @brief Gets one iterator to the one-past-last character of this string.
		//! @return Returns one iterator to the one-past-last character of this string.
		iterator end();
		//! @brief Gets one constnat iterator to the first character of this string.
		//! @return Returns one constant iterator to the first character of this string.
		const_iterator begin() const;
		//! @brief Gets one constant iterator to the one-past-last character of this string.
		//! @return Returns one constant iterator to the one-past-last character of this string.
		const_iterator end() const;
		//! @brief Gets one constnat iterator to the first character of this string.
		//! @return Returns one constant iterator to the first character of this string.
		const_iterator cbegin() const;
		//! @brief Gets one constant iterator to the one-past-last character of this string.
		//! @return Returns one constant iterator to the one-past-last character of this string.
		const_iterator cend() const;
		//! @brief Gets one reverse iterator to the last character of this string.
		//! @return Returns one reverse iterator to the last character of this string.
		reverse_iterator rbegin();
		//! @brief Gets one reverse iterator to the one-before-first character of this string.
		//! @return Returns one reverse iterator to the one-before-first character of this string.
		reverse_iterator rend();
		//! @brief Gets one constant reverse iterator to the last character of this string.
		//! @return Returns one constant reverse iterator to the last character of this string.
		const_reverse_iterator rbegin() const;
		//! @brief Gets one constant reverse iterator to the one-before-first character of this string.
		//! @return Returns one constant reverse iterator to the one-before-first character of this string.
		const_reverse_iterator rend() const;
		//! @brief Gets one constant reverse iterator to the last character of this string.
		//! @return Returns one constant reverse iterator to the last character of this string.
		const_reverse_iterator crbegin() const;
		//! @brief Gets one constant reverse iterator to the one-before-first character of this string.
		//! @return Returns one constant reverse iterator to the one-before-first character of this string.
		const_reverse_iterator crend() const;
		//! @brief Gets the size of the string, that is, the number of characters in the string, excluding the 
		//! null terminator.
		//! @return Returns the size of the string.
		usize size() const;
		//! @brief Gets the size of the string, that is, the number of characters in the string, excluding the 
		//! null terminator.
		//! @return Returns the size of the string.
		usize length() const;
		//! @brief Gets the capacity of the string, that is, the maximum number of characters that can be stored 
		//! in this string without reallocating the string buffer.
		//! @return Returns the capacity of the string.
		usize capacity() const;
		//! @brief Checks whether this string is empty, that is, the size of this string is `0`.
		//! @return Returns `true` if this string is empty, returns `false` otherwise.
		bool empty() const;
		//! @brief Increases the capacity of the string to a value greater than or equal to `new_cap`, so that it can 
		//! hold at least `new_cap` characters without reallocating the string buffer.
		//! @details If `new_cap` is smaller than or equal to @ref capacity, this function does nothing.
		//! @param[in] new_cap The new capacity value to reserve.
		void reserve(usize new_cap);
		//! @brief Resizes the string.
		//! @param[in] n The new size of the string.
		//! 
		//! If `n` is greater than @ref size, `n - size()` copied of `v` will be inserted to the back of the string.
		//! 
		//! If `n` is smaller than @ref size, `size() - n` characters will be removed from the back of the string.
		//! 
		//! If `n` is equal to @ref size, this function does nothing.
		//! @param[in] v The character to insert if `n` is greater than @ref size.
		void resize(usize n, value_type v);
		//! @brief Reduces the capacity of the string so that @ref capacity == @ref size.
		//! @details If @ref size is `0`, this function releases the internal string buffer. This can be
		//! used to clean up all dynamic memory allocated by this string.
		void shrink_to_fit();
		//! @brief Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		reference operator[] (usize n);
		//! @brief Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one constant reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		const_reference operator[] (usize n) const;
		//! @brief Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		reference at(usize n);
		//! @brief Gets the character at the specified index.
		//! @param[in] n The index of the character.
		//! @return Returns one constant reference to the character at the specified index.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		//! * `n` must be in range [`0`, `size()`).
		const_reference at(usize n) const;
		//! @brief Gets the first character of the string.
		//! @details The first character is the character with index `0`.
		//! @return Returns one reference to the first character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		reference front();
		//! @brief Gets the first character of the string.
		//! @details The first character is the character with index `0`.
		//! @return Returns one constant reference to the first character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		const_reference front() const;
		//! @brief Gets the last character of the string.
		//! @details The last character is the character with index `size() - 1`.
		//! @return Returns one reference to the last character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		reference back();
		//! @brief Gets the last character of the string.
		//! @details The last character is the character with index `size() - 1`.
		//! @return Returns one constant reference to the last character of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		const_reference back() const;
		//! @brief Removes all characters from the string, but keeps the string buffer.
		//! @details The user can call @ref shrink_to_fit after this to free the string buffer.
		void clear();
		//! @brief Pushes one character to the back of the string.
		//! @param[in] val The character to push.
		void push_back(value_type ch);
		//! @brief Removes the element from the back of the string.
		//! @par Valid Usage
		//! * @ref empty must be `false` when calling this function.
		void pop_back();
		//! @brief Assigns the string data by `count` copies of `ch`.
		//! @param[in] count The string size.
		//! @param[in] ch The character used to fill the string.
		void assign(usize count, value_type ch);
		//! @brief Assigns the string data by coping characters from another string.
		//! @param[in] str The string to copy characters from.
		void assign(const BasicString& str);
		//! @brief Assigns the string data by coping characters from one subrange of another string.
		//! @param[in] str The string to copy characters from.
		//! @param[in] pos The index of the first character to copy.
		//! @param[in] count The number of characters to copy. If this is @ref npos, strings in
		//! range [`pos`, `str.size()`) will be copied.
		//! @par Valid Usage
		//! * `pos + count` must be not greater than `str.size()`.
		void assign(const BasicString& str, usize pos, usize count = npos);
		//! @brief Assigns the string data by moving characters from another string.
		//! @param[in] str The string to move data characters from. This string will be empty after this 
		//! operation.
		void assign(BasicString&& str);
		//! @brief Assigns the string data by coping characters from the provided character array.
		//! @param[in] s The pointer to the first character to copy.
		//! @param[in] count The number of characters to copy.
		//! @par Valid Usage
		//! * If `count` is not `0`, `s` must points to a valid character array with at least `count` characters.
		void assign(const value_type* s, usize count);
		//! @brief Assigns the string data by coping characters from the provided null-terminated C string. 
		//! The length of the string is determined by the first null character.
		//! @param[in] s The pointer to one null-terminated string, where characters are copied from.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		void assign(const value_type* s);
		//! @brief Assigns the string data by coping characters from string specified by the iterator range.
		//! @param[in] first The iterator to the first character to be copied.
		//! @param[in] last The iterator to the one-past-last character to be copied.
		template <typename _InputIt>
		void assign(_InputIt first, _InputIt last);
		//! @brief Assigns the string data by coping characters from the initializer list.
		//! @param[in] ilist The initializer list to copy characters from.
		void assign(InitializerList<value_type> ilist);
		//! @brief Inserts `count` copies of characters in the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] count The number of characters to insert.
		//! @param[in] ch The character to insert.
		void insert(usize index, usize count, value_type ch);
		//! @brief Inserts one null-terminated C string at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] s The pointer to one null-terminated string, where characters are copied from.
		//! @par Valid Usage
		//! * `s` must points to a valid null-terminated string.
		void insert(usize index, const value_type* s);
		//! @brief Inserts one character array at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] s The pointer to the first character to insert.
		//! @param[in] count The number of characters to insert.
		//! @par Valid Usage
		//! * If `count` is not `0`, `s` must points to a valid character array with at least `count` characters.
		void insert(usize index, const value_type* s, usize count);
		//! @brief Inserts another string at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] str The string to insert.
		void insert(usize index, const BasicString& str);
		//! @brief Inserts one subrange of another string at the specified position.
		//! @param[in] index The index to insert the characters.
		//! @param[in] str The string to insert.
		//! @param[in] index_str The index of the first character in `str` to insert.
		//! @param[in] count The number of characters to insert.
		void insert(usize index, const BasicString& str, usize index_str, usize count);
		//! @brief Inserts one character at the specified position.
		//! @param[in] pos The iterator to the position to insert.
		//! @param[in] ch The character to insert.
		//! @return Returns one iterator to the inserted character.
		iterator insert(const_iterator pos, value_type ch);
		//! @brief Inserts `count` copies of `ch` at the specified position.
		//! @param[in] pos The iterator to the position to insert.
		//! @param[in] count The number of characters to insert.
		//! @param[in] ch The character to insert.
		//! @return Returns one iterator to the first inserted character.
		iterator insert(const_iterator pos, usize count, value_type ch);
		template <typename _InputIt>
		iterator insert(const_iterator pos, _InputIt first, _InputIt last);
		iterator insert(const_iterator pos, InitializerList<value_type> ilist);
		void erase(usize index = 0, usize count = npos);
		iterator erase(const_iterator pos);
		iterator erase(const_iterator first, const_iterator last);
		void swap(BasicString& rhs);
		void append(usize count, value_type ch);
		void append(const BasicString& str);
		void append(const BasicString& str, usize pos, usize count = npos);
		void append(const value_type* s, usize count);
		void append(const value_type* s);
		template <typename _InputIt>
		void append(_InputIt first, _InputIt last);
		void append(InitializerList<value_type> ilist);
		BasicString& operator+=(const BasicString& str);
		BasicString& operator+=(value_type ch);
		BasicString& operator+=(const value_type* s);
		BasicString& operator+=(InitializerList<value_type> ilist);
		i32 compare(const BasicString& rhs) const;
		i32 compare(usize pos1, usize count1, const BasicString& rhs) const;
		i32 compare(usize pos1, usize count1, const BasicString& rhs, usize pos2, usize count2 = npos) const;
		i32 compare(const value_type* s) const;
		i32 compare(usize pos1, usize count1, const value_type* s) const;
		i32 compare(usize pos1, usize count1, const value_type* s, usize count2) const;
		void replace(usize pos, usize count, const BasicString& str);
		void replace(const_iterator first, const_iterator last, const BasicString& str);
		void replace(usize pos, usize count, const BasicString& str, usize pos2, usize count2 = npos);
		template <typename _InputIt>
		void replace(const_iterator first, const_iterator last, _InputIt first2, _InputIt last2);
		void replace(usize pos, usize count, const value_type* cstr, usize count2);
		void replace(const_iterator first, const_iterator last, const value_type* cstr, usize count2);
		void replace(usize pos, usize count, const value_type* cstr);
		void replace(const_iterator first, const_iterator last, const value_type* cstr);
		void replace(usize pos, usize count, usize count2, value_type ch);
		void replace(const_iterator first, const_iterator last, usize count2, value_type ch);
		void replace(const_iterator first, const_iterator last, InitializerList<value_type> ilist);
		BasicString substr(usize pos = 0, usize count = npos) const;
		usize copy(value_type* dst, usize count, usize pos = 0) const;
		allocator_type get_allocator() const;

		usize find(const BasicString& str, usize pos = 0) const;
		usize find(const value_type* s, usize pos, usize count) const;
		usize find(const value_type* s, usize pos = 0) const;
		usize find(value_type ch, usize pos = 0) const;
		usize rfind(const BasicString& str, usize pos = npos) const;
		usize rfind(const value_type* s, usize pos, usize count) const;
		usize rfind(const value_type* s, usize pos = 0) const;
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

	using String = BasicString<c8>;
	using WString = BasicString<wchar_t>;
	using String16 = BasicString<c16>;
	using String32 = BasicString<c32>;

	LUNA_RUNTIME_API typeinfo_t string_type();
	template <> struct typeof_t<String> { typeinfo_t operator()() const { return string_type(); } };

	//! @}
}

#include "Impl/String.inl"