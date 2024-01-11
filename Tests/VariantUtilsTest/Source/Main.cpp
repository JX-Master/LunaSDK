/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.cpp
* @author JXMaster
* @date 2023/8/27
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>
using namespace Luna;

int main()
{
    Luna::init();
    lupanic_if_failed(add_modules({module_variant_utils()}));
    lupanic_if_failed(init_modules());
    set_log_to_platform_enabled(true);
    set_log_to_platform_verbosity(LogVerbosity::warning);
    json_test();
    diff_test();
    xml_test();
    Luna::close();
    return 0;
}