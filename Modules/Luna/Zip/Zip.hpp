/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Zip.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/Runtime/Stream.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Vector.hpp>
#include "Zip.generated.hpp"

#ifndef LUNA_ZIP_API
#define LUNA_ZIP_API
#endif

namespace Luna
{
    namespace Zip
    {
        //! @addtogroup Zip Zip
        //! ZIP and ZIP64 archive operations independent of VFS and Asset.
        //! @{

        //! Selects whether an existing archive can be edited.
        enum class OpenMode : u8
        {
            //! Allows enumeration and entry reading.
            read,
            //! Also allows staged editing and saving to a separate stream.
            read_write
        };

        //! ZIP compression method identifiers supported for encoding and decoding.
        enum class CompressionMethod : u16
        {
            //! Stores bytes without compression.
            store = 0,
            //! Compresses bytes with Deflate.
            deflate = 8
        };

        //! Describes one entry in the current archive session.
        struct EntryInfo
        {
            //! Session-local index. It is invalid after successful save or discard.
            u64 index = 0;
            //! Entry name, decoded to UTF-8. This is not a validated filesystem path.
            String name;
            //! Uncompressed byte count.
            u64 size = 0;
            //! Stored byte count; meaningful only when @ref has_compressed_size is true.
            u64 compressed_size = 0;
            //! CRC32 of uncompressed data; meaningful only when @ref has_crc32 is true.
            u32 crc32 = 0;
            //! ZIP compression identifier; imported entries may contain other identifiers.
            CompressionMethod compression = CompressionMethod::store;
            //! Whether the entry name ends with '/'.
            bool directory = false;
            //! Whether the entry is encrypted. Encryption is not supported by this module yet.
            bool encrypted = false;
            //! Whether the current stored size is known. Staged entries may not have one.
            bool has_compressed_size = false;
            //! Whether the current CRC32 is known. Staged entries may not have one.
            bool has_crc32 = false;
        };

        //! A ZIP archive session. Edits remain staged until an explicit save.
        //! @details All operations on an archive, its readers, and shared source streams
        //! must be externally serialized. Sources are retained and must not be externally
        //! modified during the session. Releasing a session discards unsaved edits.
        struct [[Luna::interface("{4E72DC50-B990-47F8-B21D-2305DBAA3A8D}")]] IArchive : virtual Interface
        {
            //! Tests whether this session has not been saved or discarded.
            virtual bool is_open() = 0;
            //! Lists current entries, skipping deleted entries. Indices may have gaps.
            virtual R<Vector<EntryInfo>> get_entries() = 0;
            //! Gets one entry's current information.
            //! @param[in] index The index returned by enumeration, lookup, or addition.
            virtual R<EntryInfo> get_entry(u64 index) = 0;
            //! Finds an entry by its exact, case-sensitive UTF-8 name.
            //! @param[in] name The entry name. Duplicate imported names remain separately
            //! addressable by index; name lookup returns the first matching entry.
            virtual R<u64> find_entry(const c8* name) = 0;
            //! Opens a sequential, read-only stream of uncompressed entry bytes.
            //! @param[in] index The entry index.
            //! @details Store and Deflate are supported; encrypted or unsupported methods
            //! return E_NOT_SUPPORTED. Directory entries return E_IS_DIRECTORY.
            //! The reader retains this archive. While any reader exists, mutations,
            //! save and discard return E_BUSY. Read through EOF to verify a stored CRC
            //! when present. Newly staged data has no stored CRC to compare yet.
            virtual R<Ref<IStream>> open_entry(u64 index) = 0;
            //! Adds a file from the complete contents of a seekable source stream.
            //! @param[in] name A non-empty UTF-8 entry name not ending with '/'.
            //! @param[in] source The source, retained and read lazily from offset zero.
            //! @param[in] compression Store or Deflate.
            //! @param[in] level Deflate level 1-9, or 0 for its default. Store requires 0.
            //! @return Returns the new session-local index. Existing names fail with E_ALREADY_EXISTS.
            virtual R<u64> add_file(const c8* name, ISeekableStream* source,
                CompressionMethod compression = CompressionMethod::deflate, u32 level = 0) = 0;
            //! Adds an explicit directory entry. A trailing '/' is appended if necessary.
            //! @param[in] name The non-empty UTF-8 directory name.
            //! @return Returns the new index. This does not create parent entries.
            virtual R<u64> add_directory(const c8* name) = 0;
            //! Replaces a file's data, preserving its current compression method.
            //! @param[in] index The existing file index.
            //! @param[in] source The complete replacement source, retained until released by the archive.
            //! @details Compression level uses the library default; use set_compression
            //! afterward to select a particular level. Entry extra fields may be removed
            //! by libzip because they can depend on the replaced data.
            virtual RV replace_file(u64 index, ISeekableStream* source) = 0;
            //! Renames exactly one entry; directory descendants are not renamed.
            //! @param[in] index The entry index.
            //! @param[in] name The new UTF-8 name. Its trailing '/' must preserve the entry kind.
            virtual RV rename_entry(u64 index, const c8* name) = 0;
            //! Deletes exactly one entry. Deleting a directory marker is not recursive.
            //! @param[in] index The entry index.
            virtual RV delete_entry(u64 index) = 0;
            //! Changes a file's encoding on the next save.
            //! @param[in] index The file index.
            //! @param[in] compression Store or Deflate.
            //! @param[in] level Deflate level 1-9 or default 0; Store requires 0.
            virtual RV set_compression(u64 index, CompressionMethod compression, u32 level = 0) = 0;
            //! Writes the complete edited ZIP to a separate output stream.
            //! @param[in] destination A writable, seekable, resizable stream. Its contents
            //! are replaced and its cursor is left at an unspecified position.
            //! @details The output must not alias the input or any entry source, even through
            //! a different handle. Pointer-identical aliases are rejected. Success closes
            //! this session. Failure leaves the session open for retry, but the output may
            //! be incomplete. No filesystem replacement or durable synchronization is implied.
            //! No write occurs before this call. Read-only sessions return E_ACCESS_DENIED.
            virtual RV save(ISeekableStream* destination) = 0;
            //! Discards unsaved edits and closes the session without writing any stream.
            //! @details Already closed sessions succeed. Open readers cause E_BUSY.
            virtual RV discard() = 0;
        };

        //! Opens an existing ZIP from a retained seekable stream.
        //! @param[in] source The entire stream, interpreted from offset zero independently
        //! of its initial cursor. Its contents and size must remain unchanged while retained.
        //! @param[in] mode Whether edits may be staged. Even read_write requires only read
        //! and seek access to this input; saving always uses a different output.
        //! @return Returns a new session, or an error for malformed/truncated ZIP input.
        LUNA_ZIP_API R<Ref<IArchive>> open_archive(ISeekableStream* source, OpenMode mode = OpenMode::read);
        //! Creates an empty editable ZIP session. No stream is written until save.
        LUNA_ZIP_API R<Ref<IArchive>> new_archive();
        //! @}
    }

    struct Module;
    //! Gets the Zip module. Initialize it before using Zip interfaces and release all
    //! archive/reader objects before Runtime shutdown.
    LUNA_ZIP_API Module* module_zip();
}
