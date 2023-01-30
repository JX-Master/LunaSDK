/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ArrayTest.cpp
* @author JXMaster
* @date 2022/9/27
*/
#include "TestCommon.hpp"
#include <Runtime/Array.hpp>

namespace Luna
{
	void array_test()
	{
		TestObject::reset();
		{
			Array<TestObject, 5> arr1;
			arr1[0] = TestObject(0);
			arr1[1] = TestObject(1);
			arr1[2] = TestObject(3);
			arr1[3] = TestObject(5);
			arr1[4] = TestObject(7);
			lutest(!arr1.empty());
			lutest(arr1.size() == 5);
			lutest(arr1[0] == TestObject(0));
			lutest(arr1[1] == TestObject(1));
			lutest(arr1[2] == TestObject(3));
			lutest(arr1[3] == TestObject(5));
			lutest(arr1[4] == TestObject(7));
			auto arr2 = arr1;
			lutest(!arr2.empty());
			lutest(arr2.size() == 5);
			lutest(arr2[0] == TestObject(0));
			lutest(arr2[1] == TestObject(1));
			lutest(arr2[2] == TestObject(3));
			lutest(arr2[3] == TestObject(5));
			lutest(arr2[4] == TestObject(7));
			auto arr3 = move(arr1);
			lutest(!arr3.empty());
			lutest(arr3.size() == 5);
			lutest(arr3[0] == TestObject(0));
			lutest(arr3[1] == TestObject(1));
			lutest(arr3[2] == TestObject(3));
			lutest(arr3[3] == TestObject(5));
			lutest(arr3[4] == TestObject(7));
			lutest(!arr1.empty());
			lutest(arr1.size() == 5);
			lutest(arr1[0] == TestObject(0));
			lutest(arr1[1] == TestObject(0));
			lutest(arr1[2] == TestObject(0));
			lutest(arr1[3] == TestObject(0));
			lutest(arr1[4] == TestObject(0));
		}
		lutest(TestObject::is_clear());
		TestObject::reset();
		{
			Array<TestObject> arr1 = { TestObject(0), TestObject(1), TestObject(3), TestObject(5), TestObject(7) };
			lutest(!arr1.empty());
			lutest(arr1.size() == 5);
			lutest(arr1[0] == TestObject(0));
			lutest(arr1[1] == TestObject(1));
			lutest(arr1[2] == TestObject(3));
			lutest(arr1[3] == TestObject(5));
			lutest(arr1[4] == TestObject(7));
			auto arr2 = arr1;
			lutest(!arr2.empty());
			lutest(arr2.size() == 5);
			lutest(arr2[0] == TestObject(0));
			lutest(arr2[1] == TestObject(1));
			lutest(arr2[2] == TestObject(3));
			lutest(arr2[3] == TestObject(5));
			lutest(arr2[4] == TestObject(7));
			auto arr3 = move(arr1);
			lutest(!arr3.empty());
			lutest(arr3.size() == 5);
			lutest(arr3[0] == TestObject(0));
			lutest(arr3[1] == TestObject(1));
			lutest(arr3[2] == TestObject(3));
			lutest(arr3[3] == TestObject(5));
			lutest(arr3[4] == TestObject(7));
			lutest(arr1.empty());
		}
		lutest(TestObject::is_clear());
		TestObject::reset();
		{
			Array<TestObject> arr1;
			lutest(arr1.empty());
			arr1 = { TestObject(0), TestObject(1), TestObject(3), TestObject(5), TestObject(7) };
			lutest(!arr1.empty());
			lutest(arr1.size() == 5);
			lutest(arr1[0] == TestObject(0));
			lutest(arr1[1] == TestObject(1));
			lutest(arr1[2] == TestObject(3));
			lutest(arr1[3] == TestObject(5));
			lutest(arr1[4] == TestObject(7));
		}
	}
}