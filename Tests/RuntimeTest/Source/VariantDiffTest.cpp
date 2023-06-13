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
#include <Luna/Runtime/VariantDiff.hpp>
#include <Luna/Runtime/VariantJSON.hpp>
namespace Luna
{
	void variant_diff_test()
	{
		//Diff_EmptyObjects_EmptyPatch
		{
			Variant empty(VariantType::object);
			Variant result = diff_variant(empty, empty);
			lutest(result.type() == VariantType::null);
		}
		//Diff_EqualBooleanProperty_NoDiff
		{
			Variant before = json_read("{\"p\": true }").get();
			Variant after = json_read("{\"p\": true }").get();
			Variant result = diff_variant(before, after);
			lutest(result.type() == VariantType::null);
		}
		//Diff_DiffBooleanProperty_ValidPatch
		{
			Variant before = json_read("{\"p\": true }").get();
			Variant after = json_read("{\"p\": false }").get();
			Variant result = diff_variant(before, after);
			lutest(result.type() == VariantType::object);
			Variant& p = result["p"];
			lutest(p.valid());
			lutest(p.type() == VariantType::array);
			lutest(p.size() == 2);
			lutest(p.at(0).boolean() == true);
			lutest(p.at(1).boolean() == false);
		}
		//Diff_BooleanPropertyDeleted_ValidPatch
		{
			Variant before = json_read("{\"p\": true }").get();
			Variant after = json_read("{ }").get();
			Variant result = diff_variant(before, after);
			lutest(result.type() == VariantType::object);
			Variant& p = result["p"];
			lutest(p.valid());
			lutest(p.type() == VariantType::array);
			lutest(p.size() == 3);
			lutest(p.at(0).boolean() == true);
			lutest(p.at(1).unum() == 0);
			lutest(p.at(2).unum() == 0);
		}
		//Diff_BooleanPropertyAdded_ValidPatch
		{
			Variant before = json_read("{ }").get();
			Variant after = json_read("{\"p\": true}").get();
			Variant result = diff_variant(before, after);
			lutest(result.type() == VariantType::object);
			Variant& p = result["p"];
			lutest(p.valid());
			lutest(p.type() == VariantType::array);
			lutest(p.size() == 1);
			lutest(p.at(0).boolean() == true);
		}
		//Diff_EfficientArrayDiffSame_NullDiff
		{
			Variant array = json_read("[1,2,3]").get();
			const Variant diff = diff_variant(array, array);
			lutest(diff.type() == VariantType::null);
		}
		//Diff_EfficientArrayDiffDifferentHeadRemoved_ValidDiff
		{
			Variant before = json_read("[1,2,3,4]").get();
			Variant after = json_read("[2,3,4]").get();
			const Variant diff = diff_variant(before, after);
			lutest(diff.valid());
			lutest(diff.size() == 2);
			lutest(diff["_0"].valid());
		}
		//Diff_EfficientArrayDiffDifferentTailRemoved_ValidDiff
		{
			Variant before = json_read("[1,2,3,4]").get();
			Variant after = json_read("[1,2,3]").get();
			const Variant diff = diff_variant(before, after);
			lutest(diff.valid());
			lutest(diff.size() == 2);
			lutest(diff["_3"].valid());
		}
		//Diff_EfficientArrayDiffDifferentHeadAdded_ValidDiff
		{
			Variant before = json_read("[1,2,3,4]").get();
			Variant after = json_read("[0,1,2,3,4]").get();
			const Variant diff = diff_variant(before, after);
			lutest(diff.valid());
			lutest(diff.size() == 2);
			lutest(diff["0"].valid());
		}
		//Diff_EfficientArrayDiffDifferentTailAdded_ValidDiff
		{
			Variant before = json_read("[1,2,3,4]").get();
			Variant after = json_read("[1,2,3,4,5]").get();
			const Variant diff = diff_variant(before, after);
			lutest(diff.valid());
			lutest(diff.size() == 2);
			lutest(diff["4"].valid());
		}
		//Diff_EfficientArrayDiffDifferentHeadTailAdded_ValidDiff
		{
			Variant before = json_read("[1,2,3,4]").get();
			Variant after = json_read("[0,1,2,3,4,5]").get();
			const Variant diff = diff_variant(before, after);
			lutest(diff.valid());
			lutest(diff.size() == 3);
			lutest(diff["0"].valid());
			lutest(diff["5"].valid());
		}
		//Diff_EfficientArrayDiffSameLengthNested_ValidDiff
		{
			Variant before = json_read("[1,2,{\"p\":false},4]").get();
			Variant after = json_read("[1,2,{\"p\":true},4]").get();
			const Variant diff = diff_variant(before, after);
			lutest(diff.valid());
			lutest(diff.size() == 2);
			lutest(diff["2"].valid());
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
			Variant before = json_read(json_source).get();
			Variant after = before;
			Variant diff = diff_variant(before, after);
			lutest(!diff.valid());
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
				lutest(iter->second == Variant(i));
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
			const Variant diff = diff_variant(before, after);
			Variant restored = before;
			patch_variant_diff(restored, diff);
			lutest(restored == after);
		}
		//Diff_IntStringDiff_ValidPatch
		{
			Variant before = json_read("1").get();
			Variant after = json_read("\"hello\"").get();
			Variant diff = diff_variant(before, after);
			lutest(diff.type() == VariantType::array);
			lutest(diff.size() == 2);
			lutest(diff[0] == before);
			lutest(diff[1] == after);
		}
		//Patch_ObjectApplyDelete_Success
		{
			Variant before = json_read("{ \"p\" : true }").get();
			Variant after = json_read("{ }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ObjectApplyAdd_Success
		{
			Variant before = json_read("{ }").get();
			Variant after = json_read("{ \"p\" : true }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
			lutest(patched["p"].type() == VariantType::boolean);
			lutest(patched["p"].boolean() == true);
		}
		//Patch_ObjectApplyEdit_Success
		{
			Variant before = json_read("{ \"p\" : false  }").get();
			Variant after = json_read("{ \"p\" : true }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
			lutest(patched["p"].type() == VariantType::boolean);
			lutest(patched["p"].boolean() == true);
		}
		//Patch_ObjectApplyEditText_Success
		{
			Variant before = json_read("{ \"p\" : \"bla1h111111111111112312weldjidjoijfoiewjfoiefjefijfoejoijfiwoejfiewjfiwejfowjwifewjfejdewdwdewqwertyqwertifwiejifoiwfei\"  }").get();
			Variant after = json_read("{ \"p\" : \"blah1\" }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
			lutest(patched["p"].type() == VariantType::string);
			lutest(patched["p"].str() == "blah1");
		}
		//Patch_NestedObjectApplyEdit_Success
		{
			Variant before = json_read("{ \"i\": { \"p\" : false } }").get();
			Variant after = json_read("{ \"i\": { \"p\" : true } }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_NestedComplexEdit_Success
		{
			Variant before = json_read("{ \"i\": { \"1\" : 1, \"2\": 2 }, \"j\": [0, 2, 4], \"k\": [1] }").get();
			Variant after = json_read("{ \"i\": { \"1\" : 1, \"2\": 3 }, \"j\": [0, 2, 3], \"k\": null }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_NestedComplexEditDifferentLeft_Success
		{
			Variant before = json_read("{ \"i\": { \"1\" : 1, \"2\": 2 }, \"j\": [0, 2, 4], \"k\": [1] }").get();
			Variant after = json_read("{ \"i\": { \"1\" : 1, \"2\": 3 }, \"j\": [0, 2, 3], \"k\": null }").get();
			Variant patch = diff_variant(json_read("{ \"k\": { \"i\": [1] } }").get(), after);
			Variant patched = before;
			
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ArrayPatchAdd_Success
		{
			Variant before = json_read("[1,2,3]").get();
			Variant after = json_read("[1,2,3,4]").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ArrayPatchRemove_Success
		{
			Variant before = json_read("[1,2,3]").get();
			Variant after = json_read("[1,2]").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ArrayPatchModify_Success
		{
			Variant before = json_read("[1,3,{\"p\":false}]").get();
			Variant after = json_read("[1,4,{\"p\": [1] }]").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ArrayPatchComplex_Success
		{
			Variant before = json_read("{\"p\": [1,2,[1],false,\"11111\",3,{\"p\":false},10,10] }").get();
			Variant after = json_read("{\"p\": [1,2,[1,3],false,\"11112\",3,{\"p\":true},10,10] }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ArrayPatchMoving_Success
		{
			Variant before = json_read("[0,1,2,3,4,5,6,7,8,9,10]").get();
			Variant after = json_read("[10,0,1,7,2,4,5,6,88,9,3]").get();
			Variant patch = json_read("{ \"8\": [88], \"_t\": \"a\", \"_3\": [\"\", 10, 3], \"_7\": [\"\", 3, 3], \"_8\": [8, 0, 0], \"_10\": [\"\", 0, 3] }").get();
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ArrayPatchMovingNonConsecutive_Success
		{
			Variant before = json_read("[0,1,3,4,5]").get();
			Variant after = json_read("[0,4,3,1,5]").get();
			Variant patch = json_read("{\"_t\": \"a\", \"_2\": [\"\", 2, 3],\"_3\": [\"\", 1, 3]}").get();
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_ArrayPatchMoveDeletingNonConsecutive_Success
		{
			Variant before = json_read("[0,1,3,4,5]").get();
			Variant after = json_read("[0,5,3]").get();
			Variant patch = json_read("{\"_t\": \"a\", \"_1\": [ 1, 0, 0], \"_3\": [4,0, 0],\"_4\": [ \"\", 1, 3 ]}").get();
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_Bug16Exception_Success
		{
			Variant before = json_read("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 2\r\n      }\r\n    ]\r\n  }\r\n}").get();
			Variant after = json_read("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\",\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 3\r\n      },\r\n      {\r\n        \"name\": \"label-header\"\r\n      }\r\n    ]\r\n  }\r\n}").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Patch_Bug16SilentFail_Success
		{
			Variant before = json_read("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 1,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      }\r\n    ]\r\n  \r\n}").get();
			Variant after = json_read("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 3,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"label-header\",\r\n        \"version\": \"1.0.0\",\r\n        \"column\": 0,\r\n        \"row\": 0,\r\n        \"columnSpan\": 1,\r\n        \"rowSpan\": 1,\r\n        \"properties\": [],\r\n        \"addedArgs\": {},\r\n        \"parent\": \"Acknowledge Unit (111)\",\r\n        \"label\": \"test\"\r\n      }\r\n    ]\r\n  }").get();
			Variant patch = diff_variant(before, after);
			Variant patched = before;
			patch_variant_diff(patched, patch);
			lutest(patched == after);
		}
		//Unpatch_ObjectApplyDelete_Success
		{
			Variant before = json_read("{ \"p\" : true }").get();
			Variant after = json_read("{ }").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ObjectApplyEdit_Success
		{
			Variant before = json_read("{ \"p\" : false }").get();
			Variant after = json_read("{ \"p\" : true }").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ObjectApplyEditText_Success
		{
			Variant before = json_read("{ \"p\" : \"bla1h111111111111112312weldjidjoijfoiewjfoiefjefijfoejoijfiwoejfiewjfiwejfowjwifewjfejdewdwdewqwertyqwertifwiejifoiwfei\" }").get();
			Variant after = json_read("{ \"p\" : \"blah1\" }").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_NestedObjectApplyEdit_Success
		{
			Variant before = json_read("{ \"i\": { \"p\" : false } }").get();
			Variant after = json_read("{ \"i\": { \"p\" : true } }").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ArrayUnpatchAdd_Success
		{
			Variant before = json_read("[1,2,3]").get();
			Variant after = json_read("[1,2,3,4]").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ArrayUnpatchRemove_Success
		{
			Variant before = json_read("[1,2,3]").get();
			Variant after = json_read("[1,2]").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ArrayUnpatchModify_Success
		{
			Variant before = json_read("[1,3,{\"p\":false}]").get();
			Variant after = json_read("[1,4,{\"p\": [1] }]").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ArrayUnpatchComplex_Success
		{
			Variant before = json_read("{\"p\": [1,2,[1],false,\"11111\",3,{\"p\":false},10,10] }").get();
			Variant after = json_read("{\"p\": [1,2,[1,3],false,\"11112\",3,{\"p\":true},10,10] }").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ArrayUnpatchMoving_Success
		{
			Variant before = json_read("[0,1,2,3,4,5,6,7,8,9,10]").get();
			Variant after = json_read("[10,0,1,7,2,4,5,6,88,9,3]").get();
			Variant patch = json_read("{ \"8\": [88], \"_t\": \"a\", \"_3\": [\"\", 10, 3], \"_7\": [\"\", 3, 3], \"_8\": [8, 0, 0], \"_10\": [\"\", 0, 3] }").get();
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ArrayPatchMovingNonConsecutive_Success
		{
			Variant before = json_read("[0,1,3,4,5]").get();
			Variant after = json_read("[0,4,3,1,5]").get();
			Variant patch = json_read("{\"_t\": \"a\", \"_2\": [\"\", 2, 3],\"_3\": [\"\", 1, 3]}").get();
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_ArrayPatchMoveDeletingNonConsecutive_Success
		{
			Variant before = json_read("[0,1,3,4,5]").get();
			Variant after = json_read("[0,5,3]").get();
			Variant patch = json_read("{\"_t\": \"a\", \"_1\": [ 1, 0, 0], \"_3\": [4,0, 0],\"_4\": [ \"\", 1, 3 ]}").get();
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_Bug16Exception_Success
		{
			Variant before = json_read("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 2\r\n      }\r\n    ]\r\n  }\r\n}").get();
			Variant after = json_read("{\r\n  \"rootRegion\": {\r\n    \"rows\": [\r\n      \"auto\",\r\n      \"auto\"\r\n    ],\r\n    \"members\": [\r\n      {\r\n        \"row\": 3\r\n      },\r\n      {\r\n        \"name\": \"label-header\"\r\n      }\r\n    ]\r\n  }\r\n}").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
		}
		//Unpatch_Bug16SilentFail_Success
		{
			Variant before = json_read("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 1,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      }\r\n    ]\r\n  \r\n}").get();
			Variant after = json_read("{\r\n    \"members\": [\r\n      {\r\n        \"name\": \"text-box\",\r\n        \"version\": \"1.0.0\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 3,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [\r\n          {\r\n            \"destPath\": \"ng-model\",\r\n            \"srcPath\": \"cmt\"\r\n          }\r\n        ],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"component-label\",\r\n        \"version\": \"1.0.0\",\r\n        \"label\": \"COMMAND_DIALOG_COMMENT\",\r\n        \"required\": false,\r\n        \"isArray\": false,\r\n        \"row\": 2,\r\n        \"rowSpan\": 1,\r\n        \"column\": 0,\r\n        \"columnSpan\": 1,\r\n        \"readOnly\": false,\r\n        \"properties\": [],\r\n        \"parent\": \"Acknowledge Unit (111)\"\r\n      },\r\n      {\r\n        \"name\": \"label-header\",\r\n        \"version\": \"1.0.0\",\r\n        \"column\": 0,\r\n        \"row\": 0,\r\n        \"columnSpan\": 1,\r\n        \"rowSpan\": 1,\r\n        \"properties\": [],\r\n        \"addedArgs\": {},\r\n        \"parent\": \"Acknowledge Unit (111)\",\r\n        \"label\": \"test\"\r\n      }\r\n    ]\r\n  }").get();
			Variant patch = diff_variant(before, after);
			Variant unpatched = after;
			reverse_variant_diff(unpatched, patch);
			lutest(unpatched == before);
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
			Variant before = json_read(before_json).get();
			Variant after = json_read(after_json).get();
			Variant diff = diff_variant(before, after);

			Variant before2 = VariantType::object;
			before2["row"] = (u64)2;
			Variant after2 = VariantType::object;
			after2["row"] = (u64)3;
			Variant diff2 = diff_variant(before2, after2);

			Vector<Variant> prefix_path = { "rootRegion", "members", (u64)0 };
			variant_diff_prefix(diff2, prefix_path);
			lutest(diff == diff2);

			Variant patched = before;
			patch_variant_diff(patched, diff);
			Variant patched2 = before;
			patch_variant_diff(patched2, diff2);
			lutest(patched == patched2);
		}
	}
}