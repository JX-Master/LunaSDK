/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TransientResourceHeap.cpp
* @author JXMaster
* @date 2023/3/5
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_RG_API LUNA_EXPORT
#include "TransientResourceHeap.hpp"

namespace Luna
{
    namespace RG
    {
        bool TransientResourceHeapSegment::allocate(u64 size, u64 alignment, u64& out_offset)
        {
            luassert(size && alignment);
            for(auto iter = m_free_sections.begin(); iter != m_free_sections.end(); ++iter)
            {
                u64 block_begin = iter->begin;
                u64 block_end = iter->end;
                u64 alloc_begin = align_upper(block_begin, alignment);
                u64 alloc_end = alloc_begin + size;
                if(alloc_end <= block_end)
                {
                    // Use this block.
                    out_offset = alloc_begin;
                    if(alloc_begin > block_begin && alloc_end < block_end)
                    {
                        iter->begin = alloc_end;
                        iter->end = block_end;
                        m_free_sections.insert(iter, {block_begin, alloc_begin});
                    }
                    else if(alloc_begin > block_begin)
                    {
                        iter->begin = block_begin;
                        iter->end = alloc_begin;
                    }
                    else if(alloc_end < block_end)
                    {
                        iter->begin = alloc_end;
                        iter->end = block_end;
                    }
                    else
                    {
                        m_free_sections.erase(iter);
                    }
                    return true;
                }
            }
            return false;
        }
        void TransientResourceHeapSegment::release(const HeapSection& section)
        {
            auto iter = lower_bound(m_free_sections.begin(), m_free_sections.end(), section);
            if(iter == m_free_sections.begin())
            {
                // Merge to front.
                if(m_free_sections.front().begin == section.end)
                {
                    m_free_sections.front().begin = section.begin;
                }
                else
                {
                    m_free_sections.insert(m_free_sections.begin(), section);
                }

            }
            else if(iter == m_free_sections.end())
            {
                // Merge to back.
                if(m_free_sections.back().end == section.begin)
                {
                    m_free_sections.back().end = section.end;
                }
                else
                {
                    m_free_sections.push_back(section);
                }
            }
            else
            {
                // Merge to middle.
                auto prior = iter - 1;
                if(prior->end == section.begin && section.end == iter->begin)
                {
                    prior->end = iter->end;
                    m_free_sections.erase(iter);
                }
                else if(prior->end == section.begin)
                {
                    prior->end = section.end;
                }
                else if(section.end == iter->begin)
                {
                    iter->begin = section.begin;
                }
                else
                {
                    m_free_sections.insert(iter, section);
                }
            }
        }

        R<Ref<RHI::IResource>> TransientResourceHeap::allocate_from_segment(usize i, u64 size, u64 alignment, const RHI::ResourceDesc& desc, const RHI::ClearValue* optimized_clear_value)
        {
            if(m_segments[i].m_heap_type != desc.heap_type) return nullptr;
            if(!test_flags(m_segments[i].m_usages, RHI::ResourceHeapUsageFlag::buffer) && desc.type == RHI::ResourceType::buffer) return nullptr;
            if((desc.type == RHI::ResourceType::texture_1d || desc.type == RHI::ResourceType::texture_2d || desc.type == RHI::ResourceType::texture_3d) && 
                (test_flags(desc.usages, RHI::ResourceUsageFlag::render_target) || test_flags(desc.usages, RHI::ResourceUsageFlag::depth_stencil)) &&
                !test_flags(m_segments[i].m_usages, RHI::ResourceHeapUsageFlag::texture_rt_ds)) return nullptr;
            if((desc.type == RHI::ResourceType::texture_1d || desc.type == RHI::ResourceType::texture_2d || desc.type == RHI::ResourceType::texture_3d) && 
                (!test_flags(desc.usages, RHI::ResourceUsageFlag::render_target) && !test_flags(desc.usages, RHI::ResourceUsageFlag::depth_stencil)) &&
                !test_flags(m_segments[i].m_usages, RHI::ResourceHeapUsageFlag::texture_non_rt_ds)) return nullptr;
            u64 offset;
            bool allocated = m_segments[i].allocate(size, alignment, offset);
            if(allocated)
            {
                auto r = m_segments[i].m_heap->new_resource(offset, desc, optimized_clear_value);
                if(failed(r))
                {
                    m_segments[i].release({offset, offset + size});
                    return r.errcode();
                }
                Ref<RHI::IResource> ret = r.get();
                ResourceAllocationInfo info {i, offset, offset + size};
                m_allocated_resources.insert(make_pair(ret.get(), info));
                return ret;
            }
            return nullptr;
        }
        R<Ref<RHI::IResource>> TransientResourceHeap::allocate(const RHI::ResourceDesc& desc, const RHI::ClearValue* optimized_clear_value)
        {
            Ref<RHI::IResource> ret;
            lutry
            {
                u64 size, alignment;
                size = m_device->get_resource_size(desc, &alignment);
                for(usize i = 0; i < m_segments.size(); ++i)
                {
                    luset(ret, allocate_from_segment(i, size, alignment, desc, optimized_clear_value));
                    if(ret) break;
                }
                if(!ret)
                {
                    TransientResourceHeapSegment segment;
                    RHI::ResourceHeapDesc heap_desc;
                    heap_desc.type = desc.heap_type;
                    heap_desc.usages = RHI::ResourceHeapUsageFlag::none;
                    if(desc.type == RHI::ResourceType::buffer)
                    {
                        set_flags(heap_desc.usages, RHI::ResourceHeapUsageFlag::buffer);
                    }
                    else if(desc.type == RHI::ResourceType::texture_1d || desc.type == RHI::ResourceType::texture_2d || desc.type == RHI::ResourceType::texture_3d)
                    {
                        if(test_flags(desc.usages, RHI::ResourceUsageFlag::render_target) || test_flags(desc.usages, RHI::ResourceUsageFlag::depth_stencil))
                        {
                            set_flags(heap_desc.usages, RHI::ResourceHeapUsageFlag::texture_rt_ds);
                        }
                        else
                        {
                            set_flags(heap_desc.usages, RHI::ResourceHeapUsageFlag::texture_non_rt_ds);
                        }
                    }
                    else lupanic();
                    if(desc.sample_count != 1)
                    {
                        set_flags(heap_desc.usages, RHI::ResourceHeapUsageFlag::texture_msaa);
                    }
                    heap_desc.size = max(SEGMENT_SIZE, size);
                    segment.m_heap_type = desc.heap_type;
                    segment.m_usages = heap_desc.usages;
                    luset(segment.m_heap, m_device->new_resource_heap(heap_desc));
                    segment.m_free_sections.push_back({0, heap_desc.size});
                    m_segments.push_back(move(segment));
                    luset(ret, allocate_from_segment(m_segments.size() - 1, size, alignment, desc, optimized_clear_value));
                    luassert(ret);
                }
            }
            lucatchret;
            return ret;
        }
        void TransientResourceHeap::release(RHI::IResource* res)
        {
            auto iter = m_allocated_resources.find(res);
            if(iter == m_allocated_resources.end()) return;
            m_segments[iter->second.index].release({iter->second.begin, iter->second.end});
            m_allocated_resources.erase(iter);
        }
        
        LUNA_RG_API Ref<ITransientResourceHeap> new_transient_resource_heap(RHI::IDevice* device)
        {
            auto ret = new_object<TransientResourceHeap>();
            ret->m_device = device;
            return ret;
        }
    }
}