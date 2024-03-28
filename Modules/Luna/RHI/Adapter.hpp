/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Adapter.hpp
* @author JXMaster
* @date 2023/11/1
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif
namespace Luna
{
    namespace RHI
    {
        //! @addtogroup RHI
        //! @{
        
        //! @interface IAdapter
        //! Represents a physical graphics device installed on the platform.
        struct IAdapter : virtual Interface
        {
            luiid("{3be9e0bb-0633-4547-ba1a-c964cf480adc}");

            //! Gets the name of the adapter.
            //! @return Returns the name of the adapter. The returned string is valid until the adapter interface is 
            //! released.
            virtual const c8* get_name() = 0;
        };

        //! Gets a list of adapters installed on the platform.
        //! @return Returns a list of adapters installed on the platform.
        //! @remark The returned list will not change after RHI module is initialized, you should 
        //! restart the module/application if you install/remove adapters at run time.
        LUNA_RHI_API Vector<Ref<IAdapter>> get_adapters();

        //! @}
    }
}