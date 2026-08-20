/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.cpp
* @author JXMaster
* @date 2023/2/28
*/
#include "../StdIO.hpp"
#include "Errno.hpp"

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

namespace Luna
{
    namespace Platform
    {
        static usize clamp_io_size(usize size)
        {
            return size > (usize)SSIZE_MAX ? (usize)SSIZE_MAX : size;
        }

        Result read_standard_input(void* buffer, usize size, usize* read_bytes)
        {
            if (read_bytes) *read_bytes = 0;
            if (!size) return Result::success;
            ssize_t result = ::read(STDIN_FILENO, buffer, clamp_io_size(size));
            if (result < 0) return encode_errno(errno);
            if (read_bytes) *read_bytes = (usize)result;
            return Result::success;
        }

        static Result write_standard_stream(int file_descriptor, const void* buffer, usize size, usize* write_bytes)
        {
            if (write_bytes) *write_bytes = 0;
            if (!size) return Result::success;

            sigset_t signal_set;
            sigemptyset(&signal_set);
            sigaddset(&signal_set, SIGPIPE);

            sigset_t old_signal_set;
            int mask_result = pthread_sigmask(SIG_BLOCK, &signal_set, &old_signal_set);
            if (mask_result) return encode_errno(mask_result);

            sigset_t pending_signals;
            if (sigpending(&pending_signals) != 0)
            {
                int pending_error = errno;
                pthread_sigmask(SIG_SETMASK, &old_signal_set, nullptr);
                return encode_errno(pending_error);
            }
            bool sigpipe_was_pending = sigismember(&pending_signals, SIGPIPE) == 1;

            ssize_t result = ::write(file_descriptor, buffer, clamp_io_size(size));
            int write_error = result < 0 ? errno : 0;
            int signal_error = 0;
            if (write_error == EPIPE && !sigpipe_was_pending)
            {
                if (sigpending(&pending_signals) != 0)
                {
                    signal_error = errno;
                }
                else if (sigismember(&pending_signals, SIGPIPE) == 1)
                {
                    int received_signal = 0;
                    signal_error = sigwait(&signal_set, &received_signal);
                }
            }

            int restore_result = pthread_sigmask(SIG_SETMASK, &old_signal_set, nullptr);
            if (result >= 0 && write_bytes) *write_bytes = (usize)result;
            if (signal_error) return encode_errno(signal_error);
            if (restore_result) return encode_errno(restore_result);
            if (result < 0) return encode_errno(write_error);
            return Result::success;
        }

        Result write_standard_output(const void* buffer, usize size, usize* write_bytes)
        {
            return write_standard_stream(STDOUT_FILENO, buffer, size, write_bytes);
        }

        Result write_standard_error(const void* buffer, usize size, usize* write_bytes)
        {
            return write_standard_stream(STDERR_FILENO, buffer, size, write_bytes);
        }
    }
}
