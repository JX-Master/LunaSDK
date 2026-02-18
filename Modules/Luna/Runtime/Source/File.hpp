/*!
* This file is a portion of LunaSDK.
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
#include "Platform/File.hpp"
#include "Error.hpp"

namespace Luna
{
    struct File : IFile
    {
        lustruct("File", "{915247e4-15b4-44ba-8781-dd7dcfd48f87}");
        luiimpl();
        lutsassert_lock();

        Platform::File m_file;

        File() {}
        ~File()
        {
            if (m_file.valid())
            {
                Platform::close_file(m_file);
            }
        }
        virtual RV read(void* buffer, usize size, usize* read_bytes) override
        {
            return encode_platform_result(Platform::read_file(m_file, buffer, size, read_bytes));
        }
        virtual RV write(const void* buffer, usize size, usize* write_bytes) override
        {
            return encode_platform_result(Platform::write_file(m_file, buffer, size, write_bytes));
        }
        virtual u64 get_size() override
        {
            return Platform::get_file_size(m_file);
        }
        virtual RV set_size(u64 sz) override
        {
            return encode_platform_result(Platform::set_file_size(m_file, sz));
        }
        virtual R<u64> tell() override
        {
            u64 cursor;
            auto r = Platform::get_file_cursor(m_file, cursor);
            if(r != Platform::Result::success) return encode_platform_result(r).errcode();
            return cursor;
        }
        virtual RV seek(i64 offset, SeekMode mode) override
        {
            return encode_platform_result(Platform::set_file_cursor(m_file, offset, mode));
        }
        virtual void flush() override
        {
            Platform::flush_file(m_file);
        }
    };
    struct FileIterator : IFileIterator
    {
        lustruct("FileIterator", "{bd87c27c-34ed-4764-8417-6ef37c316ed3}");
        luiimpl();
        lutsassert_lock();

        Platform::FileIterator m_iter;

        ~FileIterator()
        {
            if (m_iter.valid())
            {
                Platform::close_dir(m_iter);
            }
        }
        virtual bool is_valid() override
        {
            return Platform::dir_iterator_is_valid(m_iter);
        }
        virtual const char* get_filename() override
        {
            return Platform::dir_iterator_get_filename(m_iter);
        }
        virtual FileAttributeFlag get_attributes() override
        {
            return Platform::dir_iterator_get_attributes(m_iter);
        }
        virtual bool move_next() override
        {
            return Platform::dir_iterator_move_next(m_iter);
        }
    };
}