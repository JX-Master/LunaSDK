/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Variant.inl
* @author JXMaster
* @date 2023/12/5
*/
#pragma once
#include "../Variant.hpp"

namespace Luna
{
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
			if (size() != rhs.size()) return false;
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
		if (type() != VariantType::array) return Variant::npos();
		if (i >= size()) return Variant::npos();
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
		if (type() != VariantType::object) return Variant::npos();
		if (test_flags(m_object_flag, ObjectFlag::big_object))
		{
			auto iter = m_big_obj->find(k);
			if (iter != m_big_obj->end())
			{
				return iter->second;
			}
			return Variant::npos();
		}
		for (usize i = 0; i < m_array_or_object_header.m_size; ++i)
		{
			if (k == m_obj[i].first)
			{
				return m_obj[i].second;
			}
		}
		return Variant::npos();
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