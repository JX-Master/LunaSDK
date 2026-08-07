/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeDrawList.hpp
* @author JXMaster
* @date 2022/4/17
*/
#include "../ShapeDrawList.hpp"
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/RHI/Device.hpp>
#include "ShapeDrawListImpl.generated.hpp"
namespace Luna
{
    namespace VG
    {
        struct [[luna::struct("{44732F66-CE52-4493-85C3-6E0164C4EA18}")]] ShapeDrawList : IShapeDrawList
        {
            luiimpl();
            lutsassert_lock();
            Ref<RHI::IDevice> m_device;

            Ref<RHI::IBuffer> m_instance_buffer;
            Ref<RHI::IBuffer> m_state_buffer;
            Ref<IShapeBuffer> m_internal_shape_buffer;
            u64 m_instance_buffer_size;
            u64 m_instance_buffer_capacity;
            u64 m_state_buffer_size;
            u64 m_state_buffer_capacity;

            Vector<Ref<IShapeBuffer>> m_draw_call_buffers;
            Vector<ShapeDrawCall> m_draw_calls;
            Vector<ShapeInstance> m_instances;
            Vector<ShapeState> m_states;

            // Current draw state.
            Ref<IShapeBuffer> m_shape_buffer;
            Ref<RHI::ITexture> m_texture;
            RHI::SamplerDesc m_sampler;
            Float4x4U m_transform;
            RectF m_clip_rect;
            RectF m_rounded_clip_rect;
            Float4U m_rounded_clip_radii;

            // If `true`, then the current state index must be resolved again.
            bool m_state_index_dirty;
            u32 m_current_state_index;
            // If `true`, then a new resource draw call must be created.
            bool m_draw_call_dirty;

            void new_draw_call()
            {
                m_draw_calls.emplace_back();
                m_draw_call_buffers.push_back(m_shape_buffer);
                ShapeDrawCall& dc = m_draw_calls.back();
                dc.texture = m_texture;
                dc.sampler = m_sampler;
                dc.base_instance = (u32)m_instances.size();
                dc.num_instances = 0;
            }
            ShapeDrawCall& get_current_draw_call();
            u32 resolve_current_state_index();
            static RHI::SamplerDesc get_default_sampler()
            {
                return RHI::SamplerDesc(RHI::Filter::linear, RHI::Filter::linear, RHI::Filter::linear,
                    RHI::TextureAddressMode::repeat,
                    RHI::TextureAddressMode::repeat,
                    RHI::TextureAddressMode::repeat);
            }
            
            ShapeDrawList() :
                m_instance_buffer_size(0),
                m_instance_buffer_capacity(0),
                m_state_buffer_size(0),
                m_state_buffer_capacity(0),
                m_sampler(get_default_sampler()),
                m_transform(Float4x4::identity()),
                m_clip_rect(0, 0, 0, 0),
                m_rounded_clip_rect(0, 0, 0, 0),
                m_rounded_clip_radii(0.0f),
                m_state_index_dirty(true),
                m_current_state_index(0),
                m_draw_call_dirty(false)
            {
                m_internal_shape_buffer = new_shape_buffer();
            }

            virtual RHI::IDevice* get_device() override
            {
                return m_device;
            }
            virtual void reset() override;
            virtual void set_shape_buffer(IShapeBuffer* shape_buffer) override
            {
                lutsassert();
                if (m_shape_buffer != shape_buffer)
                {
                    m_draw_call_dirty = true;
                    m_shape_buffer = shape_buffer;
                }
            }
            virtual IShapeBuffer* get_shape_buffer() override
            {
                return m_shape_buffer ? m_shape_buffer : m_internal_shape_buffer;
            }
            virtual void set_texture(RHI::ITexture* tex) override
            {
                lutsassert();
                if (m_texture != tex)
                {
                    m_draw_call_dirty = true;
                    m_texture = tex;
                }
            }
            virtual RHI::ITexture* get_texture() override
            {
                return m_texture;
            }
            virtual void set_sampler(const RHI::SamplerDesc* desc) override
            {
                lutsassert();
                RHI::SamplerDesc d;
                if (!desc)
                {
                    d = get_default_sampler();
                }
                else
                {
                    d = *desc;
                }
                if (d != m_sampler)
                {
                    m_draw_call_dirty = true;
                    m_sampler = d;
                }
            }
            virtual RHI::SamplerDesc get_sampler() override
            {
                return m_sampler;
            }
            virtual void set_transform(const Float4x4U& transform) override
            {
                lutsassert();
                if (m_transform != transform)
                {
                    m_state_index_dirty = true;
                    m_transform = transform;
                }
            }
            virtual Float4x4U get_transform() override
            {
                return m_transform;
            }
            virtual void set_clip_rect(const RectF& clip_rect) override
            {
                lutsassert();
                if (m_clip_rect != clip_rect)
                {
                    m_state_index_dirty = true;
                    m_clip_rect = clip_rect;
                }
            }
            virtual RectF get_clip_rect() override
            {
                return m_clip_rect;
            }
            virtual void set_rounded_clip_rect(const RectF& clip_rect, const Float4U& corner_radii) override
            {
                lutsassert();
                if (m_rounded_clip_rect != clip_rect || m_rounded_clip_radii != corner_radii)
                {
                    m_state_index_dirty = true;
                    m_rounded_clip_rect = clip_rect;
                    m_rounded_clip_radii = corner_radii;
                }
            }
            virtual RectF get_rounded_clip_rect() override
            {
                return m_rounded_clip_rect;
            }
            virtual Float4U get_rounded_clip_radii() override
            {
                return m_rounded_clip_radii;
            }
            virtual void draw_call_barrier() override
            {
                lutsassert();
                m_draw_call_dirty = true;
            }
            virtual void draw_shape(u32 begin_command, u32 num_commands,
                const Float2U& min_position, const Float2U& max_position,
                const Float2U& min_shapecoord, const Float2U& max_shapecoord, const Float4U& color,
                const Float2U& min_texcoord, const Float2U& max_texcoord) override;
            virtual RV compile() override;
            virtual RHI::IBuffer* get_instance_buffer() override
            {
                return m_instance_buffer;
            }
            virtual u32 get_instance_buffer_size() override
            {
                return m_instance_buffer_size;
            }
            virtual RHI::IBuffer* get_state_buffer() override
            {
                return m_state_buffer;
            }
            virtual u32 get_state_buffer_size() override
            {
                return m_state_buffer_size;
            }
            virtual Span<const ShapeDrawCall> get_draw_calls() override
            {
                return m_draw_calls.cspan();
            }
        };
    }
}
