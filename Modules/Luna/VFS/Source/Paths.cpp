/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Paths.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "Paths.hpp"
#include <Luna/Runtime/File.hpp>

namespace Luna::VFS
{
    RV validate_path(const Path& path)
    {
        if((u32)path.flags() & ~(u32)PathFlag::absolute) return E_BAD_ARGUMENTS;
        for(const auto& node : path)
        {
            if(node.empty() || strlen(node.c_str()) != node.size() || !strcmp(node.c_str(), ".") ||
                !strcmp(node.c_str(), "..") || strchr(node.c_str(), '/') || strchr(node.c_str(), '\\')) return E_BAD_ARGUMENTS;
        }
        return ok;
    }

    RV validate_relative_path(const Path& path)
    {
        if(!path.root().empty() || path.flags() != PathFlag::none) return E_BAD_ARGUMENTS;
        return validate_path(path);
    }

    R<Path> absolute_native_path(const c8* native_path)
    {
        if(!native_path || !native_path[0]) return E_BAD_ARGUMENTS;
        Path path(native_path);
        if(!test_flags(path.flags(), PathFlag::absolute))
        {
            if(!path.root().empty()) return E_BAD_ARGUMENTS;
            auto cwd = get_current_dir();
            if(!cwd) return E_BAD_PLATFORM_CALL;
            Path absolute(cwd);
            release_current_dir(cwd);
            absolute.append(path);
            absolute.normalize();
            path = move(absolute);
        }
        return path;
    }
}
