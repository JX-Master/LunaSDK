/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LuaState.hpp
* @author JXMaster
* @date 2026/2/22
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_LUA_API
#define LUNA_LUA_API
#endif

namespace Luna
{
    namespace Lua
    {
        using LuaStatePtr = opaque_t; // Maps to lua_State*

        using CFunction = int(LuaStatePtr);

        using number = f64;
        using integer = i64;
        using unsigned = u64;

        LUNA_LUA_API LuaStatePtr new_state();
        LUNA_LUA_API void delete_state(LuaStatePtr state);
        LUNA_LUA_API LuaStatePtr newthread(LuaStatePtr L);
        LUNA_LUA_API int closethread(LuaStatePtr L, LuaStatePtr from);
        LUNA_LUA_API CFunction atpanic(LuaStatePtr L, CFunction panicf);

        // struct ILuaState : virtual Interface
        // {
        //     luiid("{3609ee2c-e288-4488-89f2-e0450e495cb0}");

        //     virtual Version get_version() = 0;
            
        //     virtual i32 absindex(i32 idx) = 0;
        //     virtual i32 gettop() = 0;
        //     virtual void settop(i32 idx) = 0;
        //     virtual void pushvalue(i32 idx) = 0;
        //     virtual void rotate(i32 idx, i32 n) = 0;
        //     virtual void copy(i32 fromidx, i32 toidx) = 0;
        //     virtual i32 checkstack(i32 n) = 0;
        //     virtual void xmove(ILuaState* to, i32 n) = 0;
            
        //     virtual i32 isnumber(i32 idx) = 0;
        //     virtual i32 isstring(i32 idx) = 0;
        //     virtual i32 iscfunction(i32 idx) = 0;
        //     virtual i32 isinteger(i32 idx) = 0;
        //     virtual i32 isuserdata(i32 idx) = 0;
        //     virtual i32 type(i32 idx) = 0;
        //     virtual const c8* typename(i32 tp) = 0;

        //     virtual f64 tonumberx(i32 idx, i32* isnum);
        //     i64 tointegerx(i32 idx, i32* isnum);
        //     i32 toboolean(i32 idx);
        //     const c8* tolstring(i32 idx, usize* len);
        //     u64 rawlen(i32 idx);

        // };

        // LUNA_LUA_API Ref<ILuaState> new_lua_state();
    }
}