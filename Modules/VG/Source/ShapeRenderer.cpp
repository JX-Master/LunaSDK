/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeRenderer.cpp
* @author JXMaster
* @date 2022/4/25
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "ShapeRenderer.hpp"
#include <Runtime/Math/Transform.hpp>
#include <RHI/ShaderCompileHelper.hpp>

namespace Luna
{
	namespace VG
	{
		Blob g_fill_shader_vs;
		Blob g_fill_shader_ps;
		Ref<RHI::IDescriptorSetLayout> g_fill_desc_layout;
		Ref<RHI::IShaderInputLayout> g_fill_slayout;
		Ref<RHI::IPipelineState> g_fill_pso;
		Ref<RHI::IResource> g_white_tex;

		RV init_render_resources()
		{
			using namespace RHI;
			lutry
			{
				{
					auto compiler = ShaderCompiler::new_compiler();
					compiler->set_source({ FILL_SHADER_SOURCE_VS, FILL_SHADER_SOURCE_VS_SIZE });
					compiler->set_source_name("FillVS");
					compiler->set_entry_point("main");
					compiler->set_target_format(RHI::get_current_platform_shader_target_format());
					compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
					compiler->set_shader_model(5, 0);
					compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
					luexp(compiler->compile());
					auto data = compiler->get_output();
					g_fill_shader_vs = Blob(data.data(), data.size());

					compiler->reset();
					compiler->set_source({ FILL_SHADER_SOURCE_PS, FILL_SHADER_SOURCE_PS_SIZE });
					compiler->set_source_name("FillPS");
					compiler->set_entry_point("main");
					compiler->set_target_format(RHI::get_current_platform_shader_target_format());
					compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
					compiler->set_shader_model(5, 0);
					compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
					luexp(compiler->compile());
					data = compiler->get_output();
					g_fill_shader_ps = Blob(data.data(), data.size());
				}
				{
					DescriptorSetLayoutDesc desc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::vertex),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibility::pixel),
					});
					luset(g_fill_desc_layout, get_main_device()->new_descriptor_set_layout(desc));
				}
				{
					
					ShaderInputLayoutDesc desc;
					desc.descriptor_set_layouts.push_back(g_fill_desc_layout);
					desc.flags = ShaderInputLayoutFlag::allow_input_assembler_input_layout | ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access | ShaderInputLayoutFlag::deny_hull_shader_access;
					luset(g_fill_slayout, get_main_device()->new_shader_input_layout(desc));
				}
				{
					GraphicsPipelineStateDesc desc;
					desc.input_layout = InputLayoutDesc({
							InputElementDesc("POSITION", 0, Format::rg32_float),
							InputElementDesc("SHAPECOORD", 0, Format::rg32_float),
							InputElementDesc("TEXCOORD", 0, Format::rg32_float),
							InputElementDesc("COLOR", 0, Format::rgba8_unorm),
							InputElementDesc("COMMAND_OFFSET", 0, Format::r32_uint),
							InputElementDesc("NUM_COMMANDS", 0, Format::r32_uint),
						});
					desc.shader_input_layout = g_fill_slayout;
					desc.vs = { g_fill_shader_vs.data(), g_fill_shader_vs.size() };
					desc.ps = { g_fill_shader_ps.data(), g_fill_shader_ps.size() };
					desc.blend_state = BlendDesc(false, false, { AttachmentBlendDesc(true, false, BlendFactor::src_alpha, BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::zero,
							BlendFactor::one, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
					desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, false, false, false, false);
					desc.num_render_targets = 1;
					desc.rtv_formats[0] = Format::rgba8_unorm;
					luset(g_fill_pso, get_main_device()->new_graphics_pipeline_state(desc));
				}
				{
					ResourceDesc desc = ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1);
					luset(g_white_tex, get_main_device()->new_resource(desc));
					u32 data = 0xFFFFFFFF;
					luexp(get_main_device()->copy_resource({
						ResourceCopyDesc::as_write_texture(g_white_tex, &data, sizeof(u32), sizeof(u32), 0, BoxU(0, 0, 0, 1, 1, 1))
						}));
				}
			}
			lucatchret;
			return ok;
		}
		void deinit_render_resources()
		{
			g_fill_shader_vs.clear();
			g_fill_shader_ps.clear();
			g_fill_desc_layout = nullptr;
			g_fill_slayout = nullptr;
			g_fill_pso = nullptr;
			g_white_tex = nullptr;
		}
		RV FillShapeRenderer::init(RHI::IResource* render_target)
		{
			return set_render_target(render_target);
		}
		void FillShapeRenderer::reset()
		{
			lutsassert();
		}
		RV FillShapeRenderer::set_render_target(RHI::IResource* render_target)
		{
			lutsassert();
			m_render_target = render_target;
			lutry
			{
				luset(m_rtv, render_target->get_device()->new_render_target_view(render_target));
				auto res_desc = render_target->get_desc();
				m_screen_width = (u32)res_desc.width_or_buffer_size;
				m_screen_height = (u32)res_desc.height;
			}
			lucatchret;
			return ok;
		}
		RV FillShapeRenderer::render(
			RHI::ICommandBuffer* cmdbuf,
			RHI::IResource* shape_buffer,
			u32 num_points,
			RHI::IResource* vertex_buffer,
			u32 num_vertices,
			RHI::IResource* index_buffer,
			u32 num_indices,
			const ShapeDrawCall* draw_calls,
			u32 num_draw_calls
		)
		{
			using namespace RHI;
			lutsassert();
			lutry
			{
				u32 cb_element_size = max<u32>((u32)get_main_device()->get_uniform_buffer_data_alignment(), (u32)sizeof(Float4x4U));
				u64 cb_size = cb_element_size * num_draw_calls;
				// Build constant buffer.
				if (num_draw_calls > m_cbs_capacity)
				{
					luset(m_cbs_resource, get_main_device()->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer,
						cb_size)));
					m_cbs_capacity = num_draw_calls;
				}
				void* cb_data;
				luexp(m_cbs_resource->map_subresource(0, 0, 0, &cb_data));
				for (usize i = 0; i < num_draw_calls; ++i)
				{
					Float4x4U* dest = (Float4x4U*)(((usize)cb_data) + i * cb_element_size);
					Float4x4 transform = AffineMatrix::make_rotation_z(draw_calls[i].rotation / 180.0f * PI);
					transform = mul(transform, AffineMatrix::make_translation(Float3(draw_calls[i].origin_point.x, draw_calls[i].origin_point.y, 0.0f)));
					Float4x4 mat = ProjectionMatrix::make_orthographic_off_center(0.0f, (f32)m_screen_width, 0.0f, (f32)m_screen_height, 0.0f, 1.0f);
					mat = mul(transform, mat);
					*dest = mat;
				}
				m_cbs_resource->unmap_subresource(0, 0, num_draw_calls * sizeof(Float4x4U));
				// Build view sets.
				for (usize i = 0; i < num_draw_calls; ++i)
				{
					while (m_desc_sets.size() <= i)
					{
						lulet(desc_set, get_main_device()->new_descriptor_set(DescriptorSetDesc(g_fill_desc_layout)));
						m_desc_sets.push_back(desc_set);
					}
					auto& ds = m_desc_sets[i];
					ds->set_cbv(0, m_cbs_resource, ConstantBufferViewDesc(cb_element_size * i, cb_element_size));
					ds->set_srv(1, shape_buffer, &ShaderResourceViewDesc::as_buffer(RHI::Format::r32_float, 0, num_points, 0));
					ds->set_srv(2, draw_calls[i].texture ? draw_calls[i].texture : g_white_tex);
					ds->set_sampler(3, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp));
				}
				// Build command buffer.
				Vector<ResourceBarrierDesc> barriers;
				barriers.push_back(ResourceBarrierDesc::as_transition(m_render_target, ResourceStateFlag::render_target));
				barriers.push_back(ResourceBarrierDesc::as_transition(g_white_tex, ResourceStateFlag::shader_resource_pixel));
				for (usize i = 0; i < num_draw_calls; ++i)
				{
					if (draw_calls[i].texture)
					{
						barriers.push_back(ResourceBarrierDesc::as_transition(draw_calls[i].texture, ResourceStateFlag::shader_resource_pixel));
					}
				}
				cmdbuf->resource_barriers({ barriers.data(), (u32)barriers.size()});
				RenderPassDesc desc;
				desc.color_attachments[0] = m_rtv;
				desc.color_load_ops[0] = LoadOp::clear;
				desc.color_clear_values[0] = Float4U{ 0.0f };
				cmdbuf->begin_render_pass(desc);
				cmdbuf->set_pipeline_state(g_fill_pso);
				cmdbuf->set_graphics_shader_input_layout(g_fill_slayout);
				cmdbuf->set_primitive_topology(PrimitiveTopology::triangle_list);
				cmdbuf->set_vertex_buffers(0, { &VertexBufferViewDesc(vertex_buffer, 0, sizeof(Vertex) * num_vertices, sizeof(Vertex)), 1 });
				cmdbuf->set_index_buffer({index_buffer, 0, num_indices * sizeof(u32), Format::r32_uint});
				cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)m_screen_width, (f32)m_screen_height, 0.0f, 1.0f));
				for (usize i = 0; i < num_draw_calls; ++i)
				{
					cmdbuf->set_graphics_descriptor_set(0, m_desc_sets[i]);
					if (draw_calls[i].clip_rect != RectI(0, 0, 0, 0))
					{
						cmdbuf->set_scissor_rect(draw_calls[i].clip_rect);
					}
					else
					{
						cmdbuf->set_scissor_rect(RectI(0, 0, m_screen_width, m_screen_height));
					}
					cmdbuf->draw_indexed(draw_calls[i].num_indices, draw_calls[i].base_index, 0);
				}
				cmdbuf->end_render_pass();
			}
			lucatchret;
			return ok;
		}
		LUNA_VG_API R<Ref<IShapeRenderer>> new_fill_shape_renderer(RHI::IResource* render_target)
		{
			Ref<FillShapeRenderer> renderer = new_object<FillShapeRenderer>();
			lutry
			{
				luexp(renderer->init(render_target));
			}
			lucatchret;
			return renderer;
		}
	}
}