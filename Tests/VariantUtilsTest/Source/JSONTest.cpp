/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JSONTest.cpp
* @author JXMaster
* @date 2021/8/3
*/
#include "TestCommon.hpp"
#include <Luna/VariantUtils/JSON.hpp>

namespace Luna
{
    void json_test()
    {
        {
            const c8* src =
                u8"{ \n\
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
    }
}