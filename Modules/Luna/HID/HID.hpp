/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HID.hpp
* @author JXMaster
* @date 2022/4/1
* @brief
* The Human Interface Devices (HID) module provides an abstraction layer on the platfrom's native Human Interface Devices, including mouses, keyboards,
* controllers, joysticks, sensors, cameras and so on. Most HIDs are used to provide inputs for the application, while some of them may also receive 
* feedbacks from the application like taptics, flashlights, etc. The HID module is designed to work closely to the OS layer and does not provide 
* high-level abstractions like custom actions, key-mappings and so on.
*/
#pragma once
#include <Luna/Runtime/Base.hpp>

#ifndef LUNA_HID_API
#define LUNA_HID_API
#endif
namespace Luna
{
    //! @addtogroup HID HID
    //! Human Interface Device (HID) module provides APIs to access platform's input and output devices, like mouse, 
    //! keyboard, controller, etc.
    
    struct Module;
    LUNA_HID_API Module* module_hid();
}