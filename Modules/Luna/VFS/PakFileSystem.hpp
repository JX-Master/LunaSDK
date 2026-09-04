/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PakFileSystem.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "FileSystem.hpp"
#include <Luna/Pak/Pak.hpp>
#include "PakFileSystem.generated.hpp"

namespace Luna::VFS
{
    //! @addtogroup VFS
    //! @{

    //! Supplies streams and publication for a Pak filesystem using application-owned storage.
    //! @details The filesystem retains this object. Callbacks run synchronously and must
    //! not reenter the filesystem or VFS. Editable instances exclusively own their storage.
    //! Returned stream lifetimes own cleanup, including incomplete outputs. A failed
    //! publication must preserve the output for retry and define retention at shutdown.
    //! All methods are pure virtual. Read-only providers must explicitly implement
    //! unsupported write operations by returning @ref E_NOT_SUPPORTED.
    struct [[Luna::interface("{C12A1299-B599-4C2D-8F35-66E4F6AA24A5}")]] IPakStorage : virtual Interface
    {
        //! Opens the existing package as a readable, seekable stream.
        //! @details Its bytes and size must remain unchanged while Pak retains it.
        virtual R<Ref<ISeekableStream>> open_source() = 0;
        //! Creates a fresh readable, writable, seekable, resizable output stream.
        //! @details It must not alias any retained input or staging stream, even through
        //! another handle. Called only for dirty editable packages.
        virtual R<Ref<ISeekableStream>> create_output() = 0;
        //! Publishes one complete output previously returned by create_output.
        //! @param[in] output The stream that Pak has already adopted as its next source.
        //! @details Success makes this the current package in storage. Failure must leave
        //! the previous publication unchanged and keep this output usable for retry. The
        //! filesystem retains it and retries the same stream. Success must also preserve the
        //! output's bytes and readable/seekable stream identity.
        virtual RV publish(ISeekableStream* output) = 0;
    };

    //! Creates a filesystem instance for an existing native Pak file.
    //! @param[in] native_path Native package path, resolved at creation time.
    //! @param[in] mode Read-only or read/write access to the package.
    //! @param[in] options Compression defaults and writable-file staging configuration.
    //! @return Returns an instance usable directly or through VFS::mount.
    //! @details Native writeback uses sibling temporary files and checked replacement.
    //! Failed publication retains the output for retry and blocks further edits.
    //! Call IFileSystem::flush to publish edits. Destruction discards unsaved edits.
    LUNA_VFS_API R<Ref<IFileSystem>> new_pak_file_system(const c8* native_path,
        Pak::OpenMode mode = Pak::OpenMode::read, const Pak::Options& options = Pak::Options());
    //! Creates a filesystem instance for an existing package in custom storage.
    //! @param[in] storage The retained storage provider. Must not be null.
    //! @param[in] mode Read-only or read/write access to the package.
    //! @param[in] options Compression defaults and writable-file staging configuration.
    //! @return Returns an instance with the same file, flush and lifetime semantics as
    //! the native-file overload. Creating or mounting it does not register assets.
    LUNA_VFS_API R<Ref<IFileSystem>> new_pak_file_system(IPakStorage* storage,
        Pak::OpenMode mode = Pak::OpenMode::read, const Pak::Options& options = Pak::Options());
    //! @}
}
