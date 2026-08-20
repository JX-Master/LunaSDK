/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Renderer.cpp
* @author JXMaster
* @date 2026/7/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "RendererImpl.hpp"
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <BackdropBlurCS.hpp>
#include <SDFVS.hpp>
#include <SDFPS.hpp>
#include <cmath>

namespace Luna
{
    namespace GUI
    {
        namespace
        {
            static_assert(sizeof(SDFInstance) == sizeof(f32) * 10,
                "SDFInstance must match the instance input layout used by SDFVS.");
            static_assert(sizeof(SDFState) == sizeof(f32) * 12,
                "SDFState must match the structured-buffer layout used by SDFVS.");

            constexpr u32 SDF_INSTANCE_COLOR_FLOAT_COUNT_SHIFT = 2;
            static_assert(SDF_MAX_COLOR_FLOATS <= 0x7FF,
                "SDF color float count must fit the instance encoding.");

            struct SDFFrameBuffer
            {
                Float4x4U surface_to_clip;
            };

            bool color_visible(const Float4U& color)
            {
                return color.w > 0.0f;
            }

            RHI::Format select_backdrop_intermediate_format(RHI::Format format)
            {
                using RHI::Format;
                switch(format)
                {
                case Format::rgba8_unorm:
                case Format::rgba8_unorm_srgb:
                case Format::bgra8_unorm:
                case Format::bgra8_unorm_srgb:
                    // sRGB and BGRA storage writes are not portable across
                    // the supported RHI backends.
                    return Format::rgba8_unorm;
                case Format::rgba16_float:
                    return Format::rgba16_float;
                case Format::rgba32_float:
                    return Format::rgba32_float;
                case Format::rgba16_unorm:
                case Format::rgb10a2_unorm:
                case Format::rg11b10_float:
                case Format::rgb9e5_float:
                    return Format::rgba16_float;
                default:
                    return Format::unknown;
                }
            }

            struct BackdropGaussianKernel
            {
                f32 center_weight = 1.0f;
                u32 pair_count = 0;
                Float4U pairs[MAX_BACKDROP_GAUSSIAN_PAIRS] = {};
            };

            f32 quantize_backdrop_gaussian_sigma(f32 sigma)
            {
                f32 quantized_sigma = (f32)std::floor(
                    (f64)sigma / BACKDROP_GAUSSIAN_SIGMA_QUANTIZATION + 0.5) *
                    BACKDROP_GAUSSIAN_SIGMA_QUANTIZATION;
                return min(max(quantized_sigma, 0.0f),
                    MAX_BACKDROP_WORKING_SIGMA);
            }

            u32 calculate_backdrop_downsample_passes(
                const BackdropBlurCaptureDesc& desc,
                f32 framebuffer_scale_x, f32 framebuffer_scale_y)
            {
                u32 pass_count = min<u32>(desc.downsample_level,
                    MAX_BACKDROP_DOWNSAMPLE_PASSES);
                f32 sigma = max(desc.softness, 0.0f) *
                    max(framebuffer_scale_x, framebuffer_scale_y);
                while(pass_count < MAX_BACKDROP_DOWNSAMPLE_PASSES &&
                    sigma / (f32)(1u << pass_count) >
                        MAX_BACKDROP_WORKING_SIGMA)
                {
                    ++pass_count;
                }
                return pass_count;
            }

            BackdropGaussianKernel build_backdrop_gaussian_kernel(f32 sigma)
            {
                BackdropGaussianKernel kernel;
                f64 quantized_sigma =
                    quantize_backdrop_gaussian_sigma(sigma);
                luassert(quantized_sigma >
                    BACKDROP_GAUSSIAN_FAST_PATH_SIGMA);
                u32 radius = min<u32>((u32)std::ceil(
                    quantized_sigma * BACKDROP_GAUSSIAN_SUPPORT),
                    MAX_BACKDROP_GAUSSIAN_PAIRS * 2);
                f64 weights[MAX_BACKDROP_GAUSSIAN_PAIRS * 2 + 1] = {};
                weights[0] = 1.0;
                f64 total_weight = 1.0;
                f64 inverse_two_sigma_squared =
                    0.5 / (quantized_sigma * quantized_sigma);
                for(u32 i = 1; i <= radius; ++i)
                {
                    weights[i] = std::exp(
                        -(f64)(i * i) * inverse_two_sigma_squared);
                    total_weight += weights[i] * 2.0;
                }
                kernel.center_weight = (f32)(1.0 / total_weight);
                for(u32 i = 1; i <= radius; i += 2)
                {
                    f64 first_weight = weights[i];
                    f64 second_weight =
                        i + 1 <= radius ? weights[i + 1] : 0.0;
                    f64 pair_weight = first_weight + second_weight;
                    f64 pair_offset = ((f64)i * first_weight +
                        (f64)(i + 1) * second_weight) / pair_weight;
                    luassert(kernel.pair_count <
                        MAX_BACKDROP_GAUSSIAN_PAIRS);
                    kernel.pairs[kernel.pair_count++] = Float4U(
                        (f32)pair_offset,
                        (f32)(pair_weight / total_weight), 0.0f, 0.0f);
                }
                return kernel;
            }

            f32 calculate_backdrop_capture_halo(
                const BackdropBlurCaptureDesc& desc,
                f32 framebuffer_scale_x, f32 framebuffer_scale_y)
            {
                u32 pass_count = calculate_backdrop_downsample_passes(
                    desc, framebuffer_scale_x, framebuffer_scale_y);
                u32 downsample_factor = 1u << pass_count;
                f32 downsample_support = pass_count ?
                    (f32)(downsample_factor - 1) : 0.0f;
                auto axis_support = [&](f32 framebuffer_scale)
                {
                    f32 working_sigma =
                        max(desc.softness, 0.0f) *
                        framebuffer_scale / (f32)downsample_factor;
                    f32 quantized_sigma =
                        quantize_backdrop_gaussian_sigma(
                            working_sigma);
                    f32 gaussian_support = 0.0f;
                    if(quantized_sigma >
                        BACKDROP_GAUSSIAN_FAST_PATH_SIGMA)
                    {
                        gaussian_support = (f32)std::ceil(
                            quantized_sigma *
                                BACKDROP_GAUSSIAN_SUPPORT) *
                            (f32)downsample_factor;
                    }
                    return (downsample_support + gaussian_support) /
                        framebuffer_scale;
                };
                return max(axis_support(framebuffer_scale_x),
                    axis_support(framebuffer_scale_y));
            }

            void set_backdrop_gaussian_kernel(BackdropBlurParams& parameters,
                const BackdropGaussianKernel& kernel)
            {
                parameters.gaussian_center_weight = kernel.center_weight;
                parameters.gaussian_pair_count = kernel.pair_count;
                parameters.gaussian_pair_0 = kernel.pairs[0];
                parameters.gaussian_pair_1 = kernel.pairs[1];
                parameters.gaussian_pair_2 = kernel.pairs[2];
                parameters.gaussian_pair_3 = kernel.pairs[3];
                parameters.gaussian_pair_4 = kernel.pairs[4];
                parameters.gaussian_pair_5 = kernel.pairs[5];
                parameters.gaussian_pair_6 = kernel.pairs[6];
                parameters.gaussian_pair_7 = kernel.pairs[7];
                parameters.gaussian_pair_8 = kernel.pairs[8];
                parameters.gaussian_pair_9 = kernel.pairs[9];
                parameters.gaussian_pair_10 = kernel.pairs[10];
                parameters.gaussian_pair_11 = kernel.pairs[11];
                parameters.gaussian_pair_12 = kernel.pairs[12];
            }

            bool rect_visible(const RectF& rect)
            {
                return rect.width > 0.0f && rect.height > 0.0f;
            }

            RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 min_x = max(a.offset_x, b.offset_x);
                f32 min_y = max(a.offset_y, b.offset_y);
                f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
            }

            RectF union_rect(const RectF& a, const RectF& b)
            {
                if(!rect_visible(a)) return b;
                if(!rect_visible(b)) return a;
                f32 min_x = min(a.offset_x, b.offset_x);
                f32 min_y = min(a.offset_y, b.offset_y);
                f32 max_x = max(a.offset_x + a.width, b.offset_x + b.width);
                f32 max_y = max(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(min_x, min_y, max_x - min_x, max_y - min_y);
            }

            RectF expand_rect(const RectF& rect, f32 amount)
            {
                return RectF(rect.offset_x - amount, rect.offset_y - amount,
                    rect.width + amount * 2.0f, rect.height + amount * 2.0f);
            }

            RectF expand_rect(const RectF& rect, const Float4U& outsets)
            {
                return RectF(rect.offset_x - outsets.x, rect.offset_y - outsets.y,
                    rect.width + outsets.x + outsets.z, rect.height + outsets.y + outsets.w);
            }

            bool has_clip(const RectF& clip_rect)
            {
                return clip_rect.width > 0.0f || clip_rect.height > 0.0f;
            }

            struct ClipState
            {
                RectF rect;
                RectF rounded_rect;
                Float4U rounded_radii = Float4U(0.0f);
            };

            bool has_rounded_clip(const ClipState& clip)
            {
                return has_clip(clip.rounded_rect);
            }

            RectF merge_clip_rect(const RectF& a, const RectF& b)
            {
                if(has_clip(a) && has_clip(b))
                {
                    return intersect_rect(a, b);
                }
                return has_clip(a) ? a : b;
            }

            RectF to_screen_rect(Span<const Layer> layers, u32 layer_index, const RectF& rect)
            {
                if(layer_index >= layers.size())
                {
                    return rect;
                }
                const Float2U& layer_position = layers[layer_index].screen_position;
                return RectF(layer_position.x + rect.offset_x, layer_position.y + rect.offset_y,
                    rect.width, rect.height);
            }

            RectF to_vg_rect(const FrameDesc& frame_desc, const RectF& screen_rect)
            {
                return RectF(screen_rect.offset_x,
                    frame_desc.logical_size.y - screen_rect.offset_y - screen_rect.height,
                    screen_rect.width, screen_rect.height);
            }

            RectF resolve_draw_rect(const DrawCommand& command, Span<const Element> elements)
            {
                if(command.rect_reference != DrawCommandRectReference::element ||
                    command.element == INVALID_ELEMENT || command.element >= elements.size())
                {
                    return command.rect;
                }
                const RectF& element_rect = elements[command.element].layout_result.rect;
                RectF ret;
                ret.offset_x = element_rect.offset_x + command.rect.offset_x +
                    element_rect.width * command.rect_layout_scale.x;
                ret.offset_y = element_rect.offset_y + command.rect.offset_y +
                    element_rect.height * command.rect_layout_scale.y;
                f32 scaled_width = command.rect.width + element_rect.width * command.rect_layout_scale.z;
                f32 scaled_height = command.rect.height + element_rect.height * command.rect_layout_scale.w;
                ret.width = scaled_width > 0.0f ? scaled_width :
                    (command.rect.width < 0.0f ? max(element_rect.width + scaled_width, 1.0f) :
                        max(element_rect.width - command.rect.offset_x, 1.0f));
                ret.height = scaled_height > 0.0f ? scaled_height :
                    (command.rect.height < 0.0f ? max(element_rect.height + scaled_height, 1.0f) :
                        max(element_rect.height - command.rect.offset_y, 1.0f));
                return ret;
            }

            Float2U resolve_draw_point0(const DrawCommand& command, Span<const Element> elements)
            {
                if(command.rect_reference != DrawCommandRectReference::element ||
                    command.element == INVALID_ELEMENT || command.element >= elements.size())
                {
                    return Float2U(command.rect.offset_x, command.rect.offset_y);
                }
                const RectF& element_rect = elements[command.element].layout_result.rect;
                return Float2U(
                    element_rect.offset_x + command.rect.offset_x + element_rect.width * command.rect_layout_scale.x,
                    element_rect.offset_y + command.rect.offset_y + element_rect.height * command.rect_layout_scale.y);
            }

            Float2U resolve_draw_point1(const DrawCommand& command, Span<const Element> elements)
            {
                if(command.rect_reference != DrawCommandRectReference::element ||
                    command.element == INVALID_ELEMENT || command.element >= elements.size())
                {
                    return command.point1;
                }
                const RectF& element_rect = elements[command.element].layout_result.rect;
                return Float2U(
                    element_rect.offset_x + command.point1.x + element_rect.width * command.rect_layout_scale.z,
                    element_rect.offset_y + command.point1.y + element_rect.height * command.rect_layout_scale.w);
            }

            u32 append_program_floats(Vector<f32>& destination, Span<const f32> floats)
            {
                u32 page_offset = (u32)(destination.size() % SDF_PROGRAM_PAGE_FLOATS);
                if(page_offset && page_offset + floats.size() > SDF_PROGRAM_PAGE_FLOATS)
                {
                    destination.resize(destination.size() + SDF_PROGRAM_PAGE_FLOATS - page_offset, 0.0f);
                }
                u32 first_float = (u32)destination.size();
                destination.insert(destination.end(), floats.begin(), floats.end());
                return first_float;
            }

            bool append_shape_program(Vector<f32>& destination, Span<const f32> floats,
                SDFShapeProgram& program)
            {
                RV validation = validate_sdf_shape_program(floats, &program);
                if(failed(validation)) return false;
                program.floats.first_float = append_program_floats(destination, floats);
                return true;
            }

            bool append_color_program(Vector<f32>& destination, Span<const f32> floats,
                SDFColorProgram& program)
            {
                RV validation = validate_sdf_color_program(floats, &program);
                if(failed(validation)) return false;
                program.floats.first_float = append_program_floats(destination, floats);
                return true;
            }

            template <typename _Program>
            bool resolve_context_program(Span<const f32> all_floats, const SDFBufferRange& range,
                _Program& output, RV(*validator)(Span<const f32>, _Program*))
            {
                if(!range.num_floats || range.first_float > all_floats.size() ||
                    range.num_floats > all_floats.size() - range.first_float ||
                    range.first_float / SDF_PROGRAM_PAGE_FLOATS !=
                    (range.first_float + range.num_floats - 1) / SDF_PROGRAM_PAGE_FLOATS)
                {
                    return false;
                }
                Span<const f32> program_floats(all_floats.data() + range.first_float, range.num_floats);
                RV validation = validator(program_floats, &output);
                if(failed(validation)) return false;
                output.floats = range;
                return true;
            }

            f64 perf_elapsed_ms(u64 begin, u64 end)
            {
                return (f64)(end - begin) * 1000.0 / get_ticks_per_second();
            }
        }

        RV Renderer::init(RHI::IDevice* device)
        {
            using namespace RHI;
            lutry
            {
                m_device = device ? device : get_main_device();
                lucheck(m_device);
                m_draw_list = VG::new_shape_draw_list(m_device);
                m_shape_renderer = VG::new_fill_shape_renderer();
                m_font_atlas = VG::new_font_atlas();

                DescriptorSetLayoutBinding bindings[] = {
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::read_buffer_view(1, 1, ShaderVisibilityFlag::pixel),
                    DescriptorSetLayoutBinding::read_buffer_view(2, 1, ShaderVisibilityFlag::pixel),
                    DescriptorSetLayoutBinding::read_buffer_view(3, 1, ShaderVisibilityFlag::vertex)
                };
                luset(m_sdf_descriptor_set_layout,
                    m_device->new_descriptor_set_layout(DescriptorSetLayoutDesc({bindings, 4})));
                IDescriptorSetLayout* descriptor_set_layout = m_sdf_descriptor_set_layout;
                luset(m_sdf_pipeline_layout, m_device->new_pipeline_layout(PipelineLayoutDesc(
                    {&descriptor_set_layout, 1}, PipelineLayoutFlag::allow_input_assembler_input_layout)));

                luset(m_backdrop_descriptor_set_layout,
                    m_device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                        DescriptorSetLayoutBinding::uniform_buffer_view(
                            0, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(
                            TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_write_texture_view(
                            TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::sampler(
                            3, 1, ShaderVisibilityFlag::compute)
                    })));
                IDescriptorSetLayout* backdrop_descriptor_set_layout =
                    m_backdrop_descriptor_set_layout;
                luset(m_backdrop_pipeline_layout,
                    m_device->new_pipeline_layout(PipelineLayoutDesc(
                        {&backdrop_descriptor_set_layout, 1},
                        PipelineLayoutFlag::deny_vertex_shader_access |
                        PipelineLayoutFlag::deny_pixel_shader_access)));
                ComputePipelineStateDesc backdrop_pipeline_desc;
                LUNA_CPPSL_FILL_COMPUTE_SHADER_DATA(backdrop_pipeline_desc, BackdropBlurCS);
                backdrop_pipeline_desc.pipeline_layout = m_backdrop_pipeline_layout;
                luset(m_backdrop_pipeline_state,
                    m_device->new_compute_pipeline_state(backdrop_pipeline_desc));

                Float2U vertices[] = {
                    Float2U(0.0f, 0.0f), Float2U(0.0f, 1.0f),
                    Float2U(1.0f, 1.0f), Float2U(1.0f, 0.0f)
                };
                u16 indices[] = {0, 1, 2, 0, 2, 3};
                luset(m_sdf_vertex_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::vertex_buffer, sizeof(vertices))));
                luset(m_sdf_index_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::index_buffer, sizeof(indices))));
                luset(m_sdf_frame_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::uniform_buffer, sizeof(SDFFrameBuffer))));
                void* mapped_data = nullptr;
                luexp(m_sdf_vertex_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, vertices, sizeof(vertices));
                m_sdf_vertex_buffer->unmap(0, sizeof(vertices));
                luexp(m_sdf_index_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, indices, sizeof(indices));
                m_sdf_index_buffer->unmap(0, sizeof(indices));
            }
            lucatchret;
            return ok;
        }

        RV Renderer::create_sdf_pipeline(RHI::Format render_target_format, RHI::Format depth_stencil_format,
            const RenderSurfaceDesc& surface)
        {
            using namespace RHI;
            lutry
            {
                GraphicsPipelineStateDesc desc;
                InputBindingDesc input_bindings[] = {
                    InputBindingDesc(0, sizeof(Float2U), InputRate::per_vertex),
                    InputBindingDesc(1, sizeof(SDFInstance), InputRate::per_instance)
                };
                InputAttributeDesc input_attributes[] = {
                    InputAttributeDesc(0, 0, 0, Format::rg32_float),
                    InputAttributeDesc(1, 1, offsetof(SDFInstance, draw_rect), Format::rgba32_float),
                    InputAttributeDesc(2, 1, offsetof(SDFInstance, evaluation_origin), Format::rg32_float),
                    InputAttributeDesc(3, 1, offsetof(SDFInstance, program_data), Format::rgba32_uint)
                };
                desc.input_layout = InputLayoutDesc({input_bindings, 2}, {input_attributes, 4});
                desc.pipeline_layout = m_sdf_pipeline_layout;
                desc.vs = LUNA_CPPSL_GET_SHADER_DATA(SDFVS);
                desc.ps = LUNA_CPPSL_GET_SHADER_DATA(SDFPS);
                desc.blend_state = BlendDesc({AttachmentBlendDesc(true,
                    BlendFactor::one, BlendFactor::one_minus_src_alpha, BlendOp::add,
                    BlendFactor::one, BlendFactor::one_minus_src_alpha, BlendOp::add,
                    ColorWriteMask::all)});
                desc.rasterizer_state = RasterizerDesc(FillMode::solid, surface.cull_mode,
                    false, false, false, false, false);
                desc.depth_stencil_state = DepthStencilDesc(surface.depth_test_enable,
                    surface.depth_write_enable, surface.depth_compare_function);
                desc.num_color_attachments = 1;
                desc.color_formats[0] = render_target_format;
                desc.depth_stencil_format = depth_stencil_format;
                luset(m_sdf_pipeline_state, m_device->new_graphics_pipeline_state(desc));
                m_render_target_format = render_target_format;
                m_depth_stencil_format = depth_stencil_format;
                m_depth_test_enable = surface.depth_test_enable;
                m_depth_write_enable = surface.depth_write_enable;
                m_depth_compare_function = surface.depth_compare_function;
                m_cull_mode = surface.cull_mode;
            }
            lucatchret;
            return ok;
        }

        RV Renderer::compile_draw_commands(IContext* context, RHI::ITexture* render_target)
        {
            lutry
            {
                luexp(context->generate_draw_commands());
                const FrameDesc frame_desc = context->get_frame_desc();
                Span<const Layer> layers = context->get_layers();
                Span<const Element> elements = context->get_elements();
                Span<const DrawCommand> commands = context->get_draw_commands();
                Span<const f32> context_shape_floats = context->get_sdf_shape_floats();
                Span<const f32> context_color_floats = context->get_sdf_color_floats();
                Vector<u32> command_capture_indices;
                command_capture_indices.resize(commands.size());
                for(u32& index : command_capture_indices) index = U32_MAX;
                Vector<u32> element_capture_indices;
                element_capture_indices.resize(elements.size());
                for(u32& index : element_capture_indices) index = U32_MAX;
                m_num_backdrop_captures = 0;

                Vector<ClipState> capture_clip_stack;
                for(u32 layer_index = 0; layer_index < layers.size(); ++layer_index)
                {
                    capture_clip_stack.clear();
                    const Layer& layer = layers[layer_index];
                    for(u32 command_index : layer.draw_command_indices)
                    {
                        if(command_index >= commands.size()) continue;
                        const DrawCommand& command = commands[command_index];
                        RectF resolved_rect = resolve_draw_rect(command, elements);
                        RectF element_clip;
                        if(command.element != INVALID_ELEMENT && command.element < elements.size())
                        {
                            element_clip = to_screen_rect(layers, layer_index,
                                elements[command.element].layout_result.clip_rect);
                        }
                        if(command.type == DrawCommandType::push_clip)
                        {
                            RectF requested_clip = to_screen_rect(layers, layer_index, resolved_rect);
                            ClipState pushed_clip = capture_clip_stack.empty() ?
                                ClipState() : capture_clip_stack.back();
                            pushed_clip.rect = merge_clip_rect(pushed_clip.rect, element_clip);
                            pushed_clip.rect = merge_clip_rect(pushed_clip.rect, requested_clip);
                            if(command.radius > 0.0f)
                            {
                                pushed_clip.rounded_rect = requested_clip;
                                pushed_clip.rounded_radii = Float4U(command.radius);
                            }
                            capture_clip_stack.push_back(pushed_clip);
                            continue;
                        }
                        if(command.type == DrawCommandType::pop_clip)
                        {
                            if(!capture_clip_stack.empty()) capture_clip_stack.pop_back();
                            continue;
                        }
                        if(command.type == DrawCommandType::backdrop_blur_capture)
                        {
                            if(command.element == INVALID_ELEMENT ||
                                command.element >= elements.size())
                            {
                                return set_error(E_BAD_DATA,
                                    "A backdrop capture marker must belong to an element.");
                            }
                            if(m_num_backdrop_captures >= m_backdrop_captures.size())
                            {
                                m_backdrop_captures.push_back(BackdropCapture());
                            }
                            u32 capture_index = m_num_backdrop_captures++;
                            BackdropCapture& capture = m_backdrop_captures[capture_index];
                            capture.element = command.element;
                            capture.desc = command.backdrop_blur_capture;
                            capture.consumer_bounds = RectF();
                            capture.source_rect = RectF();
                            capture.used = false;
                            element_capture_indices[command.element] = capture_index;
                            command_capture_indices[command_index] = capture_index;
                            continue;
                        }
                        if(command.type != DrawCommandType::backdrop_blur) continue;
                        if(command.element == INVALID_ELEMENT || command.element >= elements.size())
                        {
                            return set_error(E_BAD_DATA,
                                "A backdrop blur draw command must belong to an element.");
                        }
                        u32 source_element = command.element;
                        u32 capture_index = U32_MAX;
                        while(source_element != INVALID_ELEMENT && source_element < elements.size())
                        {
                            capture_index = element_capture_indices[source_element];
                            if(capture_index != U32_MAX) break;
                            source_element = elements[source_element].parent;
                        }
                        if(capture_index == U32_MAX)
                        {
                            return set_error(E_BAD_DATA,
                                "A backdrop blur draw command has no preceding self-or-ancestor capture.");
                        }
                        command_capture_indices[command_index] = capture_index;
                        RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                        ClipState clip = capture_clip_stack.empty() ?
                            ClipState() : capture_clip_stack.back();
                        clip.rect = merge_clip_rect(clip.rect, element_clip);
                        if(has_clip(clip.rect))
                        {
                            screen_rect = intersect_rect(screen_rect, clip.rect);
                        }
                        if(rect_visible(screen_rect))
                        {
                            BackdropCapture& capture = m_backdrop_captures[capture_index];
                            capture.used = true;
                            capture.consumer_bounds =
                                union_rect(capture.consumer_bounds, screen_rect);
                        }
                    }
                }

                RectF screen_bounds(0.0f, 0.0f, m_logical_width, m_logical_height);
                for(u32 capture_index = 0;
                    capture_index < m_num_backdrop_captures; ++capture_index)
                {
                    BackdropCapture& capture = m_backdrop_captures[capture_index];
                    if(!capture.used) continue;
                    if(!std::isfinite(capture.desc.softness))
                    {
                        return set_error(E_BAD_DATA,
                            "Backdrop blur softness must be finite.");
                    }
                    f32 halo = calculate_backdrop_capture_halo(
                        capture.desc,
                        (f32)m_render_target_width / m_logical_width,
                        (f32)m_render_target_height / m_logical_height);
                    capture.source_rect = intersect_rect(
                        expand_rect(capture.consumer_bounds, halo), screen_bounds);
                    if(!rect_visible(capture.source_rect))
                    {
                        capture.used = false;
                        continue;
                    }
                    luexp(prepare_backdrop_capture(capture, render_target));
                    ++m_counters.backdrop_capture_count;
                }

                m_draw_list->reset();
                m_render_batches.clear();
                m_sdf_instances.clear();
                m_sdf_states.clear();
                m_sdf_page_pairs.clear();
                m_compiled_sdf_shape_floats.clear();
                m_compiled_sdf_color_floats.clear();
                m_compiled_sdf_shape_floats.insert(m_compiled_sdf_shape_floats.end(),
                    context_shape_floats.begin(), context_shape_floats.end());
                m_compiled_sdf_color_floats.insert(m_compiled_sdf_color_floats.end(),
                    context_color_floats.begin(), context_color_floats.end());
                Vector<ClipState> clip_stack;
                RHI::SamplerDesc nearest_sampler_desc;
                nearest_sampler_desc.min_filter = RHI::Filter::nearest;
                nearest_sampler_desc.mag_filter = RHI::Filter::nearest;
                nearest_sampler_desc.mip_filter = RHI::Filter::nearest;
                nearest_sampler_desc.address_u = RHI::TextureAddressMode::clamp;
                nearest_sampler_desc.address_v = RHI::TextureAddressMode::clamp;
                nearest_sampler_desc.address_w = RHI::TextureAddressMode::clamp;
                u32 first_pending_vg_call = 0;

                auto flush_pending_vg = [&]()
                {
                    u32 vg_call_count = (u32)m_draw_list->get_draw_calls().size();
                    if(vg_call_count > first_pending_vg_call)
                    {
                        m_render_batches.push_back({RenderBatchType::vg, first_pending_vg_call,
                            vg_call_count - first_pending_vg_call, 0});
                    }
                    first_pending_vg_call = vg_call_count;
                };

                auto get_page_pair = [&](u32 shape_page, u32 color_page)
                {
                    for(u32 i = 0; i < m_sdf_page_pairs.size(); ++i)
                    {
                        if(m_sdf_page_pairs[i].shape_page == shape_page &&
                            m_sdf_page_pairs[i].color_page == color_page)
                        {
                            return i;
                        }
                    }
                    u32 index = (u32)m_sdf_page_pairs.size();
                    SDFPagePair pair;
                    pair.shape_page = shape_page;
                    pair.color_page = color_page;
                    m_sdf_page_pairs.push_back(move(pair));
                    return index;
                };

                auto resolve_sdf_state = [&](const ClipState& clip)
                {
                    SDFState state;
                    state.clip_rect = Float4U(clip.rect.offset_x, clip.rect.offset_y,
                        clip.rect.width, clip.rect.height);
                    state.rounded_clip_rect = Float4U(clip.rounded_rect.offset_x,
                        clip.rounded_rect.offset_y, clip.rounded_rect.width, clip.rounded_rect.height);
                    state.rounded_clip_radii = clip.rounded_radii;
                    for(u32 i = 0; i < m_sdf_states.size(); ++i)
                    {
                        if(!memcmp(&m_sdf_states[i], &state, sizeof(SDFState))) return i;
                    }
                    u32 state_index = (u32)m_sdf_states.size();
                    m_sdf_states.push_back(state);
                    return state_index;
                };

                auto emit_sdf = [&](const SDFDrawDesc& desc, const Float2U& evaluation_origin,
                    const RectF& raster_domain, const ClipState& clip)
                {
                    const RectF& clip_rect = clip.rect;
                    if(!desc.shape.floats.valid() || !desc.color.floats.valid()) return;
                    u32 shape_page = desc.shape.floats.first_float / SDF_PROGRAM_PAGE_FLOATS;
                    u32 color_page = desc.color.floats.first_float / SDF_PROGRAM_PAGE_FLOATS;
                    u32 local_shape_first = desc.shape.floats.first_float % SDF_PROGRAM_PAGE_FLOATS;
                    u32 local_color_first = desc.color.floats.first_float % SDF_PROGRAM_PAGE_FLOATS;
                    if(local_shape_first + desc.shape.floats.num_floats > SDF_PROGRAM_PAGE_FLOATS ||
                        local_color_first + desc.color.floats.num_floats > SDF_PROGRAM_PAGE_FLOATS)
                    {
                        return;
                    }
                    RectF shape_rect(evaluation_origin.x + desc.shape.bounds.offset_x,
                        evaluation_origin.y + desc.shape.bounds.offset_y,
                        desc.shape.bounds.width, desc.shape.bounds.height);
                    RectF draw_rect(0.0f, 0.0f, 0.0f, 0.0f);
                    if(desc.color.uses_shape_bounds)
                    {
                        draw_rect = expand_rect(shape_rect, desc.color.effect_outsets);
                    }
                    if(desc.color.uses_raster_domain)
                    {
                        RectF domain = rect_visible(raster_domain) ? raster_domain : expand_rect(shape_rect, 1.0f);
                        draw_rect = union_rect(draw_rect, domain);
                    }
                    if(!rect_visible(draw_rect) ||
                        (has_clip(clip_rect) && !rect_visible(intersect_rect(draw_rect, clip_rect))))
                    {
                        return;
                    }
                    SDFInstance instance;
                    instance.draw_rect = Float4U(draw_rect.offset_x, draw_rect.offset_y,
                        draw_rect.width, draw_rect.height);
                    instance.evaluation_origin = evaluation_origin;
                    u32 clip_flags = has_clip(clip_rect) ? 1u : 0u;
                    if(has_rounded_clip(clip)) clip_flags |= 2u;
                    u32 packed_program_data = clip_flags |
                        (desc.color.floats.num_floats << SDF_INSTANCE_COLOR_FLOAT_COUNT_SHIFT);
                    instance.program_data = UInt4U(local_shape_first, local_color_first,
                        packed_program_data, resolve_sdf_state(clip));
                    flush_pending_vg();
                    m_draw_list->draw_call_barrier();
                    u32 pair_index = get_page_pair(shape_page, color_page);
                    u32 instance_index = (u32)m_sdf_instances.size();
                    m_sdf_instances.push_back(instance);
                    if(!m_render_batches.empty() && m_render_batches.back().type == RenderBatchType::sdf &&
                        m_render_batches.back().resource_index == pair_index &&
                        m_render_batches.back().first + m_render_batches.back().count == instance_index)
                    {
                        ++m_render_batches.back().count;
                    }
                    else
                    {
                        m_render_batches.push_back({RenderBatchType::sdf, instance_index, 1, pair_index});
                    }
                };

                for(u32 layer_index = 0; layer_index < layers.size(); ++layer_index)
                {
                    const Layer& layer = layers[layer_index];
                    clip_stack.clear();
                    for(u32 command_index : layer.draw_command_indices)
                    {
                        if(command_index >= commands.size()) continue;
                        const DrawCommand& command = commands[command_index];
                        RectF resolved_rect = resolve_draw_rect(command, elements);
                        RectF element_clip;
                        RectF inherited_clip;
                        if(command.element != INVALID_ELEMENT && command.element < elements.size())
                        {
                            const Element& element = elements[command.element];
                            element_clip = to_screen_rect(layers, layer_index, element.layout_result.clip_rect);
                            if(element.parent != INVALID_ELEMENT && element.parent < elements.size())
                            {
                                inherited_clip = to_screen_rect(layers, layer_index,
                                    elements[element.parent].layout_result.clip_rect);
                            }
                        }
                        if(command.type == DrawCommandType::push_clip)
                        {
                            RectF requested_clip = to_screen_rect(layers, layer_index, resolved_rect);
                            ClipState pushed_clip = clip_stack.empty() ? ClipState() : clip_stack.back();
                            pushed_clip.rect = merge_clip_rect(pushed_clip.rect, element_clip);
                            pushed_clip.rect = merge_clip_rect(pushed_clip.rect, requested_clip);
                            if(command.radius > 0.0f)
                            {
                                pushed_clip.rounded_rect = requested_clip;
                                pushed_clip.rounded_radii = Float4U(command.radius);
                            }
                            clip_stack.push_back(pushed_clip);
                            continue;
                        }
                        if(command.type == DrawCommandType::pop_clip)
                        {
                            if(!clip_stack.empty()) clip_stack.pop_back();
                            continue;
                        }
                        if(command.type == DrawCommandType::backdrop_blur_capture)
                        {
                            u32 capture_index = command_capture_indices[command_index];
                            if(capture_index < m_num_backdrop_captures &&
                                m_backdrop_captures[capture_index].used)
                            {
                                flush_pending_vg();
                                m_draw_list->draw_call_barrier();
                                m_render_batches.push_back({
                                    RenderBatchType::backdrop_capture, 0, 0, capture_index});
                            }
                            continue;
                        }

                        ClipState clip = clip_stack.empty() ? ClipState() : clip_stack.back();
                        clip.rect = merge_clip_rect(clip.rect, element_clip);
                        RectF clip_rect = clip.rect;
                        auto visual_overflow_clip = [&]()
                        {
                            ClipState overflow_clip = clip_stack.empty() ? ClipState() : clip_stack.back();
                            overflow_clip.rect = merge_clip_rect(overflow_clip.rect, inherited_clip);
                            return overflow_clip;
                        };
                        RectF vg_clip = has_clip(clip_rect) ? to_vg_rect(frame_desc, clip_rect) : RectF();
                        m_draw_list->set_clip_rect(vg_clip);
                        RectF vg_rounded_clip = has_rounded_clip(clip) ?
                            to_vg_rect(frame_desc, clip.rounded_rect) : RectF();
                        m_draw_list->set_rounded_clip_rect(vg_rounded_clip, clip.rounded_radii);
                        m_draw_list->set_texture(nullptr);
                        m_draw_list->set_shape_buffer(nullptr);
                        m_draw_list->set_sampler(nullptr);

                        if(command.type == DrawCommandType::backdrop_blur)
                        {
                            u32 capture_index = command_capture_indices[command_index];
                            if(capture_index >= m_num_backdrop_captures ||
                                !m_backdrop_captures[capture_index].used)
                            {
                                continue;
                            }
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if((has_clip(clip_rect) &&
                                !rect_visible(intersect_rect(screen_rect, clip_rect))) ||
                                !rect_visible(screen_rect))
                            {
                                continue;
                            }
                            const BackdropCapture& capture =
                                m_backdrop_captures[capture_index];
                            const RectF& source_rect = capture.source_rect;
                            f32 min_u = (screen_rect.offset_x - source_rect.offset_x) /
                                source_rect.width;
                            f32 max_u = (screen_rect.offset_x + screen_rect.width -
                                source_rect.offset_x) / source_rect.width;
                            f32 min_v = (screen_rect.offset_y - source_rect.offset_y) /
                                source_rect.height;
                            f32 max_v = (screen_rect.offset_y + screen_rect.height -
                                source_rect.offset_y) / source_rect.height;
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            m_draw_list->set_texture(capture.blur_textures[1]);
                            auto& points =
                                m_draw_list->get_shape_buffer()->get_shape_points(true);
                            u32 begin = (u32)points.size();
                            if(command.radius > 0.0f)
                            {
                                VG::ShapeBuilder::add_rounded_rectangle_filled(points,
                                    0.0f, 0.0f, vg_rect.width, vg_rect.height,
                                    command.radius);
                            }
                            else
                            {
                                VG::ShapeBuilder::add_rectangle_filled(points,
                                    0.0f, 0.0f, vg_rect.width, vg_rect.height);
                            }
                            u32 end = (u32)points.size();
                            m_draw_list->draw_shape(begin, end - begin,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                Float2U(vg_rect.offset_x + vg_rect.width,
                                    vg_rect.offset_y + vg_rect.height),
                                Float2U(0.0f), Float2U(vg_rect.width, vg_rect.height),
                                Float4U(1.0f), Float2U(min_u, max_v),
                                Float2U(max_u, min_v));
                            continue;
                        }

                        if(command.type == DrawCommandType::rect ||
                            command.type == DrawCommandType::gradient_rect ||
                            command.type == DrawCommandType::rounded_rect ||
                            command.type == DrawCommandType::shadow)
                        {
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if(!rect_visible(screen_rect)) continue;
                            Vector<f32> shape_floats;
                            if(command.type == DrawCommandType::rounded_rect || command.type == DrawCommandType::shadow)
                            {
                                sdf_shape_add_rounded_rectangle(shape_floats,
                                    RectF(0.0f, 0.0f, screen_rect.width, screen_rect.height),
                                    Float4U(max(command.radius, 0.0f)));
                            }
                            else
                            {
                                sdf_shape_add_rectangle(shape_floats,
                                    RectF(0.0f, 0.0f, screen_rect.width, screen_rect.height));
                            }
                            SDFDrawDesc desc;
                            if(!append_shape_program(m_compiled_sdf_shape_floats,
                                shape_floats.cspan(), desc.shape))
                                continue;
                            Vector<f32> color_floats;
                            if(command.type == DrawCommandType::gradient_rect)
                            {
                                sdf_color_add_bilinear_gradient(color_floats,
                                    RectF(0.0f, 0.0f, screen_rect.width, screen_rect.height), command.color,
                                    command.color_top_right, command.color_bottom_right,
                                    command.color_bottom_left);
                            }
                            else if(command.type == DrawCommandType::shadow)
                            {
                                SDFClipDesc shadow_clip = command.shadow.mode == ShadowMode::inner ?
                                    SDFClipDesc::outer(0.0f) : SDFClipDesc::inner(0.0f);
                                sdf_color_add_shadow(color_floats, command.color, command.shadow.offset,
                                    command.shadow.softness, command.shadow.spread, shadow_clip);
                            }
                            else
                            {
                                sdf_color_add_solid(color_floats, command.color);
                            }
                            if(!append_color_program(m_compiled_sdf_color_floats,
                                color_floats.cspan(), desc.color))
                                continue;
                            ClipState sdf_clip = command.type == DrawCommandType::shadow &&
                                command.shadow.mode == ShadowMode::outer ? visual_overflow_clip() : clip;
                            emit_sdf(desc, Float2U(screen_rect.offset_x, screen_rect.offset_y),
                                screen_rect, sdf_clip);
                            continue;
                        }

                        if(command.type == DrawCommandType::sdf)
                        {
                            SDFDrawDesc desc = command.sdf;
                            if(!resolve_context_program(context_shape_floats, command.sdf.shape.floats,
                                desc.shape, validate_sdf_shape_program) ||
                                !resolve_context_program(context_color_floats, command.sdf.color.floats,
                                    desc.color, validate_sdf_color_program))
                            {
                                continue;
                            }
                            RectF screen_anchor = to_screen_rect(layers, layer_index, resolved_rect);
                            ClipState sdf_clip = desc.color.may_paint_outside ?
                                visual_overflow_clip() : clip;
                            emit_sdf(desc, Float2U(screen_anchor.offset_x, screen_anchor.offset_y),
                                screen_anchor, sdf_clip);
                            continue;
                        }

                        switch(command.type)
                        {
                        case DrawCommandType::image:
                        {
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if((has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) ||
                                !rect_visible(screen_rect) || !color_visible(command.color))
                            {
                                break;
                            }
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            m_draw_list->set_texture(command.texture);
                            if(command.nearest_sampler) m_draw_list->set_sampler(&nearest_sampler_desc);
                            auto& points = m_draw_list->get_shape_buffer()->get_shape_points(true);
                            u32 begin = (u32)points.size();
                            VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f,
                                vg_rect.width, vg_rect.height);
                            u32 end = (u32)points.size();
                            m_draw_list->draw_shape(begin, end - begin,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                Float2U(0.0f), Float2U(vg_rect.width, vg_rect.height), command.color,
                                command.min_texcoord, command.max_texcoord);
                            break;
                        }
                        case DrawCommandType::line:
                        {
                            if(!color_visible(command.color) || command.line_width <= 0.0f) break;
                            Float2U resolved_p0 = resolve_draw_point0(command, elements);
                            Float2U resolved_p1 = resolve_draw_point1(command, elements);
                            Float2U p0(layer.screen_position.x + resolved_p0.x,
                                layer.screen_position.y + resolved_p0.y);
                            Float2U p1(layer.screen_position.x + resolved_p1.x,
                                layer.screen_position.y + resolved_p1.y);
                            f32 margin = max(command.line_width, 1.0f);
                            f32 dx = abs(p1.x - p0.x);
                            f32 dy = abs(p1.y - p0.y);
                            RectF bounds(min(p0.x, p1.x) - margin, min(p0.y, p1.y) - margin,
                                max(dx + margin * 2.0f, 1.0f), max(dy + margin * 2.0f, 1.0f));
                            if(has_clip(clip_rect) && !rect_visible(intersect_rect(bounds, clip_rect))) break;
                            RectF vg_rect = to_vg_rect(frame_desc, bounds);
                            auto& points = m_draw_list->get_shape_buffer()->get_shape_points(true);
                            u32 begin = (u32)points.size();
                            Float2U local_p0(p0.x - bounds.offset_x, bounds.height - (p0.y - bounds.offset_y));
                            Float2U local_p1(p1.x - bounds.offset_x, bounds.height - (p1.y - bounds.offset_y));
                            VG::ShapeBuilder::add_line(points, local_p0.x, local_p0.y,
                                local_p1.x, local_p1.y, command.line_width);
                            u32 end = (u32)points.size();
                            m_draw_list->draw_shape(begin, end - begin,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                Float2U(0.0f), Float2U(vg_rect.width, vg_rect.height), command.color,
                                Float2U(0.0f), Float2U(1.0f));
                            break;
                        }
                        case DrawCommandType::text:
                        {
                            if(command.text.empty() || command.font_size <= 0.0f || !color_visible(command.color))
                                break;
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if((has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) ||
                                !rect_visible(screen_rect)) break;
                            FontDesc font = context->get_font(command.font);
                            if(!font.font)
                            {
                                font.font = Font::get_default_font();
                                font.font_index = 0;
                            }
                            VG::TextArrangeSection section;
                            section.font_file = font.font;
                            section.font_index = font.font_index;
                            section.font_size = command.font_size;
                            section.color = command.color;
                            section.num_chars = command.text.size();
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            VG::TextArrangeResult arranged = VG::arrange_text(command.text.c_str(), command.text.size(),
                                Span<const VG::TextArrangeSection>(&section, 1), vg_rect,
                                command.vertical_alignment, command.horizontal_alignment);
                            VG::commit_text_arrange_result(arranged,
                                Span<const VG::TextArrangeSection>(&section, 1), m_font_atlas,
                                m_draw_list);
                            break;
                        }
                        case DrawCommandType::shape:
                        {
                            if(!command.shape.buffer || command.shape.num_commands == 0 ||
                                !rect_visible(command.shape.bounds) || !rect_visible(resolved_rect) ||
                                !color_visible(command.color)) break;
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if(has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) break;
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            m_draw_list->set_shape_buffer(command.shape.buffer);
                            m_draw_list->set_texture(command.shape.texture);
                            if(command.shape.texture && command.nearest_sampler)
                                m_draw_list->set_sampler(&nearest_sampler_desc);
                            Float2U shape_min(command.shape.bounds.offset_x,
                                command.shape.bounds.offset_y + command.shape.bounds.height);
                            Float2U shape_max(command.shape.bounds.offset_x + command.shape.bounds.width,
                                command.shape.bounds.offset_y);
                            m_draw_list->draw_shape(command.shape.first_command, command.shape.num_commands,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                shape_min, shape_max, command.color,
                                command.min_texcoord, command.max_texcoord);
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
                m_draw_list->set_texture(nullptr);
                m_draw_list->set_shape_buffer(nullptr);
                m_draw_list->set_sampler(nullptr);
                m_draw_list->set_clip_rect(RectF());
                m_draw_list->set_rounded_clip_rect(RectF(), Float4U(0.0f));
                flush_pending_vg();
                luexp(m_draw_list->compile());
                m_counters.vg_draw_call_count = (u32)m_draw_list->get_draw_calls().size();
                m_counters.sdf_instance_count = (u32)m_sdf_instances.size();
                m_counters.sdf_state_count = (u32)m_sdf_states.size();
                m_counters.sdf_shape_float_count = (u32)m_compiled_sdf_shape_floats.size();
                m_counters.sdf_color_float_count = (u32)m_compiled_sdf_color_floats.size();
                m_counters.sdf_program_page_count =
                    (u32)((m_compiled_sdf_shape_floats.size() + SDF_PROGRAM_PAGE_FLOATS - 1) /
                        SDF_PROGRAM_PAGE_FLOATS) +
                    (u32)((m_compiled_sdf_color_floats.size() + SDF_PROGRAM_PAGE_FLOATS - 1) /
                        SDF_PROGRAM_PAGE_FLOATS);
                m_counters.sdf_page_pair_count = (u32)m_sdf_page_pairs.size();
                m_counters.sdf_instance_upload_bytes = m_sdf_instances.size() * sizeof(SDFInstance);
                m_counters.sdf_state_upload_bytes = m_sdf_states.size() * sizeof(SDFState);
                m_counters.sdf_upload_bytes = m_counters.sdf_instance_upload_bytes +
                    m_counters.sdf_state_upload_bytes +
                    (m_compiled_sdf_shape_floats.size() + m_compiled_sdf_color_floats.size()) * sizeof(f32);
                m_counters.sdf_draw_call_count = 0;
                for(const RenderBatch& batch : m_render_batches)
                {
                    if(batch.type == RenderBatchType::sdf) ++m_counters.sdf_draw_call_count;
                }
                m_counters.backend_switch_count = 0;
                RenderBatchType previous_backend = RenderBatchType::backdrop_capture;
                for(const RenderBatch& batch : m_render_batches)
                {
                    if(batch.type == RenderBatchType::backdrop_capture) continue;
                    if(previous_backend != RenderBatchType::backdrop_capture &&
                        previous_backend != batch.type)
                    {
                        ++m_counters.backend_switch_count;
                    }
                    previous_backend = batch.type;
                }
                m_counters.render_batch_count = (u32)m_render_batches.size();
            }
            lucatchret;
            return ok;
        }

        RV Renderer::prepare_backdrop_capture(BackdropCapture& capture,
            RHI::ITexture* render_target)
        {
            using namespace RHI;
            lutry
            {
                TextureDesc target_desc = render_target->get_desc();
                if(!test_flags(target_desc.usages, TextureUsageFlag::read_texture))
                {
                    return set_error(E_NOT_SUPPORTED,
                        "Backdrop capture requires a color target with read_texture usage.");
                }
                Format intermediate_format =
                    select_backdrop_intermediate_format(target_desc.format);
                if(intermediate_format == Format::unknown)
                {
                    return set_error(E_NOT_SUPPORTED,
                        "The GUI color-target format does not have a supported "
                        "backdrop-filtering intermediate format.");
                }

                f32 render_scale_x = (f32)m_render_target_width / m_logical_width;
                f32 render_scale_y = (f32)m_render_target_height / m_logical_height;
                u32 source_width = max<u32>((u32)ceil(
                    capture.source_rect.width * render_scale_x), 1);
                u32 source_height = max<u32>((u32)ceil(
                    capture.source_rect.height * render_scale_y), 1);
                u32 requested_downsample_passes =
                    calculate_backdrop_downsample_passes(capture.desc,
                        render_scale_x, render_scale_y);
                u32 width = source_width;
                u32 height = source_height;
                capture.downsample_pass_count = 0;
                while(capture.downsample_pass_count <
                    requested_downsample_passes && (width > 1 || height > 1))
                {
                    width = max(width >> 1, 1u);
                    height = max(height >> 1, 1u);
                    ++capture.downsample_pass_count;
                }
                u32 downsample_factor = 1u << capture.downsample_pass_count;
                f32 horizontal_sigma =
                    quantize_backdrop_gaussian_sigma(
                        max(capture.desc.softness, 0.0f) *
                        render_scale_x /
                        (f32)downsample_factor);
                f32 vertical_sigma =
                    quantize_backdrop_gaussian_sigma(
                        max(capture.desc.softness, 0.0f) *
                        render_scale_y /
                        (f32)downsample_factor);
                capture.horizontal_blur = width > 1 &&
                    horizontal_sigma > BACKDROP_GAUSSIAN_FAST_PATH_SIGMA;
                capture.vertical_blur = height > 1 &&
                    vertical_sigma > BACKDROP_GAUSSIAN_FAST_PATH_SIGMA;
                u32 gaussian_pass_count =
                    (capture.horizontal_blur ? 1u : 0u) +
                    (capture.vertical_blur ? 1u : 0u);
                capture.filter_pass_count =
                    capture.downsample_pass_count + gaussian_pass_count;
                if(!capture.filter_pass_count)
                {
                    capture.filter_pass_count = 1;
                }
                luassert(capture.filter_pass_count <=
                    MAX_BACKDROP_FILTER_PASSES);

                auto ensure_texture = [&](Ref<ITexture>& texture,
                    const TextureDesc& required_desc) -> RV
                {
                    bool recreate_texture = !texture;
                    if(texture)
                    {
                        TextureDesc existing_desc = texture->get_desc();
                        recreate_texture =
                            existing_desc.type != required_desc.type ||
                            existing_desc.format != required_desc.format ||
                            existing_desc.width != required_desc.width ||
                            existing_desc.height != required_desc.height ||
                            existing_desc.depth != required_desc.depth ||
                            existing_desc.array_size != required_desc.array_size ||
                            existing_desc.mip_levels != required_desc.mip_levels ||
                            existing_desc.sample_count !=
                                required_desc.sample_count ||
                            existing_desc.usages != required_desc.usages;
                    }
                    if(recreate_texture)
                    {
                        auto result = m_device->new_texture(
                            MemoryType::local, required_desc);
                        if(failed(result)) return result.errcode();
                        texture = result.get();
                    }
                    return ok;
                };

                usize downsample_pixels = 0;
                if(capture.downsample_pass_count)
                {
                    u32 base_width = max(source_width >> 1, 1u);
                    u32 base_height = max(source_height >> 1, 1u);
                    TextureDesc downsample_desc = TextureDesc::tex2d(
                        intermediate_format,
                        TextureUsageFlag::read_texture |
                            TextureUsageFlag::read_write_texture,
                        base_width, base_height, 1,
                        capture.downsample_pass_count);
                    luexp(ensure_texture(
                        capture.downsample_texture, downsample_desc));
                    for(u32 i = 0; i < capture.downsample_pass_count; ++i)
                    {
                        downsample_pixels +=
                            (usize)max(base_width >> i, 1u) *
                            (usize)max(base_height >> i, 1u);
                    }
                }
                else
                {
                    capture.downsample_texture = nullptr;
                }

                TextureDesc blur_desc = TextureDesc::tex2d(intermediate_format,
                    TextureUsageFlag::read_texture |
                        TextureUsageFlag::read_write_texture,
                    width, height, 1, 1);
                for(u32 i = 0; i < 2; ++i)
                {
                    luexp(ensure_texture(capture.blur_textures[i], blur_desc));
                }

                u32 uniform_alignment = (u32)m_device->check_feature(
                    DeviceFeature::uniform_buffer_data_alignment).
                    uniform_buffer_data_alignment;
                u32 parameter_buffer_size = (u32)align_upper(
                    sizeof(BackdropBlurParams), uniform_alignment);
                u32 pass_count = capture.filter_pass_count;
                for(u32 i = 0; i < pass_count; ++i)
                {
                    if(!capture.descriptor_sets[i])
                    {
                        luset(capture.descriptor_sets[i],
                            m_device->new_descriptor_set(DescriptorSetDesc(
                                m_backdrop_descriptor_set_layout)));
                    }
                    if(!capture.parameter_buffers[i] ||
                        capture.parameter_buffers[i]->get_desc().size <
                            parameter_buffer_size)
                    {
                        luset(capture.parameter_buffers[i],
                            m_device->new_buffer(MemoryType::upload,
                                BufferDesc(BufferUsageFlag::uniform_buffer,
                                    parameter_buffer_size)));
                    }
                }

                SamplerDesc sampler(Filter::linear, Filter::linear, Filter::nearest,
                    TextureAddressMode::clamp, TextureAddressMode::clamp,
                    TextureAddressMode::clamp);

                u32 pass_index = 0;
                auto configure_pass = [&](const BackdropBlurParams& parameters,
                    ITexture* source, u32 source_mip,
                    ITexture* destination, u32 destination_mip) -> RV
                {
                    void* mapped_data = nullptr;
                    RV map_result =
                        capture.parameter_buffers[pass_index]->map(
                            0, parameter_buffer_size, &mapped_data);
                    if(failed(map_result)) return map_result;
                    memcpy(mapped_data, &parameters,
                        sizeof(BackdropBlurParams));
                    capture.parameter_buffers[pass_index]->unmap(
                        0, sizeof(BackdropBlurParams));
                    RV update_result =
                        capture.descriptor_sets[pass_index]->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0,
                            BufferViewDesc::uniform_buffer(
                                capture.parameter_buffers[pass_index], 0,
                                parameter_buffer_size)),
                        WriteDescriptorSet::read_texture_view(1,
                            TextureViewDesc::tex2d(
                                source, Format::unknown, source_mip, 1)),
                        WriteDescriptorSet::read_write_texture_view(2,
                            TextureViewDesc::tex2d(
                                destination, Format::unknown,
                                destination_mip, 1)),
                        WriteDescriptorSet::sampler(3, sampler)
                    });
                    if(failed(update_result)) return update_result;
                    capture.filter_passes[pass_index] = {
                        source, destination, source_mip, destination_mip,
                        parameters.destination_size
                    };
                    ++pass_index;
                    return ok;
                };

                if(capture.downsample_pass_count)
                {
                    TextureDesc downsample_desc =
                        capture.downsample_texture->get_desc();
                    for(u32 i = 0; i < capture.downsample_pass_count; ++i)
                    {
                        BackdropBlurParams parameters{};
                        parameters.destination_size = UInt2U(
                            max(downsample_desc.width >> i, 1u),
                            max(downsample_desc.height >> i, 1u));
                        parameters.source_uv_origin = i ?
                            Float2U(0.0f) : Float2U(
                                capture.source_rect.offset_x / m_logical_width,
                                capture.source_rect.offset_y / m_logical_height);
                        parameters.source_uv_size = i ?
                            Float2U(1.0f) : Float2U(
                                capture.source_rect.width / m_logical_width,
                                capture.source_rect.height / m_logical_height);
                        parameters.filter_mode = 1;
                        bool write_final_texture =
                            !gaussian_pass_count &&
                            i + 1 == capture.downsample_pass_count;
                        luexp(configure_pass(parameters,
                            i ? capture.downsample_texture.get() :
                                render_target,
                            i ? i - 1 : 0,
                            write_final_texture ?
                                capture.blur_textures[1].get() :
                                capture.downsample_texture.get(),
                            write_final_texture ? 0 : i));
                    }
                }

                ITexture* base_source = capture.downsample_pass_count ?
                    capture.downsample_texture.get() : render_target;
                u32 base_source_mip = capture.downsample_pass_count ?
                    capture.downsample_pass_count - 1 : 0;
                Float2U base_source_uv_origin = capture.downsample_pass_count ?
                    Float2U(0.0f) : Float2U(
                        capture.source_rect.offset_x / m_logical_width,
                        capture.source_rect.offset_y / m_logical_height);
                Float2U base_source_uv_size = capture.downsample_pass_count ?
                    Float2U(1.0f) : Float2U(
                        capture.source_rect.width / m_logical_width,
                        capture.source_rect.height / m_logical_height);

                if(!capture.downsample_pass_count && !gaussian_pass_count)
                {
                    BackdropBlurParams snapshot_parameters{};
                    snapshot_parameters.destination_size =
                        UInt2U(width, height);
                    snapshot_parameters.source_uv_origin =
                        base_source_uv_origin;
                    snapshot_parameters.source_uv_size =
                        base_source_uv_size;
                    luexp(configure_pass(snapshot_parameters,
                        base_source, base_source_mip,
                        capture.blur_textures[1], 0));
                }

                if(capture.horizontal_blur)
                {
                    BackdropBlurParams horizontal_parameters{};
                    horizontal_parameters.destination_size =
                        UInt2U(width, height);
                    horizontal_parameters.source_uv_origin =
                        base_source_uv_origin;
                    horizontal_parameters.source_uv_size =
                        base_source_uv_size;
                    horizontal_parameters.sample_step = Float2U(
                        capture.downsample_pass_count ?
                            1.0f / (f32)width :
                            1.0f / (f32)m_render_target_width,
                        0.0f);
                    horizontal_parameters.filter_mode = 2;
                    set_backdrop_gaussian_kernel(horizontal_parameters,
                        build_backdrop_gaussian_kernel(horizontal_sigma));
                    luexp(configure_pass(horizontal_parameters,
                        base_source, base_source_mip,
                        capture.vertical_blur ?
                            capture.blur_textures[0].get() :
                            capture.blur_textures[1].get(),
                        0));
                }

                if(capture.vertical_blur)
                {
                    BackdropBlurParams vertical_parameters{};
                    vertical_parameters.destination_size =
                        UInt2U(width, height);
                    bool reads_horizontal_result = capture.horizontal_blur;
                    vertical_parameters.source_uv_origin =
                        reads_horizontal_result ?
                        Float2U(0.0f) : base_source_uv_origin;
                    vertical_parameters.source_uv_size =
                        reads_horizontal_result ?
                        Float2U(1.0f) : base_source_uv_size;
                    vertical_parameters.sample_step = Float2U(0.0f,
                        reads_horizontal_result ||
                            capture.downsample_pass_count ?
                            1.0f / (f32)height :
                            1.0f / (f32)m_render_target_height);
                    vertical_parameters.filter_mode = 2;
                    set_backdrop_gaussian_kernel(vertical_parameters,
                        build_backdrop_gaussian_kernel(vertical_sigma));
                    luexp(configure_pass(vertical_parameters,
                        reads_horizontal_result ?
                            capture.blur_textures[0].get() :
                            base_source,
                        reads_horizontal_result ? 0 : base_source_mip,
                        capture.blur_textures[1], 0));
                }
                luassert(pass_index == pass_count);

                usize bytes_per_pixel =
                    bits_per_pixel(intermediate_format) / 8;
                m_counters.backdrop_temporary_texture_bytes +=
                    (downsample_pixels +
                        (usize)width * (usize)height * 2u) *
                    bytes_per_pixel;
                m_counters.backdrop_filtered_pixel_count +=
                    (u64)downsample_pixels +
                    (u64)width * (u64)height *
                        (u64)gaussian_pass_count;
                if(!capture.downsample_pass_count &&
                    !gaussian_pass_count)
                {
                    m_counters.backdrop_filtered_pixel_count +=
                        (u64)width * (u64)height;
                }
                m_counters.backdrop_blur_dispatch_count += pass_count;
            }
            lucatchret;
            return ok;
        }

        RV Renderer::prepare_sdf_resources()
        {
            using namespace RHI;
            if(m_sdf_instances.empty()) return ok;
            if(m_sdf_states.empty())
                return set_error(E_BAD_DATA, "SDF instances were compiled without raster states.");
            lutry
            {
                SDFFrameBuffer frame_data;
                frame_data.surface_to_clip = m_surface_to_clip;
                void* mapped_data = nullptr;
                luexp(m_sdf_frame_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, &frame_data, sizeof(frame_data));
                m_sdf_frame_buffer->unmap(0, sizeof(frame_data));

                if(m_sdf_instance_capacity < m_sdf_instances.size())
                {
                    luset(m_sdf_instance_buffer, m_device->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::vertex_buffer,
                            sizeof(SDFInstance) * m_sdf_instances.size())));
                    m_sdf_instance_capacity = m_sdf_instances.size();
                }
                usize instance_bytes = sizeof(SDFInstance) * m_sdf_instances.size();
                luexp(m_sdf_instance_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, m_sdf_instances.data(), instance_bytes);
                m_sdf_instance_buffer->unmap(0, instance_bytes);

                if(m_sdf_state_capacity < m_sdf_states.size())
                {
                    luset(m_sdf_state_buffer, m_device->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::read_buffer,
                            sizeof(SDFState) * m_sdf_states.size())));
                    m_sdf_state_capacity = m_sdf_states.size();
                }
                usize state_bytes = sizeof(SDFState) * m_sdf_states.size();
                luexp(m_sdf_state_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, m_sdf_states.data(), state_bytes);
                m_sdf_state_buffer->unmap(0, state_bytes);

                auto upload_pages = [&](const Vector<f32>& floats, Vector<SDFProgramPage>& pages) -> RV
                {
                    usize num_pages = (floats.size() + SDF_PROGRAM_PAGE_FLOATS - 1) /
                        SDF_PROGRAM_PAGE_FLOATS;
                    pages.resize(num_pages);
                    for(usize page_index = 0; page_index < num_pages; ++page_index)
                    {
                        usize first_float = page_index * SDF_PROGRAM_PAGE_FLOATS;
                        u32 num_floats = (u32)min<usize>(SDF_PROGRAM_PAGE_FLOATS,
                            floats.size() - first_float);
                        usize num_bytes = (usize)num_floats * sizeof(f32);
                        SDFProgramPage& page = pages[page_index];
                        if(!page.buffer || page.buffer->get_desc().size < num_bytes)
                        {
                            auto buffer_result = m_device->new_buffer(MemoryType::upload,
                                BufferDesc(BufferUsageFlag::read_buffer, num_bytes));
                            if(failed(buffer_result)) return buffer_result.errcode();
                            page.buffer = buffer_result.get();
                        }
                        page.num_floats = num_floats;
                        void* page_data = nullptr;
                        RV map_result = page.buffer->map(0, 0, &page_data);
                        if(failed(map_result)) return map_result;
                        memcpy(page_data, floats.data() + first_float, num_bytes);
                        page.buffer->unmap(0, num_bytes);
                    }
                    return ok;
                };
                luexp(upload_pages(m_compiled_sdf_shape_floats, m_sdf_shape_pages));
                luexp(upload_pages(m_compiled_sdf_color_floats, m_sdf_color_pages));

                for(SDFPagePair& pair : m_sdf_page_pairs)
                {
                    if(pair.shape_page >= m_sdf_shape_pages.size() ||
                        pair.color_page >= m_sdf_color_pages.size())
                    {
                        return set_error(E_BAD_DATA, "SDF draw references an unavailable program page.");
                    }
                    lulet(descriptor_set, m_device->new_descriptor_set(
                        RHI::DescriptorSetDesc(m_sdf_descriptor_set_layout)));
                    pair.descriptor_set = descriptor_set;
                    SDFProgramPage& shape_page = m_sdf_shape_pages[pair.shape_page];
                    SDFProgramPage& color_page = m_sdf_color_pages[pair.color_page];
                    luexp(pair.descriptor_set->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0,
                            BufferViewDesc::uniform_buffer(m_sdf_frame_buffer, 0, sizeof(SDFFrameBuffer))),
                        WriteDescriptorSet::read_buffer_view(1,
                            BufferViewDesc::structured_buffer(shape_page.buffer, 0,
                                shape_page.num_floats, sizeof(f32))),
                        WriteDescriptorSet::read_buffer_view(2,
                            BufferViewDesc::structured_buffer(color_page.buffer, 0,
                                color_page.num_floats, sizeof(f32))),
                        WriteDescriptorSet::read_buffer_view(3,
                            BufferViewDesc::structured_buffer(m_sdf_state_buffer, 0,
                                (u32)m_sdf_states.size(), sizeof(SDFState)))
                    }));
                }
            }
            lucatchret;
            return ok;
        }

        void Renderer::render_sdf(RHI::ICommandBuffer* cmdbuf, const RenderBatch& batch)
        {
            using namespace RHI;
            if(batch.resource_index >= m_sdf_page_pairs.size() || !batch.count) return;
            cmdbuf->set_graphics_pipeline_state(m_sdf_pipeline_state);
            cmdbuf->set_graphics_pipeline_layout(m_sdf_pipeline_layout);
            cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)m_render_target_width,
                (f32)m_render_target_height, 0.0f, 1.0f));
            cmdbuf->set_scissor_rect(RectI(0, 0, m_render_target_width, m_render_target_height));
            cmdbuf->set_graphics_descriptor_set(0,
                m_sdf_page_pairs[batch.resource_index].descriptor_set);
            VertexBufferView vertex_views[] = {
                VertexBufferView(m_sdf_vertex_buffer, 0, sizeof(Float2U) * 4, sizeof(Float2U)),
                VertexBufferView(m_sdf_instance_buffer, 0,
                    sizeof(SDFInstance) * m_sdf_instances.size(), sizeof(SDFInstance))
            };
            cmdbuf->set_vertex_buffers(0, {vertex_views, 2});
            cmdbuf->set_index_buffer(IndexBufferView(m_sdf_index_buffer, 0,
                sizeof(u16) * 6, Format::r16_uint));
            cmdbuf->draw_indexed_instanced(6, batch.count, 0, 0, batch.first);
        }

        RV Renderer::render(IContext* context, RHI::ICommandBuffer* cmdbuf,
            const RenderTargetDesc& target, const RenderSurfaceDesc& surface)
        {
            using namespace RHI;
            RHI::ITexture* render_target = target.color_texture;
            if(!context || !cmdbuf || !render_target) return E_BAD_ARGUMENTS;
            if(surface.depth_write_enable && !surface.depth_test_enable)
            {
                return set_error(E_BAD_ARGUMENTS,
                    "GUI depth writes require depth testing to be enabled.");
            }
            if((surface.depth_test_enable || surface.depth_write_enable) &&
                !target.depth_stencil_texture)
            {
                return set_error(E_BAD_ARGUMENTS,
                    "GUI depth testing requires a depth-stencil texture.");
            }
            auto render_target_desc = render_target->get_desc();
            if(render_target_desc.type != RHI::TextureType::tex2d || render_target_desc.sample_count != 1 ||
                !test_flags(render_target_desc.usages, RHI::TextureUsageFlag::color_attachment))
            {
                return set_error(E_BAD_ARGUMENTS,
                    "The GUI color target must be a single-sample 2D color attachment.");
            }
            RHI::Format depth_stencil_format = RHI::Format::unknown;
            if(target.depth_stencil_texture)
            {
                auto depth_desc = target.depth_stencil_texture->get_desc();
                if(depth_desc.type != RHI::TextureType::tex2d || depth_desc.sample_count != 1 ||
                    depth_desc.width != render_target_desc.width || depth_desc.height != render_target_desc.height ||
                    !test_flags(depth_desc.usages, RHI::TextureUsageFlag::depth_stencil_attachment))
                {
                    return set_error(E_BAD_ARGUMENTS,
                        "The GUI depth-stencil target must match the color target extent and sample count.");
                }
                depth_stencil_format = depth_desc.format;
            }
            if(test_flags(target.color_final_state,
                    RHI::TextureStateFlag::automatic) ||
                target.color_final_state == RHI::TextureStateFlag::none ||
                (target.depth_stencil_texture &&
                    (test_flags(target.depth_stencil_final_state,
                        RHI::TextureStateFlag::automatic) ||
                    target.depth_stencil_final_state == RHI::TextureStateFlag::none)))
            {
                return set_error(E_BAD_ARGUMENTS,
                    "GUI attachment final states must be concrete resource states.");
            }
            if(render_target_desc.format != m_render_target_format ||
                depth_stencil_format != m_depth_stencil_format ||
                surface.depth_test_enable != m_depth_test_enable ||
                surface.depth_write_enable != m_depth_write_enable ||
                surface.depth_compare_function != m_depth_compare_function ||
                surface.cull_mode != m_cull_mode)
            {
                RV result = create_sdf_pipeline(render_target_desc.format, depth_stencil_format, surface);
                if(failed(result)) return result;
            }
            m_render_target_width = render_target_desc.width;
            m_render_target_height = render_target_desc.height;
            const FrameDesc frame_desc = context->get_frame_desc();
            m_logical_width = max(frame_desc.logical_size.x, 1.0f);
            m_logical_height = max(frame_desc.logical_size.y, 1.0f);
            m_surface_to_clip = surface.use_custom_transform ? surface.surface_to_clip :
                Float4x4U(ProjectionMatrix::make_orthographic_off_center(
                    0.0f, m_logical_width, m_logical_height, 0.0f, 0.0f, 1.0f));
            m_counters = RendererPerformanceCounters();
            u64 render_begin = get_ticks();
            lutry
            {
                luexp(compile_draw_commands(context, render_target));
                if(surface.use_custom_transform &&
                    m_counters.backdrop_capture_count)
                {
                    return set_error(E_NOT_SUPPORTED,
                        "Backdrop capture is only supported for the default orthographic GUI surface.");
                }
                VG::ShapeRendererPassDesc vg_pass;
                vg_pass.render_target = render_target;
                vg_pass.depth_stencil_format = depth_stencil_format;
                vg_pass.depth_test_enable = surface.depth_test_enable;
                vg_pass.depth_write_enable = surface.depth_write_enable;
                vg_pass.depth_compare_function = surface.depth_compare_function;
                vg_pass.cull_mode = surface.cull_mode;
                luexp(m_shape_renderer->begin(vg_pass));
                // GUI SDF programs use top-left surface coordinates, while the VG draw list keeps its
                // historical bottom-left coordinates. Convert VG positions before applying the public surface
                // transform so both backends have identical clip-space output.
                Float4x4 vg_to_surface(
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, m_logical_height, 0.0f, 1.0f);
                Float4x4U vg_surface_to_clip = mul(vg_to_surface,
                    m_surface_to_clip.to_float4x4());
                m_shape_renderer->draw(m_draw_list->get_instance_buffer(),
                    m_draw_list->get_state_buffer(),
                    m_draw_list->get_draw_calls(), &vg_surface_to_clip);
                luexp(m_shape_renderer->end());
                luexp(prepare_sdf_resources());
                if(!m_sdf_instances.empty())
                {
                    Vector<RHI::BufferBarrier> barriers;
                    barriers.push_back(RHI::BufferBarrier(m_sdf_vertex_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::vertex_buffer));
                    barriers.push_back(RHI::BufferBarrier(m_sdf_index_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::index_buffer));
                    barriers.push_back(RHI::BufferBarrier(m_sdf_instance_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::vertex_buffer));
                    barriers.push_back(RHI::BufferBarrier(m_sdf_state_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::shader_read_vs));
                    for(const SDFProgramPage& page : m_sdf_shape_pages)
                    {
                        barriers.push_back(RHI::BufferBarrier(page.buffer,
                            RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::shader_read_ps));
                    }
                    for(const SDFProgramPage& page : m_sdf_color_pages)
                    {
                        barriers.push_back(RHI::BufferBarrier(page.buffer,
                            RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::shader_read_ps));
                    }
                    cmdbuf->resource_barrier(barriers.cspan(), {});
                }
                m_shape_renderer->prepare(cmdbuf);
                Vector<RHI::TextureBarrier> attachment_barriers;
                attachment_barriers.push_back(RHI::TextureBarrier(render_target,
                    RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic,
                    RHI::TextureStateFlag::color_attachment_write));
                if(target.depth_stencil_texture)
                {
                    attachment_barriers.push_back(RHI::TextureBarrier(target.depth_stencil_texture,
                        RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic,
                        surface.depth_write_enable ? RHI::TextureStateFlag::depth_stencil_attachment_write :
                            RHI::TextureStateFlag::depth_stencil_attachment_read));
                }
                cmdbuf->resource_barrier({}, attachment_barriers.cspan());

                auto begin_gui_pass = [&](bool first_pass, bool last_pass)
                {
                    RHI::RenderPassDesc pass;
                    pass.color_attachments[0] = RHI::ColorAttachment(render_target,
                        first_pass ? target.color_load_op : RHI::LoadOp::load,
                        RHI::StoreOp::store, target.color_clear_value);
                    if(target.depth_stencil_texture)
                    {
                        pass.depth_stencil_attachment = RHI::DepthStencilAttachment(
                            target.depth_stencil_texture, !surface.depth_write_enable,
                            first_pass ? target.depth_load_op : RHI::LoadOp::load,
                            last_pass ? target.depth_store_op : RHI::StoreOp::store,
                            target.depth_clear_value,
                            first_pass ? target.stencil_load_op : RHI::LoadOp::load,
                            last_pass ? target.stencil_store_op : RHI::StoreOp::store,
                            target.stencil_clear_value);
                    }
                    cmdbuf->begin_render_pass(pass);
                    ++m_counters.render_pass_count;
                };
                u32 remaining_captures = m_counters.backdrop_capture_count;
                begin_gui_pass(true, remaining_captures == 0);
                for(const RenderBatch& batch : m_render_batches)
                {
                    if(batch.type == RenderBatchType::backdrop_capture)
                    {
                        cmdbuf->end_render_pass();
                        record_backdrop_capture(cmdbuf,
                            m_backdrop_captures[batch.resource_index], render_target);
                        --remaining_captures;
                        begin_gui_pass(false, remaining_captures == 0);
                    }
                    else
                    {
                        submit_batch(cmdbuf, batch);
                    }
                }
                cmdbuf->end_render_pass();

                Vector<RHI::TextureBarrier> final_barriers;
                final_barriers.push_back(RHI::TextureBarrier(render_target,
                    RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic,
                    target.color_final_state));
                if(target.depth_stencil_texture)
                {
                    final_barriers.push_back(RHI::TextureBarrier(target.depth_stencil_texture,
                        RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic,
                        target.depth_stencil_final_state));
                }
                cmdbuf->resource_barrier({}, final_barriers.cspan());
            }
            lucatchret;
            m_counters.render_ms = perf_elapsed_ms(render_begin, get_ticks());
            return ok;
        }

        void Renderer::submit_batch(RHI::ICommandBuffer* cmdbuf,
            const RenderBatch& batch)
        {
            if(!cmdbuf) return;
            if(batch.type == RenderBatchType::vg)
            {
                m_shape_renderer->submit(cmdbuf, batch.first, batch.count);
            }
            else if(batch.type == RenderBatchType::sdf)
            {
                render_sdf(cmdbuf, batch);
            }
        }

        void Renderer::record_backdrop_capture(RHI::ICommandBuffer* cmdbuf,
            const BackdropCapture& capture, RHI::ITexture* render_target)
        {
            using namespace RHI;
            u32 pass_count = capture.filter_pass_count;
            Vector<BufferBarrier> parameter_barriers;
            parameter_barriers.reserve(pass_count);
            for(u32 i = 0; i < pass_count; ++i)
            {
                parameter_barriers.push_back(BufferBarrier(
                    capture.parameter_buffers[i], BufferStateFlag::automatic,
                    BufferStateFlag::uniform_buffer_cs));
            }
            TextureBarrier render_target_barrier(render_target,
                TEXTURE_BARRIER_ALL_SUBRESOURCES,
                TextureStateFlag::automatic, TextureStateFlag::shader_read_cs);
            cmdbuf->resource_barrier(parameter_barriers.cspan(),
                {&render_target_barrier, 1});

            cmdbuf->begin_compute_pass();
            cmdbuf->set_compute_pipeline_layout(m_backdrop_pipeline_layout);
            cmdbuf->set_compute_pipeline_state(m_backdrop_pipeline_state);

            for(u32 pass_index = 0; pass_index < pass_count; ++pass_index)
            {
                const BackdropFilterPass& filter_pass =
                    capture.filter_passes[pass_index];
                ITexture* source = filter_pass.source;
                ITexture* destination = filter_pass.destination;
                u32 source_mip = filter_pass.source_mip;
                u32 destination_mip = filter_pass.destination_mip;

                if(source != render_target)
                {
                    TextureBarrier pass_barriers[] = {
                        TextureBarrier(source,
                            SubresourceIndex(source_mip, 0),
                            TextureStateFlag::automatic,
                            TextureStateFlag::shader_read_cs),
                        TextureBarrier(destination,
                            SubresourceIndex(destination_mip, 0),
                            TextureStateFlag::automatic,
                            TextureStateFlag::shader_write_cs)
                    };
                    cmdbuf->resource_barrier({}, {pass_barriers, 2});
                }
                else
                {
                    TextureBarrier destination_barrier(destination,
                        SubresourceIndex(destination_mip, 0),
                        TextureStateFlag::automatic,
                        TextureStateFlag::shader_write_cs);
                    cmdbuf->resource_barrier({},
                        {&destination_barrier, 1});
                }
                cmdbuf->set_compute_descriptor_set(
                    0, capture.descriptor_sets[pass_index]);
                cmdbuf->dispatch(
                    (u32)align_upper(filter_pass.destination_size.x, 8u) / 8u,
                    (u32)align_upper(filter_pass.destination_size.y, 8u) / 8u, 1);
            }
            cmdbuf->end_compute_pass();

            TextureBarrier final_texture_barriers[] = {
                TextureBarrier(capture.blur_textures[1],
                    TEXTURE_BARRIER_ALL_SUBRESOURCES,
                    TextureStateFlag::automatic, TextureStateFlag::shader_read_ps),
                TextureBarrier(render_target, TEXTURE_BARRIER_ALL_SUBRESOURCES,
                    TextureStateFlag::automatic,
                    TextureStateFlag::color_attachment_write)
            };
            cmdbuf->resource_barrier({}, {final_texture_barriers, 2});
        }

        RendererPerformanceCounters Renderer::get_performance_counters() const
        {
            return m_counters;
        }

        LUNA_GUI_API R<Ref<IRenderer>> new_renderer(RHI::IDevice* device)
        {
            Ref<Renderer> renderer = new_object<Renderer>();
            RV result = renderer->init(device);
            if(failed(result)) return result.errcode();
            Ref<IRenderer> ret = renderer;
            return ret;
        }
    }
}
