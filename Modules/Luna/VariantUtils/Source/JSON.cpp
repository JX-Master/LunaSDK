/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JSON.cpp
* @author JXMaster
* @date 2021/8/3
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VARIANT_UTILS_API LUNA_EXPORT
#include "../JSON.hpp"
#include <Luna/Runtime/Base64.hpp>
#include <Luna/Runtime/Base85.hpp>
#include "StringParser.hpp"
#include <charconv>
#include <cmath>
#include <limits>
#include <locale.h>
#if defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
#include <xlocale.h>
#endif

namespace Luna
{
    namespace VariantUtils
    {
#ifdef LUNA_PLATFORM_WINDOWS
        using json_locale_t = _locale_t;
#else
        using json_locale_t = locale_t;
#endif
        json_locale_t g_json_locale = nullptr;

        RV json_init()
        {
#ifdef LUNA_PLATFORM_WINDOWS
            g_json_locale = _create_locale(LC_NUMERIC, "C");
#else
            g_json_locale = newlocale(LC_NUMERIC_MASK, "C", nullptr);
#endif
            if(!g_json_locale) return E_FAILURE;
            return ok;
        }

        void json_close()
        {
            if(!g_json_locale) return;
#ifdef LUNA_PLATFORM_WINDOWS
            _free_locale(g_json_locale);
#else
            freelocale(g_json_locale);
#endif
            g_json_locale = nullptr;
        }

        static f64 parse_f64(const c8* str, c8** str_end)
        {
#ifdef LUNA_PLATFORM_WINDOWS
            return _strtod_l(str, str_end, g_json_locale);
#else
            return strtod_l(str, str_end, g_json_locale);
#endif
        }

        static i32 format_f64(c8* buffer, usize buffer_size, f64 value)
        {
#ifdef LUNA_PLATFORM_WINDOWS
            return _snprintf_l(buffer, buffer_size, "%.17g", g_json_locale, value);
#elif defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
            return snprintf_l(buffer, buffer_size, g_json_locale, "%.17g", value);
#else
            locale_t previous_locale = uselocale(g_json_locale);
            i32 result = snprintf(buffer, buffer_size, "%.17g", value);
            uselocale(previous_locale);
            return result;
#endif
        }

        static RV skip_single_line_comment(IReadContext& ctx)
        {
            lucheck(ctx.next_char_or_eof() == '/' && ctx.next_char_or_eof(1) == '/');
            ctx.consume('/');
            ctx.consume('/');
            c32 ch = ctx.next_char_or_eof();
            if (!ch) return ok;
            while (ch != '\n')
            {
                ctx.consume(ch);
                ch = ctx.next_char_or_eof();
                if (!ch) return ok;
            }
            ctx.consume(ch);// for \n.
            return ok;
        }
        static RV skip_multi_line_comment(IReadContext& ctx)
        {
            lucheck(ctx.next_char_or_eof() == '/' && ctx.next_char_or_eof(1) == '*');
            ctx.consume('/');
            ctx.consume('*');
            c32 ch = ctx.next_char_or_eof();
            if (!ch) return set_error(E_FORMAT_ERROR,
                "Unterminated comment at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
        entry:
            while (ch != '*')
            {
                ctx.consume(ch);
                ch = ctx.next_char_or_eof();
                if (!ch) return set_error(E_FORMAT_ERROR,
                    "Unterminated comment at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
            }
            ctx.consume(ch); // for *.
            ch = ctx.next_char_or_eof();
            if (!ch) return set_error(E_FORMAT_ERROR,
                "Unterminated comment at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
            if (ch == '/')
            {
                ctx.consume(ch); // for /.
                return ok;
            }
            else
            {
                goto entry;
            }
        }
        static bool is_json_whitespace(c32 ch)
        {
            return ch == 0x20 || ch == 0x09 || ch == 0x0A || ch == 0x0D;
        }

        static RV skip_whitespaces_and_comments(IReadContext& ctx, const JSONReadOptions& options)
        {
            c32 ch = ctx.next_char_or_eof();
            while (ch)
            {
                if (is_json_whitespace(ch) ||
                    (options.allow_nonstandard_whitespace && is_whitespace(ch)))
                {
                    ctx.consume(ch);
                }
                else if (options.allow_comments && ch == '/')
                {
                    c32 ch2 = ctx.next_char_or_eof(1);
                    if (ch2 == '/')
                    {
                        RV result = skip_single_line_comment(ctx);
                        if(failed(result)) return result.errcode();
                    }
                    else if (ch2 == '*')
                    {
                        RV result = skip_multi_line_comment(ctx);
                        if(failed(result)) return result.errcode();
                    }
                    else
                    {
                        break;
                    }
                }
                else break;
                ch = ctx.next_char_or_eof();
            }
            return ok;
        }

        static R<u32> read_unicode_escape(IReadContext& ctx)
        {
            u32 unicode = 0;
            for(u32 i = 0; i < 4; ++i)
            {
                c32 ch = ctx.next_char_or_eof();
                if(!(ch >= '0' && ch <= '9') && !(ch >= 'a' && ch <= 'f') &&
                    !(ch >= 'A' && ch <= 'F'))
                {
                    return set_error(E_FORMAT_ERROR,
                        "Invalid Unicode escape at line %u, pos %u.",
                        ctx.get_line(), ctx.get_pos());
                }
                unicode <<= 4;
                if(ch >= '0' && ch <= '9') unicode += ch - '0';
                else if(ch >= 'a' && ch <= 'f') unicode += ch - 'a' + 10;
                else unicode += ch - 'A' + 10;
                ctx.consume(ch);
            }
            return unicode;
        }

        static R<String> read_string_literal(IReadContext& ctx, const JSONReadOptions& options)
        {
            lucheck(ctx.next_char_or_eof() == '"');
            ctx.consume('"');
            String s;
            c32 ch = ctx.next_char_or_eof();
            while (ch)
            {
                if (ch == '\\')
                {
                    ctx.consume(ch); // for '\\'
                    ch = ctx.next_char_or_eof();
                    c32 ch2;
                    switch (ch)
                    {
                    case '"':
                        ch2 = '\"'; ctx.consume(ch); break;
                    case '\\':
                        ch2 = '\\'; ctx.consume(ch); break;
                    case '/':
                        ch2 = '/'; ctx.consume(ch); break;
                    case 'b':
                        ch2 = '\b'; ctx.consume(ch); break;
                    case 'f':
                        ch2 = '\f'; ctx.consume(ch); break;
                    case 'n':
                        ch2 = '\n'; ctx.consume(ch); break;
                    case 'r':
                        ch2 = '\r'; ctx.consume(ch); break;
                    case 't':
                        ch2 = '\t'; ctx.consume(ch); break;
                    case '0':
                        if(!options.allow_nonstandard_escapes)
                        {
                            return set_error(E_FORMAT_ERROR,
                                "Non-standard string escape at line %u, pos %u.",
                                ctx.get_line(), ctx.get_pos());
                        }
                        ch2 = '\0'; ctx.consume(ch); break;
                    case '\'':
                        if(!options.allow_nonstandard_escapes)
                        {
                            return set_error(E_FORMAT_ERROR,
                                "Non-standard string escape at line %u, pos %u.",
                                ctx.get_line(), ctx.get_pos());
                        }
                        ch2 = '\''; ctx.consume(ch); break;
                    case 'u':
                    {
                        ctx.consume(ch); // for u
                        R<u32> unicode_result = read_unicode_escape(ctx);
                        if(failed(unicode_result)) return unicode_result.errcode();
                        u32 unicode = unicode_result.get();
                        if(unicode >= 0xD800 && unicode <= 0xDBFF)
                        {
                            if(ctx.next_char_or_eof() != '\\' || ctx.next_char_or_eof(1) != 'u')
                            {
                                return set_error(E_FORMAT_ERROR,
                                    "A high surrogate must be followed by a low surrogate at line %u, pos %u.",
                                    ctx.get_line(), ctx.get_pos());
                            }
                            ctx.consume('\\');
                            ctx.consume('u');
                            R<u32> low_result = read_unicode_escape(ctx);
                            if(failed(low_result)) return low_result.errcode();
                            u32 low = low_result.get();
                            if(low < 0xDC00 || low > 0xDFFF)
                            {
                                return set_error(E_FORMAT_ERROR,
                                    "Invalid low surrogate at line %u, pos %u.",
                                    ctx.get_line(), ctx.get_pos());
                            }
                            unicode = 0x10000 + ((unicode - 0xD800) << 10) + (low - 0xDC00);
                        }
                        else if(unicode >= 0xDC00 && unicode <= 0xDFFF)
                        {
                            return set_error(E_FORMAT_ERROR,
                                "Unexpected low surrogate at line %u, pos %u.",
                                ctx.get_line(), ctx.get_pos());
                        }
                        ch2 = (c32)unicode;
                    }
                    break;
                    default:
                        return set_error(E_FORMAT_ERROR,
                            "Invalid character appeared after \"\\\" at line %d, pos %d.",
                            ctx.get_line(), ctx.get_pos());
                    }
                    c8 buf[4];
                    R<usize> buf_count = utf8_encode_char(buf, sizeof(buf), ch2);
                    if(failed(buf_count)) return buf_count.errcode();
                    s.append(buf, buf_count.get());
                    ch = ctx.next_char_or_eof();
                    continue;
                }
                if (ch == '"')
                {
                    ctx.consume(ch); // for ".
                    break;
                }
                if(ch < 0x20)
                {
                    return set_error(E_FORMAT_ERROR,
                        "Unescaped control character at line %u, pos %u.",
                        ctx.get_line(), ctx.get_pos());
                }
                c8 buf[4];
                R<usize> buf_count = utf8_encode_char(buf, sizeof(buf), ch);
                if(failed(buf_count)) return buf_count.errcode();
                s.append(buf, buf_count.get());
                ctx.consume(ch);
                ch = ctx.next_char_or_eof();
            }
            if(!ch)
            {
                return set_error(E_FORMAT_ERROR,
                    "Unterminated string at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
            }
            return s;
        }
        static R<Variant> read_value(IReadContext& ctx, const JSONReadOptions& options);
        static R<Variant> read_object(IReadContext& ctx, const JSONReadOptions& options)
        {
            lucheck(ctx.next_char_or_eof() == '{');
            ctx.consume('{');
            RV skip_result = skip_whitespaces_and_comments(ctx, options);
            if(failed(skip_result)) return skip_result.errcode();
            Variant v(VariantType::object);
            c32 ch = ctx.next_char_or_eof();
            if(ch == '}')
            {
                ctx.consume('}');
                return v;
            }
            while(ch)
            {
                if (ch != '"') return set_error(E_FORMAT_ERROR,
                    "The object field must start with a string name (line %d pos %d).",
                    ctx.get_line(), ctx.get_pos());
                R<String> name_str = read_string_literal(ctx, options);
                if (failed(name_str)) return name_str.errcode();
                skip_result = skip_whitespaces_and_comments(ctx, options);
                if(failed(skip_result)) return skip_result.errcode();
                ch = ctx.next_char_or_eof();
                if (ch != ':') return set_error(E_FORMAT_ERROR,
                    "':' expected at the end of the field name (line %d pos %d).",
                    ctx.get_line(), ctx.get_pos());
                ctx.consume(ch);
                R<Variant> val = read_value(ctx, options);
                if (failed(val)) return val.errcode();
                if(!v.insert(Name(name_str.get()), move(val.get())))
                {
                    return set_error(E_FORMAT_ERROR,
                        "Duplicate object name at line %u, pos %u.",
                        ctx.get_line(), ctx.get_pos());
                }
                skip_result = skip_whitespaces_and_comments(ctx, options);
                if(failed(skip_result)) return skip_result.errcode();
                ch = ctx.next_char_or_eof();
                if (ch == '}')
                {
                    ctx.consume('}');
                    return v;
                }
                if (ch != ',') return set_error(E_FORMAT_ERROR,
                    "',' expected at the end of the field (line %d pos %d).",
                    ctx.get_line(), ctx.get_pos());
                ctx.consume(ch);
                skip_result = skip_whitespaces_and_comments(ctx, options);
                if(failed(skip_result)) return skip_result.errcode();
                ch = ctx.next_char_or_eof();
                if(ch == '}')
                {
                    if(!options.allow_trailing_commas)
                    {
                        return set_error(E_FORMAT_ERROR,
                            "Trailing comma in object at line %u, pos %u.",
                            ctx.get_line(), ctx.get_pos());
                    }
                    ctx.consume('}');
                    return v;
                }
            }
            return set_error(E_FORMAT_ERROR, "Unexpected EOF occurred at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
        }

        static R<Variant> read_array(IReadContext& ctx, const JSONReadOptions& options)
        {
            lucheck(ctx.next_char_or_eof() == '[');
            ctx.consume('[');
            RV skip_result = skip_whitespaces_and_comments(ctx, options);
            if(failed(skip_result)) return skip_result.errcode();
            Variant v(VariantType::array);
            c32 ch = ctx.next_char_or_eof();
            if(ch == ']')
            {
                ctx.consume(']');
                return v;
            }
            while(ch)
            {
                R<Variant> val = read_value(ctx, options);
                if (failed(val)) return val.errcode();
                v.push_back(move(val.get()));
                skip_result = skip_whitespaces_and_comments(ctx, options);
                if(failed(skip_result)) return skip_result.errcode();
                ch = ctx.next_char_or_eof();
                if (ch == ']')
                {
                    ctx.consume(']');
                    return v;
                }
                if (ch != ',') return set_error(E_FORMAT_ERROR,
                    "',' expected at the end of every array item (line %d pos %d).",
                    ctx.get_line(), ctx.get_pos());
                ctx.consume(ch);
                skip_result = skip_whitespaces_and_comments(ctx, options);
                if(failed(skip_result)) return skip_result.errcode();
                ch = ctx.next_char_or_eof();
                if(ch == ']')
                {
                    if(!options.allow_trailing_commas)
                    {
                        return set_error(E_FORMAT_ERROR,
                            "Trailing comma in array at line %u, pos %u.",
                            ctx.get_line(), ctx.get_pos());
                    }
                    ctx.consume(']');
                    return v;
                }
            }
            return set_error(E_FORMAT_ERROR, "Unexpected EOF occurred at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
        }

        static bool read_blob_integer(const c8*& cursor, const c8* end, u64& value)
        {
            if(cursor == end || *cursor < '0' || *cursor > '9') return false;
            value = 0;
            while(cursor != end && *cursor >= '0' && *cursor <= '9')
            {
                u64 digit = (u64)(*cursor - '0');
                if(value > (U64_MAX - digit) / 10) return false;
                value = value * 10 + digit;
                ++cursor;
            }
            if(cursor == end || *cursor != '@') return false;
            ++cursor;
            return true;
        }

        static R<Variant> read_blob(const String& str)
        {
            if(str.size() < 8) return E_FAILURE;
            bool use_base85 = !memcmp(str.c_str(), "@base85@", 8 * sizeof(c8));
            bool use_base64 = !memcmp(str.c_str(), "@base64@", 8 * sizeof(c8));
            if(!use_base85 && !use_base64) return E_FAILURE;

            const c8* cursor = str.c_str() + 8;
            const c8* end = str.c_str() + str.size();
            u64 size = 0;
            u64 alignment = 0;
            if(!read_blob_integer(cursor, end, size) ||
                !read_blob_integer(cursor, end, alignment) ||
                size > USIZE_MAX || alignment > USIZE_MAX)
            {
                return E_FAILURE;
            }
            if(alignment && (alignment & (alignment - 1))) return E_FAILURE;

            usize payload_size = (usize)(end - cursor);
            usize expected_size = use_base85 ?
                base85_get_encoded_size((usize)size) :
                base64_get_encoded_size((usize)size);
            if(payload_size != expected_size) return E_FAILURE;
            if(use_base85 && size % 4) return E_FAILURE;

            Blob data((usize)size, (usize)alignment);
            usize decoded_size;
            if(use_base85)
            {
                decoded_size = base85_decode(
                    data.data(), data.size(), cursor, payload_size);
            }
            else
            {
                decoded_size = base64_decode(
                    data.data(), data.size(), cursor, payload_size);
            }
            if(decoded_size != size) return E_FAILURE;
            return Variant(move(data));
        }

        static R<Variant> read_string_or_blob(IReadContext& ctx, const JSONReadOptions& options)
        {
            R<String> s = read_string_literal(ctx, options);
            if (failed(s)) return s.errcode();
            if(options.decode_blobs)
            {
                auto blob = read_blob(s.get());
                if (blob.valid()) return blob;
            }
            return Variant(Name(move(s.get())));
        }

        static R<Variant> read_number(IReadContext& ctx, const JSONReadOptions& options)
        {
            String number;
            bool is_positive = true;
            bool is_floating_point = false;
            c32 ch = ctx.next_char_or_eof();
            if(ch == '-')
            {
                is_positive = false;
                number.push_back('-');
                ctx.consume(ch);
                ch = ctx.next_char_or_eof();
            }
            if(ch == '0')
            {
                number.push_back('0');
                ctx.consume(ch);
                ch = ctx.next_char_or_eof();
                if(ch >= '0' && ch <= '9')
                {
                    return set_error(E_FORMAT_ERROR,
                        "A JSON number cannot have leading zeroes at line %u, pos %u.",
                        ctx.get_line(), ctx.get_pos());
                }
            }
            else if(ch >= '1' && ch <= '9')
            {
                while(ch >= '0' && ch <= '9')
                {
                    number.push_back((c8)ch);
                    ctx.consume(ch);
                    ch = ctx.next_char_or_eof();
                }
            }
            else
            {
                return set_error(E_FORMAT_ERROR,
                    "A digit is expected at line %u, pos %u.",
                    ctx.get_line(), ctx.get_pos());
            }
            if(ch == '.')
            {
                is_floating_point = true;
                number.push_back('.');
                ctx.consume(ch);
                ch = ctx.next_char_or_eof();
                if(ch < '0' || ch > '9')
                {
                    return set_error(E_FORMAT_ERROR,
                        "A digit is expected after the decimal point at line %u, pos %u.",
                        ctx.get_line(), ctx.get_pos());
                }
                while(ch >= '0' && ch <= '9')
                {
                    number.push_back((c8)ch);
                    ctx.consume(ch);
                    ch = ctx.next_char_or_eof();
                }
            }
            if(ch == 'e' || ch == 'E')
            {
                is_floating_point = true;
                number.push_back((c8)ch);
                ctx.consume(ch);
                ch = ctx.next_char_or_eof();
                if(ch == '+' || ch == '-')
                {
                    number.push_back((c8)ch);
                    ctx.consume(ch);
                    ch = ctx.next_char_or_eof();
                }
                if(ch < '0' || ch > '9')
                {
                    return set_error(E_FORMAT_ERROR,
                        "A digit is expected in the exponent at line %u, pos %u.",
                        ctx.get_line(), ctx.get_pos());
                }
                while(ch >= '0' && ch <= '9')
                {
                    number.push_back((c8)ch);
                    ctx.consume(ch);
                    ch = ctx.next_char_or_eof();
                }
            }
            if(is_floating_point)
            {
                c8* number_end = nullptr;
                f64 value = parse_f64(number.c_str(), &number_end);
                if(number_end != number.c_str() + number.size())
                {
                    return set_error(E_FORMAT_ERROR, "Invalid JSON number.");
                }
                if(!std::isfinite(value) && !options.allow_non_finite_numbers)
                {
                    return set_error(E_FORMAT_ERROR,
                        "The JSON number is outside the finite f64 range.");
                }
                return Variant(value);
            }

            const c8* digits = number.c_str() + (is_positive ? 0 : 1);
            usize digit_count = number.size() - (is_positive ? 0 : 1);
            u64 magnitude = 0;
            for(usize i = 0; i < digit_count; ++i)
            {
                u64 digit = (u64)(digits[i] - '0');
                if(magnitude > (U64_MAX - digit) / 10)
                {
                    return set_error(E_FORMAT_ERROR,
                        "The JSON integer is outside the 64-bit integer range.");
                }
                magnitude = magnitude * 10 + digit;
            }
            if(is_positive)
            {
                return Variant(magnitude);
            }
            constexpr u64 negative_limit = (u64)I64_MAX + 1;
            if(magnitude > negative_limit)
            {
                return set_error(E_FORMAT_ERROR,
                    "The JSON integer is outside the 64-bit integer range.");
            }
            if(magnitude == negative_limit)
            {
                return Variant(I64_MIN);
            }
            return Variant(-(i64)magnitude);
        }

        static bool is_token_continuation(c32 ch)
        {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                (ch >= '0' && ch <= '9') || ch == '_';
        }

        static R<Variant> read_value(IReadContext& ctx, const JSONReadOptions& options)
        {
            RV skip_result = skip_whitespaces_and_comments(ctx, options);
            if(failed(skip_result)) return skip_result.errcode();
            c32 ch = ctx.next_char_or_eof();
            if (ch == '\0')
            {
                return set_error(E_FORMAT_ERROR, "Unexpected EOF reached at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
            }
            else if (ch == '{')
            {
                return read_object(ctx, options);
            }
            else if (ch == '[')
            {
                return read_array(ctx, options);
            }
            else if (ch == '"')
            {
                return read_string_or_blob(ctx, options);
            }
            else if (ch == 't' && ctx.next_char_or_eof(1) == 'r' && ctx.next_char_or_eof(2) == 'u' && ctx.next_char_or_eof(3) == 'e')
            {
                if(is_token_continuation(ctx.next_char_or_eof(4)))
                {
                    return set_error(E_FORMAT_ERROR, "Invalid token at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
                }
                ctx.consume('t');
                ctx.consume('r');
                ctx.consume('u');
                ctx.consume('e');
                return Variant(true);
            }
            else if (ch == 'f' && ctx.next_char_or_eof(1) == 'a' && ctx.next_char_or_eof(2) == 'l' && ctx.next_char_or_eof(3) == 's' && ctx.next_char_or_eof(4) == 'e')
            {
                if(is_token_continuation(ctx.next_char_or_eof(5)))
                {
                    return set_error(E_FORMAT_ERROR, "Invalid token at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
                }
                ctx.consume('f');
                ctx.consume('a');
                ctx.consume('l');
                ctx.consume('s');
                ctx.consume('e');
                return Variant(false);
            }
            else if (ch == 'n' && ctx.next_char_or_eof(1) == 'u' && ctx.next_char_or_eof(2) == 'l' && ctx.next_char_or_eof(3) == 'l')
            {
                if(is_token_continuation(ctx.next_char_or_eof(4)))
                {
                    return set_error(E_FORMAT_ERROR, "Invalid token at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
                }
                ctx.consume('n');
                ctx.consume('u');
                ctx.consume('l');
                ctx.consume('l');
                return Variant(VariantType::null);
            }
            else if (options.allow_non_finite_numbers && ch == 'n' &&
                ctx.next_char_or_eof(1) == 'a' && ctx.next_char_or_eof(2) == 'n')
            {
                if(is_token_continuation(ctx.next_char_or_eof(3)))
                {
                    return set_error(E_FORMAT_ERROR, "Invalid token at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
                }
                ctx.consume('n');
                ctx.consume('a');
                ctx.consume('n');
                return Variant(std::numeric_limits<f64>::quiet_NaN());
            }
            else if (options.allow_non_finite_numbers && ch == 'i' &&
                ctx.next_char_or_eof(1) == 'n' && ctx.next_char_or_eof(2) == 'f')
            {
                if(is_token_continuation(ctx.next_char_or_eof(3)))
                {
                    return set_error(E_FORMAT_ERROR, "Invalid token at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
                }
                ctx.consume('i');
                ctx.consume('n');
                ctx.consume('f');
                return Variant(std::numeric_limits<f64>::infinity());
            }
            else if (options.allow_non_finite_numbers && ch == '-' &&
                ctx.next_char_or_eof(1) == 'i' && ctx.next_char_or_eof(2) == 'n' &&
                ctx.next_char_or_eof(3) == 'f')
            {
                if(is_token_continuation(ctx.next_char_or_eof(4)))
                {
                    return set_error(E_FORMAT_ERROR, "Invalid token at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
                }
                ctx.consume('-');
                ctx.consume('i');
                ctx.consume('n');
                ctx.consume('f');
                return Variant(-std::numeric_limits<f64>::infinity());
            }
            else if (ch == '-' || ch == '0' || (ch >= '1' && ch <= '9'))
            {
                return read_number(ctx, options);
            }
            else
            {
                return set_error(E_FORMAT_ERROR,
                    "Unrecognized token: %c(0x%0x) at line %u, pos %u.",
                    (c8)ch, (u32)ch, ctx.get_line(), ctx.get_pos());
            }
        }

        inline void write_indents(String& s, u32 num_indents)
        {
            for (u32 i = 0; i < num_indents; ++i)
            {
                s.append("    ", 4);
            }
        }

        static RV write_string_value(String& s, const c8* v, usize len)
        {
            s.push_back('"');
            const c8* cur = v;
            const c8* end = v + len;
            while (cur < end)
            {
                usize num_bytes;
                R<c32> decode_result = utf8_decode_char(cur, (usize)(end - cur), &num_bytes);
                if(failed(decode_result))
                {
                    return set_error(E_BAD_DATA,
                        "The JSON string contains invalid UTF-8 data.");
                }
                c32 ch = decode_result.get();
                switch (ch)
                {
                case '\"':
                    s.append("\\\"");
                    break;
                case '\\':
                    s.append("\\\\");
                    break;
                case '/':
                    s.append("\\/");
                    break;
                case '\b':
                    s.append("\\b");
                    break;
                case '\f':
                    s.append("\\f");
                    break;
                case '\n':
                    s.append("\\n");
                    break;
                case '\r':
                    s.append("\\r");
                    break;
                case '\t':
                    s.append("\\t");
                    break;
                default:
                    if(ch < 0x20)
                    {
                        c8 escape[7];
                        snprintf(escape, sizeof(escape), "\\u%04X", (u32)ch);
                        s.append(escape);
                    }
                    else
                    {
                        s.append(cur, num_bytes);
                    }
                    break;
                }
                cur += num_bytes;
            }
            s.push_back('"');
            return ok;
        }

        static RV write_blob_value(String& s, const void* data, usize data_size, usize data_alignment)
        {
            if (data_size % 4 == 0)
            {
                c8 buf[128];
                snprintf(buf, 128, "@base85@%llu@%llu@", (long long unsigned int)data_size, (long long unsigned int)data_alignment);
                String raw(buf);
                usize encoded_size = base85_get_encoded_size(data_size);
                usize begin = raw.size();
                raw.resize(raw.size() + encoded_size + 1, '\0');
                usize encoded_chars = base85_encode(
                    raw.data() + begin, raw.size() - begin, data, data_size);
                raw.resize(begin + encoded_chars, '\0');
                RV result = write_string_value(s, raw.c_str(), raw.size());
                if(failed(result)) return result.errcode();
            }
            else
            {
                s.push_back('"');
                c8 buf[128];
                snprintf(buf, 128, "@base64@%llu@%llu@", (long long unsigned int)data_size, (long long unsigned int)data_alignment);
                s.append(buf);
                usize encoded_size = base64_get_encoded_size(data_size);
                usize offset = s.size();
                s.resize(s.size() + encoded_size + 1, '\0');
                usize encoded_chars = base64_encode(
                    s.data() + offset, encoded_size + 1, data, data_size);
                s.resize(offset + encoded_chars, '\0');
                s.push_back('"');
            }
            return ok;
        }

        static RV write_value(
            const Variant& v,
            String& s,
            const JSONWriteOptions& options,
            u32 base_indent)
        {
            switch (v.type())
            {
            case VariantType::null:
                s.append("null");
                break;
            case VariantType::object:
            {
                if (v.empty())
                {
                    s.append("{}"); // prevent indent for empty object.
                }
                else
                {
                    s.push_back('{');
                    if (options.indent)
                    {
                        ++base_indent;
                        s.push_back('\n');
                    }
                    usize count = 0;
                    for (auto& i : v.key_values())
                    {
                        if (options.indent)
                        {
                            write_indents(s, base_indent);
                        }
                        RV string_result = write_string_value(s, i.first.c_str(), i.first.size());
                        if(failed(string_result)) return string_result.errcode();
                        s.push_back(':');
                        if (options.indent)
                        {
                            s.push_back(' ');
                        }
                        RV result = write_value(i.second, s, options, base_indent);
                        if(failed(result)) return result.errcode();
                        if (count != v.size() - 1) s.push_back(',');
                        if (options.indent)
                        {
                            s.push_back('\n');
                        }
                        ++count;
                    }
                    if (options.indent)
                    {
                        --base_indent;
                        write_indents(s, base_indent);
                    }
                    s.push_back('}');
                }
            }
            break;
            case VariantType::array:
            {
                if (v.empty())
                {
                    s.append("[]");
                }
                else
                {
                    s.push_back('[');
                    for (usize i = 0; i < v.size(); ++i)
                    {
                        RV result = write_value(v[i], s, options, base_indent);
                        if(failed(result)) return result.errcode();
                        if (i != v.size() - 1) s.push_back(',');
                    }
                    s.push_back(']');
                }
            }
            break;
            case VariantType::number:
            {
                c8 buf[64];
                switch (v.number_type())
                {
                case VariantNumberType::number_f64:
                {
                    f64 value = v.fnum();
                    if(!std::isfinite(value))
                    {
                        if(!options.allow_non_finite_numbers)
                        {
                            return set_error(E_NOT_SUPPORTED,
                                "Non-finite floating-point values cannot be represented in JSON.");
                        }
                        if(std::isnan(value)) s.append("nan");
                        else s.append(std::signbit(value) ? "-inf" : "inf");
                        break;
                    }
                    i32 length = format_f64(buf, sizeof(buf), value);
                    if(length < 0 || (usize)length >= sizeof(buf))
                    {
                        return set_error(E_FAILURE, "Failed to format a JSON number.");
                    }
                    s.append(buf, (usize)length);
                    bool is_floating_point_token = false;
                    for(i32 i = 0; i < length; ++i)
                    {
                        if(buf[i] == '.' || buf[i] == 'e' || buf[i] == 'E')
                        {
                            is_floating_point_token = true;
                            break;
                        }
                    }
                    if(!is_floating_point_token) s.append(".0");
                    break;
                }
                case VariantNumberType::number_i64:
                {
                    std::to_chars_result conversion = std::to_chars(buf, buf + sizeof(buf), v.inum());
                    if(conversion.ec != std::errc())
                    {
                        return set_error(E_FAILURE, "Failed to format a JSON integer.");
                    }
                    s.append(buf, (usize)(conversion.ptr - buf));
                    break;
                }
                case VariantNumberType::number_u64:
                {
                    std::to_chars_result conversion = std::to_chars(buf, buf + sizeof(buf), v.unum());
                    if(conversion.ec != std::errc())
                    {
                        return set_error(E_FAILURE, "Failed to format a JSON integer.");
                    }
                    s.append(buf, (usize)(conversion.ptr - buf));
                    break;
                }
                default: lupanic(); break;
                }
            }
            break;
            case VariantType::string:
            {
                RV result = write_string_value(s, v.str().c_str(), v.str().size());
                if(failed(result)) return result.errcode();
            }
                break;
            case VariantType::boolean:
                s.append(v.boolean() ? "true" : "false");
                break;
            case VariantType::blob:
                if(!options.encode_blobs)
                {
                    return set_error(E_NOT_SUPPORTED,
                        "BLOB variants cannot be represented under the specified JSON options.");
                }
                RV result = write_blob_value(s, v.blob_data(), v.blob_size(), v.blob_alignment());
                if(failed(result)) return result.errcode();
                break;
            }
            return ok;
        }

        static R<Variant> read_json_context(IReadContext& ctx, const JSONReadOptions& options)
        {
            R<Variant> result = read_value(ctx, options);
            if(ctx.get_read_error().code) return ctx.get_read_error();
            if(failed(result)) return result.errcode();
            if(!options.allow_trailing_content)
            {
                RV skip_result = skip_whitespaces_and_comments(ctx, options);
                if(ctx.get_read_error().code) return ctx.get_read_error();
                if(failed(skip_result)) return skip_result.errcode();
                if(ctx.next_char_or_eof())
                {
                    return set_error(E_FORMAT_ERROR,
                        "Unexpected content after the root JSON value at line %u, pos %u.",
                        ctx.get_line(), ctx.get_pos());
                }
                if(ctx.get_read_error().code) return ctx.get_read_error();
            }
            return result;
        }

        LUNA_VARIANT_UTILS_API R<Variant> read_json(
            const c8* src,
            usize src_size,
            const JSONReadOptions& options)
        {
            lucheck(src);
            if(src_size == USIZE_MAX) src_size = strlen(src);
            BufferReadContext ctx;
            ctx.src = src;
            ctx.cur = src;
            ctx.src_size = src_size;
            ctx.line = 1;
            ctx.pos = 1;
            ctx.encoding = Encoding::utf_8;
            ctx.skip_utf16_bom();
            if(ctx.encoding != Encoding::utf_8 && !options.allow_utf16)
            {
                return set_error(E_FORMAT_ERROR,
                    "UTF-16 input is disabled by the specified JSON options.");
            }
            return read_json_context(ctx, options);
        }

        LUNA_VARIANT_UTILS_API R<Variant> read_json(const c8* src, usize src_size)
        {
            return read_json(src, src_size, JSONReadOptions());
        }

        LUNA_VARIANT_UTILS_API R<Variant> read_json(
            const c8* src,
            const JSONReadOptions& options)
        {
            return read_json(src, USIZE_MAX, options);
        }

        LUNA_VARIANT_UTILS_API R<Variant> read_json(
            IStream* stream,
            const JSONReadOptions& options)
        {
            lucheck(stream);
            StreamReadContext ctx;
            ctx.stream = stream;
            ctx.line = 1;
            ctx.pos = 1;
            RV bom_result = ctx.skip_utf16_bom();
            if(failed(bom_result)) return bom_result.errcode();
            if(ctx.encoding != Encoding::utf_8 && !options.allow_utf16)
            {
                return set_error(E_FORMAT_ERROR,
                    "UTF-16 input is disabled by the specified JSON options.");
            }
            return read_json_context(ctx, options);
        }

        LUNA_VARIANT_UTILS_API R<Variant> read_json(IStream* stream)
        {
            return read_json(stream, JSONReadOptions());
        }

        LUNA_VARIANT_UTILS_API R<String> write_json(
            const Variant& v,
            const JSONWriteOptions& options)
        {
            String result;
            RV write_result = write_value(v, result, options, 0);
            if(failed(write_result)) return write_result.errcode();
            return result;
        }

        LUNA_VARIANT_UTILS_API String write_json(const Variant& v, bool indent)
        {
            JSONWriteOptions options;
            options.indent = indent;
            R<String> result = write_json(v, options);
            lucheck(result.valid());
            return move(result.get());
        }

        LUNA_VARIANT_UTILS_API RV write_json(
            IStream* stream,
            const Variant& v,
            const JSONWriteOptions& options)
        {
            lucheck(stream);
            R<String> data = write_json(v, options);
            if(failed(data)) return data.errcode();
            return stream->write(data.get().data(), data.get().size());
        }

        LUNA_VARIANT_UTILS_API RV write_json(IStream* stream, const Variant& v, bool indent)
        {
            JSONWriteOptions options;
            options.indent = indent;
            return write_json(stream, v, options);
        }
    }
}
