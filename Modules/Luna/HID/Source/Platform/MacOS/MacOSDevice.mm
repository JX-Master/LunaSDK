/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MacOSDevice.mm
* @author JXMaster
* @date 2023/8/13
*/
#include "../../HID.hpp"
namespace Luna
{
    namespace HID
    {
		RV platform_init()
		{
			return ok;
		}
		void platform_close()
		{
		}
    }
}