/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SDLDevice.cpp
* @author JXMaster
* @date 2022/4/1
*/
#pragma once
#ifdef LUNA_HID_SDL
#include "../HID.hpp"
#include "../../../HID.hpp"
#include "../../../Mouse.hpp"
#include "../../../Keyboard.hpp"
#include "../../../Controller.hpp"

#include <SDL/SDL_mouse.h>
#include <SDL/SDL_keyboard.h>
#include <SDL/SDL_gamecontroller.h>

namespace Luna
{
    namespace HID
    {
        struct SDLDevice : IMouse, IKeyboard, IController
        {
            lustruct("HID::SDLDevice", "{321ab214-a131-4333-8ae0-6a1bb80dcfbb}");
            luiimpl();

            virtual Int2U get_cursor_pos() override;
			virtual bool get_button_state(MouseButton mouse_button) override;
			virtual RV set_cursor_pos(i32 x, i32 y) override;
			virtual bool get_key_state(KeyCode key) override;
			virtual ControllerInputState get_state(u32 index) override;
			virtual RV set_state(u32 index, const ControllerOutputState& state) override;
        };

        Ref<SDLDevice> g_device;

        Int2U SDLDevice::get_cursor_pos()
        {
            int x, y;
            SDL_GetMouseState(&x, &y);
            return Int2U(x, y);
        }

        bool WindowsDevice::get_button_state(MouseButton mouse_button)
        {
            Uint32 buttons = SDL_GetMouseState(nullptr, nullptr);
        }
    
        RV request_sdl_device(void* userdata, const Guid& iid, object_t* out_device_object)
        {
            object_t obj = g_device.object();
            object_retain(obj);
            *out_device_object = obj;
            return ok;
        }
    
        RV register_platform_devices()
        {
            register_boxed_type<SDLDevice>();
            impl_interface_for_type<SDLDevice, IMouse, IKeyboard, IController>();
            g_device = new_object<SDLDevice>();
            DeviceDesc desc;
            desc.userdata = nullptr;
            desc.supported_iids = { IMouse::__guid, IKeyboard::__guid, IController::__guid };
            desc.on_request_device = request_sdl_device;
            desc.on_unregister = nullptr;
            return register_device("SDLDevice", desc);
        }

        void unregister_platform_devices()
        {
            unregister_device("SDLDevice");
            g_device.reset();
        }

    }
}

#endif
