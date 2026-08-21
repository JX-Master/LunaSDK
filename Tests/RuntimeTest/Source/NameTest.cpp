/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file NameTest.cpp
* @author JXMaster
* @date 2020/2/20
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Hash.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/Atomic.hpp>

namespace Luna
{
    struct NameConcurrencyTestContext
    {
        u32 start;
        u32 failed;
    };

    static void name_concurrency_test_entry(void* params)
    {
        NameConcurrencyTestContext* context = (NameConcurrencyTestContext*)params;
        while (!atom_add_u32(&context->start, 0))
        {
            processor_pause();
        }
        constexpr c8 test_name[] = "NameRegistryConcurrencyTest";
        constexpr usize test_name_size = sizeof(test_name) - 1;
        for (usize i = 0; i < 50000; ++i)
        {
            Name name(test_name);
            if (name.size() != test_name_size || memcmp(name.c_str(), test_name, test_name_size))
            {
                atom_exchange_u32(&context->failed, 1);
                return;
            }
        }
    }

    void name_test()
    {
        // name object test.
        Name name1("Thomas");
        Name name2 = "Jack";
        Name name3 = String("Thomas");
        Name name4("Thomas", 3);
        Name name5(String("Thomas"), 0, 3);
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
            snprintf(str, 16, "Name%d", i);
            names[i] = str;
        }
        for (int i = 0; i < 500; ++i)
        {
            snprintf(str, 16, "Name%d", i);
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

        // Hash collisions must be resolved by comparing the complete name.
        constexpr c8 collision_name_1[] = "9iZXijw";
        constexpr c8 collision_name_2[] = "KH5Q9q46veiKJqQBgQqJ";
        constexpr usize collision_name_1_size = sizeof(collision_name_1) - 1;
        constexpr usize collision_name_2_size = sizeof(collision_name_2) - 1;
        lutest(memhash<name_id_t>(collision_name_1, collision_name_1_size) ==
            memhash<name_id_t>(collision_name_2, collision_name_2_size));
        {
            Name collision_1(collision_name_1, collision_name_1_size);
            Name collision_2(collision_name_2, collision_name_2_size);
            lutest(collision_1.id() == collision_2.id());
            lutest(collision_1 != collision_2);
            lutest(collision_1.c_str() != collision_2.c_str());
            lutest(collision_1.size() == collision_name_1_size);
            lutest(collision_2.size() == collision_name_2_size);
            lutest(!memcmp(collision_1.c_str(), collision_name_1, collision_name_1_size));
            lutest(!memcmp(collision_2.c_str(), collision_name_2, collision_name_2_size));
            lutest(collision_1 == Name(collision_name_1, collision_name_1_size));
            lutest(collision_2 == Name(collision_name_2, collision_name_2_size));
        }
        // Reverse the insertion order to cover the single-entry bucket case for both names.
        {
            Name collision_2(collision_name_2, collision_name_2_size);
            Name collision_1(collision_name_1, collision_name_1_size);
            lutest(collision_1 != collision_2);
            lutest(collision_1 == Name(collision_name_1, collision_name_1_size));
            lutest(collision_2 == Name(collision_name_2, collision_name_2_size));
        }

        // Explicit-size names may contain null characters.
        constexpr c8 embedded_null_name[] = {'a', '\0', 'b'};
        constexpr c8 leading_null_name[] = {'\0', 'x'};
        Name embedded_null(embedded_null_name, sizeof(embedded_null_name));
        Name embedded_null_copy(embedded_null_name, sizeof(embedded_null_name));
        Name leading_null(leading_null_name, sizeof(leading_null_name));
        lutest(embedded_null);
        lutest(embedded_null == embedded_null_copy);
        lutest(embedded_null != Name("a"));
        lutest(embedded_null.size() == sizeof(embedded_null_name));
        lutest(!memcmp(embedded_null.c_str(), embedded_null_name, sizeof(embedded_null_name)));
        lutest(leading_null);
        lutest(leading_null.size() == sizeof(leading_null_name));
        lutest(!memcmp(leading_null.c_str(), leading_null_name, sizeof(leading_null_name)));
        lutest(Name(embedded_null_name, 0).empty());

        String embedded_null_string(embedded_null_name, sizeof(embedded_null_name));
        Name name_from_string(embedded_null_string);
        lutest(name_from_string == embedded_null);
        Name assigned_from_string;
        assigned_from_string = embedded_null_string;
        lutest(assigned_from_string == embedded_null);

        // Assignment must preserve the source before releasing the old value.
        Name self_assigned("SelfAssignedName");
        const c8* self_assigned_string = self_assigned.c_str();
        self_assigned = self_assigned;
        lutest(self_assigned.c_str() == self_assigned_string);
        self_assigned = move(self_assigned);
        lutest(self_assigned.c_str() == self_assigned_string);
        self_assigned = self_assigned.c_str();
        lutest(self_assigned.c_str() == self_assigned_string);

        // Interning and releasing the same name concurrently must not resurrect an entry
        // that another thread is about to delete.
        NameConcurrencyTestContext concurrency_context = {0, 0};
        Vector<Ref<IThread>> concurrency_threads;
        for (usize i = 0; i < 8; ++i)
        {
            auto thread = new_thread(name_concurrency_test_entry, &concurrency_context);
            lutest(succeeded(thread));
            concurrency_threads.push_back(thread.get());
        }
        atom_exchange_u32(&concurrency_context.start, 1);
        concurrency_threads.clear();
        lutest(!atom_add_u32(&concurrency_context.failed, 0));
    }
}
