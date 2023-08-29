/*!
* This file is a portion of Luna SDK.
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

        inline bool is_whitespace(c32 ch)
		{
			return (ch == 0x20) || (ch == 0xA0) || (ch == 0x0A) || (ch == 0x0D) || (ch == 0x09);
		}

        // A common context for reading strings.
        struct IReadContext
		{
            //! Moves the cursor to next character in the string.
            //! @param[in] ch The current character to advance.
			virtual void consume(c32 ch) = 0;
            //! Reads the next character at position @ref index.
            //! @param[in] index The index of the character to read. Specify `0` to read the caracter pointed by the cursor.
			virtual c32 next_char(usize index = 0) = 0;
            //! Gets the line position of the cursor.
			virtual u32 get_line() = 0;
            //! Gets the position of the cursor in the current line.
			virtual u32 get_pos() = 0;
		};

        // A buffer read context
        struct BufferReadContext : public IReadContext
		{
			const c8* src;
			const c8* cur;
			usize src_length;
			u32 line;
			u32 pos;

			virtual void consume(c32 ch) override;
			virtual c32 next_char(usize index = 0) override;
			virtual u32 get_line() override { return line; }
			virtual u32 get_pos() override { return pos; }
		};

        // A stream read context
        struct StreamReadContext : public IReadContext
		{
			IStream* stream;
			RingDeque<c32> buffer;
			u32 line;
			u32 pos;
			virtual void consume(c32 ch) override;
		private:
			R<c32> read_one_char_from_stream();
		public:
			virtual c32 next_char(usize index = 0) override;
			virtual u32 get_line() override { return line; }
			virtual u32 get_pos() override { return pos; }
		};
    }
}