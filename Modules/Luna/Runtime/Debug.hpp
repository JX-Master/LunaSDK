/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.hpp
* @author JXMaster
* @date 2020/4/1
*/
#pragma once
#include "Base.hpp"
#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif
namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeDebugging Debugging
	//! @}

	//! @addtogroup RuntimeDebugging
    //! @{

	//! @brief Prints one debuf string to the host's debug console.
	//! @param[in] fmt The formatting syntax of the output string.
	//! @param[in] args Arguments used to format the output string.
	//! @remarks This function does not necessary output the debug string to stdout or stderr. To output string to stdout, use @ref get_std_io_stream instead.
	LUNA_RUNTIME_API void debug_printf(const c8* fmt, ...);

	//! @brief Prints one debuf string to the host's debug console.
	//! @param[in] fmt The formatting syntax of the output string.
	//! @param[in] args Arguments used to format the output string.
	//! @remarks This function does not necessary output the debug string to stdout or stderr. To output string to stdout, use @ref get_std_io_stream instead.
	LUNA_RUNTIME_API void debug_vprintf(const c8* fmt, VarList args);

	//! @}
}