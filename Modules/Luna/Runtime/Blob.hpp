/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Blob.hpp
* @author JXMaster
* @date 2020/8/17
*/
#pragma once
#include "Memory.hpp"
#include "Span.hpp"
#include "TypeInfo.hpp"
namespace Luna
{
	//! @addtogroup Runtime
	//! @{

	//! @brief Represents one binary large object (BLOB).
	//! @details Blob is used to pass arbitrary binary data.
	class Blob
	{
		byte_t* m_buffer;
		usize m_size;
		usize m_alignment;

		void do_destruct();
	public:
		//! @name Constructors
		//! @{

		//! @brief Constructs one empty blob. One empty blob will not allocate any memory. 
		Blob();
		//! @brief Constructs one blob object and allocate memory for it.
		//! @param[in] sz The size, in bytes, of the memory to allocate.
		//! @param[in] alignment The optional alignment, in bytes, of the memory to allocate.
		Blob(usize sz, usize alignment = 0);
		//! @brief Constructs the blob object with initial data. 
		//! @param[in] blob_data A pointer to the data to initialize the blob.
		//! @param[in] data_sz The size, in bytes, of the data pointed by `blob_data`.
		//! @param[in] alignment The optional alignment, in bytes, of the memory to allocate.
		//! @details The blob object will allocate memory for the data and copies the data into the blob memory.
		Blob(const byte_t* blob_data, usize data_sz, usize alignment = 0);
		//! @brief 
		template <usize _Size>
		Blob(Span<const byte_t, _Size> blob_data, usize alignment = 0);
		Blob(const Blob& rhs);
		Blob(Blob&& rhs);

		//! @}

		Blob& operator=(const Blob& rhs);
		Blob& operator=(Blob&& rhs);
		~Blob();
		const byte_t* data() const;
		byte_t* data();
		usize size() const;
		Span<const byte_t> span() const;
		Span<const byte_t> cspan() const;
		Span<byte_t> span();
		usize alignment() const;
		bool empty() const;
		void resize(usize sz);
		void clear();
		void attach(byte_t* data, usize size, usize alignment);
		byte_t* detach();
	};

	//! @}

	inline void Blob::do_destruct()
	{
		if (m_buffer)
		{
			memfree(m_buffer, m_alignment);
			m_buffer = nullptr;
		}
	}
	inline Blob::Blob() :
		m_buffer(nullptr),
		m_size(0),
		m_alignment(0) {}

	inline Blob::Blob(usize sz, usize alignment) :
		m_size(sz),
		m_alignment(alignment)
	{
		m_buffer = (byte_t*)memalloc(sz, alignment);
	}
	inline Blob::Blob(const byte_t* blob_data, usize data_sz, usize alignment) :
		m_size(data_sz),
		m_alignment(alignment)
	{
		m_buffer = (byte_t*)memalloc(data_sz, alignment);
		memcpy(m_buffer, blob_data, data_sz);
	}
	template <usize _Size>
	inline Blob::Blob(Span<const byte_t, _Size> blob_data, usize alignment) :
		m_size(blob_data.size()),
		m_alignment(alignment)
	{
		m_buffer = (byte_t*)memalloc(blob_data.size(), alignment);
		memcpy(m_buffer, blob_data.data(), blob_data.size());
	}
	inline Blob::Blob(const Blob& rhs) :
		m_size(rhs.m_size),
		m_alignment(rhs.m_alignment),
		m_buffer(nullptr)
	{
		if (m_size)
		{
			m_buffer = (byte_t*)memalloc(m_size, m_alignment);
			memcpy(m_buffer, rhs.m_buffer, m_size);
		}
	}
	inline Blob::Blob(Blob&& rhs) :
		m_size(rhs.m_size),
		m_alignment(rhs.m_alignment),
		m_buffer(rhs.m_buffer)
	{
		rhs.m_size = 0;
		rhs.m_alignment = 0;
		rhs.m_buffer = nullptr;
	}
	inline Blob& Blob::operator=(const Blob& rhs)
	{
		do_destruct();
		m_size = rhs.m_size;
		m_alignment = rhs.m_alignment;
		if (m_size)
		{
			m_buffer = (byte_t*)memalloc(m_size, m_alignment);
			memcpy(m_buffer, rhs.m_buffer, m_size);
		}
		return *this;
	}
	inline Blob& Blob::operator=(Blob&& rhs)
	{
		do_destruct();
		m_size = rhs.m_size;
		m_alignment = rhs.m_alignment;
		m_buffer = rhs.m_buffer;
		rhs.m_size = 0;
		rhs.m_alignment = 0;
		rhs.m_buffer = nullptr;
		return *this;
	}
	inline Blob::~Blob()
	{
		do_destruct();
	}
	inline const byte_t* Blob::data() const
	{
		return m_buffer;
	}
	inline byte_t* Blob::data()
	{
		return m_buffer;
	}
	inline usize Blob::size() const
	{
		return m_size;
	}
	inline Span<const byte_t> Blob::span() const
	{
		return Span<const byte_t>(data(), size());
	}
	inline Span<const byte_t> Blob::cspan() const
	{
		return Span<const byte_t>(data(), size());
	}
	inline Span<byte_t> Blob::span()
	{
		return Span<byte_t>(data(), size());
	}
	inline usize Blob::alignment() const
	{
		return m_alignment;
	}
	inline bool Blob::empty() const
	{
		return m_buffer == nullptr;
	}
	inline void Blob::resize(usize sz)
	{
		m_buffer = (byte_t*)memrealloc(m_buffer, sz, m_alignment);
		m_size = sz;
	}
	inline void Blob::clear()
	{
		memfree(m_buffer, m_alignment);
		m_buffer = nullptr;
		m_size = 0;
		m_alignment = 0;
	}
	inline void Blob::attach(byte_t* data, usize size, usize alignment)
	{
		do_destruct();
		m_buffer = data;
		m_size = size;
		m_alignment = alignment;
	}
	inline byte_t* Blob::detach()
	{
		byte_t* buf = m_buffer;
		m_buffer = nullptr;
		m_size = 0;
		m_alignment = 0;
		return buf;
	}

	LUNA_RUNTIME_API typeinfo_t blob_type();
	template <> struct typeof_t<Blob> { typeinfo_t operator()() const { return blob_type(); } };
}