/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VariantUtils.cpp
* @author JXMaster
* @date 2022/8/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VARIANT_UTILS_API LUNA_EXPORT
#include <Luna/Runtime/Module.hpp>
#include "../VariantUtils.hpp"

namespace Luna
{
    namespace VariantUtils
    {
        void xml_init();
        void xml_close();
        RV json_init();
        void json_close();

        struct ModuleVariantUtils : public Module
        {
            virtual const c8* get_name() override { return "VariantUtils"; }
            virtual RV on_init() override
            {
                RV json_result = json_init();
                if(failed(json_result)) return json_result.errcode();
                xml_init();
                return ok;
            }
            virtual void on_close() override
            {
                xml_close();
                json_close();
            }
        };
    }
    LUNA_VARIANT_UTILS_API Module* module_variant_utils()
    {
        static VariantUtils::ModuleVariantUtils m;
        return &m;
    }
}
