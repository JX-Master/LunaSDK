/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SerializeTest.cpp
* @author JXMaster
* @date 2022/5/24
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Tuple.hpp>
#include <Luna/Runtime/Array.hpp>

namespace Luna
{
    void serialize_test()
    {
        {
            Vector<i32> vec1 = { 1, 4, 2, 5, 3, 8, 6, 9, 0 };
            Variant var = serialize(vec1).get();
            Vector<i32> vec2;
            lupanic_if_failed(deserialize(vec2, var));
            lutest(vec1.size() == vec2.size());
            for (usize i = 0; i < vec1.size(); ++i)
            {
                lutest(vec1[i] == vec2[i]);
            }
        }

        {
            Vector<i8> vec1 = { 1, 4, 2, 5, 3, 8, 6, 9, 0 };
            auto var = serialize(vec1).get();
            Vector<i8> vec2;
            lupanic_if_failed(deserialize(vec2, var));
            lutest(vec1.size() == vec2.size());
            for (usize i = 0; i < vec1.size(); ++i)
            {
                lutest(vec1[i] == vec2[i]);
            }
        }

        {
            String str1 = "Test String";
            auto var = serialize(str1).get();
            String str2;
            lupanic_if_failed(deserialize(str2, var));
            lutest(str1.compare(str2) == 0);
        }

        {
            HashMap<Name, String> h1;
            h1.insert(make_pair("Player1", "Weapon1"));
            h1.insert(make_pair("Player2", "Weapon2"));
            h1.insert(make_pair("Player3", "Weapon3"));
            h1.insert(make_pair("Player4", "Weapon4"));
            h1.insert(make_pair("Player5", "Weapon5"));
            h1.insert(make_pair("Player6", "Weapon6"));
            h1.insert(make_pair("Player7", "Weapon7"));
            h1.insert(make_pair("Player8", "Weapon8"));
            h1.insert(make_pair("Player9", "Weapon9"));
            auto var = serialize(h1).get();
            HashMap<Name, String> h2;
            lupanic_if_failed(deserialize(h2, var));
            lutest(h1.size() == h2.size());
            for (auto& i : h1)
            {
                auto iter = h2.find(i.first);
                lutest(iter != h2.end());
                lutest(iter->second.compare(i.second) == 0);
            }
        }

        {
            Tuple<i32, String, Vector<Name>> h1;
            get<0>(h1) = 4;
            get<1>(h1) = "Test String";
            get<2>(h1) = { "Player1", "Player3", "Player2" };
            auto var = serialize(h1).get();
            Tuple<i32, String, Vector<Name>> h2;
            lupanic_if_failed(deserialize(h2, var));
            lutest(get<0>(h1) == get<0>(h2));
            lutest(!get<1>(h1).compare(get<1>(h2)));
            auto& v1 = get<2>(h1);
            auto& v2 = get<2>(h2);
            for (usize i = 0; i < v1.size(); ++i)
            {
                lutest(v1[i] == v2[i]);
            }
        }

        {
            Array<i32> arr1 = { 1, 4, 2, 5, 3, 8, 6, 9, 0 };
            Variant var = serialize(arr1).get();
            Array<i32> arr2;
            lupanic_if_failed(deserialize(arr2, var));
            lutest(arr1.size() == arr2.size());
            for (usize i = 0; i < arr1.size(); ++i)
            {
                lutest(arr1[i] == arr2[i]);
            }
        }

        {
            Array<i32, 9> arr1;
            arr1[0] = 1;
            arr1[1] = 4;
            arr1[2] = 2;
            arr1[3] = 5;
            arr1[4] = 3;
            arr1[5] = 8;
            arr1[6] = 6;
            arr1[7] = 9;
            arr1[8] = 0;
            Variant var = serialize(arr1).get();
            Array<i32, 9> arr2;
            lupanic_if_failed(deserialize(arr2, var));
            lutest(arr1.size() == arr2.size());
            for (usize i = 0; i < arr1.size(); ++i)
            {
                lutest(arr1[i] == arr2[i]);
            }
        }

        {
            Array<i32> arr1 = { 1, 4, 2, 5, 3, 8, 6, 9, 0 };
            Variant var = serialize(arr1).get();
            Array<i32, 9> arr2;
            lupanic_if_failed(deserialize(arr2, var));
            lutest(arr1.size() == arr2.size());
            for (usize i = 0; i < arr1.size(); ++i)
            {
                lutest(arr1[i] == arr2[i]);
            }
        }
    }
}