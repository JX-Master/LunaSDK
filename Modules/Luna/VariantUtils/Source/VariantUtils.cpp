/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VariantUtils.cpp
* @author JXMaster
* @date 2022/8/27
*/
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    namespace VariantUtils
    {
        void xml_init();
        void xml_close();
    }
    RV init()
    {
        VariantUtils::xml_init();
        return ok;
    }
    void close()
    {
        VariantUtils::xml_close();
    }
    namespace VariantUtils
    {
        LUNA_STATIC_REGISTER_MODULE(VariantUtils, "", init, close);
    }
}