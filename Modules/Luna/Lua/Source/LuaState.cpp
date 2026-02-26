/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LuaState.cpp
* @author JXMaster
* @date 2026/2/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_LUA_API LUNA_EXPORT
#include "LuaState.hpp"

namespace Luna
{
    namespace Lua
    {
        void* lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
        {
            if(nsize == 0)
            {
                memfree(ptr);
                return nullptr;
            }
            else
            {
                return realloc(ptr, nsize);
            }
        }
        LuaState::LuaState()
        {
            m_state = lua_newstate(lua_alloc, nullptr);
        }
        LuaState::~LuaState()
        {
            lua_close(m_state);
            m_state = nullptr;
        }
        LUNA_LUA_API Ref<ILuaState> new_lua_state()
        {
            return new_object<LuaState>();
        }
    }
}