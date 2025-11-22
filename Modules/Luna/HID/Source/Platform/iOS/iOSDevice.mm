/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file iOSDevice.mm
* @author JXMaster
* @date 2025/11/22
*/
#include "../../HID.hpp"

namespace Luna
{
    namespace HID
    {
        RV platform_init()
        {
            // Currently no global HID devices need explicit initialization on iOS.
            // Window events and IME handle most input on this platform.
            return ok;
        }

        void platform_close()
        {
            // Nothing to clean up for now.
        }
    }
}


