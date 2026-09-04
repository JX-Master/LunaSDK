/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Handles.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "VFS.hpp"
#include <Luna/Runtime/Algorithm.hpp>

namespace Luna::VFS
{
    MountedFile::~MountedFile()
    {
        if(!m_mutex) return;
        MutexGuard guard(m_mutex);
        if(m_mount)
        {
            auto& files = m_mount->m_files;
            auto found = find(files.begin(), files.end(), this);
            if(found != files.end()) files.erase(found);
        }
        invalidate();
    }
    void MountedFile::invalidate()
    {
        m_file = nullptr;
        m_mount = nullptr;
    }
    RV MountedFile::read(void* buffer, usize size, usize* count)
    {
        MutexGuard guard(m_mutex);
        if(count) *count = 0;
        return m_file ? m_file->read(buffer, size, count) : RV(E_BAD_CALLING_TIME);
    }
    RV MountedFile::write(const void* buffer, usize size, usize* count)
    {
        MutexGuard guard(m_mutex);
        if(count) *count = 0;
        return m_file ? m_file->write(buffer, size, count) : RV(E_BAD_CALLING_TIME);
    }
    R<u64> MountedFile::tell()
    {
        MutexGuard guard(m_mutex);
        return m_file ? m_file->tell() : R<u64>(E_BAD_CALLING_TIME);
    }
    RV MountedFile::seek(i64 offset, SeekMode mode)
    {
        MutexGuard guard(m_mutex);
        return m_file ? m_file->seek(offset, mode) : RV(E_BAD_CALLING_TIME);
    }
    u64 MountedFile::get_size()
    {
        MutexGuard guard(m_mutex);
        return m_file ? m_file->get_size() : 0;
    }
    RV MountedFile::set_size(u64 size)
    {
        MutexGuard guard(m_mutex);
        return m_file ? m_file->set_size(size) : RV(E_BAD_CALLING_TIME);
    }
    void MountedFile::flush()
    {
        MutexGuard guard(m_mutex);
        if(m_file) m_file->flush();
    }

    MountedIterator::~MountedIterator()
    {
        if(!m_mutex) return;
        MutexGuard guard(m_mutex);
        if(m_mount)
        {
            auto& iterators = m_mount->m_iterators;
            auto found = find(iterators.begin(), iterators.end(), this);
            if(found != iterators.end()) iterators.erase(found);
        }
        invalidate();
    }
    void MountedIterator::invalidate()
    {
        m_iterator = nullptr;
        m_mount = nullptr;
    }
    bool MountedIterator::is_valid()
    {
        MutexGuard guard(m_mutex);
        return m_iterator && m_iterator->is_valid();
    }
    const c8* MountedIterator::get_filename()
    {
        MutexGuard guard(m_mutex);
        return m_iterator ? m_iterator->get_filename() : nullptr;
    }
    FileAttributeFlag MountedIterator::get_attributes()
    {
        MutexGuard guard(m_mutex);
        return m_iterator ? m_iterator->get_attributes() : FileAttributeFlag::none;
    }
    bool MountedIterator::move_next()
    {
        MutexGuard guard(m_mutex);
        return m_iterator && m_iterator->move_next();
    }
}
