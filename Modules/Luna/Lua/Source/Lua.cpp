/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Lua.hpp
* @author JXMaster
* @date 2026/2/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_LUA_API LUNA_EXPORT
#include "../Lua.hpp"
#include "LuaState.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    namespace Lua
    {
        struct LuaModule : public Module
        {
            virtual const c8* get_name() override { return "Lua"; }
            virtual RV on_init() override
            {
                register_boxed_type<LuaState>();
                impl_interface_for_type<LuaState, ILuaState>();
                return ok;
            }
        };
    }

    LUNA_LUA_API Module* module_lua()
    {
        static Lua::LuaModule m;
        return &m;
    }
}