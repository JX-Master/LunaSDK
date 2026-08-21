/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file FunctionTest.hpp
* @author JXMaster
* @date 2023/6/9
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Functional.hpp>

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

    //! One callable object that contains one `TestObject`, used to test `Function` with non-trivially-copyable
    //! callable objects.
    struct TestCallable
    {
        TestObject m_obj;
        i32 operator()(i32 n1, i32 n2)
        {
            return n1 + n2 + m_obj.m_value;
        }
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
        // Tests that small callable objects are stored in the inline buffer directly without allocating heap memory.
        {
            usize alloc_count = get_memory_allocate_count();
            usize free_count = get_memory_deallocate_count();
            {
                u32 data = 10;
                auto f = [data](i32 n1, i32 n2)
                {
                    return n1 + n2 + data;
                };
                Function<i32(i32, i32)> func1(f);
                Function<i32(i32, i32)> func2(func1);
                lutest(func1(1, 2) == 13);
                lutest(func2(3, 4) == 17);
                Function<i32(i32, i32)> func3(move(func2));
                lutest(!func2);
                lutest(func3(5, 6) == 21);
                func3 = func1;
                lutest(func3(1, 2) == 13);
            }
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            lutest(alloc_count == get_memory_allocate_count());
            lutest(free_count == get_memory_deallocate_count());
#endif
            lutest(allocated == get_allocated_memory());
        }
        // Tests that copying one function wrapper from one non-const lvalue uses the copy constructor instead of the
        // template constructor, so the callable object is copied only once.
        {
            usize alloc_count = get_memory_allocate_count();
            usize free_count = get_memory_deallocate_count();
            {
                struct BigCallable
                {
                    u8 data[64];
                    i32 operator()(i32 n1, i32 n2) const
                    {
                        return n1 + n2 + (i32)data[0];
                    }
                };
                BigCallable callable;
                callable.data[0] = 5;
                Function<i32(i32, i32)> func1(callable);   // Allocates one object on heap.
                Function<i32(i32, i32)> func2(func1);       // Allocates one object on heap (copy from non-const lvalue).
                lutest(func1(1, 2) == 8);
                lutest(func2(3, 4) == 12);
            }
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            lutest(alloc_count + 2 == get_memory_allocate_count());
            lutest(free_count + 2 == get_memory_deallocate_count());
#endif
            lutest(allocated == get_allocated_memory());
        }
        // Tests that callable objects whose alignment requirement exceeds the alignment of the inline buffer are
        // allocated on heap.
        {
            usize alloc_count = get_memory_allocate_count();
            usize free_count = get_memory_deallocate_count();
            {
                struct alignas(64) OverAlignedCallable
                {
                    i32 operator()(i32 n1, i32 n2) const
                    {
                        return n1 + n2;
                    }
                };
                OverAlignedCallable callable;
                Function<i32(i32, i32)> func(callable);
                lutest(func(1, 2) == 3);
            }
#ifdef LUNA_MEMORY_PROFILER_ENABLED
            lutest(alloc_count + 1 == get_memory_allocate_count());
            lutest(free_count + 1 == get_memory_deallocate_count());
#endif
            lutest(allocated == get_allocated_memory());
        }
        // Tests that non-trivially-copyable callable objects are copied, moved and destroyed properly.
        {
            TestObject::reset();
            {
                TestCallable callable;
                callable.m_obj.m_value = 42;
                Function<i32(i32, i32)> func1(callable);       // Copies the callable, +1 TestObject copy.
                Function<i32(i32, i32)> func2(func1);          // Copies the callable, +1 TestObject copy.
                lutest(func1(1, 2) == 45);
                lutest(func2(3, 4) == 49);
                Function<i32(i32, i32)> func3(move(func1));    // Moves the callable, +1 TestObject move.
                lutest(!func1);
                lutest(func3(5, 6) == 53);
                lutest(TestObject::g_copy_ctor_count == 2);
                lutest(TestObject::g_move_ctor_count == 1);
            }
            lutest(TestObject::is_clear());
        }
        // Tests that assigning `nullptr` function pointer results in empty function wrapper.
        {
            Function<i32(i32, i32)> func((i32(*)(i32, i32))nullptr);
            lutest(!func);
            func = (i32(*)(i32, i32))nullptr;
            lutest(!func);
        }
        // Tests self copy assignment and self move assignment.
        {
            u32 data = 10;
            auto f = [data](i32 n1, i32 n2)
            {
                return n1 + n2 + data;
            };
            Function<i32(i32, i32)> func(f);
            func = func;
            lutest(func(1, 2) == 13);
            func = move(func);
            lutest(func(1, 2) == 13);
        }
        // Tests swapping between function wrappers.
        {
            u32 data = 10;
            auto f = [data](i32 n1, i32 n2)
            {
                return n1 + n2 + data;
            };
            Function<i32(i32, i32)> func1(f);
            Function<i32(i32, i32)> func2(test_func1);
            func1.swap(func2);
            lutest(func1(1, 2) == 3);
            lutest(func2(1, 2) == 13);
            Function<i32(i32, i32)> func3;
            func3.swap(func2);
            lutest(func3(1, 2) == 13);
            lutest(!func2);
            func3.swap(func3);
            lutest(func3(1, 2) == 13);
        }
        lutest(allocated == get_allocated_memory());
    }
}
