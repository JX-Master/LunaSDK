/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file NameTest.cpp
* @author JXMaster
* @date 2020/2/20
*/
#include "TestCommon.hpp"
#include <Runtime/Name.hpp>

namespace Luna
{
	void name_test()
	{
		// name object test.
		Name name1(u8"Thomas");
		Name name2 = u8"Jack";
		Name name3 = String(u8"Thomas");
		Name name4(u8"Thomas", 3);
		Name name5(String(u8"Thomas"), 0, 3);
		lutest(name1 != name2);
		lutest(name1 == name3);
		lutest(name1 != name4);

		// name string ends with \0.
		lutest(name1.c_str()[6] == 0);
		lutest(name2.c_str()[4] == 0);
		lutest(name3.c_str()[6] == 0);
		lutest(name4.c_str()[3] == 0);

		// generated names.
		char str[16];
		Name names[1000];
		for (int i = 0; i < 500; ++i)
		{
			snprintf(str, 16, u8"Name%d", i);
			names[i] = str;
		}
		for (int i = 0; i < 500; ++i)
		{
			snprintf(str, 16, u8"Name%d", i);
			names[500 + i] = str;
		}
		for (int i = 0; i < 500; ++i)
		{
			lutest(names[i] == names[500 + i]);
		}

		for (int i = 0; i < 1000000; ++i)
		{
			Name n("Sample");
		}

	}
}
