/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LuaLib.cpp
* @author JXMaster
* @date 2026/2/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_LUA_API LUNA_EXPORT
#include "../LuaLib.hpp"

#include <lua.hpp>

namespace Luna
{
    namespace Lua
    {
        LUNA_LUA_API int open_base(LuaStatePtr L)
        {
            return luaopen_base((lua_State*)L);
        }
        LUNA_LUA_API int open_package(LuaStatePtr L)
        {
            return luaopen_package((lua_State*)L);
        }
        LUNA_LUA_API int open_coroutine(LuaStatePtr L)
        {
            return luaopen_coroutine((lua_State*)L);
        }
        LUNA_LUA_API int open_debug(LuaStatePtr L)
        {
            return luaopen_debug((lua_State*)L);
        }
        LUNA_LUA_API int open_io(LuaStatePtr L)
        {
            return luaopen_io((lua_State*)L);
        }
        LUNA_LUA_API int open_math(LuaStatePtr L)
        {
            return luaopen_math((lua_State*)L);
        }
        LUNA_LUA_API int open_os(LuaStatePtr L)
        {
            return luaopen_os((lua_State*)L);
        }
        LUNA_LUA_API int open_string(LuaStatePtr L)
        {
            return luaopen_string((lua_State*)L);
        }
        LUNA_LUA_API int open_table(LuaStatePtr L)
        {
            return luaopen_table((lua_State*)L);
        }
        LUNA_LUA_API int open_utf8(LuaStatePtr L)
        {
            return luaopen_utf8((lua_State*)L);
        }
        LUNA_LUA_API void openlibs(LuaStatePtr L)
        {
            luaL_openlibs((lua_State*)L);
        }
    }
}