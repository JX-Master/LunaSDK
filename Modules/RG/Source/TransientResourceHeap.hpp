/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TransientResourceHeap.hpp
* @author JXMaster
* @date 2023/3/5
*/
#pragma once
#include "../TransientResourceHeap.hpp"

namespace Luna
{
    namespace RG
    {
        struct TransientResourceHeapSegment
        {
            struct HeapSection
            {
                u64 begin;
                u64 end;

                bool operator<(const HeapSection& rhs) const
                {
                    return begin < rhs.begin;
                }
            };

            RHI::ResourceHeapType m_heap_type;
            RHI::ResourceHeapChildType m_child_types;
            Ref<RHI::IResourceHeap> m_heap;
            Vector<HeapSection> m_free_sections;

            bool allocate(u64 size, u64 alignment, u64& out_offset);
            void release(const HeapSection& section);
        };

        struct TransientResourceHeap : ITransientResourceHeap
        {
            lustruct("RG::TransientResourceHeap", "{7d3145e6-bf69-4399-a535-c31e61fb7e03}");
            luiimpl();

            Ref<RHI::IDevice> m_device;
            Vector<TransientResourceHeapSegment> m_segments;

            struct ResourceAllocationInfo
            {
                usize index;
                u64 begin;
                u64 end;
            };

            HashMap<RHI::IResource*, ResourceAllocationInfo> m_allocated_resources;

            static constexpr u64 SEGMENT_SIZE = 128_mb;

            R<Ref<RHI::IResource>> allocate_from_segment(usize segment_index, u64 size, u64 alignment, const RHI::ResourceDesc& desc, const RHI::ClearValue* optimized_clear_value);
            virtual R<Ref<RHI::IResource>> allocate(const RHI::ResourceDesc& desc, const RHI::ClearValue* optimized_clear_value) override;
            virtual void release(RHI::IResource* res) override;
        };
    }
}