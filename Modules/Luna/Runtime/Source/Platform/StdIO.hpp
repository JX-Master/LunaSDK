/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "Result.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Reads one string from the standard input.
        //! @param[in] buffer The buffer used to accept the input stream.
        //! @param[in] size The number of `c8` characters to read from the input stream.
        //! @remark The stream shall be encoded in UTF-8 format. The input process shall be terminated when
        //! the buffer is full, or one new line (`\n`) or EOF is reached. The string stored in `buffer` shall
        //! be null-terminated, so that at most `buffer.size() - 1` characters can be accepted. If the last 
        //! input character is a multi-bytes character in UTF-8, it must either be fully written or not be written.
        //! 
        //! If the input process is terminated by one new line character, the new line character shall not be discarded
        //! and not written to the buffer.
        //! 
        //! This function must be thread-safe.
        Result std_input(c8* buffer, usize size, usize* read_bytes);

        //! Writes one string to the standard output.
        //! @param[in] buffer The buffer that contains the string to be written.
        //! @param[in] size The size of `c8` characters to write.
        //! @remark The stream is assumed to be encoded in UTF-8 format. The output process shall be terminated when
        //! one null terminator is reached, or the buffer size limit is reached. The input string is not guaranteed to
        //! be null-terminated. If one character in the buffer is a multi-bytes character in UTF-8, it should either be fully
        //! outputted or not outputted.
        //! The the output process is terminated by one null terminator, the null terminator should not be outputted.
        //! 
        //! This function must be thread-safe.
        Result std_output(const c8* buffer, usize size, usize* write_bytes);
    }
}