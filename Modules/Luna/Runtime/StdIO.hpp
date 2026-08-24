/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.hpp
* @author JXMaster
* @date 2023/2/28
*/
#pragma once
#include "Result.hpp"
#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
    //! @addtogroup Runtime
    //! @{

    //! @name Standard input/output
    //! @{

    //! Reads bytes from the standard input of the process.
    //! @details This function performs one blocking platform read operation on the standard input handle current
    //! at the time of the call. The data is transferred without text encoding, null-terminator, or newline processing.
    //! A successful call may read fewer bytes than requested. Reaching the end of the input succeeds with `0` bytes read.
    //! @param[out] buffer The buffer that receives the bytes. The buffer is not accessed if `size` is `0`.
    //! @param[in] size The maximum number of bytes to read.
    //! @param[out] read_bytes If not `nullptr`, receives the number of bytes read. This is set to `0` if `size` is `0`,
    //! the end of input is reached, or the operation fails before reading any byte.
    //! @return Returns `ok` if the operation succeeds, or an error code if the operation fails.
    //! @remark Concurrent calls are not serialized. The application must synchronize replacement of the process
    //! standard input handle against calls to this function.
    LUNA_RUNTIME_API RV read_standard_input(void* buffer, usize size, usize* read_bytes = nullptr);

    //! Writes bytes to the standard output of the process.
    //! @details This function performs one blocking platform write operation on the standard output handle current
    //! at the time of the call. The data is transferred without text encoding, null-terminator, or newline processing.
    //! A successful call may write fewer bytes than requested; callers that require a complete write must repeat the
    //! operation for the remaining bytes.
    //! @param[in] buffer The buffer that contains the bytes. The buffer is not accessed if `size` is `0`.
    //! @param[in] size The maximum number of bytes to write.
    //! @param[out] write_bytes If not `nullptr`, receives the number of bytes written. This is set to `0` if `size` is
    //! `0` or the operation fails before writing any byte.
    //! @return Returns `ok` if the operation succeeds, or an error code if the operation fails. Writing to a closed
    //! pipe returns @ref E_BAD_PIPE.
    //! @remark Concurrent calls are not serialized and their byte ordering is unspecified. The application must
    //! synchronize replacement of the process standard output handle against calls to this function.
    LUNA_RUNTIME_API RV write_standard_output(const void* buffer, usize size, usize* write_bytes = nullptr);

    //! Writes bytes to the standard error of the process.
    //! @details This function behaves like @ref write_standard_output, but writes to the standard error handle current
    //! at the time of the call. Standard error is independent from standard output so that diagnostics can be separated
    //! from protocol data written to standard output.
    //! @param[in] buffer The buffer that contains the bytes. The buffer is not accessed if `size` is `0`.
    //! @param[in] size The maximum number of bytes to write.
    //! @param[out] write_bytes If not `nullptr`, receives the number of bytes written. This is set to `0` if `size` is
    //! `0` or the operation fails before writing any byte.
    //! @return Returns `ok` if the operation succeeds, or an error code if the operation fails. Writing to a closed
    //! pipe returns @ref E_BAD_PIPE.
    //! @remark Concurrent calls are not serialized and their byte ordering is unspecified. The application must
    //! synchronize replacement of the process standard error handle against calls to this function.
    LUNA_RUNTIME_API RV write_standard_error(const void* buffer, usize size, usize* write_bytes = nullptr);

    //! @}

    //! @}
}
