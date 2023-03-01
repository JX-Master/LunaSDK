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
        
        virtual RV read(Span<byte_t> buffer, usize* read_bytes)
        {
            return OS::std_input({(c8*)buffer.data(), buffer.size()}, read_bytes);
        }

        virtual RV write(Span<const byte_t> buffer, usize* write_bytes)
        {
            return OS::std_output({(const c8*)buffer.data(), buffer.size()}, write_bytes);
        }
    };

    void std_io_init();
    void std_io_close();
}