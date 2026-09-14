/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Paths.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/Result.hpp>

namespace Luna::VFS
{
    RV validate_path(const Path& path);
    RV validate_relative_path(const Path& path);
    R<Path> absolute_native_path(const c8* path);
}
