/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Zip.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ZIP_API LUNA_EXPORT
#include "Archive.hpp"
#include "Zip.meta.generated.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    struct ZipModule : Module
    {
        const c8* get_name() override { return "Zip"; }
        RV on_init() override
        {
            Meta::register_Zip_types();
            return ok;
        }
        void on_close() override {}
    };

    LUNA_ZIP_API Module* module_zip()
    {
        static ZipModule module;
        return &module;
    }
}
