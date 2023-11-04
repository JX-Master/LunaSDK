/*!
* This file is a portion of Luna SDK.
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
    void MemoryProfiler::on_allocate(void* ptr, usize size)
    {
        MemoryBlockInfo info;
        info.size = size;
        LockGuard guard(m_lock);
        if(m_snapshoting) return;
        m_memory_blocks.insert(make_pair((usize)ptr, info));
    }
    void MemoryProfiler::on_reallocate(void* ptr, void* new_ptr, usize new_size)
    {
        LockGuard guard(m_lock);
        if(m_snapshoting) return;
        auto iter = m_memory_blocks.find((usize)ptr);
        if(iter == m_memory_blocks.end()) return;
        MemoryBlockInfo info = move(iter->second);
        info.size = new_size;
        m_memory_blocks.erase(iter);
        m_memory_blocks.insert(make_pair((usize)ptr, move(info)));
    }
    void MemoryProfiler::on_deallocate(void* ptr)
    {
        LockGuard guard(m_lock);
        if(m_snapshoting) return;
        m_memory_blocks.erase((usize)ptr);
    }
    void MemoryProfiler::on_set_memory_name(void* ptr, const Name& name)
    {
        LockGuard guard(m_lock);
        auto iter = m_memory_blocks.find((usize)ptr);
        if(iter == m_memory_blocks.end()) return;
        iter->second.name = name;
    }
    void MemoryProfiler::on_set_memory_type(void* ptr, const Name& type)
    {
        LockGuard guard(m_lock);
        auto iter = m_memory_blocks.find((usize)ptr);
        if(iter == m_memory_blocks.end()) return;
        iter->second.type = type;
    }
    void MemoryProfiler::on_set_memory_domain(void* ptr, const Name&domain)
    {
        LockGuard guard(m_lock);
        auto iter = m_memory_blocks.find((usize)ptr);
        if(iter == m_memory_blocks.end()) return;
        iter->second.domain = domain;
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
        ImGui::SetNextWindowSize({ 500.0f, 1000.0f }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Memory Usages", nullptr, ImGuiWindowFlags_NoCollapse);
        for(auto& h : heaps)
        {
            if(ImGui::CollapsingHeader(h.first.c_str()))
            {
                if(ImGui::BeginTable(h.first.c_str(), 3))
                {
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Size");
                    ImGui::TableSetupColumn("Allocation Count");
                    ImGui::TableHeadersRow();
                    for(auto& a : h.second)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", a.first.c_str());
                        ImGui::TableSetColumnIndex(1);
                        if(a.second.first >= 1_mb)
                        {
                            ImGui::Text("%.2fMB", (f64)a.second.first / (f64)1_mb);
                        }
                        else if(a.second.first >= 1_kb)
                        {
                            ImGui::Text("%.2fKB", (f64)a.second.first / (f64)1_kb);
                        }
                        else
                        {
                            ImGui::Text("%llu", (u64)a.second.first);
                        }
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%llu", (u64)a.second.second);
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
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
            case ProfilerEventId::MEMORY_REALLOCATE:
            {
                auto data = (ProfilerEventData::MemoryReallocate*)event.data;
                m_profiler->on_reallocate(data->ptr, data->new_ptr, data->new_size);
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