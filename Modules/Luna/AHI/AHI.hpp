/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AHI.hpp
* @author JXMaster
* @date 2024/1/11
*/
#pragma once
#ifndef LUNA_AHI_API
#define LUNA_AHI_API
#endif

namespace Luna
{
    //! @addtogroup AHI AHI
    //! Audio Hardware Interface (AHI) module provides uniform API to use platform's 
    //! audio input / output interface for audio capture and playback.
    
    struct Module;
    LUNA_AHI_API Module* module_ahi();
}