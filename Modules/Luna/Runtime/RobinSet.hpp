/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RobinSet.hpp
* @author JXMaster
* @date 2026/6/3
*/
#pragma once
#include "Impl/RobinHoodHashTable.hpp"

namespace Luna
{
    //! @addtogroup RuntimeContainer
    //! @{

    //! An container that contains a set of unique objects using Robin Hood open-addressing hashing algorithm.
    template <
        typename _Kty,
        typename _Hash = hash<_Kty>,
        typename _KeyEqual = equal_to<_Kty>,
        typename _Alloc = Allocator>
    class RobinSet
    {
    public:
        using key_type = _Kty;
        using value_type = _Kty;
        using allocator_type = _Alloc;
        using hasher = _Hash;
        using key_equal = _KeyEqual;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = RobinHoodHashing::Iterator<value_type, false>;
        using const_iterator = RobinHoodHashing::Iterator<value_type, true>;
    private:
        using table_type = RobinHoodHashing::HashTable<key_type, value_type, Impl::SetExtractKey<key_type, value_type>, hasher, key_equal, allocator_type>;
        table_type m_base;
        RobinSet(table_type&& base) :
            m_base(move(base)) {}
    public:
        RobinSet() :
            m_base() {}
        RobinSet(const allocator_type& alloc) :
            m_base(alloc) {}
        RobinSet(const RobinSet& rhs) :
            m_base(rhs.m_base) {}
        RobinSet(const RobinSet& rhs, const allocator_type& alloc) :
            m_base(rhs.m_base, alloc) {}
        RobinSet(RobinSet&& rhs) :
            m_base(move(rhs.m_base)) {}
        RobinSet(RobinSet&& rhs, const allocator_type& alloc) :
            m_base(move(rhs.m_base), alloc) {}
        RobinSet& operator=(const RobinSet& rhs)
        {
            m_base = rhs.m_base;
            return *this;
        }
        RobinSet& operator=(RobinSet&& rhs)
        {
            m_base = move(rhs.m_base);
            return *this;
        }
        iterator begin()
        {
            return m_base.begin();
        }
        const_iterator begin() const
        {
            return m_base.begin();
        }
        const_iterator cbegin() const
        {
            return m_base.cbegin();
        }
        iterator end()
        {
            return m_base.end();
        }
        const_iterator end() const
        {
            return m_base.end();
        }
        const_iterator cend() const
        {
            return m_base.cend();
        }
        bool empty() const
        {
            return m_base.empty();
        }
        usize size() const
        {
            return m_base.size();
        }
        usize capacity() const
        {
            return m_base.capacity();
        }
        usize hash_table_size() const
        {
            return m_base.hash_table_size();
        }
        f32 load_factor() const
        {
            return m_base.load_factor();
        }
        f32 max_load_factor() const
        {
            return m_base.max_load_factor();
        }
        void max_load_factor(f32 ml)
        {
            m_base.max_load_factor(ml);
        }
        void clear()
        {
            m_base.clear();
        }
        void shrink_to_fit()
        {
            m_base.shrink_to_fit();
        }
        hasher hash_function() const
        {
            return m_base.hash_function();
        }
        key_equal key_eq() const
        {
            return m_base.key_eq();
        }
        void rehash(usize new_buckets_count)
        {
            m_base.rehash(new_buckets_count);
        }
        void reserve(usize new_cap)
        {
            m_base.reserve(new_cap);
        }
        iterator find(const key_type& key)
        {
            return m_base.find(key);
        }
        const_iterator find(const key_type& key) const
        {
            return m_base.find(key);
        }
        usize count(const key_type& key) const
        {
            return m_base.count(key);
        }
        bool contains(const key_type& key) const
        {
            return m_base.contains(key);
        }
        Pair<iterator, bool> insert(const value_type& value)
        {
            return m_base.insert(value);
        }
        Pair<iterator, bool> insert(value_type&& value)
        {
            return m_base.insert(move(value));
        }
        template <typename... _Args>
        Pair<iterator, bool> emplace(_Args&&... args)
        {
            return m_base.emplace(forward<_Args>(args)...);
        }
        iterator erase(const_iterator pos)
        {
            return m_base.erase(pos);
        }
        usize erase(const key_type& key)
        {
            return m_base.erase(key);
        }
        void swap(RobinSet& rhs)
        {
            RobinSet tmp(move(rhs));
            rhs = move(*this);
            *this = move(tmp);
        }
        allocator_type get_allocator() const
        {
            return m_base.get_allocator();
        }
    };

    //! @}
}
