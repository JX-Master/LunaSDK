/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Pak.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/Runtime/File.hpp>
#include "Pak.generated.hpp"

#ifndef LUNA_PAK_API
#define LUNA_PAK_API
#endif

namespace Luna
{
    namespace Pak
    {
        //! @addtogroup Pak Pak
        //! Directory trees stored in single ZIP/ZIP64 streams, independent of VFS and Asset.
        //! @{

        //! Selects package access. Editable packages still read their input without writing it.
        enum class OpenMode : u8
        {
            //! Allows file reads and metadata queries.
            read,
            //! Also allows staged changes and flushing to a separate stream.
            read_write
        };

        //! Supported ZIP compression methods.
        enum class CompressionMethod : u16
        {
            //! Stores file data without compression.
            store = 0,
            //! Compresses file data with Deflate.
            deflate = 8
        };

        //! Session options for newly created files and writable staging storage.
        struct Options
        {
            //! The default compression for new files. Existing files keep their method.
            CompressionMethod compression = CompressionMethod::store;
            //! Deflate level 1-9 or default 0. Store requires 0.
            u32 compression_level = 0;
            //! Maximum size of one built-in memory staging stream, in uncompressed bytes.
            //! Does not limit read-only streaming or custom staging streams.
            u64 max_memory_file_size = 256 * 1024 * 1024;
            //! Optional retained userdata passed to @ref create_staging_stream.
            ObjRef staging_userdata;
            //! Optional factory for private, readable, writable, seekable, resizable staging streams.
            //! @details Pak resets each returned stream to zero length. It must not alias the
            //! package source or another staging stream, even through separate handles. Pak
            //! exclusively owns its contents/cursor while retained. The callback runs under
            //! the package mutex and must not reenter the package. The object's lifetime
            //! should clean up temporary storage when needed. Null uses memory staging.
            R<Ref<ISeekableStream>> (*create_staging_stream)(object_t userdata) = nullptr;
        };

        //! A filesystem-like directory tree with staged edits and repeatable flush.
        //! @details Paths are case-sensitive UTF-8 with '/' separators. Empty and '/' name
        //! the root; an optional leading slash and directory trailing slash are accepted.
        //! Other empty, '.' and '..' components, backslashes and colons are rejected.
        //! Package and file-handle operations are internally serialized. Separately opened
        //! packages or external users sharing a stream require additional synchronization.
        //! Destruction discards changes; file handles retain the package.
        struct [[Luna::interface("{B9DD7BC7-88AF-401C-8721-1682BB870599}")]] IPak : virtual Interface
        {
            //! Tests whether the package has not been explicitly discarded.
            virtual bool is_open() = 0;
            //! Tests whether this package permits edits.
            virtual bool is_read_only() = 0;
            //! Tests whether changes or a newly created empty package await flush.
            //! A successful flush clears this flag; external publication is caller-owned.
            virtual bool is_dirty() = 0;
            //! Opens a file with Runtime access flags and creation modes.
            //! @param[in] path A file path. Its parent must exist. A trailing slash requires a directory.
            //! @param[in] flags Read and/or write access. user_buffering is accepted as a hint.
            //! @param[in] creation The file creation/truncation policy. Creating/truncating requires write access.
            //! @details A file permits multiple readers or one exclusive writer; conflicting
            //! opens return E_BUSY. Writable opening copies existing data into staging unless
            //! truncating. Read-only seeking replays ZIP decoding as needed; it is not constant-time.
            //! Growth zero-fills gaps. IFile::flush only flushes staging, not the package.
            virtual R<Ref<IFile>> open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation) = 0;
            //! Queries a file or directory. Timestamp fields are zero. Read-only packages
            //! report FileAttributeFlag::read_only on all nodes.
            //! @param[in] path The file or directory path.
            virtual R<FileAttribute> get_file_attribute(const c8* path) = 0;
            //! Returns a sorted snapshot of one directory's direct children, excluding '.' and '..'.
            //! @param[in] path The directory path. Snapshots do not block editing or flush.
            virtual R<Ref<IFileIterator>> open_dir(const c8* path) = 0;
            //! Creates one directory. Its parent must exist; existing nodes return E_ALREADY_EXISTS.
            //! @param[in] path The non-root directory path.
            virtual RV create_dir(const c8* path) = 0;
            //! Copies one file's contents and compression to a new path with existing parent.
            //! @param[in] from_path The source file. An active writer causes E_BUSY.
            //! @param[in] to_path The new file path. Existing nodes cause E_ALREADY_EXISTS.
            virtual RV copy_file(const c8* from_path, const c8* to_path) = 0;
            //! Moves a file or directory subtree, with optional replacement of an existing file.
            //! @param[in] from_path The non-root source path.
            //! @param[in] to_path The new path with an existing parent.
            //! @details Open files in the affected subtree cause E_BUSY. A directory cannot
            //! be moved into itself. Validation failure leaves the tree unchanged.
            //! @param[in] flags Allows file replacement with allow_overwrite. no_copy is always satisfied
            //! within one package. Replacing directories or entries with open handles is rejected.
            virtual RV move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags = FileMoveFlag::none) = 0;
            //! Deletes one file or empty directory. The root cannot be deleted.
            //! @param[in] path The path to delete. An open file causes E_BUSY.
            virtual RV delete_file(const c8* path) = 0;
            //! Gets the selected compression of a file.
            //! @param[in] path The file path.
            virtual R<CompressionMethod> get_file_compression(const c8* path) = 0;
            //! Changes a closed file's compression at the next flush.
            //! @param[in] path The file path. Open files cause E_BUSY.
            //! @param[in] method Store or Deflate.
            //! @param[in] level Deflate level 1-9 or default 0; Store requires 0.
            virtual RV set_file_compression(const c8* path, CompressionMethod method, u32 level = 0) = 0;
            //! Writes a complete package and adopts the output for subsequent operations.
            //! @param[in] destination A distinct readable, writable, seekable, resizable stream.
            //! @details All file handles must be released, otherwise E_BUSY is returned.
            //! The output must not alias the current source or retained staging, even through
            //! other handles. Identical pointers are rejected. Output contents are replaced.
            //! Success leaves the package open, releases previous sources/staging, and clears
            //! dirty state. Failure preserves pending changes and old input for retry, but
            //! output can be partial. Even unchanged packages write a complete output.
            //! Read-only packages return E_ACCESS_DENIED. No filesystem publication or durable
            //! synchronization is implied; the adopted output must remain unchanged while retained.
            virtual RV flush(ISeekableStream* destination) = 0;
            //! Closes the package without saving. Open files cause E_BUSY. Already closed
            //! packages succeed. Destruction also discards pending changes without writes.
            virtual RV discard() = 0;
        };

        //! Opens a package and builds its directory index without decoding file payloads.
        //! @param[in] source Retained readable/seekable input, interpreted from offset zero.
        //! Its contents and size must remain unchanged while retained; Pak may move its cursor.
        //! @param[in] mode Package access. read_write does not require writable input.
        //! @param[in] options Compression defaults for new files and staging configuration.
        //! @details Invalid paths, duplicate names, file/directory collisions and non-empty
        //! directory entries return E_BAD_DATA. Encrypted/unsupported codecs return E_NOT_SUPPORTED.
        //! Missing parent directory entries are synthesized. ZIP link/permission attributes
        //! are not interpreted. ZIP/ZIP64 sizes are limited to signed 64-bit stream offsets.
        LUNA_PAK_API R<Ref<IPak>> open_pak(ISeekableStream* source,
            OpenMode mode = OpenMode::read, const Options& options = Options());
        //! Creates an empty editable package without writing any stream.
        //! @param[in] options Compression defaults and staging configuration.
        LUNA_PAK_API R<Ref<IPak>> new_pak(const Options& options = Options());
        //! @}
    }

    struct Module;
    //! Gets the Pak module, which registers Zip as a dependency. Initialize before use
    //! and release all package/file/iterator objects before Runtime shutdown.
    LUNA_PAK_API Module* module_pak();
}
