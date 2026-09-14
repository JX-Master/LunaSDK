/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file NativeFileSystem.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "FileSystem.hpp"

namespace Luna::VFS
{
    //! @addtogroup VFS
    //! @{
    //! Creates a filesystem instance rooted at an existing native directory.
    //! @param[in] native_path The native directory path, resolved at creation time.
    //! @return Returns an instance usable directly or through VFS::mount.
    //! @details Later working-directory changes do not change this instance's root.
    //! Files are ordinary Runtime IFile objects; filesystem flush has no deferred
    //! package-level work. Native permissions and symbolic-link behavior apply.
    LUNA_VFS_API R<Ref<IFileSystem>> new_native_file_system(const c8* native_path);
    //! @}
}
