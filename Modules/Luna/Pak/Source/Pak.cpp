/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Pak.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_PAK_API LUNA_EXPORT
#include "Package.hpp"
#include "Pak.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    struct PakModule : Module
    {
        const c8* get_name() override { return "Pak"; }
        RV on_register() override { return add_dependency_module(this, module_zip()); }
        RV on_init() override
        {
            Meta::register_Pak_types();
            return ok;
        }
    };

    LUNA_PAK_API Module* module_pak()
    {
        static PakModule module;
        return &module;
    }
}
