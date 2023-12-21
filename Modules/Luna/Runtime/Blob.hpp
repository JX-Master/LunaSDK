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

	//! Represents one binary large object (BLOB).
	//! @details Blob can be used as a RAII wrapper for arbitrary memory allocation. One blob object allocates and manages one memory block that 
	//! stores the blob data, which we call the "managed memory" of the blob object. The managed memory is always allocated from @ref memalloc or @ref memrealloc.
	//! The user can also allocate the memory manually then attach it to one blob object by calling @ref Blob::attach, or call @ref Blob::detach to take ownership
	//! of the managed memory from one blob object.
	class Blob
	{
		byte_t* m_buffer;
		usize m_size;
		usize m_alignment;

		void do_destruct();
	public:
		//! @name Constructors
		//! @{

		//! Constructs one empty blob. One empty blob will not allocate any memory. 
		Blob();
		//! Constructs one blob object and allocate memory for it.
		//! @param[in] sz The size, in bytes, of the memory to allocate.
		//! @param[in] alignment The optional alignment, in bytes, of the memory to allocate.
		Blob(usize sz, usize alignment = 0);
		//! Constructs the blob object with initial data. 
		//! @param[in] blob_data A pointer to the data to initialize the blob.
		//! @param[in] data_sz The size, in bytes, of the data pointed by `blob_data`.
		//! @param[in] alignment The optional alignment, in bytes, of the memory to allocate.
		//! @details The blob object will allocate memory for the data and copies the data into the blob memory.
		Blob(const byte_t* blob_data, usize data_sz, usize alignment = 0);
		//! Constructs one blob object with a list of elements.
		//! @param[in] blob_data The @ref Span that represents the list. The blob allocates enough memory to hold all elements in the list. 
		//! Every element in the list will be copied as if by @ref memcpy.
		//! @param[in] alignment The optional alignment, in bytes, of the memory to allocate.
		template <usize _Size>
		Blob(Span<const byte_t, _Size> blob_data, usize alignment = 0);
		//! Constructs one blob object by coping data from another blob object.
		//! @param[in] rhs The blob object to copy data from.
		Blob(const Blob& rhs);
		//! Constructs one blob object by moving data from another blob object.
		//! @param[in] rhs The blob object to move data from. This blob object will be empty after this operation.
		Blob(Blob&& rhs);

		//! @}

		//! @name Assignments
		//! @{

		//! Assigns the data of the blob object by coping data from another blob object.
		//! @param[in] rhs The blob object to copy data from.
		Blob& operator=(const Blob& rhs);
		//! Assigns the data of the blob object by moving data from another blob object.
		//! @param[in] rhs The blob object to move data from. This blob object will be empty after this operation.
		Blob& operator=(Blob&& rhs);

		//! @}

		~Blob();

		//! @name Accessors
		//! @{
		
		//! Gets one pointer to the data of the blob object.
		//! @return Returns the pointer to the data of the blob object. Returns `nullptr` if @ref empty returns `true`.
		const byte_t* data() const;
		//! Gets one pointer to the data of the blob object.
		//! @return Returns the pointer to the data of the blob object. Returns `nullptr` if @ref empty returns `true`.
		byte_t* data();
		//! Gets the size of the memory managed by this blob object.
		//! @return Returns the size, in bytes, of the memory managed by this blob object.
		usize size() const;
		//! Gets the managed memory of this blob object as a @ref Span of @ref byte_t.
		//! @return Returns the span that represents the managed memory of this blob object.
		Span<const byte_t> span() const;
		//! Gets the managed memory of this blob object as a const @ref Span of @ref byte_t.
		//! @return Returns the const span that represents the managed memory of this blob object.
		Span<const byte_t> cspan() const;
		//! Gets the managed memory of this blob object as a @ref Span of @ref byte_t.
		//! @return Returns the span that represents the managed memory of this blob object.
		Span<byte_t> span();
		//! Gets the alignment of the memory managed by this blob object.
		//! @return Returns the alignment, in bytes, of the memory managed by this blob object.
		usize alignment() const;
		//! Checks whether this blob object is empty, that is, contains no allocated memory. 
		//! One blob object is empty if and only if its @ref size equals to `0`.
		//! @return Returns `true` if the blob object is empty, returns `false` otherwise.
		bool empty() const;

		//! @}

		//! @name Operations
		//! @{

		//! Resizes the underlying memory.
		//! @param[in] sz The new size, in bytes, of the new managed memory for the blob object. If this is `0`, this function bahaves the
		//! same as @ref clear.
		void resize(usize sz, bool keep_content = true);
		//! Frees managed memory of this blob object. This blob object is empty after this operation.
		void clear();
		//! Attaches a user-allocated memory to the blob object as the managed memory of this blob object.
		//! @param[in] data The pointer to the memory to attach. This memory must be allocated by @ref memalloc or @ref memrealloc.
		//! @param[in] size The size, in bytes, of the memory to attach. This must be equal to the size passed to @ref memalloc or @ref memrealloc when allocating the memory.
		//! @param[in] alignment The alignment, in bytes, of the memory to attach. This must be equal to the alignment passed to @ref memalloc or @ref memrealloc when allocating the memory.
		//! This can be `0` if `0` is also passed to @ref memalloc or @ref memrealloc when allocating the memory.
		void attach(byte_t* data, usize size, usize alignment);
		//! Detaches the managed memory of one blob object. The blob object is empty after this operation.
		//! @return Returns the pointer to the detached managed memory of the blob object. Returns `nullptr` is this blob object is already empty.
		byte_t* detach();
		
		//! @}
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
	inline void Blob::resize(usize sz, bool keep_content)
	{
		if(keep_content)
		{
			m_buffer = (byte_t*)memrealloc(m_buffer, sz, m_alignment);
		}
		else
		{
			byte_t* old_buffer = m_buffer;
			m_buffer = (byte_t*)memalloc(sz, m_alignment);
			if(old_buffer) memfree(old_buffer, m_alignment);
		}
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