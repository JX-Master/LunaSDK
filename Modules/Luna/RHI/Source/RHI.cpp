/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.cpp
* @author JXMaster
* @date 2022/4/14
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "RHI.hpp"
#include <Luna/Runtime/Module.hpp>
#include "../DescriptorSet.hpp"
namespace Luna
{
    static RV register_rhi_error_codes()
    {
        if (!register_error_category(RHI::ERROR_CATEGORY, "RHI") ||
            !register_error_code(RHI::E_DEVICE_HUNG, "device_hung", "The device failed because the application submitted invalid commands.") ||
            !register_error_code(RHI::E_DEVICE_RESET, "device_reset", "The device was reset and must be recreated.") ||
            !register_error_code(RHI::E_DEVICE_REMOVED, "device_removed", "The graphics device was removed or became unavailable.") ||
            !register_error_code(RHI::E_DRIVER_INTERNAL_ERROR, "driver_internal_error", "The graphics driver reported an internal error.") ||
            !register_error_code(RHI::E_FRAME_STATISTICS_DISJOINT, "frame_statistics_disjoint", "Presentation statistics were interrupted and are disjoint.") ||
            !register_error_code(RHI::E_SWAP_CHAIN_OUT_OF_DATE, "swap_chain_out_of_date", "The swap chain is no longer compatible with its surface.") ||
            !register_error_code(RHI::E_COLOR_SPACE_NOT_SUPPORTED, "color_space_not_supported", "The requested color space is not supported."))
        {
            return set_error(E_ALREADY_EXISTS, "RHI error metadata conflicts with an existing registration.");
        }
        return ok;
    }

    namespace RHI
    {
        struct RHIModule : public Module
        {
            virtual const c8* get_name() override { return "RHI"; }
            virtual RV on_register() override
            {
                RV result = register_rhi_error_codes();
                if (failed(result.errcode())) return result;
                return add_dependency_module(this, module_window());
            }
            virtual RV on_init() override
            {
                return render_api_init();
            }
            virtual void on_close() override
            {
                render_api_close();
            }
        };
    }
    LUNA_RHI_API Module* module_rhi()
    {
        static RHI::RHIModule m;
        return &m;
    }
}
