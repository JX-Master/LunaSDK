/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainHeader.hpp
* @author JXMaster
* @date 2025/3/21
*/
#pragma once

//! @addtogroup Window
//! @{

//! The unified entry point of LunaSDK programs.
//! @param[in] argc The number of console arguments.
//! @param[in] argv An array of console arguments.
//! @return Returns 0 if the program exits normally, or platform-defined non-zero error code if not.
int luna_main(int argc, const char* argv[]);

//! @}