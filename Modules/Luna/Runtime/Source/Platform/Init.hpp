/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Init.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "Result.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Initializaes the platform layer. This is called by the Luna Runtime when Luna::init is called.
        //! All SDK services cannot be used in this call, including memory allocation/deallocation.
        Result init();

        //! Closes the platform layer. This is called by the Luna Runtime when Luna::close is called.
        //! All SDKrvices cannot be used in this call, including memory allocation/deallocation.
        void close();
    }
}