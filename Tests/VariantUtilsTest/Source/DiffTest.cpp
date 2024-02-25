/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VariantDiffTest.cpp
* @author JXMaster
* @date 2022/6/27
*/
#include "TestCommon.hpp"
#include <Luna/VariantUtils/Diff.hpp>
#include <Luna/VariantUtils/JSON.hpp>
namespace Luna
{
    using namespace VariantUtils;
    void diff_test()
    {
        //Diff_EmptyObjects_EmptyPatch
        {
            Variant empty(VariantType::object);
            Variant result = diff(empty, empty);
            luassert_always(result.type() == VariantType::null);
        }
        //Diff_EqualBooleanProperty_NoDiff
        {
            Variant before = read_json("{\"p\": true }").get();
            Variant after = read_json("{\"p\": true }").get();
            Variant result = diff(before, after);
            luassert_always(result.type() == VariantType::null);
        }
        //Diff_DiffBooleanProperty_ValidPatch
        {
            Variant before = read_json("{\"p\": true }").get();
            Variant after = read_json("{\"p\": false }").get();
            Variant result = diff(before, after);
            luassert_always(result.type() == VariantType::object);
            Variant& p = result["p"];
            luassert_always(p.valid());
            luassert_always(p.type() == VariantType::array);
            luassert_always(p.size() == 2);
            luassert_always(p.at(0).boolean() == true);
            luassert_always(p.at(1).boolean() == false);
        }
        //Diff_BooleanPropertyDeleted_ValidPatch
        {
            Variant before = read_json("{\"p\": true }").get();
            Variant after = read_json("{ }").get();
            Variant result = diff(before, after);
            luassert_always(result.type() == VariantType::object);
            Variant& p = result["p"];
            luassert_always(p.valid());
            luassert_always(p.type() == VariantType::array);
            luassert_always(p.size() == 3);
            luassert_always(p.at(0).boolean() == true);
            luassert_always(p.at(1).unum() == 0);
            luassert_always(p.at(2).unum() == 0);
        }
        //Diff_BooleanPropertyAdded_ValidPatch
        {
            Variant before = read_json("{ }").get();
            Variant after = read_json("{\"p\": true}").get();
            Variant result = diff(before, after);
            luassert_always(result.type() == VariantType::object);
            Variant& p = result["p"];
            luassert_always(p.valid());
            luassert_always(p.type() == VariantType::array);
            luassert_always(p.size() == 1);
            luassert_always(p.at(0).boolean() == true);
        }
        //Diff_EfficientArrayDiffSame_NullDiff
        {
            Variant array = read_json("[1,2,3]").get();
            const Variant delta = diff(array, array);
            luassert_always(delta.type() == VariantType::null);
        }
        //Diff_EfficientArrayDiffDifferentHeadRemoved_ValidDiff
        {
            Variant before = read_json("[1,2,3,4]").get();
            Variant after = read_json("[2,3,4]").get();
            const Variant delta = diff(before, after);
            luassert_always(delta.valid());
            luassert_always(delta.size() == 2);
            luassert_always(delta["_0"].valid());
        }
        //Diff_EfficientArrayDiffDifferentTailRemoved_ValidDiff
        {
            Variant before = read_json("[1,2,3,4]").get();
            Variant after = read_json("[1,2,3]").get();
            const Variant delta = diff(before, after);
            luassert_always(delta.valid());
            luassert_always(delta.size() == 2);
            luassert_always(delta["_3"].valid());
        }
        //Diff_EfficientArrayDiffDifferentHeadAdded_ValidDiff
        {
            Variant before = read_json("[1,2,3,4]").get();
            Variant after = read_json("[0,1,2,3,4]").get();
            const Variant delta = diff(before, after);
            luassert_always(delta.valid());
            luassert_always(delta.size() == 2);
            luassert_always(delta["0"].valid());
        }
        //Diff_EfficientArrayDiffDifferentTailAdded_ValidDiff
        {
            Variant before = read_json("[1,2,3,4]").get();
            Variant after = read_json("[1,2,3,4,5]").get();
            const Variant delta = diff(before, after);
            luassert_always(delta.valid());
            luassert_always(delta.size() == 2);
            luassert_always(delta["4"].valid());
        }
        //Diff_EfficientArrayDiffDifferentHeadTailAdded_ValidDiff
        {
            Variant before = read_json("[1,2,3,4]").get();
            Variant after = read_json("[0,1,2,3,4,5]").get();
            const Variant delta = diff(before, after);
            luassert_always(delta.valid());
            luassert_always(delta.size() == 3);
            luassert_always(delta["0"].valid());
            luassert_always(delta["5"].valid());
        }
        //Diff_EfficientArrayDiffSameLengthNested_ValidDiff
        {
            Variant before = read_json("[1,2,{\"p\":false},4]").get();
            Variant after = read_json("[1,2,{\"p\":true},4]").get();
            const Variant delta = diff(before, after);
            luassert_always(delta.valid());
            luassert_always(delta.size() == 2);
            luassert_always(delta["2"].valid());
        }
        //Diff_EfficientArrayDiffSameWithObject_NoDiff
        {
            const c8 json_source[] = R"(
{
    "@context": [
        "http://www.w3.org/ns/csvw",
        {
            "@language": "en",
            "@base": "http://example.org"
        }
    ]
})";
            Variant before = read_json(json_source).get();
            Variant after = before;
            Variant delta = diff(before, after);
            luassert_always(!delta.valid());
        }
        {
            const u64 array_size = 500;
            HashMap<Name, Variant> entry;
            for (u64 i = 0; i < array_size; ++i)
            {
                c8 buf[32];
                snprintf(buf, 32, "_%lld", i);
                entry.insert(make_pair(Name(buf), i));
            }
            for (u64 i = 0; i < array_size; ++i)
            {
                c8 buf[32];
                snprintf(buf, 32, "_%lld", i);
                auto iter = entry.find(buf);
                luassert_always(iter->second == Variant(i));
            }
        }
        //Diff_EfficientArrayDiffHugeArrays_NoStackOverflow
        {
            const u64 array_size = 1000;
            Variant before;
            Variant after;
            for (u64 i = 0; i < array_size; ++i)
            {
                before.push_back(Variant(i));
            }
            for (u64 i = array_size / 2; i < array_size; ++i)
            {
                after.push_back(Variant(i));
            }
            const Variant delta = diff(before, after);
            Variant restored = before;
            patch(restored, delta);
            luassert_always(restored == after);
        }
        //Diff_IntStringDiff_ValidPatch
        {
            Variant before = read_json("1").get();
            Variant after = read_json("\"hello\"").get();
            Variant delta = diff(before, after);
            luassert_always(delta.type() == VariantType::array);
            luassert_always(delta.size() == 2);
            luassert_always(delta[0] == before);
            luassert_always(delta[1] == after);
        }
        //Patch_ObjectApplyDelete_Success
        {
            Variant before = read_json("{ \"p\" : true }").get();
            Variant after = read_json("{ }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ObjectApplyAdd_Success
        {
            Variant before = read_json("{ }").get();
            Variant after = read_json("{ \"p\" : true }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
            luassert_always(patched["p"].type() == VariantType::boolean);
            luassert_always(patched["p"].boolean() == true);
        }
        //Patch_ObjectApplyEdit_Success
        {
            Variant before = read_json("{ \"p\" : false  }").get();
            Variant after = read_json("{ \"p\" : true }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
            luassert_always(patched["p"].type() == VariantType::boolean);
            luassert_always(patched["p"].boolean() == true);
        }
        //Patch_ObjectApplyEditText_Success
        {
            Variant before = read_json("{ \"p\" : \"bla1h111111111111112312weldjidjoijfoiewjfoiefjefijfoejoijfiwoejfiewjfiwejfowjwifewjfejdewdwdewqwertyqwertifwiejifoiwfei\"  }").get();
            Variant after = read_json("{ \"p\" : \"blah1\" }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
            luassert_always(patched["p"].type() == VariantType::string);
            luassert_always(patched["p"].str() == "blah1");
        }
        //Patch_NestedObjectApplyEdit_Success
        {
            Variant before = read_json("{ \"i\": { \"p\" : false } }").get();
            Variant after = read_json("{ \"i\": { \"p\" : true } }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_NestedComplexEdit_Success
        {
            Variant before = read_json("{ \"i\": { \"1\" : 1, \"2\": 2 }, \"j\": [0, 2, 4], \"k\": [1] }").get();
            Variant after = read_json("{ \"i\": { \"1\" : 1, \"2\": 3 }, \"j\": [0, 2, 3], \"k\": null }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_NestedComplexEditDifferentLeft_Success
        {
            Variant before = read_json("{ \"i\": { \"1\" : 1, \"2\": 2 }, \"j\": [0, 2, 4], \"k\": [1] }").get();
            Variant after = read_json("{ \"i\": { \"1\" : 1, \"2\": 3 }, \"j\": [0, 2, 3], \"k\": null }").get();
            Variant delta = diff(read_json("{ \"k\": { \"i\": [1] } }").get(), after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ArrayPatchAdd_Success
        {
            Variant before = read_json("[1,2,3]").get();
            Variant after = read_json("[1,2,3,4]").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ArrayPatchRemove_Success
        {
            Variant before = read_json("[1,2,3]").get();
            Variant after = read_json("[1,2]").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ArrayPatchModify_Success
        {
            Variant before = read_json("[1,3,{\"p\":false}]").get();
            Variant after = read_json("[1,4,{\"p\": [1] }]").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ArrayPatchComplex_Success
        {
            Variant before = read_json("{\"p\": [1,2,[1],false,\"11111\",3,{\"p\":false},10,10] }").get();
            Variant after = read_json("{\"p\": [1,2,[1,3],false,\"11112\",3,{\"p\":true},10,10] }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ArrayPatchMoving_Success
        {
            Variant before = read_json("[0,1,2,3,4,5,6,7,8,9,10]").get();
            Variant after = read_json("[10,0,1,7,2,4,5,6,88,9,3]").get();
            Variant delta = read_json("{ \"8\": [88], \"_t\": \"a\", \"_3\": [\"\", 10, 3], \"_7\": [\"\", 3, 3], \"_8\": [8, 0, 0], \"_10\": [\"\", 0, 3] }").get();
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ArrayPatchMovingNonConsecutive_Success
        {
            Variant before = read_json("[0,1,3,4,5]").get();
            Variant after = read_json("[0,4,3,1,5]").get();
            Variant delta = read_json("{\"_t\": \"a\", \"_2\": [\"\", 2, 3],\"_3\": [\"\", 1, 3]}").get();
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_ArrayPatchMoveDeletingNonConsecutive_Success
        {
            Variant before = read_json("[0,1,3,4,5]").get();
            Variant after = read_json("[0,5,3]").get();
            Variant delta = read_json("{\"_t\": \"a\", \"_1\": [ 1, 0, 0], \"_3\": [4,0, 0],\"_4\": [ \"\", 1, 3 ]}").get();
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_Bug16Exception_Success
        {
            Variant before = read_json("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 2\r\n      }\r\n    ]\r\n  }\r\n}").get();
            Variant after = read_json("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\",\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 3\r\n      },\r\n      {\r\n        \"name\": \"label-header\"\r\n      }\r\n    ]\r\n  }\r\n}").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Patch_Bug16SilentFail_Success
        {
            Variant before = read_json("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 1,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      }\r\n    ]\r\n  \r\n}").get();
            Variant after = read_json("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 3,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"label-header\",\r\n        \"version\": \"1.0.0\",\r\n        \"column\": 0,\r\n        \"row\": 0,\r\n        \"columnSpan\": 1,\r\n        \"rowSpan\": 1,\r\n        \"properties\": [],\r\n        \"addedArgs\": {},\r\n        \"parent\": \"Acknowledge Unit (111)\",\r\n        \"label\": \"test\"\r\n      }\r\n    ]\r\n  }").get();
            Variant delta = diff(before, after);
            Variant patched = before;
            patch(patched, delta);
            luassert_always(patched == after);
        }
        //Unpatch_ObjectApplyDelete_Success
        {
            Variant before = read_json("{ \"p\" : true }").get();
            Variant after = read_json("{ }").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ObjectApplyEdit_Success
        {
            Variant before = read_json("{ \"p\" : false }").get();
            Variant after = read_json("{ \"p\" : true }").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ObjectApplyEditText_Success
        {
            Variant before = read_json("{ \"p\" : \"bla1h111111111111112312weldjidjoijfoiewjfoiefjefijfoejoijfiwoejfiewjfiwejfowjwifewjfejdewdwdewqwertyqwertifwiejifoiwfei\" }").get();
            Variant after = read_json("{ \"p\" : \"blah1\" }").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_NestedObjectApplyEdit_Success
        {
            Variant before = read_json("{ \"i\": { \"p\" : false } }").get();
            Variant after = read_json("{ \"i\": { \"p\" : true } }").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ArrayUnpatchAdd_Success
        {
            Variant before = read_json("[1,2,3]").get();
            Variant after = read_json("[1,2,3,4]").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ArrayUnpatchRemove_Success
        {
            Variant before = read_json("[1,2,3]").get();
            Variant after = read_json("[1,2]").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ArrayUnpatchModify_Success
        {
            Variant before = read_json("[1,3,{\"p\":false}]").get();
            Variant after = read_json("[1,4,{\"p\": [1] }]").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ArrayUnpatchComplex_Success
        {
            Variant before = read_json("{\"p\": [1,2,[1],false,\"11111\",3,{\"p\":false},10,10] }").get();
            Variant after = read_json("{\"p\": [1,2,[1,3],false,\"11112\",3,{\"p\":true},10,10] }").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ArrayUnpatchMoving_Success
        {
            Variant before = read_json("[0,1,2,3,4,5,6,7,8,9,10]").get();
            Variant after = read_json("[10,0,1,7,2,4,5,6,88,9,3]").get();
            Variant delta = read_json("{ \"8\": [88], \"_t\": \"a\", \"_3\": [\"\", 10, 3], \"_7\": [\"\", 3, 3], \"_8\": [8, 0, 0], \"_10\": [\"\", 0, 3] }").get();
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ArrayPatchMovingNonConsecutive_Success
        {
            Variant before = read_json("[0,1,3,4,5]").get();
            Variant after = read_json("[0,4,3,1,5]").get();
            Variant delta = read_json("{\"_t\": \"a\", \"_2\": [\"\", 2, 3],\"_3\": [\"\", 1, 3]}").get();
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_ArrayPatchMoveDeletingNonConsecutive_Success
        {
            Variant before = read_json("[0,1,3,4,5]").get();
            Variant after = read_json("[0,5,3]").get();
            Variant delta = read_json("{\"_t\": \"a\", \"_1\": [ 1, 0, 0], \"_3\": [4,0, 0],\"_4\": [ \"\", 1, 3 ]}").get();
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_Bug16Exception_Success
        {
            Variant before = read_json("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 2\r\n      }\r\n    ]\r\n  }\r\n}").get();
            Variant after = read_json("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\",\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 3\r\n      },\r\n      {\r\n        \"name\": \"label-header\"\r\n      }\r\n    ]\r\n  }\r\n}").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        //Unpatch_Bug16SilentFail_Success
        {
            Variant before = read_json("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 1,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      }\r\n    ]\r\n  \r\n}").get();
            Variant after = read_json("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 3,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"label-header\",\r\n        \"version\": \"1.0.0\",\r\n        \"column\": 0,\r\n        \"row\": 0,\r\n        \"columnSpan\": 1,\r\n        \"rowSpan\": 1,\r\n        \"properties\": [],\r\n        \"addedArgs\": {},\r\n        \"parent\": \"Acknowledge Unit (111)\",\r\n        \"label\": \"test\"\r\n      }\r\n    ]\r\n  }").get();
            Variant delta = diff(before, after);
            Variant unpatched = after;
            reverse(unpatched, delta);
            luassert_always(unpatched == before);
        }
        // Test diff prefix.
        {
            const c8 before_json[] = R"(
{
        "rootRegion":
        {
                "members":
                [
                        {
                                "row":2
                        }
                ]
        }
}
)";
            const c8 after_json[] = R"(
{
        "rootRegion":
        {
                "members":
                [
                        {
                                "row":3
                        }
                ]
        }
}
)";
            Variant before = read_json(before_json).get();
            Variant after = read_json(after_json).get();
            Variant delta = diff(before, after);

            Variant before2 = VariantType::object;
            before2["row"] = (u64)2;
            Variant after2 = VariantType::object;
            after2["row"] = (u64)3;
            Variant delta2 = diff(before2, after2);

            Vector<Variant> prefix_path = { "rootRegion", "members", (u64)0 };
            add_diff_prefix(delta2, prefix_path);
            luassert_always(delta == delta2);

            Variant patched = before;
            patch(patched, delta);
            Variant patched2 = before;
            patch(patched2, delta2);
            luassert_always(patched == patched2);
        }
    }
}