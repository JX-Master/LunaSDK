/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TupleTest.cpp
* @author JXMaster
* @date 2021/9/23
*/
#include "TestCommon.hpp"
#include <Runtime/Tuple.hpp>

namespace Luna
{
	void tuple_test()
	{
		{
			Tuple<i32, i32, i32, i32> t(1, 2, 3, 4);
			static_assert(sizeof(Tuple<i32, i32, i32, i32>) == sizeof(i32) * 4, "Wrong tuple size.");
			static_assert(alignof(Tuple<i32, i32, i32, i32>) == alignof(i32), "Wrong tuple alignment.");
			lutest(get<0>(t) == 1);
			lutest(get<1>(t) == 2);
			lutest(get<2>(t) == 3);
			lutest(get<3>(t) == 4);

			Tuple<TestObject, TestObject, TestObject, TestObject> t2(1, 2, 3, 4);
		}

		lutest(TestObject::is_clear());
		TestObject::reset();
	}
}