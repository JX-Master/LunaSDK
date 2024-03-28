
/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VG.hpp
* @author JXMaster
* @date 2024/1/11
*/
#pragma once
#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif
namespace Luna
{
    //! @addtogroup VG Vector Graphics
    //! Vector Graphics (VG) module provides functions to render GPU-accelerated vector graphics on 2D or 3D space.
    
    struct Module;
    LUNA_VG_API Module* module_vg();
}