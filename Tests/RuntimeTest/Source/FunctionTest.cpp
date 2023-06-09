/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file FunctionTest.hpp
* @author JXMaster
* @date 2023/6/9
*/
#include "TestCommon.hpp"
#include <Runtime/Functional.hpp>

namespace Luna
{
    i32 test_func1(i32 n1, i32 n2)
    {
        return n1 + n2;
    }

    struct Foo {
        i32 data = 10;
        i32 operator()(i32 n1, i32 n2)
        {
            return n1 + n2 + data;
        }
    };

    struct Bar : public Foo
    {

    };

    void function_test()
    {
        usize allocated = get_allocated_memory();
        {
            Function<i32(i32, i32)> func1(test_func1);
            lutest(func1(1, 2) == 3);
            Bar bar;
            Function<i32(i32, i32)> func2(bar);
            lutest(func2(3, 4) == 17);
            u32 data = 10;
            auto func = [data](int n1, int n2)
            {
                return n1 + n2 + data;
            };
            Function<i32(i32, i32)> func3(func);
            lutest(func3(5, 6) == 21);
        }
        lutest(allocated == get_allocated_memory());
        {
            Function<i32(i32, i32)> func;
            lutest(!func);
            func = test_func1;
            lutest(func(1, 2) == 3);
            Bar bar;
            func = bar;
            lutest(func(3, 4) == 17);
            u32 data = 10;
            auto f = [data](int n1, int n2)
            {
                return n1 + n2 + data;
            };
            func = f;
            lutest(func(5, 6) == 21);
        }
        lutest(allocated == get_allocated_memory());
        {
            Bar bar;
            Function<i32(i32, i32)> func = bar;
            lutest(func(3, 4) == 17);
            auto func2 = func;
            lutest(func2(3, 4) == 17);
        }
        lutest(allocated == get_allocated_memory());
    }
}