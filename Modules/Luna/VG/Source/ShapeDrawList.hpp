/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeDrawList.hpp
* @author JXMaster
* @date 2022/4/17
*/
#include "../VG.hpp"
#include <Luna/Runtime/TSAssert.hpp>
namespace Luna
{
	namespace VG
	{
		struct ShapeDrawCallResource
		{
			Vector<Vertex> vertices;
			Vector<u32> indices;
		};

		struct ShapeDrawList : IShapeDrawList
		{
			lustruct("VG::ShapeDrawList", "{44732F66-CE52-4493-85C3-6E0164C4EA18}");
			luiimpl();
			lutsassert_lock();
			Ref<RHI::IBuffer> m_vertex_buffer;
			Ref<RHI::IBuffer> m_index_buffer;
			u32 m_vertex_buffer_size;
			u32 m_index_buffer_size;
			usize m_vertex_buffer_capacity;
			usize m_index_buffer_capacity;

			Vector<ShapeDrawCall> m_draw_calls;
			Vector<ShapeDrawCallResource> m_draw_call_resources;

			// Current draw state.
			Ref<IShapeAtlas> m_atlas;
			Ref<RHI::ITexture> m_texture;
			RHI::SamplerDesc m_sampler;
			Float2U m_origin;
			f32 m_rotation;
			RectI m_clip_rect;

			// If `true`, then the draw call should be re-targeted.
			bool m_state_dirty;
			// There is a range of draw calls that can accept new elements, which starts from m_dc_barrier_index,
			// and end with m_draw_calls.size().
			// The draw calls after this index can accept new elements.
			u32 m_dc_barrier_index;
			// The target draw call index for the current pipeline state.
			i32 m_target_dc_index;

			void new_draw_call()
			{
				m_draw_calls.emplace_back();
				ShapeDrawCall& dc = m_draw_calls.back();
				dc.atlas = m_atlas;
				dc.texture = m_texture;
				dc.sampler = m_sampler;
				dc.origin_point = m_origin;
				dc.rotation = m_rotation;
				dc.clip_rect = m_clip_rect;
			}
			ShapeDrawCall& get_current_draw_call();
			ShapeDrawCallResource& get_draw_call_resource(usize index);
			// Tests if the specified state is equal with the current set state.
			bool state_equal(u32 index);
			RHI::SamplerDesc get_default_sampler()
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
				m_texture(nullptr),
				m_sampler(get_default_sampler()),
				m_clip_rect(0, 0, 0, 0),
				m_state_dirty(false),
				m_dc_barrier_index(0),
				m_target_dc_index(0)
			{}

			virtual void reset() override;
            virtual void drawcall_barrier() override
			{
				lutsassert();
				m_dc_barrier_index = (u32)m_draw_calls.size();
			}
            virtual void set_shape_atlas(IShapeAtlas* atlas) override
			{
				lutsassert();
				if (m_atlas != atlas)
				{
					m_state_dirty = true;
					m_atlas = atlas;
				}
			}
            virtual IShapeAtlas* get_shape_atlas() override
			{
				return m_atlas;
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
            virtual void set_clip_rect(const RectI& clip_rect) override
			{
				lutsassert();
				if (m_clip_rect != clip_rect)
				{
					m_state_dirty = true;
					m_clip_rect = clip_rect;
				}
			}
            virtual RectI get_clip_rect() override
			{
				return m_clip_rect;
			}
            virtual void append_draw_list(IShapeDrawList* draw_list) override;
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
