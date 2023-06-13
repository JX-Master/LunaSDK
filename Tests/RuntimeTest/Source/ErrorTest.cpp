/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ErrorTest.cpp
* @author JXMaster
* @date 2020/2/23
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/HashSet.hpp>

namespace Luna
{
	void error_test()
	{
		/*
		prototype:

		errtype TestError
		{
			err1,
			err2,
			err3,
			err4,

			SubError
			{
				err5,
				err6,
				err7,
				err8,
				err9,
			}
		}
		*/

		ErrCode err1 = get_error_code_by_name("TestError", "err1");
		ErrCode err2 = get_error_code_by_name("TestError", "err2");
		ErrCode err3 = get_error_code_by_name("TestError", "err3");
		ErrCode err4 = get_error_code_by_name("TestError", "err4");
		ErrCode err5 = get_error_code_by_name("TestError::SubError", "err5");
		ErrCode err6 = get_error_code_by_name("TestError::SubError", "err6");
		ErrCode err7 = get_error_code_by_name("TestError::SubError", "err7");
		ErrCode err8 = get_error_code_by_name("TestError::SubError", "err8");
		ErrCode err9 = get_error_code_by_name("TestError::SubError", "err9");

		errcat_t test_error = get_error_category_by_name("TestError");
		auto errs = get_all_error_codes_of_category(test_error);
		auto subcats = get_all_error_subcategories_of_category(test_error);
		errcat_t sub_error = get_error_category_by_name("TestError::SubError");
		auto suberrs = get_all_error_codes_of_category(sub_error);

		lutest(subcats.size() == 1);
		lutest(errs.size() == 4);
		lutest(suberrs.size() == 5);
		lutest(subcats[0] == sub_error);

		lutest(get_error_code_category(err1) == test_error);
		lutest(get_error_code_category(err2) == test_error);
		lutest(get_error_code_category(err3) == test_error);
		lutest(get_error_code_category(err4) == test_error);
		lutest(get_error_code_category(err5) == sub_error);
		lutest(get_error_code_category(err6) == sub_error);
		lutest(get_error_code_category(err7) == sub_error);
		lutest(get_error_code_category(err8) == sub_error);
		lutest(get_error_code_category(err9) == sub_error);

		HashSet<ErrCode> s1;
		for (auto i : errs)
		{
			s1.insert(i);
		}
		HashSet<ErrCode> s2;
		for (auto i : suberrs)
		{
			s2.insert(i);
		}

		lutest(s1.contains(err1));
		lutest(s1.contains(err2));
		lutest(s1.contains(err3));
		lutest(s1.contains(err4));
		lutest(s2.contains(err5));
		lutest(s2.contains(err6));
		lutest(s2.contains(err7));
		lutest(s2.contains(err8));
		lutest(s2.contains(err9));
	}
}