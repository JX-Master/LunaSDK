/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2022/8/2
*/
#include "../RHITestBed/RHITestBed.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>

using namespace Luna;

void run()
{
    lupanic_if_failed(RHITestBed::run());
}

int main()
{
    if (!Luna::init()) return 0;
    lupanic_if_failed(add_modules({module_rhi_test_bed()}));
    auto r = init_modules();
    if (failed(r))
    {
        log_error("RHITest", "%s", explain(r.errcode()));
    }
    else run();
    Luna::close();
    return 0;
}
