/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SparseHashTable.hpp
* @author JXMaster
* @date 2026/6/3
* @brief A hash table implementation that stores elements in a sparse array and
* bucket chains by sparse-array index.
*/
#pragma once
#include "../Base.hpp"
#include "../Functional.hpp"
#include "../Algorithm.hpp"
#include "../Allocator.hpp"
#include "../MemoryUtils.hpp"
#include "HashTableBase.hpp"
#include <cmath> // for ceilf

namespace Luna
{
    namespace SparseHashing
    {
        constexpr usize EMPTY_SLOT = USIZE_MAX;
        constexpr usize INITIAL_BUFFER_SIZE = 16;
        constexpr f32 INITIAL_LOAD_FACTOR = 2.0f;

        inline usize bitset_size(usize capacity)
        {
            return (capacity + 7) / 8;
        }

        inline usize round_up_power_of_two(usize value)
        {
            if (value <= 1) return value;
            --value;
            for (usize shift = 1; shift < sizeof(usize) * 8; shift <<= 1)
            {
                value |= value >> shift;
            }
            return value + 1;
        }

        inline usize minimum_bucket_count(usize size, f32 max_load_factor)
        {
            if (!size) return 0;
            usize ret = (usize)ceilf((f32)size / max_load_factor);
            return round_up_power_of_two(max(ret, (usize)1));
        }

        template <typename _Ty, bool _Const>
        struct Iterator
        {
            using value_type = _Ty;
            using pointer = conditional_t<_Const, const value_type*, value_type*>;
            using reference = conditional_t<_Const, const value_type&, value_type&>;
            using iterator_category = forward_iterator_tag;

            pointer m_values;
            const u8* m_occupied;
            usize m_index;
            usize m_end;

            Iterator() :
                m_values(nullptr),
                m_occupied(nullptr),
                m_index(0),
                m_end(0) {}
            Iterator(pointer values, const u8* occupied, usize index, usize end) :
                m_values(values),
                m_occupied(occupied),
                m_index(index),
                m_end(end)
            {
                skip_empty();
            }
            Iterator(const Iterator<_Ty, false>& rhs) :
                m_values(rhs.m_values),
                m_occupied(rhs.m_occupied),
                m_index(rhs.m_index),
                m_end(rhs.m_end) {}
            reference operator*() const
            {
                return m_values[m_index];
            }
            pointer operator->() const
            {
                return m_values + m_index;
            }
            Iterator& operator++()
            {
                ++m_index;
                skip_empty();
                return *this;
            }
            Iterator operator++(int)
            {
                Iterator temp(*this);
                ++(*this);
                return temp;
            }
            bool operator==(const Iterator& rhs) const
            {
                return m_values == rhs.m_values && m_index == rhs.m_index;
            }
            bool operator!=(const Iterator& rhs) const
            {
                return !(*this == rhs);
            }
        private:
            void skip_empty()
            {
                while (m_index < m_end && !bit_test(m_occupied, m_index))
                {
                    ++m_index;
                }
            }
        };

        template <typename _Kty,
            typename _Vty,
            typename _ExtractKey,
            typename _Hash = hash<_Kty>,
            typename _KeyEqual = equal_to<_Kty>,
            typename _Alloc = Allocator>
        class HashTable
        {
        public:
            using key_type = _Kty;
            using value_type = _Vty;
            using allocator_type = _Alloc;
            using hasher = _Hash;
            using key_equal = _KeyEqual;
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using iterator = Iterator<value_type, false>;
            using const_iterator = Iterator<value_type, true>;
            using extract_key = _ExtractKey;

            // -------------------- Begin of ABI compatible part --------------------

            OptionalPair<allocator_type, value_type*> m_allocator_and_values;
            usize* m_hashes;
            usize* m_nexts;
            u8* m_occupied;
            usize* m_buckets;
            usize m_capacity;
            usize m_sparse_size;
            usize m_size;
            usize m_first_free;
            usize m_hole_count;
            usize m_bucket_count;
            f32 m_max_load_factor;

            // --------------------  End of ABI compatible part  --------------------

        private:
            template <typename _Ty>
            _Ty* allocate(usize n)
            {
                return m_allocator_and_values.first().template allocate<_Ty>(n);
            }
            template <typename _Ty>
            void deallocate(_Ty* ptr, usize n)
            {
                m_allocator_and_values.first().template deallocate<_Ty>(ptr, n);
            }
            value_type* values() const
            {
                return m_allocator_and_values.second();
            }
            void initialize_bucket_buffer(usize* buckets, usize count)
            {
                for (usize i = 0; i < count; ++i)
                {
                    buckets[i] = EMPTY_SLOT;
                }
            }
            void rebuild_bucket_chains(usize* buckets, usize bucket_count)
            {
                initialize_bucket_buffer(buckets, bucket_count);
                for (usize i = 0; i < m_sparse_size; ++i)
                {
                    if (bit_test(m_occupied, i))
                    {
                        usize bucket = m_hashes[i] % bucket_count;
                        m_nexts[i] = buckets[bucket];
                        buckets[bucket] = i;
                    }
                }
            }
            void free_value_storage()
            {
                if (values())
                {
                    deallocate<value_type>(values(), m_capacity);
                    m_allocator_and_values.second() = nullptr;
                }
                if (m_hashes)
                {
                    deallocate<usize>(m_hashes, m_capacity);
                    m_hashes = nullptr;
                }
                if (m_nexts)
                {
                    deallocate<usize>(m_nexts, m_capacity);
                    m_nexts = nullptr;
                }
                if (m_occupied)
                {
                    deallocate<u8>(m_occupied, bitset_size(m_capacity));
                    m_occupied = nullptr;
                }
            }
            void free_bucket_storage()
            {
                if (m_buckets)
                {
                    deallocate<usize>(m_buckets, m_bucket_count);
                    m_buckets = nullptr;
                }
            }
            void internal_reserve_values(usize new_capacity)
            {
                if (new_capacity <= m_capacity) return;
                value_type* new_values = allocate<value_type>(new_capacity);
                usize* new_hashes = allocate<usize>(new_capacity);
                usize* new_nexts = allocate<usize>(new_capacity);
                usize new_occupied_size = bitset_size(new_capacity);
                u8* new_occupied = allocate<u8>(new_occupied_size);
                memzero(new_occupied, new_occupied_size);
                if (values())
                {
                    memcpy(new_hashes, m_hashes, sizeof(usize) * m_capacity);
                    memcpy(new_nexts, m_nexts, sizeof(usize) * m_capacity);
                    for (usize i = 0; i < m_sparse_size; ++i)
                    {
                        if (bit_test(m_occupied, i))
                        {
                            copy_relocate(new_values + i, values() + i);
                            bit_set(new_occupied, i);
                        }
                    }
                    free_value_storage();
                }
                m_allocator_and_values.second() = new_values;
                m_hashes = new_hashes;
                m_nexts = new_nexts;
                m_occupied = new_occupied;
                m_capacity = new_capacity;
            }
            void internal_clear()
            {
                for (usize i = 0; i < m_sparse_size; ++i)
                {
                    if (bit_test(m_occupied, i))
                    {
                        destruct(values() + i);
                    }
                }
                if (m_occupied)
                {
                    memzero(m_occupied, bitset_size(m_capacity));
                }
                if (m_buckets)
                {
                    initialize_bucket_buffer(m_buckets, m_bucket_count);
                }
                m_sparse_size = 0;
                m_size = 0;
                m_first_free = EMPTY_SLOT;
                m_hole_count = 0;
            }
            void internal_clear_and_free_table()
            {
                internal_clear();
                free_value_storage();
                free_bucket_storage();
                m_capacity = 0;
                m_bucket_count = 0;
            }
            void increment_reserve(usize new_cap)
            {
                if (new_cap > m_capacity)
                {
                    usize new_capacity = max(max(new_cap, m_capacity * 2), INITIAL_BUFFER_SIZE);
                    internal_reserve_values(new_capacity);
                }
                usize desired_bucket_count = minimum_bucket_count(new_cap, m_max_load_factor);
                if (desired_bucket_count > m_bucket_count)
                {
                    rehash(desired_bucket_count);
                }
            }
            usize hash_key(const key_type& key) const
            {
                return hasher()(key);
            }
            iterator internal_find(const key_type& key, usize h)
            {
                if (!m_bucket_count) return end();
                usize i = m_buckets[h % m_bucket_count];
                while (i != EMPTY_SLOT)
                {
                    if (m_hashes[i] == h && key_equal()(key, extract_key()(values()[i])))
                    {
                        return iterator(values(), m_occupied, i, m_sparse_size);
                    }
                    i = m_nexts[i];
                }
                return end();
            }
            const_iterator internal_find(const key_type& key, usize h) const
            {
                if (!m_bucket_count) return end();
                usize i = m_buckets[h % m_bucket_count];
                while (i != EMPTY_SLOT)
                {
                    if (m_hashes[i] == h && key_equal()(key, extract_key()(values()[i])))
                    {
                        return const_iterator(values(), m_occupied, i, m_sparse_size);
                    }
                    i = m_nexts[i];
                }
                return end();
            }
            usize allocate_sparse_slot()
            {
                if (m_first_free != EMPTY_SLOT)
                {
                    usize ret = m_first_free;
                    m_first_free = m_nexts[ret];
                    --m_hole_count;
                    bit_set(m_occupied, ret);
                    return ret;
                }
                luassert(m_sparse_size < m_capacity);
                usize ret = m_sparse_size++;
                bit_set(m_occupied, ret);
                return ret;
            }
            template <typename... _Args>
            iterator internal_insert(usize h, _Args&&... args)
            {
                increment_reserve(m_size + 1);
                usize pos = allocate_sparse_slot();
                direct_construct(values() + pos, forward<_Args>(args)...);
                m_hashes[pos] = h;
                usize bucket = h % m_bucket_count;
                m_nexts[pos] = m_buckets[bucket];
                m_buckets[bucket] = pos;
                ++m_size;
                return iterator(values(), m_occupied, pos, m_sparse_size);
            }
            void internal_copy_from(const HashTable& rhs)
            {
                m_max_load_factor = rhs.m_max_load_factor;
                if (rhs.m_size)
                {
                    reserve(rhs.m_size);
                    for (usize i = 0; i < rhs.m_sparse_size; ++i)
                    {
                        if (bit_test(rhs.m_occupied, i))
                        {
                            insert(rhs.values()[i]);
                        }
                    }
                }
            }
            void internal_move_from_with_different_allocator(HashTable& rhs)
            {
                m_max_load_factor = rhs.m_max_load_factor;
                if (rhs.m_size)
                {
                    reserve(rhs.m_size);
                    for (usize i = 0; i < rhs.m_sparse_size; ++i)
                    {
                        if (bit_test(rhs.m_occupied, i))
                        {
                            insert(move(rhs.values()[i]));
                        }
                    }
                    rhs.clear();
                }
            }

        public:
            bool empty() const
            {
                return m_size == 0;
            }
            usize size() const
            {
                return m_size;
            }
            usize capacity() const
            {
                return m_capacity;
            }
            usize hash_table_size() const
            {
                return m_bucket_count;
            }
            f32 load_factor() const
            {
                if (!m_bucket_count) return 0.0f;
                return (f32)m_size / (f32)m_bucket_count;
            }
            f32 max_load_factor() const
            {
                return m_max_load_factor;
            }
            void clear()
            {
                internal_clear();
            }
            void shrink_to_fit()
            {
                if (!m_size)
                {
                    internal_clear_and_free_table();
                    return;
                }
                value_type* old_values = values();
                usize* old_hashes = m_hashes;
                usize* old_nexts = m_nexts;
                u8* old_occupied = m_occupied;
                usize old_capacity = m_capacity;
                usize old_sparse_size = m_sparse_size;
                value_type* new_values = allocate<value_type>(m_size);
                usize* new_hashes = allocate<usize>(m_size);
                usize* new_nexts = allocate<usize>(m_size);
                u8* new_occupied = allocate<u8>(bitset_size(m_size));
                memzero(new_occupied, bitset_size(m_size));
                usize new_index = 0;
                for (usize i = 0; i < old_sparse_size; ++i)
                {
                    if (bit_test(old_occupied, i))
                    {
                        copy_relocate(new_values + new_index, old_values + i);
                        new_hashes[new_index] = old_hashes[i];
                        bit_set(new_occupied, new_index);
                        ++new_index;
                    }
                }
                if (old_values) deallocate<value_type>(old_values, old_capacity);
                if (old_hashes) deallocate<usize>(old_hashes, old_capacity);
                if (old_nexts) deallocate<usize>(old_nexts, old_capacity);
                if (old_occupied) deallocate<u8>(old_occupied, bitset_size(old_capacity));
                m_allocator_and_values.second() = new_values;
                m_hashes = new_hashes;
                m_nexts = new_nexts;
                m_occupied = new_occupied;
                m_capacity = m_size;
                m_sparse_size = m_size;
                m_first_free = EMPTY_SLOT;
                m_hole_count = 0;
                usize desired_bucket_count = minimum_bucket_count(m_size, m_max_load_factor);
                if (desired_bucket_count == m_bucket_count)
                {
                    rebuild_bucket_chains(m_buckets, m_bucket_count);
                }
                else
                {
                    rehash(desired_bucket_count);
                }
            }
            //! Sorts elements by sparse-array iteration order and rebuilds hash bucket chains.
            //! @remark This invalidates all iterators, references and pointers to elements.
            template <typename _Compare>
            void sort(_Compare comp)
            {
                if (!m_size || (m_size == 1 && !m_hole_count))
                {
                    return;
                }
                usize* order = allocate<usize>(m_size);
                usize order_size = 0;
                for (usize i = 0; i < m_sparse_size; ++i)
                {
                    if (bit_test(m_occupied, i))
                    {
                        order[order_size++] = i;
                    }
                }
                luassert(order_size == m_size);
                if (m_size > 1)
                {
                    value_type* old_values = values();
                    Luna::sort(order, order + m_size, [old_values, comp](usize lhs, usize rhs) {
                        return comp(old_values[lhs], old_values[rhs]);
                    });
                }

                value_type* old_values = values();
                usize* old_hashes = m_hashes;
                usize* old_nexts = m_nexts;
                u8* old_occupied = m_occupied;
                usize old_capacity = m_capacity;
                value_type* new_values = allocate<value_type>(m_capacity);
                usize* new_hashes = allocate<usize>(m_capacity);
                usize* new_nexts = allocate<usize>(m_capacity);
                u8* new_occupied = allocate<u8>(bitset_size(m_capacity));
                memzero(new_occupied, bitset_size(m_capacity));
                for (usize i = 0; i < m_size; ++i)
                {
                    usize old_index = order[i];
                    copy_relocate(new_values + i, old_values + old_index);
                    new_hashes[i] = old_hashes[old_index];
                    bit_set(new_occupied, i);
                }
                deallocate<value_type>(old_values, old_capacity);
                deallocate<usize>(old_hashes, old_capacity);
                deallocate<usize>(old_nexts, old_capacity);
                deallocate<u8>(old_occupied, bitset_size(old_capacity));
                deallocate<usize>(order, m_size);

                m_allocator_and_values.second() = new_values;
                m_hashes = new_hashes;
                m_nexts = new_nexts;
                m_occupied = new_occupied;
                m_sparse_size = m_size;
                m_first_free = EMPTY_SLOT;
                m_hole_count = 0;
                if (m_bucket_count)
                {
                    rebuild_bucket_chains(m_buckets, m_bucket_count);
                }
                else
                {
                    rehash(minimum_bucket_count(m_size, m_max_load_factor));
                }
            }
            //! Sorts elements in non-descending order.
            //! @remark This invalidates all iterators, references and pointers to elements.
            void sort()
            {
                sort(less<value_type>());
            }
            hasher hash_function() const
            {
                return hasher();
            }
            key_equal key_eq() const
            {
                return key_equal();
            }
            void rehash(usize new_bucket_count)
            {
                new_bucket_count = round_up_power_of_two(max(new_bucket_count, minimum_bucket_count(m_size, m_max_load_factor)));
                if (new_bucket_count == m_bucket_count)
                {
                    return;
                }
                usize* buckets = nullptr;
                if (new_bucket_count)
                {
                    buckets = allocate<usize>(new_bucket_count);
                    rebuild_bucket_chains(buckets, new_bucket_count);
                }
                free_bucket_storage();
                m_buckets = buckets;
                m_bucket_count = new_bucket_count;
            }
            void reserve(usize new_cap)
            {
                if (new_cap > m_capacity)
                {
                    internal_reserve_values(new_cap);
                }
                usize desired_bucket_count = minimum_bucket_count(new_cap, m_max_load_factor);
                if (desired_bucket_count > m_bucket_count)
                {
                    rehash(desired_bucket_count);
                }
            }
            void max_load_factor(f32 ml)
            {
                lucheck(ml > 0.0f);
                m_max_load_factor = ml;
                if (load_factor() > m_max_load_factor)
                {
                    rehash(0);
                }
            }
            HashTable() :
                m_allocator_and_values(allocator_type(), nullptr),
                m_hashes(nullptr),
                m_nexts(nullptr),
                m_occupied(nullptr),
                m_buckets(nullptr),
                m_capacity(0),
                m_sparse_size(0),
                m_size(0),
                m_first_free(EMPTY_SLOT),
                m_hole_count(0),
                m_bucket_count(0),
                m_max_load_factor(INITIAL_LOAD_FACTOR) {}
            HashTable(const allocator_type& alloc) :
                m_allocator_and_values(alloc, nullptr),
                m_hashes(nullptr),
                m_nexts(nullptr),
                m_occupied(nullptr),
                m_buckets(nullptr),
                m_capacity(0),
                m_sparse_size(0),
                m_size(0),
                m_first_free(EMPTY_SLOT),
                m_hole_count(0),
                m_bucket_count(0),
                m_max_load_factor(INITIAL_LOAD_FACTOR) {}
            HashTable(const HashTable& rhs) :
                HashTable()
            {
                internal_copy_from(rhs);
            }
            HashTable(const HashTable& rhs, const allocator_type& alloc) :
                HashTable(alloc)
            {
                internal_copy_from(rhs);
            }
            HashTable(HashTable&& rhs) :
                m_allocator_and_values(move(rhs.m_allocator_and_values.first()), rhs.values()),
                m_hashes(rhs.m_hashes),
                m_nexts(rhs.m_nexts),
                m_occupied(rhs.m_occupied),
                m_buckets(rhs.m_buckets),
                m_capacity(rhs.m_capacity),
                m_sparse_size(rhs.m_sparse_size),
                m_size(rhs.m_size),
                m_first_free(rhs.m_first_free),
                m_hole_count(rhs.m_hole_count),
                m_bucket_count(rhs.m_bucket_count),
                m_max_load_factor(rhs.m_max_load_factor)
            {
                rhs.m_allocator_and_values.second() = nullptr;
                rhs.m_hashes = nullptr;
                rhs.m_nexts = nullptr;
                rhs.m_occupied = nullptr;
                rhs.m_buckets = nullptr;
                rhs.m_capacity = 0;
                rhs.m_sparse_size = 0;
                rhs.m_size = 0;
                rhs.m_first_free = EMPTY_SLOT;
                rhs.m_hole_count = 0;
                rhs.m_bucket_count = 0;
            }
            HashTable(HashTable&& rhs, const allocator_type& alloc) :
                HashTable(alloc)
            {
                if (m_allocator_and_values.first() == rhs.m_allocator_and_values.first())
                {
                    m_allocator_and_values.second() = rhs.values();
                    m_hashes = rhs.m_hashes;
                    m_nexts = rhs.m_nexts;
                    m_occupied = rhs.m_occupied;
                    m_buckets = rhs.m_buckets;
                    m_capacity = rhs.m_capacity;
                    m_sparse_size = rhs.m_sparse_size;
                    m_size = rhs.m_size;
                    m_first_free = rhs.m_first_free;
                    m_hole_count = rhs.m_hole_count;
                    m_bucket_count = rhs.m_bucket_count;
                    m_max_load_factor = rhs.m_max_load_factor;
                    rhs.m_allocator_and_values.second() = nullptr;
                    rhs.m_hashes = nullptr;
                    rhs.m_nexts = nullptr;
                    rhs.m_occupied = nullptr;
                    rhs.m_buckets = nullptr;
                    rhs.m_capacity = 0;
                    rhs.m_sparse_size = 0;
                    rhs.m_size = 0;
                    rhs.m_first_free = EMPTY_SLOT;
                    rhs.m_hole_count = 0;
                    rhs.m_bucket_count = 0;
                }
                else
                {
                    internal_move_from_with_different_allocator(rhs);
                }
            }
            HashTable& operator=(const HashTable& rhs)
            {
                if (this != &rhs)
                {
                    internal_clear_and_free_table();
                    internal_copy_from(rhs);
                }
                return *this;
            }
            HashTable& operator=(HashTable&& rhs)
            {
                if (this != &rhs)
                {
                    internal_clear_and_free_table();
                    if (m_allocator_and_values.first() == rhs.m_allocator_and_values.first())
                    {
                        m_allocator_and_values.second() = rhs.values();
                        m_hashes = rhs.m_hashes;
                        m_nexts = rhs.m_nexts;
                        m_occupied = rhs.m_occupied;
                        m_buckets = rhs.m_buckets;
                        m_capacity = rhs.m_capacity;
                        m_sparse_size = rhs.m_sparse_size;
                        m_size = rhs.m_size;
                        m_first_free = rhs.m_first_free;
                        m_hole_count = rhs.m_hole_count;
                        m_bucket_count = rhs.m_bucket_count;
                        m_max_load_factor = rhs.m_max_load_factor;
                        rhs.m_allocator_and_values.second() = nullptr;
                        rhs.m_hashes = nullptr;
                        rhs.m_nexts = nullptr;
                        rhs.m_occupied = nullptr;
                        rhs.m_buckets = nullptr;
                        rhs.m_capacity = 0;
                        rhs.m_sparse_size = 0;
                        rhs.m_size = 0;
                        rhs.m_first_free = EMPTY_SLOT;
                        rhs.m_hole_count = 0;
                        rhs.m_bucket_count = 0;
                    }
                    else
                    {
                        internal_move_from_with_different_allocator(rhs);
                    }
                }
                return *this;
            }
            ~HashTable()
            {
                internal_clear_and_free_table();
            }
            iterator begin()
            {
                return iterator(values(), m_occupied, 0, m_sparse_size);
            }
            const_iterator begin() const
            {
                return const_iterator(values(), m_occupied, 0, m_sparse_size);
            }
            const_iterator cbegin() const
            {
                return begin();
            }
            iterator end()
            {
                return iterator(values(), m_occupied, m_sparse_size, m_sparse_size);
            }
            const_iterator end() const
            {
                return const_iterator(values(), m_occupied, m_sparse_size, m_sparse_size);
            }
            const_iterator cend() const
            {
                return end();
            }
            iterator find(const key_type& key)
            {
                usize h = hash_key(key);
                return internal_find(key, h);
            }
            const_iterator find(const key_type& key) const
            {
                usize h = hash_key(key);
                return internal_find(key, h);
            }
            bool contains(const key_type& key) const
            {
                auto iter = find(key);
                return iter != end();
            }
            usize count(const key_type& key) const
            {
                return contains(key) ? 1 : 0;
            }
            Pair<iterator, bool> insert(const value_type& value)
            {
                usize h = hash_key(extract_key()(value));
                auto iter = internal_find(extract_key()(value), h);
                if (iter != end())
                {
                    return make_pair(iter, false);
                }
                return make_pair(internal_insert(h, value), true);
            }
            Pair<iterator, bool> insert(value_type&& value)
            {
                usize h = hash_key(extract_key()(value));
                auto iter = internal_find(extract_key()(value), h);
                if (iter != end())
                {
                    return make_pair(iter, false);
                }
                return make_pair(internal_insert(h, move(value)), true);
            }
            Pair<iterator, bool> insert_or_assign(const value_type& value)
            {
                usize h = hash_key(extract_key()(value));
                auto iter = internal_find(extract_key()(value), h);
                if (iter != end())
                {
                    (*iter) = value;
                    return make_pair(iter, false);
                }
                return make_pair(internal_insert(h, value), true);
            }
            Pair<iterator, bool> insert_or_assign(value_type&& value)
            {
                usize h = hash_key(extract_key()(value));
                auto iter = internal_find(extract_key()(value), h);
                if (iter != end())
                {
                    (*iter) = move(value);
                    return make_pair(iter, false);
                }
                return make_pair(internal_insert(h, move(value)), true);
            }
            template <typename _M>
            Pair<iterator, bool> insert_or_assign(const key_type& key, _M&& value)
            {
                usize h = hash_key(key);
                auto iter = internal_find(key, h);
                if (iter != end())
                {
                    iter->second = forward<_M>(value);
                    return make_pair(iter, false);
                }
                return make_pair(internal_insert(h, value_type(key, forward<_M>(value))), true);
            }
            template <typename _M>
            Pair<iterator, bool> insert_or_assign(key_type&& key, _M&& value)
            {
                usize h = hash_key(key);
                auto iter = internal_find(key, h);
                if (iter != end())
                {
                    iter->second = forward<_M>(value);
                    return make_pair(iter, false);
                }
                return make_pair(internal_insert(h, value_type(move(key), forward<_M>(value))), true);
            }
            template <typename... _Args>
            Pair<iterator, bool> emplace(_Args&&... args)
            {
                Unconstructed<value_type> value;
                value.construct(forward<_Args>(args)...);
                usize h = hash_key(extract_key()(value.get()));
                iterator iter = internal_find(extract_key()(value.get()), h);
                if (iter != end())
                {
                    value.destruct();
                    return make_pair(iter, false);
                }
                increment_reserve(m_size + 1);
                usize pos = allocate_sparse_slot();
                copy_relocate(values() + pos, &(value.get()));
                m_hashes[pos] = h;
                usize bucket = h % m_bucket_count;
                m_nexts[pos] = m_buckets[bucket];
                m_buckets[bucket] = pos;
                ++m_size;
                return make_pair(iterator(values(), m_occupied, pos, m_sparse_size), true);
            }
            iterator erase(const_iterator pos)
            {
                usize index = pos.m_index;
                luassert(index < m_sparse_size && bit_test(m_occupied, index));
                usize bucket = m_hashes[index] % m_bucket_count;
                usize* link = m_buckets + bucket;
                while (*link != index)
                {
                    link = m_nexts + *link;
                }
                *link = m_nexts[index];
                destruct(values() + index);
                bit_reset(m_occupied, index);
                m_nexts[index] = m_first_free;
                m_first_free = index;
                ++m_hole_count;
                --m_size;
                return iterator(values(), m_occupied, index + 1, m_sparse_size);
            }
            usize erase(const key_type& key)
            {
                auto iter = find(key);
                if (iter != end())
                {
                    erase(iter);
                    return 1;
                }
                return 0;
            }
            allocator_type get_allocator() const
            {
                return m_allocator_and_values.first();
            }
        };
    }
}
