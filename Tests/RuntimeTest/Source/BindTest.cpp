/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BindTest.hpp
* @author JXMaster
* @date 2022/12/6
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Functional.hpp>

namespace Luna
{
    i32 test_func1(i32 n1, i32 n2, i32 n3, const i32& n4, i32 n5)
    {
        return (n1 + n2 + n3) * n4 + n5;
    }

    struct Foo {
        i32 data = 10;
        i32 print_sum(int n1, int n2)
        {
            return n1 + n2 + data;
        }
    };

    struct Bar : public Foo
    {

    };

    void invoke_test()
    {
        //! Invokes one global function.
        auto ret = invoke(test_func1, 3, 4, 5, 6, 7);
        lutest(ret == 79);

        //! Invokes one member function.
        Foo foo;
        ret = invoke(&Foo::print_sum, foo, 3, 4);
        lutest(ret == 17);
        foo.data = 20;
        ret = invoke(&Foo::print_sum, foo, 3, 4);
        lutest(ret == 27);

        //! Invokes one member variable.
        i32& member_val = invoke(&Foo::data, foo);
        member_val = 10;
        lutest(foo.data == 10);

        //! Invokes one base class member function by passing one derived class object.
        Bar bar;
        ret = invoke(&Foo::print_sum, bar, 3, 4);
        lutest(ret == 17);

        //! Invokes one lambda.
        u32 data = 10;
        auto func = [data](int n1, int n2)
        {
            return n1 + n2 + data;
        };
        ret = invoke(func, 5, 6);
        lutest(ret == 21);
        data = 20;
        ret = invoke(func, 5, 6);
        lutest(ret == 21);
    }
}