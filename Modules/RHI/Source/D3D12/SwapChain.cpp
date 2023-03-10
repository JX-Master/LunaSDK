/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChain.cpp
* @author JXMaster
* @date 2019/9/20
*/
#include "SwapChain.hpp"

#ifdef LUNA_RHI_D3D12

#include "Resource.hpp"
#include "../../RHI.hpp"
#include "SwapChainVS.hpp"
#include "SwapChainPS.hpp"
#include <Window/Windows/Win32Window.hpp>

namespace Luna
{
	namespace RHI
	{ 
		extern ComPtr<IDXGIFactory1> g_dxgi;
		struct SwapChainVert
		{
			Float2U pos;
			Float2U uv;
		};

		RV SwapChain::init_shared_res()
		{
			LockGuard guard(m_device->m_swap_chain_shared_resource_lock);
			if (m_device->m_swap_chain_shared_resource_initialized)
			{
				return ok;
			}
			lutry
			{
				// The event is created in signaled state, so reset it first.
				if (!::ResetEvent(m_event))
				{
					return BasicError::bad_platform_call();
				}
				// Upload vertex data.
				ComPtr<ID3D12Resource> upload_buffer;
				{
					SwapChainVert verts[3];
					/*
					0(-1, 1)		1(3,1)
					________________
					|___|___|	.
			(-1,-1) |___|___|(1, -1)
					|	.
					|
					2(-1, -3)
					*/
					verts[0] = { Float2U(-1.0f, 1.0f), Float2U(0.0f, 0.0f) };
					verts[1] = { Float2U(3.0f, 1.0f), Float2U(2.0f, 0.0f) };
					verts[2] = { Float2U(-1.0f, -3.0f), Float2U(0.0f, 2.0f) };

					D3D12_RESOURCE_DESC res_desc;
					res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
					res_desc.Alignment = 0;
					res_desc.Width = sizeof(SwapChainVert) * 3;
					res_desc.Height = 1;
					res_desc.DepthOrArraySize = 1;
					res_desc.MipLevels = 1;
					res_desc.Format = DXGI_FORMAT_UNKNOWN;
					res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
					res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
					res_desc.SampleDesc.Count = 1;
					res_desc.SampleDesc.Quality = 0;
					D3D12_HEAP_PROPERTIES heap_props;
					heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
					heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
					heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
					heap_props.CreationNodeMask = 0;
					heap_props.VisibleNodeMask = 0;
					if (FAILED(m_device->m_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE,
						&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&m_device->m_swap_chain_vert_buf))))
					{
						return BasicError::bad_platform_call();
					}

					// Create a temp buffer on upload heap.
					heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

					if (FAILED(m_device->m_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE,
						&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&upload_buffer))))
					{
						return BasicError::bad_platform_call();
					}

					// Upload the data.
					void* data;
					D3D12_RANGE range;
					range.Begin = 1;
					range.End = 0;
					upload_buffer->Map(0, &range, &data);
					memcpy(data, verts, sizeof(SwapChainVert) * 3);
					range.Begin = 0;
					range.End = sizeof(SwapChainVert) * 3;
					upload_buffer->Unmap(0, &range);
					m_li->CopyBufferRegion(m_device->m_swap_chain_vert_buf.Get(), 0, upload_buffer.Get(), 0, sizeof(SwapChainVert) * 3);
					if (FAILED(m_li->Close()))
					{
						return BasicError::bad_platform_call();
					}
					ID3D12CommandList* exe_list = m_li.Get();
					m_queue->m_queue->ExecuteCommandLists(1, &exe_list);
					u64 wait_value = m_fence->GetCompletedValue() + 1;
					if (FAILED(m_fence->SetEventOnCompletion(wait_value, m_event)))
					{
						return BasicError::bad_platform_call();
					}
					if (FAILED(m_queue->m_queue->Signal(m_fence.Get(), wait_value)))
					{
						return BasicError::bad_platform_call();
					}
				}
				// Create Pipeline State.
				{
					{
						D3D12_ROOT_SIGNATURE_DESC root_signature;
						root_signature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
							D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
							D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
							D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
						root_signature.NumParameters = 1;	// SRV t0.
						root_signature.NumStaticSamplers = 1;	// Sampler s0.
						D3D12_ROOT_PARAMETER root_parameter;
						root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
						root_parameter.DescriptorTable.NumDescriptorRanges = 1;
						root_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
						D3D12_DESCRIPTOR_RANGE range;
						range.BaseShaderRegister = 0;
						range.NumDescriptors = 1;
						range.OffsetInDescriptorsFromTableStart = 0;
						range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
						range.RegisterSpace = 0;
						root_parameter.DescriptorTable.pDescriptorRanges = &range;
						root_signature.pParameters = &root_parameter;
						D3D12_STATIC_SAMPLER_DESC sampler;
						sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
						sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
						sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						sampler.MaxAnisotropy = 1;
						sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
						sampler.MinLOD = 0;
						sampler.MaxLOD = 20;
						sampler.MipLODBias = 0.0f;
						sampler.RegisterSpace = 0;
						sampler.ShaderRegister = 0;
						sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
						root_signature.pStaticSamplers = &sampler;
						ComPtr<ID3DBlob> err;
						if (FAILED(D3D12SerializeRootSignature(&root_signature, D3D_ROOT_SIGNATURE_VERSION_1_0, m_device->m_swap_chain_root_signature_data.GetAddressOf(), err.GetAddressOf())))
						{
							return set_error(BasicError::bad_platform_call(), "Failed to create root signature for Swap Chain: %s", (const char*)err->GetBufferPointer());
						}
					}
				}

				// Waits the vertex data to be uploaded.
				DWORD wait_result = WaitForSingleObject(m_event, INFINITE);
				if (wait_result != WAIT_OBJECT_0)
				{
					return BasicError::bad_platform_call();
				}
				if (FAILED(m_ca->Reset()))
				{
					return BasicError::bad_platform_call();
				}
				if (FAILED(m_li->Reset(m_ca.Get(), NULL)))
				{
					return BasicError::bad_platform_call();
				}
			}
			lucatchret;
			m_device->m_swap_chain_shared_resource_initialized = true;
			return ok;
		}

		RV SwapChain::init(Window::IWindow* window, CommandQueue* queue, const SwapChainDesc& desc)
		{
			m_window = window;
			m_queue = queue;
			m_device = static_cast<Device*>(queue->get_device()->get_object());
			ComPtr<IDXGIFactory2> dxgifac;
			g_dxgi.As(&dxgifac);

			m_event = ::CreateEventA(NULL, TRUE, TRUE, NULL);
			if (!m_event)
			{
				return BasicError::bad_platform_call();
			}
			m_desc = desc;
			if (!m_desc.width || !m_desc.height)
			{
				UInt2U sz = window->get_size();
				if (!m_desc.width)
				{
					m_desc.width = sz.x;
				}
				if (!m_desc.height)
				{
					m_desc.height = sz.y;
				}
			}

			DXGI_SWAP_CHAIN_DESC1 d;
			d.Width = m_desc.width;
			d.Height = m_desc.height;
			d.Format = encode_pixel_format(m_desc.pixel_format);
			d.Stereo = FALSE;
			d.SampleDesc.Count = 1;
			d.SampleDesc.Quality = 0;
			d.BufferCount = m_desc.buffer_count;
			d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
			d.Scaling = DXGI_SCALING_NONE;
			d.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			d.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			d.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			HWND hwnd = query_interface<Window::IWin32Window>(window->get_object())->get_hwnd();

			if (FAILED(dxgifac->CreateSwapChainForHwnd(queue->m_queue.Get(), hwnd, &d, NULL, NULL, &m_sc)))
			{
				return BasicError::bad_platform_call();
			}

			HRESULT hr = m_device->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_device->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_ca));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_device->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ca.Get(), NULL, IID_PPV_ARGS(&m_li));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}

			D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
			heap_desc.NodeMask = 0;
			heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heap_desc.NumDescriptors = 8;	// Hard coded here.
			heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			hr = m_device->m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_rtvs));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			m_rtv_size = m_device->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			heap_desc.NumDescriptors = 1;
			heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			hr = m_device->m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_srv));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}

			lutry
			{
				luexp(init_shared_res());

			// Create root signature.
			if (FAILED(m_device->m_device->CreateRootSignature(0, m_device->m_swap_chain_root_signature_data->GetBufferPointer(), m_device->m_swap_chain_root_signature_data->GetBufferSize(),
				IID_PPV_ARGS(&m_root_signature))))
			{
				return BasicError::bad_platform_call();
			}
			luexp(reset_back_buffer_resources(m_desc));

			hr = m_li->Close();
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			}
			lucatchret;
			return ok;
		}
		inline void fill_shader_data(D3D12_SHADER_BYTECODE& dest, ID3DBlob* src)
		{
			dest.BytecodeLength = src ? src->GetBufferSize() : 0;
			dest.pShaderBytecode = src ? src->GetBufferPointer() : nullptr;
		}
		RV SwapChain::reset_back_buffer_resources(const SwapChainDesc& desc)
		{
			lucheck(desc.buffer_count <= 8);
			// Fetch resources.
			m_back_buffers.resize(desc.buffer_count);
			D3D12_CPU_DESCRIPTOR_HANDLE h = m_rtvs->GetCPUDescriptorHandleForHeapStart();
			for (u32 i = 0; i < desc.buffer_count; ++i)
			{
				HRESULT hr = m_sc->GetBuffer(i, IID_PPV_ARGS(&m_back_buffers[i]));
				if (FAILED(hr))
				{
					return BasicError::bad_platform_call();
				}
				m_device->m_device->CreateRenderTargetView(m_back_buffers[i].Get(), NULL, h);
				h.ptr += m_rtv_size;
			}
			m_current_back_buffer = 0;

			// Recreate pso on format change.
			if (!m_pso || m_desc.pixel_format != desc.pixel_format)
			{
				m_pso = nullptr;
				D3D12_INPUT_ELEMENT_DESC input_elements[2];
				input_elements[0].AlignedByteOffset = 0;
				input_elements[0].Format = DXGI_FORMAT_R32G32_FLOAT;
				input_elements[0].InputSlot = 0;
				input_elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				input_elements[0].InstanceDataStepRate = 0;
				input_elements[0].SemanticIndex = 0;
				input_elements[0].SemanticName = "POSITION";
				input_elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				input_elements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
				input_elements[1].InputSlot = 0;
				input_elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				input_elements[1].InstanceDataStepRate = 0;
				input_elements[1].SemanticIndex = 0;
				input_elements[1].SemanticName = "TEXCOORD";

				D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline = {};
				pipeline.InputLayout.NumElements = 2;
				pipeline.InputLayout.pInputElementDescs = input_elements;
				pipeline.pRootSignature = m_root_signature.Get();
				pipeline.VS.pShaderBytecode = SWAPCHAINVS_CSO_DATA;
				pipeline.VS.BytecodeLength = SWAPCHAINVS_CSO_SIZE;
				pipeline.PS.pShaderBytecode = SWAPCHAINPS_CSO_DATA;
				pipeline.PS.BytecodeLength = SWAPCHAINPS_CSO_SIZE;
				pipeline.DS.BytecodeLength = 0;
				pipeline.DS.pShaderBytecode = NULL;
				pipeline.HS.BytecodeLength = 0;
				pipeline.HS.pShaderBytecode = NULL;
				pipeline.GS.BytecodeLength = 0;
				pipeline.GS.pShaderBytecode = NULL;
				pipeline.StreamOutput.NumEntries = 0;
				pipeline.StreamOutput.NumStrides = 0;
				pipeline.StreamOutput.pBufferStrides = NULL;
				pipeline.StreamOutput.pSODeclaration = NULL;
				pipeline.StreamOutput.RasterizedStream = 0;
				pipeline.BlendState.AlphaToCoverageEnable = FALSE;
				pipeline.BlendState.IndependentBlendEnable = FALSE;
				pipeline.BlendState.RenderTarget[0].BlendEnable = FALSE;
				pipeline.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
				pipeline.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
				pipeline.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
				pipeline.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				pipeline.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
				pipeline.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
				pipeline.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
				pipeline.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
				pipeline.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				pipeline.RasterizerState.AntialiasedLineEnable = FALSE;
				pipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
				pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
				pipeline.RasterizerState.DepthBias = 0;
				pipeline.RasterizerState.DepthBiasClamp = 0.0f;
				pipeline.RasterizerState.DepthClipEnable = FALSE;
				pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
				pipeline.RasterizerState.ForcedSampleCount = 0;
				pipeline.RasterizerState.FrontCounterClockwise = FALSE;
				pipeline.RasterizerState.MultisampleEnable = FALSE;
				pipeline.RasterizerState.SlopeScaledDepthBias = 0.0f;
				pipeline.DepthStencilState.DepthEnable = FALSE;
				pipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
				pipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
				pipeline.DepthStencilState.StencilEnable = FALSE;
				pipeline.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
				pipeline.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
				pipeline.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				pipeline.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
				pipeline.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
				pipeline.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
				pipeline.DepthStencilState.BackFace = pipeline.DepthStencilState.FrontFace;
				pipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
				pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				pipeline.NumRenderTargets = 1;
				pipeline.RTVFormats[0] = encode_pixel_format(desc.pixel_format);
				pipeline.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
				pipeline.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
				pipeline.RTVFormats[3] = DXGI_FORMAT_UNKNOWN;
				pipeline.RTVFormats[4] = DXGI_FORMAT_UNKNOWN;
				pipeline.RTVFormats[5] = DXGI_FORMAT_UNKNOWN;
				pipeline.RTVFormats[6] = DXGI_FORMAT_UNKNOWN;
				pipeline.RTVFormats[7] = DXGI_FORMAT_UNKNOWN;
				pipeline.DSVFormat = DXGI_FORMAT_UNKNOWN;
				pipeline.SampleMask = 0xFFFFFFFF;
				pipeline.SampleDesc.Count = 1;
				pipeline.SampleDesc.Quality = 0;
				pipeline.CachedPSO.CachedBlobSizeInBytes = 0;
				pipeline.CachedPSO.pCachedBlob = NULL;
				pipeline.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
				pipeline.NodeMask = 0;
				if (FAILED(m_device->m_device->CreateGraphicsPipelineState(&pipeline, IID_PPV_ARGS(&m_pso))))
				{
					return BasicError::bad_platform_call();
				}
			}
			m_desc = desc;
			return ok;
		}

		RV SwapChain::present(IResource* resource, u32 subresource)
		{
			lutsassert();
			wait();
			HRESULT hr = m_ca->Reset();
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_li->Reset(m_ca.Get(), m_pso.Get());
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			if (!::ResetEvent(m_event))
			{
				return BasicError::bad_platform_call();
			}
			// Blit the resource to the back buffer.
			Resource* res = static_cast<Resource*>(resource->get_object());
			m_device->m_device->CreateShaderResourceView(res->m_res.Get(), NULL, m_srv->GetCPUDescriptorHandleForHeapStart());
			m_li->SetGraphicsRootSignature(m_root_signature.Get());
			m_li->SetDescriptorHeaps(1, m_srv.GetAddressOf());
			m_li->SetGraphicsRootDescriptorTable(0, m_srv->GetGPUDescriptorHandleForHeapStart());
			D3D12_CPU_DESCRIPTOR_HANDLE h = m_rtvs->GetCPUDescriptorHandleForHeapStart();
			h.ptr += m_rtv_size * m_current_back_buffer;
			m_li->OMSetRenderTargets(1, &h, TRUE, NULL);
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = m_device->m_swap_chain_vert_buf->GetGPUVirtualAddress();
			vbv.SizeInBytes = sizeof(SwapChainVert) * 3;
			vbv.StrideInBytes = sizeof(SwapChainVert);
			m_li->IASetVertexBuffers(0, 1, &vbv);
			D3D12_VIEWPORT vp;
			vp.MaxDepth = 1.0f;
			vp.MinDepth = 0.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
			vp.Width = (FLOAT)m_desc.width;
			vp.Height = (FLOAT)m_desc.height;
			m_li->RSSetViewports(1, &vp);
			D3D12_RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = m_desc.width;
			rect.bottom = m_desc.height;
			m_li->RSSetScissorRects(1, &rect);

			if (!res->m_states.empty())
			{
				ResourceState state = res->m_states[0];
				if (state != ResourceState::common && state != ResourceState::shader_resource_pixel)
				{
					D3D12_RESOURCE_BARRIER b;
					b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					b.Transition.pResource = res->m_res.Get();
					b.Transition.StateBefore = encode_resource_state(state);
					b.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
					b.Transition.Subresource = 0;
					m_li->ResourceBarrier(1, &b);
					res->m_states[0] = ResourceState::shader_resource_pixel;
				}
			}

			D3D12_RESOURCE_BARRIER b;
			b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			b.Transition.pResource = m_back_buffers[m_current_back_buffer].Get();
			b.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			b.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			b.Transition.Subresource = 0;
			m_li->ResourceBarrier(1, &b);
			m_li->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_li->DrawInstanced(3, 1, 0, 0);
			b.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			b.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			m_li->ResourceBarrier(1, &b);

			if (FAILED(m_li->Close()))
			{
				return BasicError::bad_platform_call();
			}

			ID3D12CommandList* cmdlist = m_li.Get();
			m_queue->m_queue->ExecuteCommandLists(1, &cmdlist);
			hr = m_sc->Present(m_desc.vertical_synchronized ? 1 : 0, 0);
			if (FAILED(hr))
			{
				if (hr == DXGI_ERROR_DEVICE_RESET)
				{
					return set_error(BasicError::bad_platform_call(), "IDXGISwapChain::Present failed - DXGI_ERROR_DEVICE_RESET");
				}
				else if (hr == DXGI_ERROR_DEVICE_REMOVED)
				{
					return set_error(BasicError::bad_platform_call(), "IDXGISwapChain::Present failed - DXGI_ERROR_DEVICE_REMOVED");
				}
				return set_error(BasicError::bad_platform_call(), "IDXGISwapChain::Present failed - Error Code: %d", hr);
			}

			u64 wait_value = m_fence->GetCompletedValue() + 1;
			if (FAILED(m_fence->SetEventOnCompletion(wait_value, m_event)))
			{
				return BasicError::bad_platform_call();
			}
			if (FAILED(m_queue->m_queue->Signal(m_fence.Get(), wait_value)))
			{
				return BasicError::bad_platform_call();
			}
			m_current_back_buffer = (m_current_back_buffer + 1) % (m_desc.buffer_count);
			return ok;
		}
		RV SwapChain::reset(const SwapChainDesc& desc)
		{
			lutsassert();
			wait();
			SwapChainDesc modified_desc = desc;
			if (!modified_desc.buffer_count)
			{
				modified_desc.buffer_count = m_desc.buffer_count;
			}
			if (modified_desc.pixel_format == Format::unknown)
			{
				modified_desc.pixel_format = m_desc.pixel_format;
			}
			m_back_buffers.clear();

			if (!modified_desc.width || !modified_desc.height)
			{
				auto sz = m_window->get_size();
				if (!modified_desc.width)
				{
					modified_desc.width = sz.x;
				}
				if (!modified_desc.height)
				{
					modified_desc.height = sz.y;
				}
			}

			if (modified_desc.pixel_format == Format::unknown)
			{
				modified_desc.pixel_format = m_desc.pixel_format;
			}

			// Resize the back buffer.
			HRESULT hr = m_sc->ResizeBuffers(modified_desc.buffer_count, modified_desc.width, modified_desc.height, encode_pixel_format(modified_desc.pixel_format), DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			return reset_back_buffer_resources(modified_desc);
		}
	}
}

#endif