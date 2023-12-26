/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Variant.hpp
* @author JXMaster
* @date 2021/12/3
*/
#pragma once
#include "Base.hpp"
#include "Vector.hpp"
#include "Name.hpp"
#include "Blob.hpp"
#include "HashMap.hpp"

namespace Luna
{
	//! @addtogroup Runtime
	//! @{

	//! Defines all possible types of one @ref Variant object.
	//! @remark To fetch the type of one variant, call @ref Variant::type
	enum class VariantType : u8
	{
		//! Indicates a null variant. 
		//! @details A null variant represents the absence of value for the variant object.
		null = 0,
		//! Indicates an object variant.
		//! @details An object variant contains one set of @ref Variant objects, which acts as children of the current variant. 
		//! Children in object variant are indexed by @ref Name strings, and do not have a particular order.
		object = 1,
		//! Indicates an array variant.
		//! @details An array variant contains one array of @ref Variant objects, which acts as children of the current variant.
		array = 2,
		//! Indicates a number variant.
		//! @details A number variant stores a number of integer or floating-point type. The number type of one number variant is represented by @ref VariantNumberType 
		//! and can be fetched by calling @ref Variant::number_type method. The number value of the variant can be fetched by calling @ref Variant::unum, 
		//! @ref Variant::inum and @ref Variant::fnum methods, each of them returns the underlying number in specified format with implicit number type conversion when needed.	
		number = 3,
		//! Indicates a string variant.
		//! @details A string variant contains one string represented by a @ref Name object. You can fetch the underlying string by calling @ref Variant::str or @ref Variant::c_str, 
		//! the former one returns one @ref Name object while the later one returns one C-style string (`const c8*`).
		string = 4,
		//! Indicates a Boolean variant.
		//! @details A Boolean variant stores a Boolean value with only two possible values: `true` and `false`. 
		//! The Boolean value of one Boolean variant can be fetched by calling @ref Variant::boolean method.
		boolean = 5,
		//! Indicates a BLOB (binary large object) variant.
		//! @details A BLOB variant can store arbitrary binary data. The data pointer, size and alignment of the data can be fetched by calling @ref Variant::blob_data, @ref Variant::blob_size 
		//! and @ref Variant::blob_alignment methods.
		blob = 6
	};

	//! Defines all possible number types of one number variant.
	//! @remark To fetch the number type of one number variant, call @ref Variant::number_type.
	enum class VariantNumberType : u8
	{
		//! This variant is not a number variant.
		not_number = 0,
		//! The number is stored in signed 64-bit integer format.
		number_i64 = 1,
		//! The number is stored in unsigned 64-bit integer format.
		number_u64 = 2,
		//! The number is stored in 64-bit floating-point format.
		number_f64 = 3,
	};

	//! Represents a dynamic typed object that stores data in a schema-less (self-described) manner. 
	class Variant
	{
	public:
		class ArrayEnumerator
		{
		public:
			friend class Variant;
			using iterator = Variant*;
			using const_iterator = const Variant*;
			iterator begin()
			{
				return m_begin;
			}
			const_iterator begin() const
			{
				return m_begin;
			}
			const_iterator cbegin() const
			{
				return m_begin;
			}
			iterator end()
			{
				return m_end;
			}
			const_iterator end() const
			{
				return m_end;
			}
			const_iterator cend() const
			{
				return m_end;
			}
		private:
			iterator m_begin;
			iterator m_end;
		};
		class ConstArrayEnumerator
		{
		public:
			friend class Variant;
			using const_iterator = const Variant*;
			const_iterator begin() const
			{
				return m_begin;
			}
			const_iterator cbegin() const
			{
				return m_begin;
			}
			const_iterator end() const
			{
				return m_end;
			}
			const_iterator cend() const
			{
				return m_end;
			}
		private:
			const_iterator m_begin;
			const_iterator m_end;
		};
		class ObjectEnumerator;
		template <bool _Const>
		struct ObjectIterator
		{
			using value_type = Pair<const Name, Variant>;
			using pointer = conditional_t<_Const, const value_type*, value_type*>;
			using reference = conditional_t<_Const, const value_type&, value_type&>;
			using iterator_category = forward_iterator_tag;

			using small_iter = conditional_t<_Const, const value_type*, value_type*>;
			using big_iter = conditional_t<_Const, HashMap<Name, Variant>::const_iterator, HashMap<Name, Variant>::iterator>;

			ObjectIterator(small_iter iter) :
				m_is_big(false),
				m_small_iter(iter) {}
			ObjectIterator(big_iter big_iter) :
				m_is_big(true),
				m_big_iter(big_iter) {}
			ObjectIterator(const ObjectIterator<false>& rhs) :
				m_is_big(rhs.m_is_big)
			{
				if (m_is_big)
				{
					m_big_iter = rhs.m_big_iter;
				}
				else
				{
					m_small_iter = rhs.m_small_iter;
				}
			}
			reference operator*() const
			{
				if (m_is_big)
				{
					return *m_big_iter;
				}
				else
				{
					return *m_small_iter;
				}
			}
			pointer operator->() const
			{
				if (m_is_big)
				{
					return m_big_iter.operator->();
				}
				else
				{
					return m_small_iter;
				}
			}
			ObjectIterator& operator++()
			{
				if (m_is_big)
				{
					++m_big_iter;
				}
				else
				{
					++m_small_iter;
				}
				return *this;
			}
			ObjectIterator operator++(int)
			{
				ObjectIterator temp(*this);
				if (m_is_big)
				{
					temp.m_big_iter++;
				}
				else
				{
					temp.m_small_iter++;
				}
				return temp;
			}
			bool operator==(const ObjectIterator& rhs) const
			{
				if (m_is_big != rhs.m_is_big) return false;
				return m_is_big ? m_big_iter == rhs.m_big_iter : m_small_iter == rhs.m_small_iter;
			}
			bool operator!=(const ObjectIterator& rhs) const
			{
				return !(*this == rhs);
			}

		private:
			bool m_is_big;
			union
			{
				small_iter m_small_iter;
				big_iter m_big_iter;
			};
		};
		class ObjectEnumerator
		{
		public:
			friend class Variant;
			using iterator = ObjectIterator<false>;
			using const_iterator = ObjectIterator<true>;
			iterator begin();
			const_iterator begin() const
			{
				return cbegin();
			}
			const_iterator cbegin() const;
			iterator end();
			const_iterator end() const
			{
				return cend();
			}
			const_iterator cend() const;
		private:
			Variant* m_value;
		};
		class ConstObjectEnumerator
		{
		public:
			friend class Variant;
			using const_iterator = ObjectIterator<true>;
			const_iterator begin() const
			{
				return cbegin();
			}
			const_iterator cbegin() const;
			const_iterator end() const
			{
				return cend();
			}
			const_iterator cend() const;
		private:
			const Variant* m_value;
		};
		using value_enumerator = ArrayEnumerator;
		using const_value_enumerator = ConstArrayEnumerator;
		using key_value_enumerator = ObjectEnumerator;
		using const_key_value_enumerator = ConstObjectEnumerator;

		//! Initializes one empty variant with the specified variant type.
		//! @param[in] type The of the new variant. If not specified, one variant with @ref VariantType::null will be constructed.
		Variant(VariantType type = VariantType::null);
		//! Initializes one variant by coping data from another variant.
		//! @param[in] rhs The variant to copy from.
		Variant(const Variant& rhs);
		//! Initializes one variant by moving data from another variant.
		//! @param[in] rhs The variant to move from.
		Variant(Variant&& rhs);
		//! Initializes one variant of @ref VariantType::number and @ref VariantNumberType::number_i64 and sets its data to the specified value.
		//! @param[in] v The number value to set.
		Variant(i64 v);
		//! Initializes one variant of @ref VariantType::number and @ref VariantNumberType::number_u64 and sets its data to the specified value.
		//! @param[in] v The number value to set.
		Variant(u64 v);
		//! Initializes one variant of @ref VariantType::number and @ref VariantNumberType::number_f64 and sets its data to the specified value.
		//! @param[in] v The number value to set.
		Variant(f64 v);
		//! Initializes one variant of @ref VariantType::string and sets its data to the specified value.
		//! @param[in] v The string to set.
		Variant(const Name& v);
		//! Initializes one variant of @ref VariantType::string and sets its data to the specified value.
		//! @param[in] v The string to set.
		Variant(Name&& v);
		//! Initializes one variant of @ref VariantType::string and sets its data to the specified value.
		//! @param[in] v The string to set.
		Variant(const c8* v);
		//! Initializes one variant of @ref VariantType::boolean and sets its data to the specified value.
		//! @param[in] v The Boolean value to set.
		Variant(bool v);
		//! Initializes one variant of @ref VariantType::blob and sets its data to the specified value.
		//! @param[in] blob_data The blob data to set. The blob data will be copied into the variant.
		Variant(const Blob& blob_data);
		//! Initializes one variant of @ref VariantType::blob and sets its data to the specified value.
		//! @param[in] blob_data The blob data to set. The blob data will be moved into the variant.
		Variant(Blob&& blob_data);

		~Variant();

		//! Assigns one variant by coping data from another variant.
		//! @param[in] rhs The variant to copy from.
		//! @return Returns `*this`.
		Variant& operator=(const Variant& rhs);
		//! Assigns one variant by moving data from another variant.
		//! @param[in] rhs The variant to move from.
		//! @return Returns `*this`.
		Variant& operator=(Variant&& rhs);
		//! Assigns one variant with one number.
		//! The variant type will be @ref VariantType::number and @ref VariantNumberType::number_u64 after this assignment.
		//! @param[in] v The number value to set.
		//! @return Returns `*this`.
		Variant& operator=(u64 v);
		//! Assigns one variant with one number.
		//! The variant type will be @ref VariantType::number and @ref VariantNumberType::number_i64 after this assignment.
		//! @param[in] v The number value to set.
		//! @return Returns `*this`.
		Variant& operator=(i64 v);
		//! Assigns one variant with one number.
		//! The variant type will be @ref VariantType::number and @ref VariantNumberType::number_f64 after this assignment.
		//! @param[in] v The number value to set.
		//! @return Returns `*this`.
		Variant& operator=(f64 v);
		//! Assigns one variant with one string.
		//! The variant type will be @ref VariantType::string after this assignment.
		//! @param[in] v The string to set.
		//! @return Returns `*this`.
		Variant& operator=(const Name& v);
		//! Assigns one variant with one string.
		//! The variant type will be @ref VariantType::string after this assignment.
		//! @param[in] v The string to set.
		//! @return Returns `*this`.
		Variant& operator=(Name&& v);
		//! Assigns one variant with one string.
		//! The variant type will be @ref VariantType::string after this assignment.
		//! @param[in] v The string to set.
		//! @return Returns `*this`.
		Variant& operator=(const c8* v);
		//! Assigns one variant with one Boolean value.
		//! The variant type will be @ref VariantType::boolean after this assignment.
		//! @param[in] v The Boolean value to set.
		//! @return Returns `*this`.
		Variant& operator=(bool v);
		//! Assigns one variant with one blob.
		//! The variant type will be @ref VariantType::blob after this assignment.
		//! @param[in] blob_data The blob data to set. The blob data will be copied into the variant.
		//! @return Returns `*this`.
		Variant& operator=(const Blob& blob_data);
		//! Assigns one variant with one blob.
		//! The variant type will be @ref VariantType::blob after this assignment.
		//! @param[in] blob_data The blob data to set. The blob data will be moved into the variant.
		//! @return Returns `*this`.
		Variant& operator=(Blob&& blob_data);
		//! Compares two variant for equality.
		//! @param[in] rhs The variant to compare.
		//! @return Returns `true` if two variants are equal. Returns `false` otherwise.
		//! @remark The comparison is proceeded as follows:
		//! 1. If `this->type() != ths.type()`, returns `false`.
		//! 2. Otherwise, checks the type of the variant:
		//! 	1. If `type()` is `VariantType::null`, returns `true`.
		//! 	2. If `type()` is `VariantType::object`, returns `true` if two variants have the same key-value pairs. Every element of `rhs` will be compared with the element with the same key in `*this` recursively.
		//! 	3. If `type()` is `VariantType::array`, returns `true` if two variants have the same array data and size. Every element will be compared with the element with the same index in `*this` recursively.
		//! 	4. If `type()` is `VariantType::number`, returns `true` if two variants have the same number type and value.
		//! 	5. If `type()` is `VariantType::string`, returns `true` if two variants have the same string data. The string data is compared by comparing the underlying @ref Name objects that contain the string.
		//! 	6. If `type()` is `VariantType::boolean`, returns `true` if two variants have the same Boolean value.
		//! 	7. If `type()` is `VariantType::blob`, returns `true` if two variants have the same blob data. The blob data is compared using @ref memcmp.
		bool operator==(const Variant& rhs) const;
		//! Compares two variant for non-equality.
		//! @details See remarks of @ref operator== for details.
		//! @param[in] rhs The variant to compare.
		//! @return Returns `true` if two variants are not equal. Returns `false` otherwise.
		bool operator!=(const Variant& rhs) const;

		//! Gets the type of the variant.
		//! @return Returns the type of the variant.
		VariantType type() const;
		//! Gets the number type of the variant.
		//! @return Returns the number type of the variant. 
		//! Returns @ref VariantType::not_number if @ref type of the variant is not @ref VariantType::number, 
		VariantNumberType number_type() const;
		//! Checks whether the variant is valid.
		//! @return Returns `true` if @ref type of the variant is not @ref VariantType::null. Returns `false` otherwise.
		bool valid() const;
		//! Checks whether the variant contains no child variant.
		//! @return Returns `true` if @ref size of the variant is `0`. Returns `false` otherwise.
		bool empty() const;
		
		//! Gets the child variant when this is an array variant.
		//! @param[in] i The index of the child variant to fetch.
		//! @return Returns one reference to the child variant at the specified index.
		//! Returns @ref npos if the current variant is not an array variant, or if the index is invalid.
		const Variant& at(usize i) const;
		//! Gets the child variant when this is an array variant.
		//! @param[in] i The index of the child variant to fetch.
		//! @return Returns one reference to the child variant at the specified index.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * `i` must be smaller than @ref size of the variant.
		Variant& at(usize i);
		//! Gets the child variant when this is an object variant.
		//! @param[in] k The key string of the child variant to fetch.
		//! @return Returns the child variant with the specified key.
		//! Returns @ref npos if the current variant is not an object variant, or if the key is not found.
		const Variant& find(const Name& k) const;
		//! Gets the child variant with the specified key, or inserts one new child variant with that key if not exists.
		//! @param[in] k The key string of the child variant to fetch or insert.
		//! @return Returns one reference to the found or inserted child variant.
		//! @remark The variant inserted by this function will have @ref VariantType::null type.
		//! @par Valid Usage
		//! * The current variant must be an object variant or a null variant. If the variant is a null variant, it will
		//! be transformed into one object variant before the new key-variant pair is inserted.
		Variant& find_or_insert(const Name& k);
		//! Shortcut for @ref at.
		const Variant& operator[](usize i) const;
		//! Shortcut for @ref at.
		Variant& operator[](usize i);
		//! Shortcut for @ref find.
		const Variant& operator[](const Name& k) const;
		//! Shortcut for @ref find_or_insert.
		Variant& operator[](const Name& k);
		//! Gets the number of the child variants of this variant.
		//! @return Returns the number of the child variants of this variant.
		//! Returns `0` if this variant is not an array or object variant.
		usize size() const;
		//! Checks whether one child variant with the specified key exists.
		//! @return Returns `true` if the current variant is a object variant, and the child variant with the specified key exists.
		//! Returns `false` otherwise.
		bool contains(const Name& k) const;
		//! Gets one enumerator that can be used to enumerate all child variants of one array variant.
		//! @return Returns one enumerator that can be used to enumerate all child variants.
		//! Returns one enumerator with an empty range if @ref type of the variant is not @ref VariantType::array.
		//! @remark The returned enumerator will have `begin` and `end` methods that can be used like all other containers in Luna SDK. You can
		//! also use the enumerator in a range-based for loop like so:
		//! ```
		//! for (auto i : variant.values())
		//! {
		//! 	//...
		//! }
		//! ```
		value_enumerator values();
		//! Gets one enumerator that can be used to enumerate all child variants of one array variant.
		//! @return Returns one enumerator that can be used to enumerate all child variants.
		//! Returns one enumerator with an empty range if @ref type of the variant is not @ref VariantType::array.
		//! @remark The returned enumerator will have `begin` and `end` methods that can be used like all other containers in Luna SDK. You can
		//! also use the enumerator in a range-based for loop like so:
		//! ```
		//! for (auto& i : variant.values())
		//! {
		//! 	//...
		//! }
		//! ```
		const_value_enumerator values() const;
		//! Gets one enumerator to enumerate all child key-variant pairs of one object variant.
		//! @return Returns one enumerator to enumerate all child key-variant pairs.
		//! Returns one enumerator with an empty range if @ref type of the variant is not @ref VariantType::object.
		//! @remark The returned enumerator will have `begin` and `end` methods that can be used like all other containers in Luna SDK. You can
		//! also use the enumerator in a range-based for loop like so:
		//! ```
		//! for (auto& i : variant.key_values())
		//! {
		//! 	i.first; // key.
		//!		i.second; // value.
		//! }s
		//! ```
		key_value_enumerator key_values();
		//! Gets one enumerator to enumerate all child key-variant pairs of one object variant.
		//! @return Returns one enumerator to enumerate all child key-variant pairs.
		//! Returns one enumerator with an empty range if @ref type of the variant is not @ref VariantType::object.
		//! @remark The returned enumerator will have `begin` and `end` methods that can be used like all other containers in Luna SDK. You can
		//! also use the enumerator in a range-based for loop like so:
		//! ```
		//! for (auto& i : variant.key_values())
		//! {
		//! 	i.first; // key.
		//!		i.second; // value.
		//! }s
		//! ```
		const_key_value_enumerator key_values() const;

		//! Copy-inserts one variant to the specified index.
		//! @param[in] i The index to insert the variant to.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant. 
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		//! * `i` must be smaller to equal to @ref size.
		void insert(usize i, const Variant& val);
		//! Move-inserts one variant to the specified index.
		//! @param[in] i The index to insert the variant to.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		//! * `i` must not be greater than @ref size.
		void insert(usize i, Variant&& val);
		//! Copy-inserts one variant to the end of the variant array.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		void push_back(const Variant& val);
		//! Move-inserts one variant to the end of the variant array.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		void push_back(Variant&& val);
		//! Removes one child variant from the current variant.
		//! @param[in] i The index of the child variant to remove.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * `i` must be smaller than @ref size.
		void erase(usize i);
		//! Erases one range [begin, end) of child variants from the current variant.
		//! @param[in] begin The index of first child variant to remove.
		//! @param[in] end The index of one-past-last child variant to remove.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * `begin` must not be greater than `end`.
		//! * `end` must not be greater than @ref size.
		void erase(usize begin, usize end);
		//! Erases the last child variant from the child variants array of the current variant.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * This variant must not be @ref empty.
		void pop_back();
		//! Copy-inserts the variant with the specified key as the child variant of the current variant.
		//! @param[in] k The key string of the variant to insert.
		//! @param[in] val The variant to insert.
		//! @return Returns `true` if the variant is successfully inserted. Returns `false` if one variant with the 
		//! specified key already exists, in such case, the existing variant is not modified.
		//! @par Valid Usage
		//! * The current variant must be an object variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty object variant before the insertion is performed.
		bool insert(const Name& k, const Variant& val);
		//! Move-inserts the variant with the specified key as the child variant of the current variant.
		//! @param[in] k The key string of the variant to insert.
		//! @param[in] val The variant to insert.
		//! @return Returns `true` if the variant is successfully inserted. Returns `false` if one variant with the 
		//! specified key already exists, in such case, the existing variant is not modified.
		//! @par Valid Usage
		//! * The current variant must be an object variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty object variant before the insertion is performed.
		bool insert(const Name& k, Variant&& val);
		//! Removes the child variant with the specified key from the current variant.
		//! @param[in] k The key string of the variant to remove.
		//! @return Returns `true` if one child variant with the specified key is successfully removed. Returns `false` otherwise.
		//! @par Valid Usage
		//! * The current variant must be an object variant.
		bool erase(const Name& k);

		//! Gets the string of one string variant.
		//! @param[in] default_value The optional default string to return.
		//! @return Returns the string data of one variant if @ref type is @ref VariantType::string.
		//! Returns `default_value` otherwise.
		Name str(const Name& default_value = Name()) const;
		//! Gets the C string of one string variant.
		//! @param[in] default_value The optional default string to return.
		//! @return Returns the string data of one variant if @ref type is @ref VariantType::string.
		//! Returns `default_value` otherwise.
		const c8* c_str(const c8* default_value = "") const;
		//! Gets the data of one number variant as one signed 64-bit integer.
		//! @param[in] default_value The optional default number to return.
		//! @return Returns the value as one signed 64-bit integer. If the value is in another number format, it will be 
		//! converted automatically. Returns `default_value` if the variant is not a number variant.
		i64 inum(i64 default_value = 0) const;
		//! Gets the data of one number variant as one unsigned 64-bit integer.
		//! @param[in] default_value The optional default number to return.
		//! @return Returns the value as one unsigned 64-bit integer. If the value is in another number format, it will be 
		//! converted automatically. Returns `default_value` if the variant is not a number variant.
		u64 unum(u64 default_value = 0) const;
		//! Gets the data of one number variant as one 64-bit floating-point number.
		//! @param[in] default_value The optional default number to return.
		//! @return Returns the value as one 64-bit floating-point number. If the value is in another number format, it will be 
		//! converted automatically. Returns `default_value` if the variant is not a number variant.
		f64 fnum(f64 default_value = 0) const;
		//! Gets the data of one Boolean variant.
		//! @param[in] default_value The optional default Boolean value to return.
		//! @return Returns the data of one Boolean variant. Returns `default_value` if the variant is not a Boolean variant.
		bool boolean(bool default_value = false) const;
		//! Gets the data pointer of one BLOB variant.
		//! @return Returns the data pointer if the variant is a BLOB variant with @ref blob_size greater than `0`. 
		//! Returns `nullptr` otherwise.
		byte_t* blob_data();
		//! Gets the data pointer of one BLOB variant.
		//! @return Returns the data pointer if the variant is a BLOB variant with @ref blob_size greater than `0`. 
		//! Returns `nullptr` otherwise.
		const byte_t* blob_data() const;
		//! Gets the data size, in bytes, of one BLOB variant.
		//! @return Returns the size of the data if the variant is a BLOB variant. Returns 0 otherwise.
		usize blob_size() const;
		//! Gets the data alignment, in bytes, of one BLOB variant.
		//! @return Returns the alignment of the data if the variant is a BLOB variant. Returns 0 otherwise.
		usize blob_alignment() const;
		//! Detaches and gets the data of one BLOB variant as a @ref Blob object. 
		//! @details After this operation, this variant is still a BLOB variant, but contains no binary data ( @ref blob_data returns
		//! `nullptr`, and @ref blob_size returns `0`).
		//! @return Returns the detached blob data if the variant is a BLOB variant. Returns one empty @ref Blob object otherwise.
		Blob blob_detach();

		//! Gets one reference to one global constant variant that contains no data.
		//! @details This is used as the default return value if one query operation fails, so that the user can chain multiple `[]` operations
		//! like data["persons"][0]["name"] without the need to handle null variant explicitly (query child variants of one null variant also returns `npos`).
		//! @return Returns one reference to one global constant variant that contains no data.
		static LUNA_RUNTIME_API const Variant& npos();
		
	private:
		friend class ObjectEnumerator;
		friend class ConstObjectEnumerator;
		// We use the real hash map to store the object (marked by `big_object`) if its children count is greater than this.
		static constexpr usize BIG_OBJECT_THRESHOLD = U8_MAX;
		enum class BlobFlag : u8
		{
			none = 0,
			big_blob = 0x01,	// For blob whose size >= 4G or alignment > MAX_ALIGN.
		};
		enum class ObjectFlag : u8
		{
			none = 0,
			big_object = 0x01, // For objects whose children count >= 65536.
		};
		enum class ArrayFlag : u8
		{
			none = 0,
			big_array = 0x01, // For array whose children count >= 65536.
		};

		// One variant value takes 16 bytes, with the following memory layout:
		//  0 : 7	: Data type
		//  8 : 15	: Flags based on the type.
		// 16 : 31	: Reserved (unused)
		// 32 : 63	: Blob size (for blobs whose size < 4GB) or CompoundTypeHeader for small array or object.
		// 64 : 127	: Data payload. If data size is greater than 8 bytes, stores the pointer to the actual data.

		VariantType m_type;
		union
		{
			VariantNumberType m_num_type;
			BlobFlag m_blob_flag;
			ObjectFlag m_object_flag;
			ArrayFlag m_array_flag;
		};
		// If the capacity of one object or array is smaller than `BIG_OBJECT_THRESHOLD` (for object) or 65536 (for array), 
		// we use this to store the size and capacity of the object or array (object is stored as array rather than a hash map, 
		// since it is small). In such case, `obj` or `arr` field points to the data diectly.
		// Otherwise, we allocate the structure independently on heap. 
		// In sush case, `big_obj` or `big_arr` field is used to store the pointer to the data structure, and the object is stored 
		// using hash map rather than vector.
		struct CompoundTypeHeader
		{
			u16 m_size;
			u16 m_capacity;
		};
		union
		{
			u32 m_blob_size;
			CompoundTypeHeader m_array_or_object_header;
		};
		union
		{
			i64 m_ii;
			u64 m_ui;
			f64 m_fi;
			bool m_b;
			byte_t* m_blob;
			Blob* m_big_blob;
			Name m_str;
			Pair<const Name, Variant>* m_obj;
			HashMap<Name, Variant>* m_big_obj;
			Variant* m_arr;
			Vector<Variant>* m_big_arr;
		};
		void do_destruct();
		void do_construct(VariantType type);
		void do_construct(const Variant& rhs);
		void do_construct(Variant&& rhs);
		void do_construct(const Vector<Pair<const Name, Variant>>& values);
		void do_construct(Vector<Pair<const Name, Variant>>&& values);
		void do_construct(const Vector<Variant>& values);
		void do_construct(Vector<Variant>&& values);
		void do_construct(const Blob& blob_data);
		void do_construct(Blob&& blob_data);
		void do_construct(const Name& v);
		void do_construct(Name&& v);
		void do_construct(i64 v);
		void do_construct(u64 v);
		void do_construct(f64 v);
		void do_construct(bool v);

		bool do_small_arr_reserve(usize new_cap);
		bool do_small_obj_reserve(usize new_cap);

		void do_small_arr_insert(usize i, const Variant& v);
		void do_small_arr_insert(usize i, Variant&& v);
		void do_small_arr_push(const Variant& v);
		void do_small_arr_push(Variant&& v);
		void do_small_arr_erase(usize i);
		void do_small_arr_erase(usize begin, usize end);
		void do_small_arr_pop();

		Variant& do_small_obj_push(const Name& k);
		bool do_small_obj_insert(const Name& k, const Variant& v);
		bool do_small_obj_insert(const Name& k, Variant&& v);
		bool do_small_obj_erase(const Name& k);
	};

	//! @}
}

#include "Impl/Variant.inl"