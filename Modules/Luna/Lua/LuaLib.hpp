/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LuaLib.hpp
* @author JXMaster
* @date 2026/2/27
*/
#pragma once
#include "Lua.hpp"

namespace Luna
{
    namespace Lua
    {
        LUNA_LUA_API int open_base(LuaStatePtr L);
        LUNA_LUA_API int open_package(LuaStatePtr L);
        LUNA_LUA_API int open_coroutine(LuaStatePtr L);
        LUNA_LUA_API int open_debug(LuaStatePtr L);
        LUNA_LUA_API int open_io(LuaStatePtr L);
        LUNA_LUA_API int open_math(LuaStatePtr L);
        LUNA_LUA_API int open_os(LuaStatePtr L);
        LUNA_LUA_API int open_string(LuaStatePtr L);
        LUNA_LUA_API int open_table(LuaStatePtr L);
        LUNA_LUA_API int open_utf8(LuaStatePtr L);

        /* open all libraries */
        LUNA_LUA_API void openlibs(LuaStatePtr L);
    }
}