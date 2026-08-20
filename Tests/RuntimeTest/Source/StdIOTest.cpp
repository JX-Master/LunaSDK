/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIOTest.cpp
* @author JXMaster
* @date 2026/8/14
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/MemoryUtils.hpp>
#include <Luna/Runtime/StdIO.hpp>

#ifdef LUNA_PLATFORM_WINDOWS
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#else
#include <unistd.h>
#endif

namespace Luna
{
    static const u8 INPUT_DATA[] = {'A', '\n', 0, 'B', 0xE4, 0xB8, 0xAD};
    static const u8 OUTPUT_DATA[] = {'O', 0, '\n', 0xFF};
    static const u8 ERROR_DATA[] = {'E', 0, '\r', '\n'};

#ifdef LUNA_PLATFORM_WINDOWS
    static void write_pipe(HANDLE pipe, const void* data, usize size)
    {
        DWORD written_bytes = 0;
        lutest(WriteFile(pipe, data, (DWORD)size, &written_bytes, nullptr));
        lutest(written_bytes == size);
    }

    static usize read_pipe(HANDLE pipe, void* data, usize size)
    {
        DWORD read_bytes = 0;
        lutest(ReadFile(pipe, data, (DWORD)size, &read_bytes, nullptr));
        return (usize)read_bytes;
    }

    static void test_standard_input()
    {
        HANDLE read_pipe_handle = nullptr;
        HANDLE write_pipe_handle = nullptr;
        lutest(CreatePipe(&read_pipe_handle, &write_pipe_handle, nullptr, 0));
        write_pipe(write_pipe_handle, INPUT_DATA, sizeof(INPUT_DATA));
        CloseHandle(write_pipe_handle);

        HANDLE old_standard_input = GetStdHandle(STD_INPUT_HANDLE);
        lutest(SetStdHandle(STD_INPUT_HANDLE, read_pipe_handle));
        u8 buffer[sizeof(INPUT_DATA)] = {};
        usize first_read_bytes = 0;
        usize second_read_bytes = 0;
        usize eof_read_bytes = 1;
        RV first_result = read_standard_input(buffer, 3, &first_read_bytes);
        RV second_result = read_standard_input(buffer + first_read_bytes,
            sizeof(buffer) - first_read_bytes, &second_read_bytes);
        u8 eof_buffer = 0;
        RV eof_result = read_standard_input(&eof_buffer, 1, &eof_read_bytes);
        lutest(SetStdHandle(STD_INPUT_HANDLE, old_standard_input));
        CloseHandle(read_pipe_handle);

        lutest(first_result.valid());
        lutest(second_result.valid());
        lutest(eof_result.valid());
        lutest(first_read_bytes == 3);
        lutest(first_read_bytes + second_read_bytes == sizeof(INPUT_DATA));
        lutest(eof_read_bytes == 0);
        lutest(!memcmp(buffer, INPUT_DATA, sizeof(INPUT_DATA)));
    }

    static void test_standard_output_and_error()
    {
        HANDLE output_read_pipe = nullptr;
        HANDLE output_write_pipe = nullptr;
        HANDLE error_read_pipe = nullptr;
        HANDLE error_write_pipe = nullptr;
        lutest(CreatePipe(&output_read_pipe, &output_write_pipe, nullptr, 0));
        lutest(CreatePipe(&error_read_pipe, &error_write_pipe, nullptr, 0));

        HANDLE old_standard_output = GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE old_standard_error = GetStdHandle(STD_ERROR_HANDLE);
        lutest(SetStdHandle(STD_OUTPUT_HANDLE, output_write_pipe));
        lutest(SetStdHandle(STD_ERROR_HANDLE, error_write_pipe));
        usize output_write_bytes = 0;
        usize error_write_bytes = 0;
        RV output_result = write_standard_output(OUTPUT_DATA, sizeof(OUTPUT_DATA), &output_write_bytes);
        RV error_result = write_standard_error(ERROR_DATA, sizeof(ERROR_DATA), &error_write_bytes);
        lutest(SetStdHandle(STD_OUTPUT_HANDLE, old_standard_output));
        lutest(SetStdHandle(STD_ERROR_HANDLE, old_standard_error));
        CloseHandle(output_write_pipe);
        CloseHandle(error_write_pipe);

        u8 output_buffer[sizeof(OUTPUT_DATA)] = {};
        u8 error_buffer[sizeof(ERROR_DATA)] = {};
        usize output_read_bytes = read_pipe(output_read_pipe, output_buffer, sizeof(output_buffer));
        usize error_read_bytes = read_pipe(error_read_pipe, error_buffer, sizeof(error_buffer));
        CloseHandle(output_read_pipe);
        CloseHandle(error_read_pipe);

        lutest(output_result.valid());
        lutest(error_result.valid());
        lutest(output_write_bytes == sizeof(OUTPUT_DATA));
        lutest(error_write_bytes == sizeof(ERROR_DATA));
        lutest(output_read_bytes == sizeof(OUTPUT_DATA));
        lutest(error_read_bytes == sizeof(ERROR_DATA));
        lutest(!memcmp(output_buffer, OUTPUT_DATA, sizeof(OUTPUT_DATA)));
        lutest(!memcmp(error_buffer, ERROR_DATA, sizeof(ERROR_DATA)));
    }

    static void test_broken_output_pipe()
    {
        HANDLE read_pipe_handle = nullptr;
        HANDLE write_pipe_handle = nullptr;
        lutest(CreatePipe(&read_pipe_handle, &write_pipe_handle, nullptr, 0));
        CloseHandle(read_pipe_handle);

        HANDLE old_standard_output = GetStdHandle(STD_OUTPUT_HANDLE);
        lutest(SetStdHandle(STD_OUTPUT_HANDLE, write_pipe_handle));
        usize write_bytes = 1;
        RV result = write_standard_output(OUTPUT_DATA, sizeof(OUTPUT_DATA), &write_bytes);
        lutest(SetStdHandle(STD_OUTPUT_HANDLE, old_standard_output));
        CloseHandle(write_pipe_handle);

        lutest(!result.valid());
        lutest(result.errcode() == BasicError::bad_pipe());
        lutest(write_bytes == 0);
    }
#else
    static void write_pipe(int pipe, const void* data, usize size)
    {
        ssize_t written_bytes = ::write(pipe, data, size);
        lutest(written_bytes == (ssize_t)size);
    }

    static usize read_pipe(int pipe, void* data, usize size)
    {
        ssize_t read_bytes = ::read(pipe, data, size);
        lutest(read_bytes >= 0);
        return (usize)read_bytes;
    }

    static void test_standard_input()
    {
        int pipe_handles[2];
        lutest(::pipe(pipe_handles) == 0);
        write_pipe(pipe_handles[1], INPUT_DATA, sizeof(INPUT_DATA));
        ::close(pipe_handles[1]);

        int old_standard_input = ::dup(STDIN_FILENO);
        lutest(old_standard_input >= 0);
        lutest(::dup2(pipe_handles[0], STDIN_FILENO) == STDIN_FILENO);
        ::close(pipe_handles[0]);
        u8 buffer[sizeof(INPUT_DATA)] = {};
        usize first_read_bytes = 0;
        usize second_read_bytes = 0;
        usize eof_read_bytes = 1;
        RV first_result = read_standard_input(buffer, 3, &first_read_bytes);
        RV second_result = read_standard_input(buffer + first_read_bytes,
            sizeof(buffer) - first_read_bytes, &second_read_bytes);
        u8 eof_buffer = 0;
        RV eof_result = read_standard_input(&eof_buffer, 1, &eof_read_bytes);
        lutest(::dup2(old_standard_input, STDIN_FILENO) == STDIN_FILENO);
        ::close(old_standard_input);

        lutest(first_result.valid());
        lutest(second_result.valid());
        lutest(eof_result.valid());
        lutest(first_read_bytes == 3);
        lutest(first_read_bytes + second_read_bytes == sizeof(INPUT_DATA));
        lutest(eof_read_bytes == 0);
        lutest(!memcmp(buffer, INPUT_DATA, sizeof(INPUT_DATA)));
    }

    static void test_standard_output_and_error()
    {
        int output_pipe[2];
        int error_pipe[2];
        lutest(::pipe(output_pipe) == 0);
        lutest(::pipe(error_pipe) == 0);

        int old_standard_output = ::dup(STDOUT_FILENO);
        int old_standard_error = ::dup(STDERR_FILENO);
        lutest(old_standard_output >= 0);
        lutest(old_standard_error >= 0);
        lutest(::dup2(output_pipe[1], STDOUT_FILENO) == STDOUT_FILENO);
        lutest(::dup2(error_pipe[1], STDERR_FILENO) == STDERR_FILENO);
        ::close(output_pipe[1]);
        ::close(error_pipe[1]);
        usize output_write_bytes = 0;
        usize error_write_bytes = 0;
        RV output_result = write_standard_output(OUTPUT_DATA, sizeof(OUTPUT_DATA), &output_write_bytes);
        RV error_result = write_standard_error(ERROR_DATA, sizeof(ERROR_DATA), &error_write_bytes);
        lutest(::dup2(old_standard_output, STDOUT_FILENO) == STDOUT_FILENO);
        lutest(::dup2(old_standard_error, STDERR_FILENO) == STDERR_FILENO);
        ::close(old_standard_output);
        ::close(old_standard_error);

        u8 output_buffer[sizeof(OUTPUT_DATA)] = {};
        u8 error_buffer[sizeof(ERROR_DATA)] = {};
        usize output_read_bytes = read_pipe(output_pipe[0], output_buffer, sizeof(output_buffer));
        usize error_read_bytes = read_pipe(error_pipe[0], error_buffer, sizeof(error_buffer));
        ::close(output_pipe[0]);
        ::close(error_pipe[0]);

        lutest(output_result.valid());
        lutest(error_result.valid());
        lutest(output_write_bytes == sizeof(OUTPUT_DATA));
        lutest(error_write_bytes == sizeof(ERROR_DATA));
        lutest(output_read_bytes == sizeof(OUTPUT_DATA));
        lutest(error_read_bytes == sizeof(ERROR_DATA));
        lutest(!memcmp(output_buffer, OUTPUT_DATA, sizeof(OUTPUT_DATA)));
        lutest(!memcmp(error_buffer, ERROR_DATA, sizeof(ERROR_DATA)));
    }

    static void test_broken_output_pipe()
    {
        int pipe_handles[2];
        lutest(::pipe(pipe_handles) == 0);
        ::close(pipe_handles[0]);

        int old_standard_output = ::dup(STDOUT_FILENO);
        lutest(old_standard_output >= 0);
        lutest(::dup2(pipe_handles[1], STDOUT_FILENO) == STDOUT_FILENO);
        ::close(pipe_handles[1]);
        usize write_bytes = 1;
        RV result = write_standard_output(OUTPUT_DATA, sizeof(OUTPUT_DATA), &write_bytes);
        lutest(::dup2(old_standard_output, STDOUT_FILENO) == STDOUT_FILENO);
        ::close(old_standard_output);

        lutest(!result.valid());
        lutest(result.errcode() == BasicError::bad_pipe());
        lutest(write_bytes == 0);
    }
#endif

    static void test_zero_size_standard_io()
    {
        usize transferred_bytes = 1;
        lutest(read_standard_input(nullptr, 0, &transferred_bytes).valid());
        lutest(transferred_bytes == 0);
        transferred_bytes = 1;
        lutest(write_standard_output(nullptr, 0, &transferred_bytes).valid());
        lutest(transferred_bytes == 0);
        transferred_bytes = 1;
        lutest(write_standard_error(nullptr, 0, &transferred_bytes).valid());
        lutest(transferred_bytes == 0);
    }

    void std_io_test()
    {
        test_zero_size_standard_io();
        test_standard_input();
        test_standard_output_and_error();
        test_broken_output_pipe();
    }
}
