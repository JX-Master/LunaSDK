/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RG.hpp
* @author JXMaster
* @date 2024/1/11
*/
#pragma once
#ifndef LUNA_RG_API
#define LUNA_RG_API
#endif
namespace Luna
{
    //! @addtogroup RG Render Graph
    //! Render Graph (RG) module provides functions to construct render pass dependency graph to automatically reuse in-frame 
    //! transient render resources to reduce memory comsumption. 
    struct Module;
    LUNA_RG_API Module* module_rg();
}