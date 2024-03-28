/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ListTest.cpp
* @author JXMaster
* @date 2021/5/3
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/List.hpp>

namespace Luna
{
    void list_test()
    {
        // List();
        {
            List<int> l;
            lutest(l.size() == 0);
            lutest(l.empty());
            lutest(l.begin() == l.end());
        }

        // explicit List(usize count)
        {
            const int test_size = 42;
            List<int> l(test_size);
            lutest(!l.empty());
            lutest(l.size() == test_size);

            lutest(all_of(l.begin(), l.end(), [](int e)
            { return e == 0; }));
        }

        // List(usize count, const_reference value)
        {
            const int test_size = 42;
            const int test_val = 435;

            List<int> l(42, test_val);
            lutest(!l.empty());
            lutest(l.size() == test_size);

            lutest(all_of(l.begin(), l.end(), [=](int e)
            { return e == test_val; }));
        }

        // List(const List& rhs)
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> b(a);
            lutest(a == b);
            lutest(a.size() == b.size());
        }

        // List(List&& rhs)
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            lutest(!a.empty());
            lutest(a.size() == 10);

            List<int> b(move(a));
            lutest(a.empty());
            lutest(!b.empty());
            lutest(a.size() == 0);
            lutest(b.size() == 10);

            lutest(a != b);
            lutest(a.size() != b.size());
        }

        // List(InitializerList<value_type> ilist)
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            for_each(a.begin(), a.end(), [&](int e)
            {
                static int inc = 0;
                lutest(inc++ == e);
            });
        }

        // List(_InputIt first, _InputIt last)
        {
            List<int> ref = { 3, 4, 5, 6, 7 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            auto start = a.begin();
            advance(start, 3);

            auto end = start;
            advance(end, 5);

            List<int> b(start, end);

            lutest(b == ref);

            lutest(a.size() == 10);
            lutest(b.size() == 5);

            lutest(!b.empty());
            lutest(!a.empty());
        }

        // List& operator=(const List& rhs)
        // List& operator=(List&& rhs)
        // List& operator=(InitializerList<value_type> ilist)
        {
            const List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> b = a;
            lutest(a.size() == 10);
            lutest(b.size() == 10);
            lutest(!a.empty());
            lutest(!b.empty());
            lutest(b == a);

            List<int> c = move(b);
            lutest(b.empty());

            lutest(c == a);
            lutest(c.size() == 10);
        }

        // void swap(List& other)
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> b = {};

            lutest(!a.empty());
            lutest(b.empty());

            b.swap(a);

            lutest(a.empty());
            lutest(!b.empty());
        }

        // void assign(usize count, const_reference value)
        {
            List<int> ref = { 42, 42, 42, 42 };
            List<int> a = { 0, 1, 2, 3 };
            a.assign(4, 42);
            lutest(a == ref);
            lutest(!a.empty());
            lutest(a.size() == 4);
        }

        // void assign(_InputIt first, _InputIt last)
        {
            List<int> ref = List<int>{ 3, 4, 5, 6, 7 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> b;

            auto start = a.begin();
            advance(start, 3);

            auto end = start;
            advance(end, 5);

            b.assign(start, end);

            lutest(b == ref);

            lutest(a.size() == 10);
            lutest(b.size() == 5);

            lutest(!b.empty());
            lutest(!a.empty());
        }

        // void assign(InitializerList<value_type> ilist)
        {
            List<int> ref = List<int>{ 3, 4, 5, 6, 7 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> b;

            auto start = a.begin();
            advance(start, 3);

            auto end = start;
            advance(end, 5);

            b.assign(start, end);

            lutest(b == ref);

            lutest(a.size() == 10);
            lutest(b.size() == 5);

            lutest(!b.empty());
            lutest(!a.empty());
        }

        // iterator       begin() 
        // const_iterator begin() const 
        // const_iterator cbegin() const 
        // iterator       end() 
        // const_iterator end() const 
        // const_iterator cend() const 
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            {
                static int inc = 0;
                auto iter = a.begin();
                while (iter != a.end())
                {
                    lutest(*iter++ == inc++);
                }
            }

            {
                static int inc = 0;
                auto iter = a.cbegin();
                while (iter != a.cend())
                {
                    lutest(*iter++ == inc++);
                }
            }
        }

        // reverse_iterator       rbegin()
        // const_reverse_iterator rbegin() const
        // const_reverse_iterator crbegin() const
        // reverse_iterator       rend()
        // const_reverse_iterator rend() const
        // const_reverse_iterator crend() const 
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            {
                static int inc = 9;
                auto iter = a.rbegin();
                while (iter != a.rend())
                {
                    lutest(*iter == inc--);
                    iter++;
                }
            }

            {
                static int inc = 9;
                auto iter = a.crbegin();
                while (iter != a.crend())
                {
                    lutest(*iter == inc--);
                    iter++;
                }
            }
        }

        // bool empty() const 
        {
            {
                List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                lutest(!a.empty());
            }

            {
                List<int> a = {};
                lutest(a.empty());
            }
        }

        // usize size() const
        {
            {
                List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                lutest(a.size() == 10);
            }

            {
                List<int> a = { 0, 1, 2, 3, 4 };
                lutest(a.size() == 5);
            }

            {
                List<int> a = { 0, 1 };
                lutest(a.size() == 2);
            }

            {
                List<int> a = {};
                lutest(a.size() == 0);
            }
        }

        // void resize(usize n, const value_type& value)
        // void resize(usize n)
        {
            {
                List<int> a;
                a.resize(10);
                lutest(a.size() == 10);
                lutest(!a.empty());
                lutest(all_of(a.begin(), a.end(), [](int i)
                { return i == 0; }));
            }

            {
                List<int> a;
                a.resize(10, 42);
                lutest(a.size() == 10);
                lutest(!a.empty());
                lutest(all_of(a.begin(), a.end(), [](int i)
                { return i == 42; }));
            }
        }

        // reference       front()
        // const_reference front() const
        {
            {
                List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                lutest(a.front() == 0);

                a.front() = 42;
                lutest(a.front() == 42);
            }

            {
                const List<int> a = { 5, 6, 7, 8, 9 };
                lutest(a.front() == 5);
            }

            {
                List<int> a = { 9 };
                lutest(a.front() == 9);

                a.front() = 42;
                lutest(a.front() == 42);
            }
        }

        // reference       back()
        // const_reference back() const
        {
            {
                List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                lutest(a.back() == 9);

                a.back() = 42;
                lutest(a.back() == 42);
            }

            {
                const List<int> a = { 5, 6, 7, 8, 9 };
                lutest(a.back() == 9);
            }

            {
                List<int> a = { 9 };
                lutest(a.back() == 9);

                a.back() = 42;
                lutest(a.back() == 42);
            }
        }

        // void emplace_front(Args&&... args)
        // void emplace_front(value_type&& value)
        // void emplace_front(const value_type& value)
        {
            List<int> ref = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
            List<int> a;

            for (int i = 0; i < 10; i++)
                a.emplace_front(i);

            lutest(a == ref);
        }

        // template <typename... Args>
        // void emplace_back(Args&&... args)
        // void emplace_back(value_type&& value)
        // void emplace_back(const value_type& value)
        {
            {
                List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                List<int> a;

                for (int i = 0; i < 10; i++)
                    a.emplace_back(i);

                lutest(a == ref);
            }

            {
                struct A
                {
                    A() : mValue(0) {}
                    A(int in) : mValue(in) {}
                    int mValue;
                    bool operator==(const A& other) const { return mValue == other.mValue; }
                };

                {
                    List<A> ref = { {1}, {2}, {3} };
                    List<A> a;

                    a.emplace_back(1);
                    a.emplace_back(2);
                    a.emplace_back(3);

                    lutest(a == ref);
                }

                {
                    List<A> ref = { {1}, {2}, {3} };
                    List<A> a;

                    a.emplace_back(A(1));
                    a.emplace_back(A(2));
                    a.emplace_back(A(3));

                    lutest(a == ref);
                }


                {
                    List<A> ref = { {1}, {2}, {3} };
                    List<A> a;

                    A a1(1);
                    A a2(2);
                    A a3(3);

                    a.emplace_back(a1);
                    a.emplace_back(a2);
                    a.emplace_back(a3);

                    lutest(a == ref);
                }
            }
        }

        // void push_front(const value_type& value)
        // void push_front(value_type&& x)
        {
            {
                List<int> ref = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
                List<int> a;

                for (int i = 0; i < 10; i++)
                    a.push_front(i);

                lutest(a == ref);

            }
        }

        // void push_back(const value_type& value)
        // void push_back(value_type&& x)
        {
            {
                List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                List<int> a;

                for (int i = 0; i < 10; i++)
                    a.push_back(i);

                lutest(a == ref);
            }

            {
                struct A { int mValue; };
                List<A> a;
                a.push_back(A{ 42 });
                lutest(a.back().mValue == 42);
            }
        }

        // void pop_front()
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            for (unsigned i = 0; i < 10; i++)
            {
                lutest(unsigned(a.front()) == i);
                a.pop_front();
            }
        }

        // void pop_back()
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            for (unsigned i = 0; i < 10; i++)
            {
                lutest(unsigned(a.back()) == (9 - i));
                a.pop_back();
            }
        }

        // iterator emplace(const_iterator pos, Args&&... args)
        // iterator emplace(const_iterator pos, value_type&& value)
        // iterator emplace(const_iterator pos, const value_type& value)
        {
            List<int> ref = { 0, 1, 2, 3, 4, 42, 5, 6, 7, 8, 9 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            auto insert_pos = a.begin();
            advance(insert_pos, 5);

            a.emplace(insert_pos, 42);
            lutest(a == ref);
        }

        // iterator insert(const_iterator pos)
        // iterator insert(const_iterator pos, const value_type& value)
        // iterator insert(const_iterator pos, value_type&& value)
        {
            List<int> ref = { 0, 1, 2, 3, 4, 42, 5, 6, 7, 8, 9 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            auto insert_pos = a.begin();
            advance(insert_pos, 5);

            a.insert(insert_pos, 42);
            lutest(a == ref);
        }

        // iterator insert(const_iterator pos, usize count, const value_type& value)
        {
            List<int> ref = { 0, 1, 2, 3, 4, 42, 42, 42, 42, 5, 6, 7, 8, 9 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            auto insert_pos = a.begin();
            advance(insert_pos, 5);

            auto result = a.insert(insert_pos, 4, 42);
            lutest(a == ref);
            lutest(*result == 42);
            lutest(*(--result) == 4);
        }

        // iterator insert(const_iterator pos, _InputIt first, _InputIt last)
        {
            List<int> to_insert = { 42, 42, 42, 42 };
            List<int> ref = { 0, 1, 2, 3, 4, 42, 42, 42, 42, 5, 6, 7, 8, 9 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            auto insert_pos = a.begin();
            advance(insert_pos, 5);

            auto result = a.insert(insert_pos, to_insert.begin(), to_insert.end());
            lutest(a == ref);
            lutest(*result == 42);
            lutest(*(--result) == 4);
        }

        // iterator insert(const_iterator pos, InitializerList<value_type> ilist)
        {
            List<int> ref = { 0, 1, 2, 3, 4, 42, 42, 42, 42, 5, 6, 7, 8, 9 };
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            auto insert_pos = a.begin();
            advance(insert_pos, 5);

            a.insert(insert_pos, { 42, 42, 42, 42 });
            lutest(a == ref);
        }

        // iterator erase(const_iterator pos)
        {
            List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a = { 0, 1, 2, 3, 4, 42, 5, 6, 7, 8, 9 };

            auto erase_pos = a.begin();
            advance(erase_pos, 5);

            auto iter_after_removed = a.erase(erase_pos);
            lutest(*iter_after_removed == 5);
            lutest(a == ref);
        }

        // iterator erase(const_iterator first, const_iterator last)
        {
            List<int> a = { 0, 1, 2, 3, 4, 42, 42, 42, 42, 5, 6, 7, 8, 9 };
            List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            auto erase_begin = a.begin();
            advance(erase_begin, 5);

            auto erase_end = erase_begin;
            advance(erase_end, 4);

            a.erase(erase_begin, erase_end);
            lutest(a == ref);
        }

        // void clear()
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            a.clear();
            lutest(a.empty());
            lutest(a.size() == 0);
        }

        // void remove(const T& value)
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> ref = { 0, 1, 2, 3, 5, 6, 7, 8, 9 };
            a.remove(4);
            lutest(a == ref);
        }

        // void remove_if(_UnaryPredicate p)
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> ref = { 0, 1, 2, 3, 5, 6, 7, 8, 9 };
            a.remove_if([](int e) { return e == 4; });
            lutest(a == ref);
        }

        // void reverse()
        {
            List<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> ref = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
            a.reverse();
            lutest(a == ref);
        }

        // void splice(const_iterator pos, List& other)
        {
            const List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a1 = { 0, 1, 2, 3, 4 };
            List<int> a2 = { 5, 6, 7, 8, 9 };

            List<int> a;
            a.splice(a.begin(), a2);
            a.splice(a.begin(), a1);

            lutest(a == ref);
            lutest(a1.empty());
            lutest(a2.empty());
        }

        // void splice(const_iterator pos, List& other, const_iterator it)
        {
            const List<int> ref = { 0, 5 };
            List<int> a1 = { -1, -1, 0 };
            List<int> a2 = { -1, -1, 5 };

            auto a1_begin = a1.begin();
            auto a2_begin = a2.begin();

            advance(a1_begin, 2);
            advance(a2_begin, 2);

            List<int> a;
            a.splice(a.begin(), a2, a2_begin);
            a.splice(a.begin(), a1, a1_begin);

            lutest(a == ref);
            lutest(!a1.empty());
            lutest(!a2.empty());
        }

        // void splice(const_iterator pos, List& other, const_iterator first, const_iterator last)
        {
            const List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a1 = { -1, -1, 0, 1, 2, 3, 4, -1, -1 };
            List<int> a2 = { -1, -1, 5, 6, 7, 8, 9, -1, -1 };

            auto a1_begin = a1.begin();
            auto a2_begin = a2.begin();
            auto a1_end = a1.end();
            auto a2_end = a2.end();

            advance(a1_begin, 2);
            advance(a2_begin, 2);
            advance(a1_end, -2);
            advance(a2_end, -2);

            List<int> a;
            a.splice(a.begin(), a2, a2_begin, a2_end);
            a.splice(a.begin(), a1, a1_begin, a1_end);

            const List<int> rref = { -1, -1, -1, -1 };  // post splice reference list
            lutest(a == ref);
            lutest(a1 == rref);
            lutest(a2 == rref);
        }

        // void splice(const_iterator pos, List&& other)
        {
            const List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a1 = { 0, 1, 2, 3, 4 };
            List<int> a2 = { 5, 6, 7, 8, 9 };

            List<int> a;
            a.splice(a.begin(), move(a2));
            a.splice(a.begin(), move(a1));

            lutest(a == ref);
            lutest(a1.empty());
            lutest(a2.empty());
        }

        // void splice(const_iterator pos, List&& other, const_iterator it)
        {
            const List<int> ref = { 0, 5 };
            List<int> a1 = { -1, -1, 0 };
            List<int> a2 = { -1, -1, 5 };

            auto a1_begin = a1.begin();
            auto a2_begin = a2.begin();

            advance(a1_begin, 2);
            advance(a2_begin, 2);

            List<int> a;
            a.splice(a.begin(), move(a2), a2_begin);
            a.splice(a.begin(), move(a1), a1_begin);

            lutest(a == ref);
            lutest(!a1.empty());
            lutest(!a2.empty());
        }

        // void splice(const_iterator pos, List&& other, const_iterator first, const_iterator last)
        {
            const List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a1 = { -1, -1, 0, 1, 2, 3, 4, -1, -1 };
            List<int> a2 = { -1, -1, 5, 6, 7, 8, 9, -1, -1 };

            auto a1_begin = a1.begin();
            auto a2_begin = a2.begin();
            auto a1_end = a1.end();
            auto a2_end = a2.end();

            advance(a1_begin, 2);
            advance(a2_begin, 2);
            advance(a1_end, -2);
            advance(a2_end, -2);

            List<int> a;
            a.splice(a.begin(), move(a2), a2_begin, a2_end);
            a.splice(a.begin(), move(a1), a1_begin, a1_end);

            const List<int> rref = { -1, -1, -1, -1 };  // post splice reference list
            lutest(a == ref);
            lutest(a1 == rref);
            lutest(a2 == rref);
        }


        // void merge(List& x)
        // void merge(List&& x)
        // void merge(List& x, _Compare comp)
        {
            List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a1 = { 0, 1, 2, 3, 4 };
            List<int> a2 = { 5, 6, 7, 8, 9 };
            a1.merge(a2);
            lutest(a1 == ref);
        }

        // void merge(List&& x, _Compare comp)
        {
            List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a1 = { 0, 1, 2, 3, 4 };
            List<int> a2 = { 5, 6, 7, 8, 9 };
            a1.merge(a2, [](int lhs, int rhs) { return lhs < rhs; });
            lutest(a1 == ref);
        }

        // void unique();
        {
            List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 3, 3, 3,
                                  4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9 };
            a.unique();
            lutest(a == ref);
        }

        // void unique(_BinaryPredicate p);
        {
            static bool bBreakComparison;
            struct A
            {
                int mValue;
                bool operator==(const A& other) const { return bBreakComparison ? false : mValue == other.mValue; }
            };

            List<A> ref = { {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9} };
            List<A> a = { {0}, {0}, {0}, {0}, {0}, {0}, {1}, {2}, {2}, {2}, {2}, {3}, {4}, {5},
                                {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {8}, {9}, {9}, {9} };

            bBreakComparison = true;
            a.unique(); // noop because broken comparison operator
            lutest(a != ref);

            a.unique([](const A& lhs, const A& rhs) { return lhs.mValue == rhs.mValue; });

            bBreakComparison = false;
            lutest(a == ref);
        }

        // void sort()
        {
            List<int> ref = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            List<int> a = { 9, 4, 5, 3, 1, 0, 6, 2, 7, 8 };

            a.sort();
            lutest(a == ref);
        }

        // void sort(_Compare comp)
        {
            struct A
            {
                int mValue;
                bool operator==(const A& other) const { return mValue == other.mValue; }
            };

            List<A> ref = { {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9} };
            List<A> a = { {1}, {0}, {2}, {9}, {4}, {5}, {6}, {7}, {3}, {8} };

            a.sort([](const A& lhs, const A& rhs) { return lhs.mValue < rhs.mValue; });
            lutest(a == ref);
        }
    }
}