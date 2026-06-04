/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHIUtility.cpp
* @author JXMaster
* @date 2025/9/5
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_UTILITY_API LUNA_EXPORT
#include "../RHIUtility.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include "ResourceReadContextImpl.hpp"
#include "ResourceWriteContextImpl.hpp"
#include "MipmapGenerationContextImpl.hpp"
#include "BlitContextImpl.hpp"
#include "RHIUtility.meta.generated.hpp"

namespace Luna
{
    namespace RHIUtility
    {
        struct RHIUtilityModule : public Module
        {
            virtual const c8* get_name() override { return "RHIUtility"; }
            virtual RV on_register() override
            {
                return add_dependency_module(this, module_rhi());
            }
            virtual RV on_init() override
            {
                Meta::register_RHIUtility_types();
                return ok;
            }
            virtual void on_close() override
            {
                cleanup_mipmap_generation_states();
            }
        };
    }
    LUNA_RHI_UTILITY_API Module* module_rhi_utility()
    {
        static RHIUtility::RHIUtilityModule m;
        return &m;
    }
}
