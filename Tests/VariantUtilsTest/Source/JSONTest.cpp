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
#include <cmath>
#include <locale.h>

namespace Luna
{
    struct JSONTestStream : IStream
    {
        const c8* data;
        usize data_size;
        usize cursor = 0;
        bool fail_reads = false;

        JSONTestStream(const c8* data, usize data_size) :
            data(data), data_size(data_size) {}

        virtual object_t get_object() override { return nullptr; }

        virtual RV read(void* buffer, usize size, usize* read_bytes) override
        {
            if(fail_reads) return BasicError::bad_data();
            usize remaining = data_size - cursor;
            usize bytes_to_read = size < remaining ? size : remaining;
            if(bytes_to_read)
            {
                memcpy(buffer, data + cursor, bytes_to_read);
                cursor += bytes_to_read;
            }
            if(read_bytes) *read_bytes = bytes_to_read;
            return ok;
        }

        virtual RV write(const void*, usize, usize*) override
        {
            return BasicError::not_supported();
        }
    };

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
            auto explicit_infinity = read_json("inf", options);
            luassert_always(explicit_infinity.valid());
            luassert_always(std::isinf(explicit_infinity.get().fnum()));
            luassert_always(!std::signbit(explicit_infinity.get().fnum()));
            auto explicit_negative_infinity = read_json("-inf", options);
            luassert_always(explicit_negative_infinity.valid());
            luassert_always(std::isinf(explicit_negative_infinity.get().fnum()));
            luassert_always(std::signbit(explicit_negative_infinity.get().fnum()));
            auto explicit_nan = read_json("nan", options);
            luassert_always(explicit_nan.valid());
            luassert_always(std::isnan(explicit_nan.get().fnum()));
            luassert_always(!read_json("infinite", options).valid());
            luassert_always(!read_json("nan_value", options).valid());
            options.allow_non_finite_numbers = false;
            luassert_always(!read_json("1e400", options).valid());
            luassert_always(!read_json("inf", options).valid());
            luassert_always(!read_json("-inf", options).valid());
            luassert_always(!read_json("nan", options).valid());
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
            JSONWriteOptions options;
            options.allow_non_finite_numbers = true;
            auto infinity = write_json(Variant(F64_INFINITY), options);
            luassert_always(infinity.valid());
            luassert_always(!strcmp(infinity.get().c_str(), "inf"));
            auto infinity_round_trip = read_json(infinity.get().c_str(), JSONReadOptions());
            luassert_always(infinity_round_trip.valid());
            luassert_always(std::isinf(infinity_round_trip.get().fnum()));
            luassert_always(!std::signbit(infinity_round_trip.get().fnum()));

            auto negative_infinity = write_json(Variant(-F64_INFINITY), options);
            luassert_always(negative_infinity.valid());
            luassert_always(!strcmp(negative_infinity.get().c_str(), "-inf"));
            auto negative_infinity_round_trip = read_json(
                negative_infinity.get().c_str(), JSONReadOptions());
            luassert_always(negative_infinity_round_trip.valid());
            luassert_always(std::isinf(negative_infinity_round_trip.get().fnum()));
            luassert_always(std::signbit(negative_infinity_round_trip.get().fnum()));

            auto nan = write_json(Variant(F64_NAN), options);
            luassert_always(nan.valid());
            luassert_always(!strcmp(nan.get().c_str(), "nan"));
            auto nan_round_trip = read_json(nan.get().c_str(), JSONReadOptions());
            luassert_always(nan_round_trip.valid());
            luassert_always(std::isnan(nan_round_trip.get().fnum()));

            options.allow_non_finite_numbers = false;
            luassert_always(!write_json(Variant(F64_INFINITY), options).valid());
            luassert_always(!write_json(Variant(-F64_INFINITY), options).valid());
            luassert_always(!write_json(Variant(F64_NAN), options).valid());
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

    static void json_utf8_validation_test()
    {
        using namespace VariantUtils;

        JSONReadOptions strict_options = JSONReadOptions::strict();
        const c8 invalid_leading_byte[] = {'"', (c8)0x80, '"'};
        const c8 overlong_encoding[] = {'"', (c8)0xC0, (c8)0xAF, '"'};
        const c8 truncated_sequence[] = {'"', (c8)0xE2, (c8)0x82};
        const c8 encoded_surrogate[] = {
            '"', (c8)0xED, (c8)0xA0, (c8)0x80, '"'};
        const c8 out_of_range_codepoint[] = {
            '"', (c8)0xF4, (c8)0x90, (c8)0x80, (c8)0x80, '"'};
        const c8 embedded_null[] = {'"', 'a', '\0', 'b', '"'};
        const c8 invalid_trailing_byte[] = {'1', (c8)0x80};

        luassert_always(!read_json(invalid_leading_byte,
            sizeof(invalid_leading_byte), JSONReadOptions()).valid());
        luassert_always(!read_json(overlong_encoding,
            sizeof(overlong_encoding), strict_options).valid());
        luassert_always(!read_json(truncated_sequence,
            sizeof(truncated_sequence), strict_options).valid());
        luassert_always(!read_json(encoded_surrogate,
            sizeof(encoded_surrogate), strict_options).valid());
        luassert_always(!read_json(out_of_range_codepoint,
            sizeof(out_of_range_codepoint), strict_options).valid());
        luassert_always(!read_json(embedded_null,
            sizeof(embedded_null), strict_options).valid());
        luassert_always(!read_json(invalid_trailing_byte,
            sizeof(invalid_trailing_byte), strict_options).valid());

        JSONTestStream invalid_stream(invalid_leading_byte, sizeof(invalid_leading_byte));
        luassert_always(!read_json(&invalid_stream, strict_options).valid());
        JSONTestStream truncated_stream(truncated_sequence, sizeof(truncated_sequence));
        luassert_always(!read_json(&truncated_stream, strict_options).valid());
        JSONTestStream invalid_trailing_stream(
            invalid_trailing_byte, sizeof(invalid_trailing_byte));
        luassert_always(!read_json(&invalid_trailing_stream, strict_options).valid());
        JSONTestStream failing_stream(nullptr, 0);
        failing_stream.fail_reads = true;
        luassert_always(!read_json(&failing_stream, strict_options).valid());

        const c8 invalid_string_bytes[] = {(c8)0x80};
        Variant invalid_string(Name(invalid_string_bytes, sizeof(invalid_string_bytes)));
        luassert_always(!write_json(invalid_string, JSONWriteOptions()).valid());
        luassert_always(!write_json(invalid_string, JSONWriteOptions::strict()).valid());

        Variant invalid_key_object(VariantType::object);
        invalid_key_object[Name(invalid_string_bytes, sizeof(invalid_string_bytes))] =
            Variant((u64)1);
        luassert_always(!write_json(invalid_key_object, JSONWriteOptions()).valid());
        luassert_always(!write_json(
            invalid_key_object, JSONWriteOptions::strict()).valid());
    }

    static void json_object_name_validation_test()
    {
        using namespace VariantUtils;

        JSONReadOptions strict_options = JSONReadOptions::strict();
        luassert_always(!read_json(
            "{\"value\":1,\"value\":2}", JSONReadOptions()).valid());
        luassert_always(!read_json(
            "{\"value\":1,\"value\":2}", strict_options).valid());
        luassert_always(!read_json(
            "{\"value\":1,\"\\u0076alue\":2}", strict_options).valid());

        const c8 embedded_name_bytes[] = {'a', '\0', 'b'};
        auto embedded_name = read_json("\"a\\u0000b\"", strict_options);
        luassert_always(embedded_name.valid());
        Name embedded_name_value = embedded_name.get().str();
        luassert_always(embedded_name_value.size() == sizeof(embedded_name_bytes));
        luassert_always(!memcmp(embedded_name_value.c_str(), embedded_name_bytes,
            sizeof(embedded_name_bytes)));
        auto embedded_name_json = write_json(embedded_name.get(), JSONWriteOptions::strict());
        luassert_always(embedded_name_json.valid());
        luassert_always(!strcmp(
            embedded_name_json.get().c_str(), "\"a\\u0000b\""));

        auto embedded_key = read_json("{\"a\\u0000b\":1}", strict_options);
        luassert_always(embedded_key.valid());
        Name embedded_key_name(embedded_name_bytes, sizeof(embedded_name_bytes));
        luassert_always(embedded_key.get().contains(embedded_key_name));
        luassert_always(embedded_key.get()[embedded_key_name].unum() == 1);
        luassert_always(!read_json(
            "{\"a\\u0000b\":1,\"a\\u0000b\":2}", strict_options).valid());

        const c8* collision_name_1 = "9iZXijw";
        const c8* collision_name_2 = "KH5Q9q46veiKJqQBgQqJ";
        Name name_1(collision_name_1);
        Name name_2(collision_name_2);
        luassert_always(name_1.id() == name_2.id());
        luassert_always(name_1 != name_2);
        auto collision_object = read_json(
            "{\"9iZXijw\":1,\"KH5Q9q46veiKJqQBgQqJ\":2}", strict_options);
        luassert_always(collision_object.valid());
        luassert_always(collision_object.get().size() == 2);
        luassert_always(collision_object.get()[name_1].unum() == 1);
        luassert_always(collision_object.get()[name_2].unum() == 2);
        auto collision_json = write_json(
            collision_object.get(), JSONWriteOptions::strict());
        luassert_always(collision_json.valid());
        auto collision_round_trip = read_json(
            collision_json.get().c_str(), strict_options);
        luassert_always(collision_round_trip.valid());
        luassert_always(collision_round_trip.get().size() == 2);
        luassert_always(collision_round_trip.get()[name_1].unum() == 1);
        luassert_always(collision_round_trip.get()[name_2].unum() == 2);
    }

    static void json_locale_independence_test()
    {
        using namespace VariantUtils;

        const c8* current_locale = setlocale(LC_NUMERIC, nullptr);
        String saved_locale(current_locale ? current_locale : "C");
        const c8* comma_locale_candidates[] = {
            "de_DE.UTF-8", "de_DE.utf8", "fr_FR.UTF-8", "French_France.1252"};
        for(const c8* candidate : comma_locale_candidates)
        {
            if(setlocale(LC_NUMERIC, candidate)) break;
        }

        auto parsed = read_json("1.5", JSONReadOptions::strict());
        auto written = write_json(Variant(1.5), JSONWriteOptions::strict());
        setlocale(LC_NUMERIC, saved_locale.c_str());

        luassert_always(parsed.valid());
        luassert_always(parsed.get().fnum() == 1.5);
        luassert_always(written.valid());
        luassert_always(!strcmp(written.get().c_str(), "1.5"));
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
        json_utf8_validation_test();
        json_object_name_validation_test();
        json_locale_independence_test();
    }
}
