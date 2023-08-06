/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.hpp
* @author JXMaster
* @date 2019/9/29
*/
#pragma once
#include "../TSAssert.hpp"
#include "../File.hpp"
#include "OS.hpp"

namespace Luna
{
	struct File : IFile
	{
		lustruct("File", "{915247e4-15b4-44ba-8781-dd7dcfd48f87}");
		luiimpl();
		lutsassert_lock();

		opaque_t m_file;

		File() :
			m_file(nullptr) {}
		~File()
		{
			if (m_file)
			{
				OS::close_file(m_file);
			}
		}
		virtual RV read(void* buffer, usize size, usize* read_bytes) override
		{
			return OS::read_file(m_file, buffer, size, read_bytes);
		}
		virtual RV write(const void* buffer, usize size, usize* write_bytes) override
		{
			return OS::write_file(m_file, buffer, size, write_bytes);
		}
		virtual u64 get_size() override
		{
			return OS::get_file_size(m_file);
		}
		virtual RV set_size(u64 sz) override
		{
			return OS::set_file_size(m_file, sz);
		}
		virtual R<u64> tell() override
		{
			return OS::get_file_cursor(m_file);
		}
		virtual RV seek(i64 offset, SeekMode mode) override
		{
			return OS::set_file_cursor(m_file, offset, mode);
		}
		virtual void flush() override
		{
			OS::flush_file(m_file);
		}
	};
	struct FileIterator : IFileIterator
	{
		lustruct("FileIterator", "{bd87c27c-34ed-4764-8417-6ef37c316ed3}");
		luiimpl();
		lutsassert_lock();

		opaque_t m_handle;

		FileIterator() :
			m_handle(nullptr) {}
		~FileIterator()
		{
			if (m_handle)
			{
				OS::close_dir(m_handle);
			}
		}
		virtual bool is_valid() override
		{
			return OS::dir_iterator_is_valid(m_handle);
		}
		virtual const char* get_filename() override
		{
			return OS::dir_iterator_get_filename(m_handle);
		}
		virtual FileAttributeFlag get_attributes() override
		{
			return OS::dir_iterator_get_attributes(m_handle);
		}
		virtual bool move_next() override
		{
			return OS::dir_iterator_move_next(m_handle);
		}
	};
}