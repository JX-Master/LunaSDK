/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HashTest.cpp
* @author JXMaster
* @date 2020/2/19
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/UnorderedMap.hpp>
#include <Luna/Runtime/UnorderedSet.hpp>
#include <Luna/Runtime/UnorderedMultiMap.hpp>
#include <Luna/Runtime/UnorderedMultiSet.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Random.hpp>

namespace Luna
{
	struct round_10_hash
	{
		usize operator()(int i)
		{
			return i % 10;
		}
	};

	void open_hash_test()
	{
		TestObject::reset();

		{
			UnorderedSet<int> h;
			lutest(h.empty());
			lutest(h.size() == 0);
			//! When the set is constructed, it does not allocate
			//! any dynamic memory, thus does not have any bucket.
			lutest(h.bucket_count() == 0);

			for (int i = 0; i < 100; ++i)
			{
				h.insert(i);
			}
			lutest(h.size() == 100);

			h.clear();
			lutest(h.empty());
			lutest(h.size() == 0);
			//! When the set gets cleared, it frees all dynamic 
			//! memory, thus does not have any bucket.
			lutest(h.bucket_count() == 0);
		}

		{
			UnorderedMultiSet<int> h;
			lutest(h.empty());
			lutest(h.size() == 0);
			//! When the set is constructed, it does not allocate
			//! any dynamic memory, thus does not have any bucket.
			lutest(h.bucket_count() == 0);

			for (int i = 0; i < 100; ++i)
			{
				h.insert(i);
			}
			lutest(h.size() == 100);

			h.clear();
			lutest(h.empty());
			lutest(h.size() == 0);
			//! When the set gets cleared, it frees all dynamic 
			//! memory, thus does not have any bucket.
			lutest(h.bucket_count() == 0);
		}

		{
			// UnorderedSet
			// size_type          size() const
			// bool               empty() const
			// iterator			  insert(const value_type& value);
			// iterator			  insert(value_type&& value);
			// iterator           find(const key_type& k);
			// const_iterator     find(const key_type& k) const;

			UnorderedSet<int> h;
			constexpr size_t count = 10000;
			lutest(h.empty());
			lutest(h.size() == 0);
			for (int i = 0; i < count; ++i)
			{
				h.insert(i);
			}
			lutest(!h.empty());
			lutest(h.size() == count);
			for (auto iter = h.begin(); iter != h.end(); ++iter)
			{
				int value = *iter;
				lutest(value < count);
			}
			for (int i = 0; i < count * 2; ++i)
			{
				auto iter = h.find(i);
				if (i < count)
				{
					lutest(iter != h.end());
				}
				else
				{
					lutest(iter == h.end());
				}
			}

			// iterator       begin();
			// const_iterator begin() const;
			// iterator       end();
			// const_iterator end() const;
			int* const intarr = (int*)memalloc(count * sizeof(int));
			memset(intarr, 0, count * sizeof(int));
			int n_count = 0;
			for (auto iter = h.begin(); iter != h.end(); ++iter, ++n_count)
			{
				int i = *iter;
				lutest((i >= 0) && (i < count) && (intarr[i] == 0));
				intarr[i] = 1;
			}
			lutest(n_count == count);
			memfree(intarr);
		}

		{
			// UnorderedMultiSet
			// size_type          size() const
			// bool               empty() const
			// iterator           find(const key_type& k);
			// const_iterator     find(const key_type& k) const;

			UnorderedMultiSet<int> h;
			constexpr size_t count = 10000;
			lutest(h.empty());
			lutest(h.size() == 0);
			for (int i = 0; i < count; ++i)
			{
				h.insert(i);
			}
			lutest(!h.empty());
			lutest(h.size() == count);
			for (auto iter = h.begin(); iter != h.end(); ++iter)
			{
				int value = *iter;
				lutest(value < count);
			}
			for (int i = 0; i < count * 2; ++i)
			{
				auto iter = h.find(i);
				if (i < count)
				{
					lutest(iter != h.end());
				}
				else
				{
					lutest(iter == h.end());
				}
			}

			// iterator       begin();
			// const_iterator begin() const;
			// iterator       end();
			// const_iterator end() const;
			int* const intarr = (int*)memalloc(count * sizeof(int));
			memset(intarr, 0, count * sizeof(int));
			int n_count = 0;
			for (auto iter = h.begin(); iter != h.end(); ++iter, ++n_count)
			{
				int i = *iter;
				lutest((i >= 0) && (i < count) && (intarr[i] == 0));
				intarr[i] = 1;
			}
			lutest(n_count == count);
			memfree(intarr);
		}
		{
			// Ctors and dtor.
			UnorderedSet<int> s1;
			lutest(s1.size() == 0);
			for (int i = 0; i < 10; ++i)
			{
				s1.insert(i);
			}
			lutest(s1.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s1.find(i) != s1.end());
			}
			// Copy ctor.
			UnorderedSet<int> s2(s1);
			lutest(s2.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s2.find(i) != s2.end());
			}
			// Move ctor.
			UnorderedSet<int> s3(move(s1));
			lutest(s3.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s3.find(i) != s3.end());
			}
			lutest(s1.size() == 0);
			// Copy assign
			s1 = s2;
			lutest(s1.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s1.find(i) != s1.end());
			}
			lutest(s2.size() == 10);
			// Move assig.
			s3.clear();
			s3 = move(s2);
			lutest(s3.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s3.find(i) != s3.end());
			}
			lutest(s2.size() == 0);
		}
		{
			// Ctors and dtor.
			UnorderedMultiSet<int> s1;
			lutest(s1.size() == 0);
			for (int i = 0; i < 10; ++i)
			{
				s1.insert(i);
			}
			lutest(s1.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s1.find(i) != s1.end());
			}
			// Copy ctor.
			UnorderedMultiSet<int> s2(s1);
			lutest(s2.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s2.find(i) != s2.end());
			}
			// Move ctor.
			UnorderedMultiSet<int> s3(move(s1));
			lutest(s3.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s3.find(i) != s3.end());
			}
			lutest(s1.size() == 0);
			// Copy assign
			s1 = s2;
			lutest(s1.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s1.find(i) != s1.end());
			}
			lutest(s2.size() == 10);
			// Move assig.
			s3.clear();
			s3 = move(s2);
			lutest(s3.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s3.find(i) != s3.end());
			}
			lutest(s2.size() == 0);
		}
		TestObject::reset();
		{
			// UnorderedMap
			// iterator			insert(const value_type& value);
			// iterator			insert(value_type&& value);
			// iterator			erase(const_iterator iter);
			// usize			erase(const key_type& key);
			UnorderedMap<int, TestObject> h;
			h.insert(make_pair(3, TestObject(4, true)));
			TestObject obj(5, false);
			h.insert(make_pair(4, obj));
			TestObject obj2(6, true);
			h.insert(make_pair(5, move(obj2)));
			auto iter = h.find(3);
			lutest(h.size() == 3);
			lutest(iter != h.end() && iter->second == TestObject(4, true));
			iter = h.find(4);
			lutest(iter != h.end() && iter->second == TestObject(5, true));
			iter = h.find(5);
			lutest(iter != h.end() && iter->second == TestObject(6, true));
			h.erase(iter);
			lutest(h.size() == 2);
			h.erase(4);
			lutest(h.size() == 1);
			iter = h.find(3);
			lutest(iter != h.end() && iter->second == TestObject(4, true));
		}
		lutest(TestObject::is_clear());
		TestObject::reset();

		{
			// size_type bucket_count() const
			// size_type bucket_size(size_type n) const
			// float load_factor() const
			// float get_max_load_factor() const;
			// void  set_max_load_factor(float fMaxLoadFactor);
			// void rehash(size_type n);

			UnorderedSet<int> h;
			f32 lf = h.load_factor();
			lutest(lf == 0.0f);
			h.max_load_factor(65536.0f * 512.0f);
			f32 max_lf = h.max_load_factor();
			lutest(max_lf == (65536.0f * 512.0f));
			h.rehash(20);
			size_t n = h.bucket_count();
			lutest((n >= 20) && (n < 25));
			for (int i = 0; i < 10000; ++i)
			{
				h.insert(i);	// This also tests for high loading.
			}
			size_t n2 = h.bucket_count();
			lutest(n2 == n);	// Verify no rehashing has occurred, due to our high load factor.
			n = h.bucket_size(0);	// force rehash & shrink.
			lutest(n >= ((h.size() / h.bucket_count()) / 2));// It will be some high value. We divide by 2 to give it some slop.

			// local_iterator       begin(size_type n);
			// local_iterator       end(size_type n);
			// const_local_iterator begin(size_type n) const;
			// const_local_iterator end(size_type n) const;	
			size_t b = h.bucket_count() - 1;
			for (auto iter = h.begin(b); iter != h.end(b); ++iter)
			{
				int v = *iter;
				lutest((hash<int>()(v) % h.bucket_count()) == b);	// This is how we hash value into bucket.
			}
			// clear();
			h.clear();
			lutest(h.empty());
			lutest(h.size() == 0);
			lutest(h.bucket_count() == 0);	// Currently clear operation will free all buckets, this behavior may change later.
		}

		{
			// size_type bucket_count() const
			// size_type bucket_size(size_type n) const
			// float load_factor() const
			// float get_max_load_factor() const;
			// void  set_max_load_factor(float fMaxLoadFactor);
			// void rehash(size_type n);

			UnorderedMultiSet<int> h;
			f32 lf = h.load_factor();
			lutest(lf == 0.0f);
			h.max_load_factor(65536.0f * 512.0f);
			f32 max_lf = h.max_load_factor();
			lutest(max_lf == (65536.0f * 512.0f));
			h.rehash(20);
			size_t n = h.bucket_count();
			lutest((n >= 20) && (n < 25));
			for (int i = 0; i < 10000; ++i)
			{
				h.insert(i);	// This also tests for high loading.
			}
			size_t n2 = h.bucket_count();
			lutest(n2 == n);	// Verify no rehashing has occurred, due to our high load factor.
			n = h.bucket_size(0);	// force rehash & shrink.
			lutest(n >= ((h.size() / h.bucket_count()) / 2));// It will be some high value. We divide by 2 to give it some slop.

			// local_iterator       begin(size_type n);
			// local_iterator       end(size_type n);
			// const_local_iterator begin(size_type n) const;
			// const_local_iterator end(size_type n) const;	
			size_t b = h.bucket_count() - 1;
			for (auto iter = h.begin(b); iter != h.end(b); ++iter)
			{
				int v = *iter;
				lutest((hash<int>()(v) % h.bucket_count()) == b);	// This is how we hash value into bucket.
			}

			// clear();
			h.clear();
			lutest(h.empty());
			lutest(h.size() == 0);
			lutest(h.bucket_count() == 0);	// Currently clear operation will free all buckets, this behavior may change later.
		}

		{
			// clone()
			// swap()
			UnorderedSet<int> h1;
			for (int i = 0; i < 10; ++i)
			{
				h1.insert(i);
			}
			lutest(h1.size() == 10);
			auto h2 = h1;
			lutest(h2.size() == 10);
			h2.insert(12);
			lutest(h1.size() == 10);
			lutest(h2.size() == 11);
			h1.swap(h2);
			lutest(h1.size() == 11);
			lutest(h2.size() == 10);
		}

		{
			// clone()
			// swap()
			UnorderedMultiSet<int> h1;
			for (int i = 0; i < 10; ++i)
			{
				h1.insert(i);
			}
			lutest(h1.size() == 10);
			auto h2 = h1;
			lutest(h2.size() == 10);
			h2.insert(12);
			lutest(h1.size() == 10);
			lutest(h2.size() == 11);
			h1.swap(h2);
			lutest(h1.size() == 11);
			lutest(h2.size() == 10);
		}

		{
			// node_type extract()
			// insert(node_type&&)

			UnorderedSet<int> h1;
			UnorderedSet<int> h2;
			h1.insert(1);
			lutest(h1.size() == 1);
			auto node = h1.extract(h1.begin());
			lutest(h1.size() == 0);
			h2.insert(move(node));
			lutest(h2.size() == 1);
			lutest(h2.find(1) != h2.end());
		}

		{
			// node_type extract()
			// insert(node_type&&)

			UnorderedMultiSet<int> h1;
			UnorderedMultiSet<int> h2;
			h1.insert(1);
			lutest(h1.size() == 1);
			auto node = h1.extract(h1.begin());
			lutest(h1.size() == 0);
			h2.insert(move(node));
			lutest(h2.size() == 1);
			lutest(h2.find(1) != h2.end());
		}

		{
			// UnorderedMap, UnorderedSet
			// Pair<iterator, bool> insert(const value_type& value)
			// Pair<iterator, bool> insert(value_type&& value)
			// UnorderedMultiMap, UnorderedMultiSet
			// iterator insert(const value_type& value)
			// iterator insert(value_type&& value)
			UnorderedMap<int, int> h1;
			UnorderedMultiMap<int, int> h2;
			auto r1 = h1.insert(make_pair(3, 3));
			auto r2 = h1.insert(make_pair(3, 4));
			h2.insert(make_pair(3, 3));
			h2.insert(make_pair(3, 4));
			lutest(r1.second == true);
			lutest(r2.second == false);
			lutest(h1.size() == 1);
			lutest(h2.size() == 2);

			// Erase with repeat key.
			auto r3 = h1.erase(3);
			auto r4 = h2.erase(3);
			lutest(r3 == 1);
			lutest(r4 == 2);
			lutest(h1.size() == 0);
			lutest(h2.size() == 0);
		}

		{
			// Pair<iterator, iterator> equal_range(const key_type& key)
			// Pair<const_iterator, const_iterator> equal_range(const key_type& key) const
			// usize count(const key_type& key) const
			UnorderedMap<int, int> h1;
			UnorderedMultiMap<int, int> h2;
			h1.insert(make_pair(3, 3));
			h1.insert(make_pair(3, 4));
			h2.insert(make_pair(3, 3));
			h2.insert(make_pair(3, 4));

			auto range1 = h1.equal_range(3);
			auto range2 = h2.equal_range(3);
			auto c1 = h1.count(3);
			auto c2 = h2.count(3);

			lutest(range1.first->second == 3);
			++range1.first;
			lutest(range1.first == range1.second);
			lutest(range2.first->second == 3);
			++range2.first;
			lutest(range2.first->second == 4);
			++range2.first;
			lutest(range2.first == range2.second);
			lutest(c1 == 1);
			lutest(c2 == 2);
		}
		TestObject::reset();
		{
			// UnorderedMap, UnorderedSet
			// Pair<iterator, bool> emplace(_Args... args)
			// UnorderedMultiMap, UnorderedMultiSet
			// iterator emplace(_Args... args)

			UnorderedMap<int, TestObject> h1;
			UnorderedMultiMap<int, TestObject> h2;
			h1.emplace(3, TestObject(5, true));
			h1.emplace(3, TestObject(7, true));
			h2.emplace(3, TestObject(9, true));
			h2.emplace(3, TestObject(11, true));
			lutest(h1.size() == 1);
			lutest(h2.size() == 2);
		}
		lutest(TestObject::is_clear());
		TestObject::reset();
	}

	void robin_hood_hash_test()
	{
		{
			HashSet<int> h;
			lutest(h.empty());
			lutest(h.size() == 0);
			//! When the set is constructed, it does not allocate
			//! any dynamic memory.
			lutest(h.buffer_size() == 0);
			h.max_load_factor(1.0f);

			for (int i = 0; i < 100; ++i)
			{
				h.insert(i);
			}
			lutest(h.size() == 100);
			h.shrink_to_fit();
			lutest(h.buffer_size() == 100);

			h.clear();
			lutest(h.empty());
			lutest(h.size() == 0);
			//! When the set gets cleared, it retains the buffer.
			lutest(h.buffer_size() == 100);
			//! The bucket can be freed by calling shrink_to_fit.
			h.shrink_to_fit();
			lutest(h.buffer_size() == 0);
		}

		{
			// HashMap
			// size_type          size() const
			// bool               empty() const
			// iterator			  insert(const value_type& value);
			// iterator			  insert(value_type&& value);
			// iterator           find(const key_type& k);
			// const_iterator     find(const key_type& k) const;

			HashMap<int, int> h;
			constexpr size_t count = 10000;
			lutest(h.empty());
			lutest(h.size() == 0);
			for (int i = 0; i < count; ++i)
			{
				h.insert(make_pair(i, i + 1));
			}
			lutest(!h.empty());
			lutest(h.size() == count);
			for (int i = 0; i < count; ++i)
			{
				auto iter = h.find(i);
				lutest(iter != h.end());
				lutest(iter->second == i + 1);
			}
			for (int i = 0; i < count * 2; ++i)
			{
				auto iter = h.find(i);
				if (i < count)
				{
					lutest(iter != h.end());
				}
				else
				{
					lutest(iter == h.end());
				}
			}

			// iterator       begin();
			// const_iterator begin() const;
			// iterator       end();
			// const_iterator end() const;
			int* const intarr = (int*)memalloc(count * sizeof(int));
			memset(intarr, 0, count * sizeof(int));
			int n_count = 0;
			for (auto iter = h.begin(); iter != h.end(); ++iter, ++n_count)
			{
				int i = iter->first;
				lutest((i >= 0) && (i < count) && (intarr[i] == 0));
				intarr[i] = 1;
			}
			lutest(n_count == count);
			memfree(intarr);
		}
		{
			// Ctors and dtor.
			HashSet<int> s1;
			lutest(s1.size() == 0);
			lutest(s1.capacity() == 0);
			for (int i = 0; i < 10; ++i)
			{
				s1.insert(i);
			}
			lutest(s1.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s1.find(i) != s1.end());
			}
			// Copy ctor.
			HashSet<int> s2(s1);
			lutest(s2.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s2.find(i) != s2.end());
			}
			// Move ctor.
			HashSet<int> s3(move(s1));
			lutest(s3.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s3.find(i) != s3.end());
			}
			lutest(s1.size() == 0);
			// Copy assign
			s1 = s2;
			lutest(s1.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s1.find(i) != s1.end());
			}
			lutest(s2.size() == 10);
			// Move assig.
			s3.clear();
			s3 = move(s2);
			lutest(s3.size() == 10);
			for (int i = 0; i < 10; ++i)
			{
				lutest(s3.find(i) != s3.end());
			}
			lutest(s2.size() == 0);
		}
		TestObject::reset();
		{
			// HashMap
			// iterator			insert(const value_type& value);
			// iterator			insert(value_type&& value);
			// iterator			erase(const_iterator iter);
			// usize			erase(const key_type& key);
			HashMap<int, TestObject> h;
			h.insert(make_pair(3, TestObject(4, true)));
			TestObject obj(5, false);
			h.insert(make_pair(4, obj));
			TestObject obj2(6, true);
			h.insert(make_pair(5, move(obj2)));
			auto iter = h.find(3);
			lutest(h.size() == 3);
			lutest(iter != h.end() && iter->second == TestObject(4, true));
			iter = h.find(4);
			lutest(iter != h.end() && iter->second == TestObject(5, true));
			iter = h.find(5);
			lutest(iter != h.end() && iter->second == TestObject(6, true));
			h.erase(iter);
			lutest(h.size() == 2);
			h.erase(4);
			lutest(h.size() == 1);
			iter = h.find(3);
			lutest(iter != h.end() && iter->second == TestObject(4, true));
		}
		lutest(TestObject::is_clear());
		TestObject::reset();
		{
			// size_type bucket_size(size_type n) const
			// float load_factor() const
			// float get_max_load_factor() const;
			// void  set_max_load_factor(float fMaxLoadFactor);
			// void rehash(size_type n);

			HashSet<int> h;
			f32 lf = h.load_factor();
			lutest(lf == 0.0f);
			h.max_load_factor(1.0f);
			f32 max_lf = h.max_load_factor();
			lutest(max_lf == (1.0f));
			h.rehash(10000);
			size_t n = h.buffer_size();
			lutest(n == 10000);
			for (int i = 0; i < 10000; ++i)
			{
				h.insert(i);	// This also tests for high loading.
			}
			size_t n2 = h.buffer_size();
			lutest(n2 == n);	// Verify no rehashing has occurred, due to our high load factor.
			for (int i = 0; i < 10000; ++i)
			{
				h.insert(i);	// This also tests for high loading.
			}
			size_t n3 = h.buffer_size();
			lutest(n3 == n);	// Verify no rehashing has occurred, because all second insertions are failed.
		}

		{
			// clone()
			// swap()
			HashSet<int> h1;
			for (int i = 0; i < 10; ++i)
			{
				h1.insert(i);
			}
			lutest(h1.size() == 10);
			auto h2 = h1;
			lutest(h2.size() == 10);
			h2.insert(12);
			lutest(h1.size() == 10);
			lutest(h2.size() == 11);
			h1.swap(h2);
			lutest(h1.size() == 11);
			lutest(h2.size() == 10);
		}

		{
			// HashMap, HashSet
			// Pair<iterator, bool> insert(const value_type& value)
			// Pair<iterator, bool> insert(value_type&& value)
			HashMap<int, int> h1;
			auto r1 = h1.insert(make_pair(3, 3));
			auto r2 = h1.insert(make_pair(3, 4));
			lutest(r1.second == true);
			lutest(r2.second == false);
			lutest(h1.size() == 1);

			// Erase with repeat key.
			auto r3 = h1.erase(3);
			lutest(r3 == 1);
			lutest(h1.size() == 0);
		}
		TestObject::reset();
		{
			// HashMap, HashSet
			// Pair<iterator, bool> emplace(_Args... args)

			HashMap<int, TestObject> h1;
			h1.emplace(3, TestObject(5, true));
			h1.emplace(3, TestObject(7, true));
			lutest(h1.size() == 1);
			auto iter = h1.find(3);
			lutest(iter != h1.end() && iter->second == TestObject(5, true));
		}
		lutest(TestObject::is_clear());
		TestObject::reset();

		{
			// Bug 20220504: robinhood_insert infinite loop when:
			// 1. Two values are hashed to the same slot.
			// 2. One value alives.
			// 3. Another value is inserted and removed many times, so that all empty slots are replaced by tombstones.
			// In such case, since old version of robinhood_insert never replaces tombstones when existing_dist == dist, 
			// the search algorithm cannot find one available slot to insert the second value. We change the algorithm so
			// that if the desired pos is a tombstone, the swapping still occurs.
			HashMap<int, int, round_10_hash> h1;
			h1.insert(make_pair(11, 1));
			for (int i = 0; i < 1000; ++i)
			{
				h1.insert(make_pair(1, 1));
				h1.erase(1);
			}
		}

		{
			// Bug 20220627: robinhood_insert returns wrong pos when the key is not inserted in order.

			Vector<u64> ids;
			HashMap<u64, u64> h;
			const usize num_ids = 500;
			for (u64 i = 0; i < num_ids; ++i)
			{
				u64 id;
				bool conflict;
				do
				{
					id = random_u64();
					conflict = false;
					for (u64 i : ids)
					{
						if (i == id)
						{
							conflict = true;
							break;
						}
					}

				} while (conflict);
				ids.push_back(id);
			}
			for (u64 i = 0; i < num_ids; ++i)
			{
				auto iter = h.insert(make_pair(ids[i], 0));
				iter.first->second = i;
			}
			for (u64 i = 0; i < num_ids; ++i)
			{
				auto iter = h.find(ids[i]);
				lutest(iter->second == i);
			}
		}
	}
}