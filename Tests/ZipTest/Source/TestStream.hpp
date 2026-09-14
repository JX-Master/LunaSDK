/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file TestStream.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/Runtime/Stream.hpp>
#include <Luna/Runtime/Vector.hpp>
#include "TestStream.generated.hpp"

namespace Luna
{
    struct [[Luna::struct("{AA5C00C0-5F63-4488-B1BE-38513D62A5E0}")]] ZipTestStream : ISeekableStream
    {
        luiimpl();
        Vector<byte_t> bytes;
        u64 position = 0;
        usize chunk_size = USIZE_MAX;
        u64 fail_write_at = U64_MAX;
        bool fail_resize = false;
        bool fail_commit = false;
        bool read_only = false;
        u64 bytes_read = 0;
        u64 bytes_written = 0;

        RV read(void* buffer, usize size, usize* count) override
        {
            usize amount = position >= bytes.size() ? 0 : min<usize>(size, bytes.size() - (usize)position);
            amount = min(amount, chunk_size);
            if(amount) memcpy(buffer, bytes.data() + (usize)position, amount);
            position += amount;
            bytes_read += amount;
            if(count) *count = amount;
            return ok;
        }
        RV write(const void* buffer, usize size, usize* count) override
        {
            if(count) *count = 0;
            if(read_only) return E_ACCESS_DENIED;
            if(position >= fail_write_at) return E_IO_ERROR;
            usize amount = min<usize>(min<u64>(size, fail_write_at - position), chunk_size);
            if(position > USIZE_MAX - amount) return E_FILE_TOO_BIG;
            if(position + amount > bytes.size())
            {
                if(position + amount > bytes.capacity()) bytes.reserve(max<usize>((usize)position + amount, bytes.capacity() * 2));
                bytes.resize((usize)position + amount);
            }
            if(amount) memcpy(bytes.data() + (usize)position, buffer, amount);
            position += amount;
            bytes_written += amount;
            if(count) *count = amount;
            return ok;
        }
        R<u64> tell() override { return position; }
        RV seek(i64 offset, SeekMode mode) override
        {
            i64 base = mode == SeekMode::begin ? 0 : (mode == SeekMode::current ? (i64)position : (i64)bytes.size());
            if(offset < -base || (offset > 0 && base > I64_MAX - offset)) return E_BAD_ARGUMENTS;
            position = (u64)(base + offset);
            return ok;
        }
        u64 get_size() override { return bytes.size(); }
        RV set_size(u64 size) override
        {
            if(read_only) return E_ACCESS_DENIED;
            if(fail_resize || (fail_commit && size)) return E_IO_ERROR;
            if(size > USIZE_MAX) return E_FILE_TOO_BIG;
            bytes.resize((usize)size);
            return ok;
        }
    };
}
