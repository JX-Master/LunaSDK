/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.hpp
* @author JXMaster
* @date 2023/2/28
*/
#include "../StdIO.hpp"
#include "OS.hpp"
namespace Luna
{
    struct StdIOStream : IStream
    {
        lustruct("StdIOStream", "4cbc48b8-b15b-423f-9535-d3435bef3055");
        luiimpl();
        
        virtual RV read(void* buffer, usize size, usize* read_bytes) override
        {
            return OS::std_input((c8*)buffer, size / sizeof(c8), read_bytes);
        }

        virtual RV write(const void* buffer, usize size, usize* write_bytes) override
        {
            return OS::std_output((const c8*)buffer, size / sizeof(c8), write_bytes);
        }
    };

    void std_io_init();
    void std_io_close();
}