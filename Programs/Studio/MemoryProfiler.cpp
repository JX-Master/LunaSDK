/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MemoryProfiler.cpp
* @author JXMaster
* @date 2023/11/3
*/
#include "MemoryProfiler.hpp"

namespace Luna
{
    namespace
    {
        String memory_size_text(usize size)
        {
            String ret;
            if(size >= 1_mb)
            {
                strprintf(ret, "%.2fMB", (f64)size / (f64)1_mb);
            }
            else if(size >= 1_kb)
            {
                strprintf(ret, "%.2fKB", (f64)size / (f64)1_kb);
            }
            else
            {
                strprintf(ret, "%llu", (u64)size);
            }
            return ret;
        }
    }

    void MemoryProfiler::on_allocate(void* ptr, usize size)
    {
        MemoryBlockInfo info;
        info.size = size;
        LockGuard guard(m_lock);
        if(m_snapshoting) return;
        m_memory_blocks.insert(make_pair((usize)ptr, info));
    }
    void MemoryProfiler::on_deallocate(void* ptr)
    {
        LockGuard guard(m_lock);
        if(m_snapshoting) return;
        m_memory_blocks.erase((usize)ptr);
    }
    void MemoryProfiler::on_set_memory_name(void* ptr, const c8* name)
    {
        Name n = name;
        LockGuard guard(m_lock);
        auto iter = m_memory_blocks.find((usize)ptr);
        if(iter == m_memory_blocks.end()) return;
        iter->second.name = move(n);
    }
    void MemoryProfiler::on_set_memory_type(void* ptr, const c8* type)
    {
        Name t = type;
        LockGuard guard(m_lock);
        auto iter = m_memory_blocks.find((usize)ptr);
        if(iter == m_memory_blocks.end()) return;
        iter->second.type = move(t);
    }
    void MemoryProfiler::on_set_memory_domain(void* ptr, const c8* domain)
    {
        Name d = domain;
        LockGuard guard(m_lock);
        auto iter = m_memory_blocks.find((usize)ptr);
        if(iter == m_memory_blocks.end()) return;
        iter->second.domain = move(d);
    }
    void MemoryProfiler::render()
    {
        LockGuard guard(m_lock);
        m_snapshoting = true;
        // Take snapshot.
        auto blocks = m_memory_blocks;
        m_snapshoting = false;
        guard.unlock();
        // Domain -> Type -> Size/Count
        HashMap<Name, HashMap<Name, Pair<usize, usize>>> heaps;
        Name _default = "Default";
        Name _unknown = "[Unknown]";
        for(auto& b : blocks)
        {
            auto domain = b.second.domain;
            if(!domain) domain = _default;
            auto iter = heaps.find(domain);
            if(iter == heaps.end())
            {
                iter = heaps.insert(make_pair(domain, HashMap<Name, Pair<usize, usize>>())).first;
            }
            auto type = b.second.type;
            if(!type) type = _unknown;
            auto iter2 = iter->second.find(type);
            if(iter2 == iter->second.end())
            {
                iter2 = iter->second.insert(make_pair(type, make_pair(0, 0))).first;
            }
            iter2->second.first += b.second.size;
            iter2->second.second += 1;
        }
        GUI::BeginWindow("Memory Usages", GUI::GUISize::fixed(500.0f, 1000.0f));
        GUI::BeginScrollView("Memory Usage List", GUI::GUISize::fixed(484.0f, 940.0f));
        for(auto& h : heaps)
        {
            GUI::GUIItemHandle heap_header = GUI::CollapsingHeader(h.first.c_str());
            if(GUI::GetItemState(heap_header, GUI::GUIState::open()))
            {
                GUI::GUITableDesc table;
                table.columns = 3;
                table.style.padding = GUI::GUIEdgeInsets::xy(8.0f, 4.0f);
                table.style.border_size = 1.0f;
                table.style.background_mode = GUI::GUITableBackgroundMode::alternate_rows;
                table.style.background_color = Float4U(0.08f, 0.10f, 0.12f, 0.72f);
                table.style.alternate_background_color = Float4U(0.12f, 0.14f, 0.17f, 0.72f);
                table.style.row_separators = true;
                table.style.column_separators = true;
                table.style.resize_fixed_columns = true;
                table.column_sizes.push_back(GUI::GUITableTrackSize::fixed(240.0f));
                table.column_sizes.push_back(GUI::GUITableTrackSize::fixed(110.0f));
                table.column_sizes.push_back(GUI::GUITableTrackSize::fixed(120.0f));
                GUI::BeginTableLayout(h.first.c_str(), table);
                {
                    GUI::Text("Type");
                    GUI::Text("Size");
                    GUI::Text("Allocation Count");
                    for(auto& a : h.second)
                    {
                        String count_text;
                        strprintf(count_text, "%llu", (u64)a.second.second);
                        GUI::Text(a.first.c_str());
                        GUI::Text(memory_size_text(a.second.first).c_str());
                        GUI::Text(count_text.c_str());
                    }
                    GUI::EndTableLayout();
                }
            }
        }
        GUI::EndScrollView();
        GUI::EndWindow();
    }
    void MemoryProfilerCallback::operator()(const ProfilerEvent& event)
    {
        switch(event.id)
        {
            case ProfilerEventId::MEMORY_ALLOCATE:
            {
                auto data = (ProfilerEventData::MemoryAllocate*)event.data;
                m_profiler->on_allocate(data->ptr, data->size);
            }
            break;
            case ProfilerEventId::MEMORY_DEALLOCATE:
            {
                auto data = (ProfilerEventData::MemoryDeallocate*)event.data;
                m_profiler->on_deallocate(data->ptr);
            }
            break;
            case ProfilerEventId::SET_MEMORY_NAME:
            {
                auto data = (ProfilerEventData::SetMemoryName*)event.data;
                m_profiler->on_set_memory_name(data->ptr, data->name);
            }
            break;
            case ProfilerEventId::SET_MEMORY_TYPE:
            {
                auto data = (ProfilerEventData::SetMemoryType*)event.data;
                m_profiler->on_set_memory_type(data->ptr, data->type);
            }
            break;
            case ProfilerEventId::SET_MEMORY_DOMAIN:
            {
                auto data = (ProfilerEventData::SetMemoryDomain*)event.data;
                m_profiler->on_set_memory_domain(data->ptr, data->domain);
            }
            break;
            default: break;
        }
    }
}
