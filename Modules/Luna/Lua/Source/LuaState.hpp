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
#include "../LuaState.hpp"
#include <Luna/Runtime/TypeInfo.hpp>

#include <lua.hpp>

namespace Luna
{
    namespace Lua
    {
        struct LuaState : ILuaState
        {
            lustruct("Lua::LuaState", "{8d30b385-f07c-4bf6-96de-4e57fb9c7b4c}");
            luiimpl();

            LuaState();
            ~LuaState();

            lua_State* m_state = nullptr;
        };
    }
}