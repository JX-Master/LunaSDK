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

	//! @brief Defines all possible types of one @ref Variant object.
	//! @remark To fetch the type of one variant, call @ref Variant::type
	enum class VariantType : u8
	{
		//! Indicates a null variant. 
		//! A null variant represents the absence of value for the variant object.
		null = 0,
		//! Indicates an object variant.
		//! An object variant contains one set of @ref Variant objects, which acts as children of the current variant. 
		//! Children in object variant are indexed by @ref Name strings, and do not have a particular order.
		object = 1,
		//! Indicates an array variant.
		//! An array variant contains one array of @ref Variant objects, which acts as children of the current variant.
		array = 2,
		//! Indicates a number variant.
		//! A number variant stores a number of integer or floating-point type. The number type of one number variant is represented by @ref VariantNumberType 
		//! and can be fetched by calling @ref Variant::number_type method. The number value of the variant can be fetched by calling @ref Variant::unum, 
		//! @ref Variant::inum and @ref Variant::fnum methods, each of them returns the underlying number in specified format with implicit number type conversion when needed.	
		number = 3,
		//! Indicates a string variant.
		//! A string variant contains one string represented by a @ref Name object. You can fetch the underlying string by calling @ref Variant::str or @ref Variant::c_str, 
		//! the former one returns one @ref Name object while the later one returns one C-style string (`const c8*`).
		string = 4,
		//! Indicates a Boolean variant.
		//! A Boolean variant stores a Boolean value with only two possible values: `true` and `false`. 
		//! The Boolean value of one Boolean variant can be fetched by calling @ref Variant::boolean method.
		boolean = 5,
		//! Indicates a BLOB (binary large object) variant.
		//! A BLOB variant can store arbitrary binary data. The data pointer, size and alignment of the data can be fetched by calling @ref Variant::blob_data, @ref Variant::blob_size 
		//! and @ref Variant::blob_alignment methods.
		blob = 6
	};

	//! @brief Defines all possible number types of one number variant.
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

	//! @brief Represents a dynamic typed object that stores data in a schema-less (self-described) manner. 
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

		//! @name Constructors
		//! @{

		//! Initializes one empty value with the specified value type.
		Variant(VariantType type = VariantType::null);
		//! Initializes the value with one copy of another value.
		Variant(const Variant& rhs);
		//! Initializes the value by moving the data of another value to this value.
		Variant(Variant&& rhs);
		//! Initializes one object variant and sets the children using the values provided.
		Variant(const Vector<Pair<const Name, Variant>>& values);
		Variant(Vector<Pair<const Name, Variant>>&& values);
		//! Initializes one array variant and sets the children using the values provided.
		Variant(const Vector<Variant>& values);
		Variant(Vector<Variant>&& values);
		//! Initializes one signed-integer-typed value and sets its data to the specified value.
		Variant(i64 v);
		//! Initializes one unsigned-integer-typed value and sets its data to the specified value.
		Variant(u64 v);
		//! Initializes one floating-point-number-typed value and sets its data to the specified value.
		Variant(f64 v);
		//! Initializes one string variant and sets its data to the specified value.
		Variant(const Name& v);
		//! Initializes one string variant and sets its data to the specified value.
		Variant(Name&& v);
		//! Initializes one string variant and sets its data to the specified value.
		Variant(const c8* v);
		//! Initializes one Boolean variant and sets its data to the specified value.
		Variant(bool v);
		//! Initializes one BLOB variant and sets its data to the specified value.
		Variant(const Blob& blob_data);
		Variant(Blob&& blob_data);

		//! @}

		~Variant();
		Variant& operator=(const Variant& rhs);
		Variant& operator=(Variant&& rhs);
		Variant& operator=(const Vector<Pair<const Name, Variant>>& values);
		Variant& operator=(Vector<Pair<const Name, Variant>>&& values);
		Variant& operator=(const Vector<Variant>& values);
		Variant& operator=(Vector<Variant>&& values);
		Variant& operator=(u64 v);
		Variant& operator=(i64 v);
		Variant& operator=(f64 v);
		Variant& operator=(const Name& v);
		Variant& operator=(Name&& v);
		Variant& operator=(const c8* v);
		Variant& operator=(bool v);
		Variant& operator=(const Blob& blob_data);
		Variant& operator=(Blob&& blob_data);
		bool operator==(const Variant& rhs) const;
		bool operator!=(const Variant& rhs) const;

		//! @name Accessors
		//! @{

		//! @brief Gets the type of the variant.
		//! @return Returns the type of the variant.
		VariantType type() const;
		//! @brief Gets the number type of the variant.
		//! @return Returns the number type of the variant. 
		//! Returns @ref VariantType::not_number if @ref type of the variant is not @ref VariantType::number, 
		VariantNumberType number_type() const;
		//! @brief Checks whether the variant is valid.
		//! @return Returns `true` if @ref type of the variant is not @ref VariantType::null. Returns `false` otherwise.
		bool valid() const;
		//! @brief Checks whether the variant contains no child variant.
		//! @return Returns `true` if @ref size of the variant is `0`. Returns `false` otherwise.
		bool empty() const;

		//! @}

		//! @name Accessing child variants
		//! @{
		
		//! @brief Gets the child variant when this is an array variant.
		//! @param[in] i The index of the child variant to fetch.
		//! @return Returns one reference to the child variant at the specified index.
		//! Returns @ref npos if the current variant is not an array variant, or if the index is invalid.
		const Variant& at(usize i) const;
		//! @brief Gets the child variant when this is an array variant.
		//! @param[in] i The index of the child variant to fetch.
		//! @return Returns one reference to the child variant at the specified index.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * `i` must be smaller than @ref size of the variant.
		Variant& at(usize i);
		//! @brief Gets the child variant when this is an object variant.
		//! @param[in] k The key string of the child variant to fetch.
		//! @return Returns the child variant with the specified key.
		//! Returns @ref npos if the current variant is not an object variant, or if the key is not found.
		const Variant& find(const Name& k) const;
		//! @brief Gets the child variant with the specified key, or inserts one new child variant with that key if not exists.
		//! @param[in] k The key string of the child variant to fetch or insert.
		//! @return Returns one reference to the found or inserted child variant.
		//! @remark The variant inserted by this function will have @ref VariantType::null type.
		//! @par Valid Usage
		//! * The current variant must be an object variant or a null variant. If the variant is a null variant, it will
		//! be transformed into one object variant before the new key-variant pair is inserted.
		Variant& find_or_insert(const Name& k);
		//! @brief Shortcut for @ref at.
		const Variant& operator[](usize i) const;
		//! @brief Shortcut for @ref at.
		Variant& operator[](usize i);
		//! @brief Shortcut for @ref find.
		const Variant& operator[](const Name& k) const;
		//! @brief Shortcut for @ref find_or_insert.
		Variant& operator[](const Name& k);
		//! @brief Gets the number of the child variants of this variant.
		//! @return Returns the number of the child variants of this variant.
		//! Returns `0` if this variant is not an array or object variant.
		usize size() const;
		//! @brief Checks whether one child variant with the specified key exists.
		//! @return Returns `true` if the current variant is a object variant, and the child variant with the specified key exists.
		//! Returns `false` otherwise.
		bool contains(const Name& k) const;
		//! @brief Gets one enumerator that can be used to enumerate all child variants of one array variant.
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
		//! @brief Gets one enumerator that can be used to enumerate all child variants of one array variant.
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
		//! @brief Gets one enumerator to enumerate all child key-variant pairs of one object variant.
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
		//! @brief Gets one enumerator to enumerate all child key-variant pairs of one object variant.
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

		//! @}

		//! @name Adding and removing child variants.
		//! @{

		//! @brief Copy-inserts one variant to the specified index.
		//! @param[in] i The index to insert the variant to.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant. 
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		//! * `i` must be smaller to equal to @ref size.
		void insert(usize i, const Variant& val);
		//! @brief Move-inserts one variant to the specified index.
		//! @param[in] i The index to insert the variant to.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		//! * `i` must not be greater than @ref size.
		void insert(usize i, Variant&& val);
		//! @brief Copy-inserts one variant to the end of the variant array.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		void push_back(const Variant& val);
		//! @brief Move-inserts one variant to the end of the variant array.
		//! @param[in] val The variant to insert.
		//! @par Valid Usage
		//! * The current variant must be an array variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty array variant before the insertion is performed.
		void push_back(Variant&& val);
		//! @brief Removes one child variant from the current variant.
		//! @param[in] i The index of the child variant to remove.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * `i` must be smaller than @ref size.
		void erase(usize i);
		//! @brief Erases one range [begin, end) of child variants from the current variant.
		//! @param[in] begin The index of first child variant to remove.
		//! @param[in] end The index of one-past-last child variant to remove.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * `begin` must not be greater than `end`.
		//! * `end` must not be greater than @ref size.
		void erase(usize begin, usize end);
		//! @brief Erases the last child variant from the child variants array of the current variant.
		//! @par Valid Usage
		//! * The current variant must be an array variant.
		//! * This variant must not be @ref empty.
		void pop_back();
		//! @brief Copy-inserts the variant with the specified key as the child variant of the current variant.
		//! @param[in] k The key string of the variant to insert.
		//! @param[in] val The variant to insert.
		//! @return Returns `true` if the variant is successfully inserted. Returns `false` if one variant with the 
		//! specified key already exists, in such case, the existing variant is not modified.
		//! @par Valid Usage
		//! * The current variant must be an object variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty object variant before the insertion is performed.
		bool insert(const Name& k, const Variant& val);
		//! @brief Move-inserts the variant with the specified key as the child variant of the current variant.
		//! @param[in] k The key string of the variant to insert.
		//! @param[in] val The variant to insert.
		//! @return Returns `true` if the variant is successfully inserted. Returns `false` if one variant with the 
		//! specified key already exists, in such case, the existing variant is not modified.
		//! @par Valid Usage
		//! * The current variant must be an object variant or a null variant.
		//! If the current variant is a null variant, it will be converted to an empty object variant before the insertion is performed.
		bool insert(const Name& k, Variant&& val);
		//! @brief Removes the child variant with the specified key from the current variant.
		//! @param[in] k The key string of the variant to remove.
		//! @return Returns `true` if one child variant with the specified key is successfully removed. Returns `false` otherwise.
		//! @par Valid Usage
		//! * The current variant must be an object variant.
		bool erase(const Name& k);

		//! @}

		//! @name Fetching variant data
		//! @{

		//! @brief Gets the string of one string variant.
		//! @param[in] default_value The optional default string to return.
		//! @return Returns the string data of one variant if @ref type is @ref VariantType::string.
		//! Returns `default_value` otherwise.
		Name str(const Name& default_value = Name()) const;
		//! @brief Gets the C string of one string variant.
		//! @param[in] default_value The optional default string to return.
		//! @return Returns the string data of one variant if @ref type is @ref VariantType::string.
		//! Returns `default_value` otherwise.
		const c8* c_str(const c8* default_value = "") const;
		//! @brief Gets the data of one number variant as one signed 64-bit integer.
		//! @param[in] default_value The optional default number to return.
		//! @return Returns the value as one signed 64-bit integer. If the value is in another number format, it will be 
		//! converted automatically. Returns `default_value` if the variant is not a number variant.
		i64 inum(i64 default_value = 0) const;
		//! @brief Gets the data of one number variant as one unsigned 64-bit integer.
		//! @param[in] default_value The optional default number to return.
		//! @return Returns the value as one unsigned 64-bit integer. If the value is in another number format, it will be 
		//! converted automatically. Returns `default_value` if the variant is not a number variant.
		u64 unum(u64 default_value = 0) const;
		//! @brief Gets the data of one number variant as one 64-bit floating-point number.
		//! @param[in] default_value The optional default number to return.
		//! @return Returns the value as one 64-bit floating-point number. If the value is in another number format, it will be 
		//! converted automatically. Returns `default_value` if the variant is not a number variant.
		f64 fnum(f64 default_value = 0) const;
		//! @brief Gets the data of one Boolean variant.
		//! @param[in] default_value The optional default Boolean value to return.
		//! @return Returns the data of one Boolean variant. Returns `default_value` if the variant is not a Boolean variant.
		bool boolean(bool default_value = false) const;
		//! @brief Gets the data pointer of one BLOB variant.
		//! @return Returns the data pointer if the variant is a BLOB variant with @ref blob_size greater than `0`. 
		//! Returns `nullptr` otherwise.
		byte_t* blob_data();
		//! @brief Gets the data pointer of one BLOB variant.
		//! @return Returns the data pointer if the variant is a BLOB variant with @ref blob_size greater than `0`. 
		//! Returns `nullptr` otherwise.
		const byte_t* blob_data() const;
		//! @brief Gets the data size, in bytes, of one BLOB variant.
		//! @return Returns the size of the data if the variant is a BLOB variant. Returns 0 otherwise.
		usize blob_size() const;
		//! @brief Gets the data alignment, in bytes, of one BLOB variant.
		//! @return Returns the alignment of the data if the variant is a BLOB variant. Returns 0 otherwise.
		usize blob_alignment() const;
		//! @brief Detaches and gets the data of one BLOB variant as a @ref Blob object. 
		//! @details After this operation, this variant is still a BLOB variant, but contains no binary data ( @ref blob_data returns
		//! `nullptr`, and @ref blob_size returns `0`).
		//! @return Returns the detached blob data if the variant is a BLOB variant. Returns one empty @ref Blob object otherwise.
		Blob blob_detach();

		//! @}
		
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

	//! @brief Returns one reference to one global constant variant that contains no data.
	//! @details This is used as the default return value if one query operation fails, so that the user can chain multiple `[]` operations
	//! like data["persons"][0]["name"] without the need to handle null variant explicitly (query child variants of one null variant also returns `npos`).
	LUNA_RUNTIME_API const Variant& npos();

//! @}

	static_assert(sizeof(Variant) == 16, "Wrong Variant size.");
	static_assert(alignof(Variant) == 8, "Wrong Variant alignment.");

	inline Variant::ObjectEnumerator::iterator Variant::ObjectEnumerator::begin()
	{
		if (m_value->type() != VariantType::object)
		{
			return iterator((Pair<const Name, Variant>*)nullptr);
		}
		if (test_flags(m_value->m_object_flag, Variant::ObjectFlag::big_object))
		{
			return iterator(m_value->m_big_obj->begin());
		}
		return iterator(m_value->m_obj);
	}
	inline Variant::ObjectEnumerator::const_iterator Variant::ObjectEnumerator::cbegin() const
	{
		if (m_value->type() != VariantType::object)
		{
			return const_iterator((const Pair<const Name, Variant>*)nullptr);
		}
		if (test_flags(m_value->m_object_flag, Variant::ObjectFlag::big_object))
		{
			return const_iterator(m_value->m_big_obj->cbegin());
		}
		return const_iterator(m_value->m_obj);
	}
	inline Variant::ObjectEnumerator::iterator Variant::ObjectEnumerator::end()
	{
		if (m_value->type() != VariantType::object)
		{
			return iterator((Pair<const Name, Variant>*)nullptr);
		}
		if (test_flags(m_value->m_object_flag, Variant::ObjectFlag::big_object))
		{
			return iterator(m_value->m_big_obj->end());
		}
		return iterator(m_value->m_obj + m_value->m_array_or_object_header.m_size);
	}
	inline Variant::ObjectEnumerator::const_iterator Variant::ObjectEnumerator::cend() const
	{
		if (m_value->type() != VariantType::object)
		{
			return const_iterator((const Pair<const Name, Variant>*)nullptr);
		}
		if (test_flags(m_value->m_object_flag, Variant::ObjectFlag::big_object))
		{
			return const_iterator(m_value->m_big_obj->cend());
		}
		return const_iterator(m_value->m_obj + m_value->m_array_or_object_header.m_size);
	}
	inline Variant::ConstObjectEnumerator::const_iterator Variant::ConstObjectEnumerator::cbegin() const
	{
		if (m_value->type() != VariantType::object)
		{
			return const_iterator((const Pair<const Name, Variant>*)nullptr);
		}
		if (test_flags(m_value->m_object_flag, Variant::ObjectFlag::big_object))
		{
			return const_iterator(m_value->m_big_obj->cbegin());
		}
		return const_iterator(m_value->m_obj);
	}
	inline Variant::ConstObjectEnumerator::const_iterator Variant::ConstObjectEnumerator::cend() const
	{
		if (m_value->type() != VariantType::object)
		{
			return const_iterator((const Pair<const Name, Variant>*)nullptr);
		}
		if (test_flags(m_value->m_object_flag, Variant::ObjectFlag::big_object))
		{
			return const_iterator(m_value->m_big_obj->cend());
		}
		return const_iterator(m_value->m_obj + m_value->m_array_or_object_header.m_size);
	}
	inline Variant::Variant(VariantType type)
	{
		do_construct(type);
	}
	inline Variant::Variant(const Variant& rhs)
	{
		do_construct(rhs);
	}
	inline Variant::Variant(Variant&& rhs)
	{
		do_construct(move(rhs));
	}
	inline Variant::Variant(const Vector<Pair<const Name, Variant>>& values)
	{
		do_construct(values);
	}
	inline Variant::Variant(Vector<Pair<const Name, Variant>>&& values)
	{
		do_construct(move(values));
	}
	inline Variant::Variant(const Vector<Variant>& values)
	{
		do_construct(values);
	}
	inline Variant::Variant(Vector<Variant>&& values)
	{
		do_construct(move(values));
	}
	inline Variant::Variant(i64 v)
	{
		do_construct(v);
	}
	inline Variant::Variant(u64 v)
	{
		do_construct(v);
	}
	inline Variant::Variant(f64 v)
	{
		do_construct(v);
	}
	inline Variant::Variant(const Name& v)
	{
		do_construct(v);
	}
	inline Variant::Variant(Name&& v)
	{
		do_construct(move(v));
	}
	inline Variant::Variant(const c8* v)
	{
		do_construct(Name(v));
	}
	inline Variant::Variant(bool v)
	{
		do_construct(v);
	}
	inline Variant::Variant(const Blob& blob_data)
	{
		do_construct(blob_data);
	}
	inline Variant::Variant(Blob&& blob_data)
	{
		do_construct(move(blob_data));
	}
	inline Variant::~Variant()
	{
		do_destruct();
	}
	inline Variant& Variant::operator=(const Variant& rhs)
	{
		do_destruct();
		do_construct(rhs);
		return *this;
	}
	inline Variant& Variant::operator=(Variant&& rhs)
	{
		do_destruct();
		do_construct(move(rhs));
		return *this;
	}
	inline Variant& Variant::operator=(const Vector<Pair<const Name, Variant>>& values)
	{
		do_destruct();
		do_construct(values);
		return *this;
	}
	inline Variant& Variant::operator=(Vector<Pair<const Name, Variant>>&& values)
	{
		do_destruct();
		do_construct(move(values));
		return *this;
	}
	inline Variant& Variant::operator=(const Vector<Variant>& values)
	{
		do_destruct();
		do_construct(values);
		return *this;
	}
	inline Variant& Variant::operator=(Vector<Variant>&& values)
	{
		do_destruct();
		do_construct(move(values));
		return *this;
	}
	inline Variant& Variant::operator=(u64 v)
	{
		do_destruct();
		do_construct(v);
		return *this;
	}
	inline Variant& Variant::operator=(i64 v)
	{
		do_destruct();
		do_construct(v);
		return *this;
	}
	inline Variant& Variant::operator=(f64 v)
	{
		do_destruct();
		do_construct(v);
		return *this;
	}
	inline Variant& Variant::operator=(const Name& v)
	{
		do_destruct();
		do_construct(v);
		return *this;
	}
	inline Variant& Variant::operator=(Name&& v)
	{
		do_destruct();
		do_construct(move(v));
		return *this;
	}
	inline Variant& Variant::operator=(const c8* v)
	{
		do_destruct();
		do_construct(Name(v));
		return *this;
	}
	inline Variant& Variant::operator=(bool v)
	{
		do_destruct();
		do_construct(v);
		return *this;
	}
	inline Variant& Variant::operator=(const Blob& blob_data)
	{
		do_destruct();
		do_construct(blob_data);
		return *this;
	}
	inline Variant& Variant::operator=(Blob&& blob_data)
	{
		do_destruct();
		do_construct(move(blob_data));
		return *this;
	}
	inline bool Variant::operator==(const Variant& rhs) const
	{
		if (m_type != rhs.m_type) return false;
		switch (m_type)
		{
		case VariantType::null:
			return true;
		case VariantType::object:
			//if (size() != rhs.size()) return false;
			{
				for (auto& i : key_values())
				{
					const Variant& rv = rhs.find(i.first);
					if (rv != i.second)
					{
						return false;
					}
				}
				return true;
			}
		case VariantType::array:
			if (size() != rhs.size()) return false;
			return equal(values().begin(), values().end(), rhs.values().begin());
		case VariantType::number:
			return (m_num_type == rhs.m_num_type) && (m_ii == rhs.m_ii);
		case VariantType::string:
			return m_str == rhs.m_str;
		case VariantType::boolean:
			return m_b == rhs.m_b;
		case VariantType::blob:
			return memcmp(blob_data(), rhs.blob_data(), blob_size()) == 0;
		default:
			lupanic();
			return false;
		}
	}
	inline bool Variant::operator!=(const Variant& rhs) const
	{
		return !(*this == rhs);
	}
	inline VariantType Variant::type() const
	{
		return m_type;
	}
	inline VariantNumberType Variant::number_type() const
	{
		if (type() != VariantType::number) return VariantNumberType::not_number;
		return m_num_type;
	}
	inline bool Variant::valid() const
	{
		return m_type != VariantType::null;
	}
	inline bool Variant::empty() const
	{
		return size() == 0;
	}
	inline const Variant& Variant::at(usize i) const
	{
		if (type() != VariantType::array) return npos();
		if (i >= size()) return npos();
		if(test_flags(m_array_flag, ArrayFlag::big_array)) return m_big_arr->at(i);
		return m_arr[i];
	}
	inline Variant& Variant::at(usize i)
	{
		lucheck(type() == VariantType::array);
		lucheck(i < size());
		if (test_flags(m_array_flag, ArrayFlag::big_array)) return m_big_arr->at(i);
		return m_arr[i];
	}
	inline const Variant& Variant::find(const Name& k) const
	{
		if (type() != VariantType::object) return npos();
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto iter = m_big_obj->find(k);
			if (iter != m_big_obj->end())
			{
				return iter->second;
			}
			return npos();
		}
		for (usize i = 0; i < m_array_or_object_header.m_size; ++i)
		{
			if (k == m_obj[i].first)
			{
				return m_obj[i].second;
			}
		}
		return npos();
	}
	inline Variant& Variant::find_or_insert(const Name& k)
	{
		if (type() == VariantType::null)
		{
			do_construct(VariantType::object);
		}
		lucheck(type() == VariantType::object);
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto iter = m_big_obj->find(k);
			if (iter != m_big_obj->end())
			{
				return iter->second;
			}
			auto res = m_big_obj->insert(make_pair(k, Variant()));
			return res.first->second;
		}
		for (usize i = 0; i < m_array_or_object_header.m_size; ++i)
		{
			if (k == m_obj[i].first)
			{
				return m_obj[i].second;
			}
		}
		return do_small_obj_push(k);
	}
	inline const Variant& Variant::operator[](usize i) const
	{
		return at(i);
	}
	inline Variant& Variant::operator[](usize i)
	{
		return at(i);
	}
	inline const Variant& Variant::operator[](const Name& k) const
	{
		return find(k);
	}
	inline Variant& Variant::operator[](const Name& k)
	{
		return find_or_insert(k);
	}
	inline usize Variant::size() const
	{
		if (type() == VariantType::array)
		{
			return test_flags(m_array_flag, ArrayFlag::big_array) ? m_big_arr->size() : m_array_or_object_header.m_size;
		}
		else if (type() == VariantType::object)
		{
			return test_flags(m_object_flag, ObjectFlag::big_object) ? m_big_obj->size() : m_array_or_object_header.m_size;
		}
		else if (type() == VariantType::blob)
		{
			return blob_size();
		}
		else
		{
			return 0;
		}
	}
	inline bool Variant::contains(const Name& k) const
	{
		if (type() != VariantType::object) return false;
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto iter = m_big_obj->find(k);
			return (iter != m_big_obj->end());
		}
		for (usize i = 0; i < m_array_or_object_header.m_size; ++i)
		{
			if (m_obj[i].first == k) return true;
		}
		return false;
	}
	inline Variant::value_enumerator Variant::values()
	{
		value_enumerator ret;
		if (type() != VariantType::array)
		{
			ret.m_begin = nullptr;
			ret.m_end = nullptr;
		}
		else if(test_flags(m_array_flag, ArrayFlag::big_array))
		{
			ret.m_begin = m_big_arr->data();
			ret.m_end = ret.m_begin + m_big_arr->size();
		}
		else
		{
			ret.m_begin = m_arr;
			ret.m_end = ret.m_begin + m_array_or_object_header.m_size;
		}
		return ret;
	}
	inline Variant::const_value_enumerator Variant::values() const
	{
		const_value_enumerator ret;
		if (type() != VariantType::array)
		{
			ret.m_begin = nullptr;
			ret.m_end = nullptr;
		}
		else if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			ret.m_begin = m_big_arr->data();
			ret.m_end = ret.m_begin + m_big_arr->size();
		}
		else
		{
			ret.m_begin = m_arr;
			ret.m_end = ret.m_begin + m_array_or_object_header.m_size;
		}
		return ret;
	}
	inline Variant::key_value_enumerator Variant::key_values()
	{
		key_value_enumerator ret;
		ret.m_value = this;
		return ret;
	}
	inline Variant::const_key_value_enumerator Variant::key_values() const
	{
		const_key_value_enumerator ret;
		ret.m_value = this;
		return ret;
	}
	inline void Variant::insert(usize i, const Variant& val)
	{
		if (type() == VariantType::null)
		{
			do_construct(VariantType::array);
		}
		lucheck(type() == VariantType::array);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->insert(m_big_arr->begin() + i, val);
		}
		else
		{
			do_small_arr_insert(i, val);
		}
	}
	inline void Variant::insert(usize i, Variant&& val)
	{
		if (type() == VariantType::null)
		{
			do_construct(VariantType::array);
		}
		lucheck(type() == VariantType::array);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->insert(m_big_arr->begin() + i, move(val));
		}
		else
		{
			do_small_arr_insert(i, move(val));
		}
	}
	inline void Variant::push_back(const Variant& val)
	{
		if (type() == VariantType::null)
		{
			do_construct(VariantType::array);
		}
		lucheck(type() == VariantType::array);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->push_back(val);
		}
		else
		{
			do_small_arr_push(val);
		}
	}
	inline void Variant::push_back(Variant&& val)
	{
		if (type() == VariantType::null)
		{
			do_construct(VariantType::array);
		}
		lucheck(type() == VariantType::array);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->push_back(move(val));
		}
		else
		{
			do_small_arr_push(move(val));
		}
	}
	inline void Variant::erase(usize i)
	{
		lucheck(type() == VariantType::array);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->erase(m_big_arr->begin() + i);
		}
		else
		{
			do_small_arr_erase(i);
		}
	}
	inline void Variant::erase(usize begin, usize end)
	{
		lucheck(type() == VariantType::array);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->erase(m_big_arr->begin() + begin, m_big_arr->begin() + end);
		}
		else
		{
			do_small_arr_erase(begin, end);
		}
	}
	inline void Variant::pop_back()
	{
		lucheck(type() == VariantType::array);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->pop_back();
		}
		else
		{
			do_small_arr_pop();
		}
	}
	inline bool Variant::insert(const Name& k, const Variant& val)
	{
		if (type() == VariantType::null)
		{
			do_construct(VariantType::object);
		}
		lucheck(type() == VariantType::object);
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto res = m_big_obj->insert(make_pair(k, val));
			return res.second;
		}
		return do_small_obj_insert(k, val);
	}
	inline bool Variant::insert(const Name& k, Variant&& val)
	{
		if (type() == VariantType::null)
		{
			do_construct(VariantType::object);
		}
		lucheck(type() == VariantType::object);
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto res = m_big_obj->insert(make_pair(k, move(val)));
			return res.second;
		}
		return do_small_obj_insert(k, move(val));
	}
	inline bool Variant::erase(const Name& k)
	{
		lucheck(type() == VariantType::object);
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto res = m_big_obj->erase(k);
			return res != 0;
		}
		return do_small_obj_erase(k);
	}
	inline Name Variant::str(const Name& default_value) const
	{
		return type() == VariantType::string ? m_str : default_value;
	}
	inline const c8* Variant::c_str(const c8* default_value) const
	{
		return type() == VariantType::string ? m_str.c_str() : default_value;
	}
	inline i64 Variant::inum(i64 default_value) const
	{
		if (type() == VariantType::number)
		{
			switch (m_num_type)
			{
			case VariantNumberType::number_f64:
				return (i64)m_fi;
			case VariantNumberType::number_i64:
				return m_ii;
			case VariantNumberType::number_u64:
				return (i64)m_ui;
			default:
				lupanic();
			}
		}
		return default_value;
	}
	inline u64 Variant::unum(u64 default_value) const
	{
		if (type() == VariantType::number)
		{
			switch (m_num_type)
			{
			case VariantNumberType::number_f64:
				return (u64)m_fi;
			case VariantNumberType::number_i64:
				return (u64)m_ii;
			case VariantNumberType::number_u64:
				return m_ui;
			default:
				lupanic();
			}
		}
		return default_value;
	}
	inline f64 Variant::fnum(f64 default_value) const
	{
		if (type() == VariantType::number)
		{
			switch (m_num_type)
			{
			case VariantNumberType::number_f64:
				return m_fi;
			case VariantNumberType::number_i64:
				return (f64)m_ii;
			case VariantNumberType::number_u64:
				return (f64)m_ui;
			default:
				lupanic();
			}
		}
		return default_value;
	}
	inline bool Variant::boolean(bool default_value) const
	{
		return type() == VariantType::boolean ? m_b : default_value;
	}
	inline byte_t* Variant::blob_data()
	{
		if (type() == VariantType::blob)
		{
			return test_flags(m_blob_flag, BlobFlag::big_blob) ? m_big_blob->data() : m_blob;
		}
		return nullptr;
	}
	inline const byte_t* Variant::blob_data() const
	{
		if (type() == VariantType::blob)
		{
			return test_flags(m_blob_flag, BlobFlag::big_blob) ? m_big_blob->data() : m_blob;
		}
		return nullptr;
	}
	inline usize Variant::blob_size() const
	{
		if (type() == VariantType::blob)
		{
			return test_flags(m_blob_flag, BlobFlag::big_blob) ? m_big_blob->size() : m_blob_size;
		}
		return 0;
	}
	inline usize Variant::blob_alignment() const
	{
		if (type() == VariantType::blob)
		{
			return test_flags(m_blob_flag, BlobFlag::big_blob) ? m_big_blob->alignment() : 0;
		}
		return 0;
	}
	inline Blob Variant::blob_detach()
	{
		if (type() == VariantType::blob)
		{
			Blob ret;
			if (test_flags(m_blob_flag, BlobFlag::big_blob))
			{
				ret = move(*m_big_blob);
			}
			else
			{
				ret.attach(m_blob, m_blob_size, 0);
				m_blob = nullptr;
				m_blob_size = 0;
			}
			return ret;
		}
		return Blob();
	}
	inline void Variant::do_destruct()
	{
		switch (m_type)
		{
		case VariantType::object:
			if (test_flags(m_object_flag, ObjectFlag::big_object))
			{
				memdelete(m_big_obj);
			}
			else
			{
				if (m_obj)
				{
					destruct_range(m_obj, m_obj + m_array_or_object_header.m_size);
					memfree(m_obj);
				}
			}
			break;
		case VariantType::array:
			if (test_flags(m_array_flag, ArrayFlag::big_array))
			{
				memdelete(m_big_arr);
			}
			else
			{
				if (m_arr)
				{
					destruct_range(m_arr, m_arr + m_array_or_object_header.m_size);
					memfree(m_arr);
				}
			}
			break;
		case VariantType::string:
			m_str.~Name();
			break;
		case VariantType::blob:
			if (test_flags(m_blob_flag, BlobFlag::big_blob))
			{
				memdelete(m_big_blob);
			}
			else
			{
				if (m_blob)
				{
					memfree(m_blob);
				}
			}
			break;
        default: break;
		}
	}
	inline void Variant::do_construct(VariantType type)
	{
		m_type = type;
		switch (m_type)
		{
		case VariantType::object:
			m_object_flag = ObjectFlag::none;
			m_array_or_object_header.m_size = 0;
			m_array_or_object_header.m_capacity = 0;
			m_obj = nullptr;
			break;
		case VariantType::array:
			m_array_flag = ArrayFlag::none;
			m_array_or_object_header.m_size = 0;
			m_array_or_object_header.m_capacity = 0;
			m_arr = nullptr;
			break;
		case VariantType::number:
			m_ui = 0;
			m_num_type = VariantNumberType::number_u64;
			break;
		case VariantType::string:
			new (&m_str) Name();
			break;
		case VariantType::boolean:
			m_b = false;
			break;
		case VariantType::blob:
			m_blob_flag = BlobFlag::none;
			m_blob_size = 0;
			m_blob = nullptr;
			break;
		case VariantType::null:
			break;
		}
	}
	inline void Variant::do_construct(const Variant& rhs)
	{
		m_type = rhs.m_type;
		switch (m_type)
		{
		case VariantType::object:
			m_object_flag = rhs.m_object_flag;
			if (test_flags(m_object_flag, ObjectFlag::big_object))
			{
				m_big_obj = memnew<HashMap<Name, Variant>>(*rhs.m_big_obj);
			}
			else
			{
				m_array_or_object_header.m_size = rhs.m_array_or_object_header.m_size;
				m_array_or_object_header.m_capacity = rhs.m_array_or_object_header.m_capacity;
				m_obj = (Pair<const Name, Variant>*)memalloc(
					sizeof(Pair<const Name, Variant>) * m_array_or_object_header.m_capacity);
				copy_construct_range(rhs.m_obj, rhs.m_obj + rhs.m_array_or_object_header.m_size, m_obj);
			}
			break;
		case VariantType::array:
			m_array_flag = rhs.m_array_flag;
			if (test_flags(m_array_flag, ArrayFlag::big_array))
			{
				m_big_arr = memnew<Vector<Variant>>(*rhs.m_big_arr);
			}
			else
			{
				m_array_or_object_header.m_size = rhs.m_array_or_object_header.m_size;
				m_array_or_object_header.m_capacity = rhs.m_array_or_object_header.m_capacity;
				m_arr = (Variant*)memalloc(sizeof(Variant) * m_array_or_object_header.m_capacity);
				copy_construct_range(rhs.m_arr, rhs.m_arr + rhs.m_array_or_object_header.m_size, m_arr);
			}
			break;
		case VariantType::number:
			m_ui = rhs.m_ui;
			m_num_type = rhs.m_num_type;
			break;
		case VariantType::string:
			new (&m_str) Name(rhs.m_str);
			break;
		case VariantType::boolean:
			m_b = rhs.m_b;
			break;
		case VariantType::blob:
			m_blob_flag = rhs.m_blob_flag;
			if (test_flags(m_blob_flag, BlobFlag::big_blob))
			{
				m_big_blob = memnew<Blob>(*rhs.m_big_blob);
			}
			else
			{
				m_blob_size = rhs.m_blob_size;
				m_blob = (byte_t*)memalloc(m_blob_size);
				memcpy(m_blob, rhs.m_blob, m_blob_size);
			}
			break;
		case VariantType::null:
			break;
		}
	}
	inline void Variant::do_construct(Variant&& rhs)
	{
		m_type = rhs.m_type;
		switch (m_type)
		{
		case VariantType::object:
			m_object_flag = rhs.m_object_flag;
			if (test_flags(m_object_flag, ObjectFlag::big_object))
			{
				m_big_obj = rhs.m_big_obj;	// Transfer ownership directly.
				rhs.m_big_obj = 0;
				reset_flags(rhs.m_object_flag, ObjectFlag::big_object);
			}
			else
			{
				m_array_or_object_header.m_size = rhs.m_array_or_object_header.m_size;
				m_array_or_object_header.m_capacity = rhs.m_array_or_object_header.m_capacity;
				m_obj = rhs.m_obj;
			}
			rhs.m_obj = nullptr;
			rhs.m_array_or_object_header.m_size = 0;
			rhs.m_array_or_object_header.m_capacity = 0;
			break;
		case VariantType::array:
			m_array_flag = rhs.m_array_flag;
			if (test_flags(m_array_flag, ArrayFlag::big_array))
			{
				m_big_arr = rhs.m_big_arr;
				rhs.m_big_arr = nullptr;
				reset_flags(rhs.m_array_flag, ArrayFlag::big_array);
			}
			else
			{
				m_array_or_object_header.m_size = rhs.m_array_or_object_header.m_size;
				m_array_or_object_header.m_capacity = rhs.m_array_or_object_header.m_capacity;
				m_arr = rhs.m_arr;
			}
			rhs.m_arr = nullptr;
			rhs.m_array_or_object_header.m_size = 0;
			rhs.m_array_or_object_header.m_capacity = 0;
			break;
		case VariantType::number:
			m_ui = rhs.m_ui;
			m_num_type = rhs.m_num_type;
			break;
		case VariantType::string:
			new (&m_str) Name(move(rhs.m_str));
			break;
		case VariantType::boolean:
			m_b = rhs.m_b;
			break;
		case VariantType::blob:
			m_blob_flag = rhs.m_blob_flag;
			if (test_flags(m_blob_flag, BlobFlag::big_blob))
			{
				m_big_blob = memnew<Blob>(move(*rhs.m_big_blob));
			}
			else
			{
				m_blob_size = rhs.m_blob_size;
				m_blob = rhs.m_blob;
				rhs.m_blob = nullptr;
				rhs.m_blob_size = 0;
			}
			break;
		case VariantType::null:
			break;
		}
	}
	inline void Variant::do_construct(const Vector<Pair<const Name, Variant>>& values)
	{
		m_type = VariantType::object;
		m_object_flag = ObjectFlag::none;
		if (values.size() > BIG_OBJECT_THRESHOLD)
		{
			set_flags(m_object_flag, ObjectFlag::big_object);
			m_big_obj = memnew<HashMap<Name, Variant>>();
			for (auto& p : values)
			{
				m_big_obj->insert(p);
			}
		}
		else
		{
			m_array_or_object_header.m_size = (u16)values.size();
			m_array_or_object_header.m_capacity = (u16)values.size();
			m_obj = (Pair<const Name, Variant>*)memalloc(sizeof(Pair<const Name, Variant>) * values.size());
			copy_construct_range(values.begin(), values.end(), m_obj);
		}
	}
	inline void Variant::do_construct(Vector<Pair<const Name, Variant>>&& values)
	{
		m_type = VariantType::object;
		m_object_flag = ObjectFlag::none;
		if (values.size() > BIG_OBJECT_THRESHOLD)
		{
			set_flags(m_object_flag, ObjectFlag::big_object);
			m_big_obj = memnew<HashMap<Name, Variant>>();
			for (auto& p : values)
			{
				m_big_obj->insert(move(p));
			}
		}
		else
		{
			m_array_or_object_header.m_size = (u16)values.size();
			m_array_or_object_header.m_capacity = (u16)values.size();
			m_obj = (Pair<const Name, Variant>*)memalloc(sizeof(Pair<const Name, Variant>) * values.size());
			move_construct_range(values.begin(), values.end(), m_obj);
			values.clear();
		}
	}
	inline void Variant::do_construct(const Vector<Variant>& values)
	{
		m_type = VariantType::array;
		m_array_flag = ArrayFlag::none;
		if (values.size() > (usize)U16_MAX)
		{
			set_flags(m_array_flag, ArrayFlag::big_array);
			m_big_arr = memnew<Vector<Variant>>(values);
		}
		else
		{
			m_array_or_object_header.m_size = (u16)values.size();
			m_array_or_object_header.m_capacity = (u16)values.size();
			m_arr = (Variant*)memalloc(sizeof(Variant) * values.size());
			copy_construct_range(values.begin(), values.end(), m_arr);
		}
	}
	inline void Variant::do_construct(Vector<Variant>&& values)
	{
		m_type = VariantType::array;
		m_array_flag = ArrayFlag::none;
		if (values.size() > (usize)U16_MAX)
		{
			set_flags(m_array_flag, ArrayFlag::big_array);
			m_big_arr = memnew<Vector<Variant>>(move(values));
		}
		else
		{
			m_array_or_object_header.m_size = (u16)values.size();
			m_array_or_object_header.m_capacity = (u16)values.size();
			m_arr = (Variant*)memalloc(sizeof(Variant) * values.size());
			move_construct_range(values.begin(), values.end(), m_arr);
			values.clear();
		}
	}
	inline void Variant::do_construct(const Blob& blob_data)
	{
		m_type = VariantType::blob;
		m_blob_flag = BlobFlag::none;
		if (blob_data.size() > U32_MAX || blob_data.alignment() > MAX_ALIGN)
		{
			set_flags(m_blob_flag, BlobFlag::big_blob);
			m_big_blob = memnew<Blob>(blob_data);
		}
		else
		{
			m_blob_size = (u32)blob_data.size();
			m_blob = (byte_t*)memalloc(m_blob_size);
			memcpy(m_blob, blob_data.data(), m_blob_size);
		}
	}
	inline void Variant::do_construct(Blob&& blob_data)
	{
		m_type = VariantType::blob;
		m_blob_flag = BlobFlag::none;
		if (blob_data.size() > U32_MAX || blob_data.alignment() > MAX_ALIGN)
		{
			set_flags(m_blob_flag, BlobFlag::big_blob);
			m_big_blob = memnew<Blob>(move(blob_data));
		}
		else
		{
			m_blob_size = (u32)blob_data.size();
			m_blob = blob_data.detach();
		}
	}
	inline void Variant::do_construct(const Name& v)
	{
		m_type = VariantType::string;
		new (&m_str) Name(v);
	}
	inline void Variant::do_construct(Name&& v)
	{
		m_type = VariantType::string;
		new (&m_str) Name(move(v));
	}
	inline void Variant::do_construct(i64 v)
	{
		m_type = VariantType::number;
		m_num_type = VariantNumberType::number_i64;
		m_ii = v;
	}
	inline void Variant::do_construct(u64 v)
	{
		m_type = VariantType::number;
		m_num_type = VariantNumberType::number_u64;
		m_ui = v;
	}
	inline void Variant::do_construct(f64 v)
	{
		m_type = VariantType::number;
		m_num_type = VariantNumberType::number_f64;
		m_fi = v;
	}
	inline void Variant::do_construct(bool v)
	{
		m_type = VariantType::boolean;
		m_b = v;
	}
	inline bool Variant::do_small_arr_reserve(usize new_cap)
	{
		if (new_cap > m_array_or_object_header.m_capacity)
		{
			new_cap = max(max(new_cap, (usize)m_array_or_object_header.m_capacity * 2), (usize)4);
			if (new_cap <= (usize)U16_MAX)
			{
				Variant* new_buf = (Variant*)memalloc(sizeof(Variant) * new_cap);
				if (m_arr)
				{
					copy_relocate_range(m_arr, m_arr + (usize)m_array_or_object_header.m_size, new_buf);
					memfree(m_arr);
				}
				m_arr = new_buf;
				m_array_or_object_header.m_capacity = (u16)new_cap;
				return false;
			}
			else
			{
				// Promote to big vector.
				set_flags(m_array_flag, ArrayFlag::big_array);
				Vector<Variant>* new_arr = memnew<Vector<Variant>>();
				new_arr->reserve(new_cap);
				for (usize i = 0; i < (usize)m_array_or_object_header.m_size; ++i)
				{
					new_arr->push_back(move(m_arr[i]));
				}
				destruct_range(m_arr, m_arr + (usize)m_array_or_object_header.m_size);
				memfree(m_arr);
				m_arr = nullptr;
				m_big_arr = new_arr;
				return true;
			}
		}
		return false;
	}
	inline bool Variant::do_small_obj_reserve(usize new_cap)
	{
		if (new_cap > m_array_or_object_header.m_capacity)
		{
			new_cap = max(max(new_cap, (usize)m_array_or_object_header.m_capacity * 2), (usize)4);
			if (new_cap <= BIG_OBJECT_THRESHOLD)
			{
				Pair<const Name, Variant>* new_buf = (Pair<const Name, Variant>*)memalloc(sizeof(Pair<const Name, Variant>) * new_cap);
				if (m_obj)
				{
					copy_relocate_range(m_obj, m_obj + (usize)m_array_or_object_header.m_size, new_buf);
					memfree(m_obj);
				}
				m_obj = new_buf;
				m_array_or_object_header.m_capacity = (u16)new_cap;
				return false;
			}
			else
			{
				// Promote to big hash map.
				set_flags(m_object_flag, ObjectFlag::big_object);
				HashMap<Name, Variant>* new_obj = memnew<HashMap<Name, Variant>>();
				for (usize i = 0; i < (usize)m_array_or_object_header.m_size; ++i)
				{
					new_obj->insert(move(m_obj[i]));
				}
				destruct_range(m_obj, m_obj + (usize)m_array_or_object_header.m_size);
				memfree(m_obj);
				m_obj = nullptr;
				m_big_obj = new_obj;
				return true;
			}
		}
		return false;
	}
	inline void Variant::do_small_arr_insert(usize i, const Variant& v)
	{
		do_small_arr_reserve((usize)m_array_or_object_header.m_size + 1);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->insert(m_big_arr->begin() + i, v);
		}
		else
		{
			if (i != (usize)m_array_or_object_header.m_size)
			{
				move_relocate_range_backward(m_arr + i, m_arr + (usize)m_array_or_object_header.m_size,
					m_arr + (usize)m_array_or_object_header.m_size + 1);
			}
			new (m_arr + i) Variant(v);
			++m_array_or_object_header.m_size;
		}
	}
	inline void Variant::do_small_arr_insert(usize i, Variant&& v)
	{
		do_small_arr_reserve((usize)m_array_or_object_header.m_size + 1);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->insert(m_big_arr->begin() + i, move(v));
		}
		else
		{
			if (i != (usize)m_array_or_object_header.m_size)
			{
				move_relocate_range_backward(m_arr + i, m_arr + (usize)m_array_or_object_header.m_size,
					m_arr + (usize)m_array_or_object_header.m_size + 1);
			}
			new (m_arr + i) Variant(move(v));
			++m_array_or_object_header.m_size;
		}
	}
	inline void Variant::do_small_arr_push(const Variant& v)
	{
		do_small_arr_reserve((usize)m_array_or_object_header.m_size + 1);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->push_back(v);
		}
		else
		{
			new (m_arr + (usize)m_array_or_object_header.m_size) Variant(v);
			++m_array_or_object_header.m_size;
		}
	}
	inline void Variant::do_small_arr_push(Variant&& v)
	{
		do_small_arr_reserve((usize)m_array_or_object_header.m_size + 1);
		if (test_flags(m_array_flag, ArrayFlag::big_array))
		{
			m_big_arr->push_back(move(v));
		}
		else
		{
			new (m_arr + (usize)m_array_or_object_header.m_size) Variant(move(v));
			++m_array_or_object_header.m_size;
		}
	}
	inline void Variant::do_small_arr_erase(usize i)
	{
		lucheck(i < (usize)m_array_or_object_header.m_size);
		m_arr[i].~Variant();
		if (i != (usize)m_array_or_object_header.m_size - 1)
		{
			move_relocate_range(m_arr + i + 1, m_arr + (usize)m_array_or_object_header.m_size, m_arr + i);
		}
		--m_array_or_object_header.m_size;
	}
	inline void Variant::do_small_arr_erase(usize begin, usize end)
	{
		lucheck(end >= begin);
		lucheck(end <= (usize)m_array_or_object_header.m_size);
		destruct_range(m_arr + begin, m_arr + end);
		if (end != (usize)m_array_or_object_header.m_size)
		{
			move_relocate_range(m_arr + end, m_arr + (usize)m_array_or_object_header.m_size, m_arr + begin);
		}
		m_array_or_object_header.m_size -= (u16)(end - begin);
	}
	inline void Variant::do_small_arr_pop()
	{
		lucheck(!empty());
		m_arr[(usize)m_array_or_object_header.m_size - 1].~Variant();
		--m_array_or_object_header.m_size;
	}
	inline Variant& Variant::do_small_obj_push(const Name& k)
	{
		do_small_obj_reserve((usize)m_array_or_object_header.m_size + 1);
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto res = m_big_obj->insert(make_pair(k, Variant()));
			return res.first->second;
		}
		new (m_obj + (usize)m_array_or_object_header.m_size) Pair<const Name, Variant>(k, Variant());
		++m_array_or_object_header.m_size;
		return m_obj[(usize)m_array_or_object_header.m_size - 1].second;
	}
	inline bool Variant::do_small_obj_insert(const Name& k, const Variant& v)
	{
		do_small_obj_reserve((usize)m_array_or_object_header.m_size + 1);
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto res = m_big_obj->insert(make_pair(k, v));
			return res.second;
		}
		for (usize i = 0; i < (usize)m_array_or_object_header.m_size; ++i)
		{
			if (m_obj[i].first == k)
			{
				return false;
			}
		}
		new (m_obj + (usize)m_array_or_object_header.m_size) Pair<const Name, Variant>(k, v);
		++m_array_or_object_header.m_size;
		return true;
	}
	inline bool Variant::do_small_obj_insert(const Name& k, Variant&& v)
	{
		do_small_obj_reserve((usize)m_array_or_object_header.m_size + 1);
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto res = m_big_obj->insert(make_pair(k, move(v)));
			return res.second;
		}
		for (usize i = 0; i < (usize)m_array_or_object_header.m_size; ++i)
		{
			if (m_obj[i].first == k)
			{
				return false;
			}
		}
		new (m_obj + (usize)m_array_or_object_header.m_size) Pair<const Name, Variant>(k, move(v));
		++m_array_or_object_header.m_size;
		return true;
	}
	inline bool Variant::do_small_obj_erase(const Name& k)
	{
		for (usize i = 0; i < (usize)m_array_or_object_header.m_size; ++i)
		{
			if (m_obj[i].first == k)
			{
				m_obj[i].~Pair<const Name, Variant>();
				if (i != (usize)m_array_or_object_header.m_size - 1)
				{
					move_relocate_range(m_obj + i + 1, m_obj + (usize)m_array_or_object_header.m_size, m_obj + i);
				}
				--m_array_or_object_header.m_size;
				return true;
			}
		}
		return false;
	}
}
