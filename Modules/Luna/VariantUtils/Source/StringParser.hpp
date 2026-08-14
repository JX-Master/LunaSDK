/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StringParser.hpp
* @author JXMaster
* @date 2023/8/28
*/
#pragma once
#include <Luna/Runtime/Base.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Stream.hpp>
#include <Luna/Runtime/RingDeque.hpp>

namespace Luna
{
    namespace VariantUtils
    {
        // The common function for parsing string content (JSON, XML, etc).
        enum class Encoding : u8
        {
            utf_8 = 0,
            utf_16_le = 1,
            utf_16_be = 2,
        };

        inline bool is_whitespace(c32 ch)
        {
            return (ch == 0x20) || (ch == 0xA0) || (ch == 0x0A) || (ch == 0x0D) || (ch == 0x09);
        }

        // A common context for reading strings.
        struct IReadContext
        {
        private:
            ErrCode m_read_error = ErrCode(0);
        public:
            //! Moves the cursor to next character in the string.
            //! @param[in] ch The current character to advance.
            virtual void consume(c32 ch) = 0;
            //! Reads the next character at position @ref index.
            //! @param[in] index The index of the character to read. Specify `0` to read the caracter pointed by the cursor.
            //! @return Returns the character, or `0` if the requested position is past the end of input.
            //! Returns an error if the input encoding is invalid or the underlying stream cannot be read.
            virtual R<c32> next_char(usize index = 0) = 0;
            //! Reads the next character and records any error for deferred propagation by the parser.
            //! @param[in] index The index of the character to read.
            //! @return Returns the character, or `0` if the requested position is past the end of input or an error occurs.
            c32 next_char_or_eof(usize index = 0)
            {
                if(m_read_error.code) return 0;
                R<c32> result = next_char(index);
                if(failed(result))
                {
                    m_read_error = result.errcode();
                    return 0;
                }
                return result.get();
            }
            //! Gets the first error recorded by @ref next_char_or_eof.
            ErrCode get_read_error() const { return m_read_error; }
            //! Gets the line position of the cursor.
            virtual u32 get_line() = 0;
            //! Gets the position of the cursor in the current line.
            virtual u32 get_pos() = 0;
        };

        // A buffer read context
        struct BufferReadContext : public IReadContext
        {
            Encoding encoding = Encoding::utf_8;
            const void* src;
            const void* cur;
            usize src_size;
            u32 line;
            u32 pos;

            virtual void consume(c32 ch) override;
            virtual R<c32> next_char(usize index = 0) override;
            virtual u32 get_line() override { return line; }
            virtual u32 get_pos() override { return pos; }

            void skip_utf16_bom();
        };

        // A stream read context
        struct StreamReadContext : public IReadContext
        {
            Encoding encoding = Encoding::utf_8;
            IStream* stream;
            RingDeque<u8> stream_buffer;
            RingDeque<c32> buffer;
            u32 line;
            u32 pos;
            virtual void consume(c32 ch) override;
        private:
            R<c32> read_one_char_from_stream();
        public:
            virtual R<c32> next_char(usize index = 0) override;
            virtual u32 get_line() override { return line; }
            virtual u32 get_pos() override { return pos; }

            RV stream_read(void* buf, usize read_size, usize* read_bytes);

            RV skip_utf16_bom();
        };
    }
}
