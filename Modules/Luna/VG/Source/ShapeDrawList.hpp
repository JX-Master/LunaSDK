/*!
* This file is a portion of Luna SDK.
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
namespace Luna
{
	namespace VG
	{
		struct ShapeDrawList : IShapeDrawList
		{
			lustruct("VG::ShapeDrawList", "{44732F66-CE52-4493-85C3-6E0164C4EA18}");
			luiimpl();
			lutsassert_lock();
			Ref<RHI::IDevice> m_device;

			Ref<RHI::IBuffer> m_vertex_buffer;
			Ref<RHI::IBuffer> m_index_buffer;
			Ref<RHI::IBuffer> m_internal_shape_buffer;
			u64 m_vertex_buffer_size;
			u64 m_index_buffer_size;
			u64 m_internal_shape_buffer_size;
			u64 m_vertex_buffer_capacity;
			u64 m_index_buffer_capacity;
			u64 m_internal_shape_buffer_capacity;

			Vector<ShapeDrawCall> m_draw_calls;
			Vector<Vertex> m_vertices;
			Vector<u32> m_indices;
			Vector<f32> m_internal_shape_points;

			// Current draw state.
			Ref<RHI::IBuffer> m_shape_buffer;
			Ref<RHI::ITexture> m_texture;
			RHI::SamplerDesc m_sampler;
			Float2U m_origin;
			f32 m_rotation;

			// If `true`, then the draw call should be re-targeted.
			bool m_state_dirty;

			void new_draw_call()
			{
				m_draw_calls.emplace_back();
				ShapeDrawCall& dc = m_draw_calls.back();
				dc.shape_buffer = m_shape_buffer;
				dc.texture = m_texture;
				dc.sampler = m_sampler;
				dc.origin_point = m_origin;
				dc.rotation = m_rotation;
				dc.base_index = (u32)m_indices.size();
				dc.num_indices = 0;
			}
			ShapeDrawCall& get_current_draw_call();
			static RHI::SamplerDesc get_default_sampler()
			{
				return RHI::SamplerDesc(RHI::Filter::linear, RHI::Filter::linear, RHI::Filter::linear,
					RHI::TextureAddressMode::repeat,
					RHI::TextureAddressMode::repeat,
					RHI::TextureAddressMode::repeat);
			}
			
			ShapeDrawList() :
				m_vertex_buffer_size(0),
				m_vertex_buffer_capacity(0),
				m_index_buffer_size(0),
				m_index_buffer_capacity(0),
				m_internal_shape_buffer_size(0),
				m_internal_shape_buffer_capacity(0),
				m_sampler(get_default_sampler()),
				m_origin(0.0f),
				m_rotation(0.0f),
				m_state_dirty(false)
			{}

			virtual void reset() override;
			virtual Vector<f32>& get_shape_points() override
			{
				lutsassert();
				return m_internal_shape_points;
			}
            virtual void set_shape_buffer(RHI::IBuffer* shape_buffer) override
			{
				lutsassert();
				if (m_shape_buffer != shape_buffer)
				{
					m_state_dirty = true;
					m_shape_buffer = shape_buffer;
				}
			}
            virtual RHI::IBuffer* get_shape_buffer() override
			{
				return m_shape_buffer;
			}
            virtual void set_texture(RHI::ITexture* tex) override
			{
				lutsassert();
				if (m_texture != tex)
				{
					m_state_dirty = true;
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
					m_state_dirty = true;
					m_sampler = d;
				}
			}
            virtual RHI::SamplerDesc get_sampler() override
			{
				return m_sampler;
			}
            virtual void set_origin(const Float2& origin) override
			{
				lutsassert();
				if (m_origin != origin)
				{
					m_state_dirty = true;
					m_origin = origin;
				}
			}
            virtual Float2 get_origin() override
			{
				return m_origin;
			}
            virtual void set_rotation(f32 degrees) override
			{
				lutsassert();
				if (m_rotation != degrees)
				{
					m_state_dirty = true;
					m_rotation = degrees;
				}
			}
            virtual f32 get_rotation() override
			{
				return m_rotation;
			}
            virtual void draw_shape_raw(Span<const Vertex> vertices, Span<const u32> indices) override;
            virtual void draw_shape(u32 begin_command, u32 num_commands,
				const Float2U& min_position, const Float2U& max_position,
				const Float2U& min_shapecoord, const Float2U& max_shapecoord, u32 color,
				const Float2U& min_texcoord, const Float2U& max_texcoord) override;
            virtual RV close() override;
            virtual RHI::IBuffer* get_vertex_buffer() override
			{
				return m_vertex_buffer;
			}
            virtual u32 get_vertex_buffer_size() override
			{
				return m_vertex_buffer_size;
			}
            virtual RHI::IBuffer* get_index_buffer() override
			{
				return m_index_buffer;
			}
            virtual u32 get_index_buffer_size() override
			{
				return m_index_buffer_size;
			}
            virtual Vector<ShapeDrawCall> get_draw_calls() override
			{
				return m_draw_calls;
			}
		};
	}
}
