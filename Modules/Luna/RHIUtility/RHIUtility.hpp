/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHIUtility.hpp
* @author JXMaster
* @date 2025/8/21
*/
#pragma once

#ifndef LUNA_RHI_UTILITY_API
#define LUNA_RHI_UTILITY_API
#endif

namespace Luna
{
    namespace RHIUtility
    {
        //! @addtogroup RHIUtility RHIUtility
        //! The of RHI utility module contains high-level functions implemented using RHI API for common tasks.
    }

    struct Module;
    LUNA_RHI_UTILITY_API Module* module_rhi_utility();
}