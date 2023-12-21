/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.hpp
* @author JXMaster
* @date 2023/2/28
*/
#pragma once
#include "Stream.hpp"
#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
    //! @addtogroup Runtime
	//! @{

    //! @name Standard input/output
    //! @{

    //! Gets the stream object that is connected to the system standard input/output device.
    //! @details All read operation from the stream reads input from the standard input device; 
    //! all write operations to the stream outputs data to the standard output device.
    //! 
    //! @remark The `read` method of the standard IO stream reads data from the standard input until the 
    //! provided buffer is full, or one new line or EOF is reached. The data being read is one UTF-8
    //! (array of `c8`) string. One null terminator is always added to the read string. The new line character
    //! `\n` will not be read. If `read_bytes` is not `nullptr`, it stores the number of `c8` characters written
    //! to the buffer.
    //! 
    //! The `write` method of the standard IO stream writes data to the standard output until the buffer size
    //! or one null terminator is reached. The data to be written is interpreted as one UTF-8 
    //! (array of `c8`) string. If `write_bytes` is not `nullptr`, it stores the numbder of `c8` characters outputted
    //! to the stream.
    LUNA_RUNTIME_API IStream* get_std_io_stream();

    //! @}

	//! @}
}