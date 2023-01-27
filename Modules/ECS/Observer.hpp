/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Observer.hpp
* @author JXMaster
* @date 2023/1/4
*/
#pragma once
#include "Cluster.hpp"
#include <Runtime/Ref.hpp>

namespace Luna
{
    namespace ECS
    {
        struct IWorld;
        
        //! Represents one observer that monitors a particlar set of clusters.
        struct Observer;

        //! The function used to check whether one cluster should be observed by the current observer.
        using observer_func_t = bool(ObjRef userdata, IWorld* world, Cluster* cluster);

        
    }
}