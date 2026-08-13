/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JSONTest.cpp
* @author JXMaster
* @date 2021/8/3
*/
#include "TestCommon.hpp"
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/Runtime/Math/Math.hpp>

namespace Luna
{
    static void json_read_options_test()
    {
        using namespace VariantUtils;

        {
            JSONReadOptions options;
            options.allow_comments = true;
            luassert_always(read_json("/* comment */ 1", options).valid());
            options.allow_comments = false;
            luassert_always(!read_json("/* comment */ 1", options).valid());
        }
        {
            JSONReadOptions options;
            options.allow_trailing_commas = true;
            luassert_always(read_json("[1,]", options).valid());
            luassert_always(read_json("{\"value\":1,}", options).valid());
            options.allow_trailing_commas = false;
            luassert_always(!read_json("[1,]", options).valid());
            luassert_always(!read_json("{\"value\":1,}", options).valid());
        }
        {
            JSONReadOptions options;
            options.allow_nonstandard_escapes = true;
            auto result = read_json("\"\\'\"", options);
            luassert_always(result.valid());
            luassert_always(result.get().str() == "'");
            options.allow_nonstandard_escapes = false;
            luassert_always(!read_json("\"\\'\"", options).valid());
        }
        {
            const c8 nonstandard_whitespace[] = {(c8)0xC2, (c8)0xA0, 'n', 'u', 'l', 'l'};
            JSONReadOptions options;
            options.allow_nonstandard_whitespace = true;
            luassert_always(read_json(
                nonstandard_whitespace, sizeof(nonstandard_whitespace), options).valid());
            options.allow_nonstandard_whitespace = false;
            luassert_always(!read_json(
                nonstandard_whitespace, sizeof(nonstandard_whitespace), options).valid());
        }
        {
            JSONReadOptions options;
            options.allow_trailing_content = true;
            auto result = read_json("1 2", options);
            luassert_always(result.valid());
            luassert_always(result.get().unum() == 1);
            options.allow_trailing_content = false;
            luassert_always(!read_json("1 2", options).valid());
        }
        {
            const c8* blob_string = "\"@base64@3@0@YWJj\"";
            JSONReadOptions options;
            options.decode_blobs = true;
            auto blob_result = read_json(blob_string, options);
            luassert_always(blob_result.valid());
            luassert_always(blob_result.get().type() == VariantType::blob);
            options.decode_blobs = false;
            auto string_result = read_json(blob_string, options);
            luassert_always(string_result.valid());
            luassert_always(string_result.get().type() == VariantType::string);
            luassert_always(string_result.get().str() == "@base64@3@0@YWJj");
        }
        {
            const u8 utf16_json[] = {
                0xFF, 0xFE, 'n', 0, 'u', 0, 'l', 0, 'l', 0
            };
            JSONReadOptions options;
            options.allow_utf16 = true;
            luassert_always(read_json(
                (const c8*)utf16_json, sizeof(utf16_json), options).valid());
            options.allow_utf16 = false;
            luassert_always(!read_json(
                (const c8*)utf16_json, sizeof(utf16_json), options).valid());
        }
        {
            JSONReadOptions options;
            options.allow_non_finite_numbers = true;
            auto infinity = read_json("1e400", options);
            luassert_always(infinity.valid());
            luassert_always(isinf(infinity.get().fnum()));
            options.allow_non_finite_numbers = false;
            luassert_always(!read_json("1e400", options).valid());
        }

        JSONReadOptions strict_options = JSONReadOptions::strict();
        luassert_always(!strict_options.allow_comments);
        luassert_always(!strict_options.allow_trailing_commas);
        luassert_always(!strict_options.allow_nonstandard_escapes);
        luassert_always(!strict_options.allow_nonstandard_whitespace);
        luassert_always(!strict_options.allow_trailing_content);
        luassert_always(!strict_options.decode_blobs);
        luassert_always(!strict_options.allow_utf16);
        luassert_always(!strict_options.allow_non_finite_numbers);
        luassert_always(read_json(
            "{\"jsonrpc\":\"2.0\",\"id\":1}", strict_options).valid());
    }

    static void json_write_options_test()
    {
        using namespace VariantUtils;

        Variant object(VariantType::object);
        object["value"] = Variant((i64)1);
        {
            JSONWriteOptions options;
            options.indent = true;
            auto indented = write_json(object, options);
            luassert_always(indented.valid());
            luassert_always(indented.get().find('\n') != String::npos);
            options.indent = false;
            auto compact = write_json(object, options);
            luassert_always(compact.valid());
            luassert_always(compact.get().find('\n') == String::npos);
        }
        {
            const c8 data[] = {'a', 'b', 'c'};
            Variant blob(Blob((const byte_t*)data, sizeof(data), 0));
            JSONWriteOptions options;
            options.encode_blobs = true;
            luassert_always(write_json(blob, options).valid());
            options.encode_blobs = false;
            luassert_always(!write_json(blob, options).valid());
        }
        {
            Variant infinity(F64_INFINITY);
            JSONWriteOptions options;
            options.allow_non_finite_numbers = true;
            luassert_always(write_json(infinity, options).valid());
            options.allow_non_finite_numbers = false;
            luassert_always(!write_json(infinity, options).valid());
        }

        JSONWriteOptions strict_options = JSONWriteOptions::strict();
        luassert_always(!strict_options.indent);
        luassert_always(!strict_options.encode_blobs);
        luassert_always(!strict_options.allow_non_finite_numbers);
        auto strict_result = write_json(object, strict_options);
        luassert_always(strict_result.valid());
        luassert_always(!strcmp(strict_result.get().c_str(), "{\"value\":1}"));
    }

    static void json_validation_test()
    {
        using namespace VariantUtils;

        JSONReadOptions options = JSONReadOptions::strict();
        luassert_always(!read_json("\"unterminated", options).valid());
        luassert_always(!read_json("/* unterminated", JSONReadOptions()).valid());
        luassert_always(!read_json("[1 2]", options).valid());
        luassert_always(!read_json("01", options).valid());
        luassert_always(!read_json("1.", options).valid());
        luassert_always(!read_json("1e", options).valid());
        luassert_always(!read_json("\"line\nfeed\"", options).valid());
        luassert_always(!read_json("\"\\uD800\"", options).valid());
        luassert_always(!read_json("\"\\uDC00\"", options).valid());

        auto unicode = read_json("\"\\uD83D\\uDE00\"", options);
        luassert_always(unicode.valid());
        luassert_always(unicode.get().str() == "😀");

        JSONWriteOptions write_options = JSONWriteOptions::strict();
        auto escaped_control = write_json(Variant(Name("\a")), write_options);
        luassert_always(escaped_control.valid());
        luassert_always(!strcmp(escaped_control.get().c_str(), "\"\\u0007\""));
        luassert_always(read_json(escaped_control.get().c_str(), options).valid());

        auto floating_point = write_json(Variant(1.0), write_options);
        luassert_always(floating_point.valid());
        auto floating_point_round_trip = read_json(floating_point.get().c_str(), options);
        luassert_always(floating_point_round_trip.valid());
        luassert_always(
            floating_point_round_trip.get().number_type() == VariantNumberType::number_f64);
    }

    void json_test()
    {
        {
            const c8* src =
                "{ \n\
    \"status\": \"0000\", \n\
    \"message\" : \"success\", \n\
    \"response\" : true, \n\
    \"no_reply\" : false, \n\
    \"data\" : { \n\
        \"title\": { \n\
            \"id\": \"001\", \n\
            \"name\" : \"Player HP\" \n\
        }, \n\
        \"content\" : [ \n\
            { \n\
                \"id\": 1, \n\
                \"value\" : \"37.0\" \n\
            }, \n\
            { \n\
                \"id\": 2, \n\
                 \"value\" : \"72.3\" \n\
            } \n\
        ], \n\
        \"meta\": null \n\
    } \n\
}";
            R<Variant> v = VariantUtils::read_json(src);
            luassert_always(succeeded(v));

            String s = VariantUtils::write_json(v.get());
            R<Variant> v2 = VariantUtils::read_json(s.c_str());
            luassert_always(succeeded(v2));

            luassert_always(v.get() == v2.get());
        }
        
        {
            // Blob test.
            const c8 d[17] = "Sample BLOB Data";
            Blob blob((const byte_t*)d, 17, 0);
            Variant blob_var(move(blob));

            String s2 = VariantUtils::write_json(blob_var);
            R<Variant> blob_var2_r = VariantUtils::read_json(s2.c_str());
            luassert_always(succeeded(blob_var2_r));
            Variant& blob_var2 = blob_var2_r.get();
            luassert_always(blob_var == blob_var2);
        }

        {
            // Base85 Blob test.
            const c8 d[20] = "<Sample BLOB Data >";
            Blob blob((const byte_t*)d, 20, 0);
            Variant blob_var(move(blob));

            String s2 = VariantUtils::write_json(blob_var);
            R<Variant> blob_var2_r = VariantUtils::read_json(s2.c_str());
            luassert_always(succeeded(blob_var2_r));
            Variant& blob_var2 = blob_var2_r.get();
            luassert_always(blob_var == blob_var2);
        }

        {
            // Bugfix: reading negative number will result in positive number.
            Variant var1((i64)-3);
            String s = VariantUtils::write_json(var1);
            R<Variant> var2 = VariantUtils::read_json(s.c_str());
            luassert_always(succeeded(var2));
            luassert_always(var1 == var2.get());
        }

        json_read_options_test();
        json_write_options_test();
        json_validation_test();
    }
}
