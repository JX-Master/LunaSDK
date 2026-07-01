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
#include <Luna/GUI/Editor.hpp>

namespace Luna
{
    namespace
    {
        using MemoryHeapSnapshot = HashMap<Name, HashMap<Name, Pair<usize, usize>>>;

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

        MemoryHeapSnapshot make_memory_heap_snapshot(const HashMap<usize, MemoryProfiler::MemoryBlockInfo>& blocks)
        {
            MemoryHeapSnapshot heaps;
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
            return heaps;
        }

        GUICore::LayoutInput fixed_height(f32 height)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUICore::TableTrackDesc table_column(f32 width)
        {
            GUICore::TableTrackDesc track;
            track.kind = GUICore::TableTrackSizeKind::pixels;
            track.value = width;
            return track;
        }

        GUICore::TableTrackDesc table_row(f32 height)
        {
            GUICore::TableTrackDesc track;
            track.kind = GUICore::TableTrackSizeKind::pixels;
            track.value = height;
            return track;
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
    void MemoryProfiler::render(GUICore::IContext* context, const GUICore::LayoutInput& layout)
    {
        luassert(context);
        LockGuard guard(m_lock);
        m_snapshoting = true;
        auto blocks = m_memory_blocks;
        m_snapshoting = false;
        guard.unlock();

        MemoryHeapSnapshot heaps = make_memory_heap_snapshot(blocks);
        GUICore::id_t scope = context->make_id("memory_profiler");
        context->push_data_scope(scope);
        GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("root"), "Memory Usages", layout);
        GUI::text(context, context->make_id("title"), "Memory Usages", fixed_height(28.0f));
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, context->make_id("scroll"), "Memory Usage List", fixed_height(940.0f));
        GUICore::ElementHandle content = GUI::begin_v_layout(context, context->make_id("content"), "Memory Usage Content");
        for(auto& h : heaps)
        {
            context->push_data_scope(context->make_id(h.first.c_str()));
            if(GUI::collapsing_header(context, context->make_id("heap"), h.first.c_str()))
            {
                GUICore::LayoutInput table_layout;
                table_layout.width.kind = GUICore::SizeKind::fixed;
                table_layout.width.value = 484.0f;
                table_layout.height.kind = GUICore::SizeKind::fit;
                table_layout.margin = Float4U(0.0f, 4.0f, 0.0f, 8.0f);
                GUICore::ElementHandle table = GUI::begin_table_layout(context, context->make_id("table"), h.first.c_str(), table_layout);
                GUICore::TableTrackDesc columns[3] = {
                    table_column(240.0f),
                    table_column(110.0f),
                    table_column(120.0f)
                };
                GUI::set_table_columns(context, Span<const GUICore::TableTrackDesc>(columns, 3));
                GUI::set_table_gap(context, Float2U(1.0f, 1.0f));
                GUI::set_table_cell_padding(context, Float4U(8.0f, 4.0f, 8.0f, 4.0f));
                GUICore::TableTrackDesc row = table_row(28.0f);
                if(GUI::begin_table_row(context, row))
                {
                    GUI::text(context, context->make_id("header_type"), "Type");
                    GUI::text(context, context->make_id("header_size"), "Size");
                    GUI::text(context, context->make_id("header_count"), "Allocation Count");
                    GUI::end_table_row(context);
                }
                for(auto& a : h.second)
                {
                    context->push_data_scope(context->make_id(a.first.c_str()));
                    if(GUI::begin_table_row(context, row))
                    {
                        String count_text;
                        strprintf(count_text, "%llu", (u64)a.second.second);
                        GUI::text(context, context->make_id("type"), a.first.c_str());
                        GUI::text(context, context->make_id("size"), memory_size_text(a.second.first).c_str());
                        GUI::text(context, context->make_id("count"), count_text.c_str());
                        GUI::end_table_row(context);
                    }
                    context->pop_data_scope();
                }
                lupanic_if_failed(GUI::end_table_layout(context, table));
            }
            context->pop_data_scope();
        }
        lupanic_if_failed(GUI::end_v_layout(context, content, GUICore::FlexLayoutDesc()));
        lupanic_if_failed(GUI::end_scroll_view(context, scroll));
        lupanic_if_failed(GUI::end_v_layout(context, root, GUICore::FlexLayoutDesc()));
        context->pop_data_scope();
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
