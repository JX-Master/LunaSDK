using System;
using Luna.Image;
using Luna.Runtime;
using Luna.RHI;
using Luna.RHIUtility;
using Luna.Window;

internal static class RhiCSharpTestApp
{
    public static void Run(RhiCSharpTestCase testCase = RhiCSharpTestCase.All)
    {
        Runtime.Init();
        WindowModule.Init("RHICSharpTest");
        ImageModule.Init();

        try
        {
            Luna.RHI.Module.Init();
            Luna.RHIUtility.Module.Init();
        }
        catch (ErrorException ex)
        {
            Console.WriteLine($"RHICSharpTest skipped: {ex.Message}");
            return;
        }

        if (testCase == RhiCSharpTestCase.All)
        {
            RunSingleCase(RhiCSharpTestCase.Empty);
            RunSingleCase(RhiCSharpTestCase.Clear);
            RunSingleCase(RhiCSharpTestCase.Triangle);
            RunSingleCase(RhiCSharpTestCase.Texture);
            RunSingleCase(RhiCSharpTestCase.Box);
        }
        else
        {
            RunSingleCase(testCase);
        }

        Console.WriteLine($"RHICSharpTest {testCase} passed.");
    }

    private static void RunSingleCase(RhiCSharpTestCase testCase)
    {
        using var window = WindowModule.CreateWindow(new WindowCreationDesc
        {
            Title = $"RHICSharpTest.{testCase}",
            X = 100,
            Y = 100,
            Width = 640,
            Height = 480
        });
        window.SetForeground();

        using var device = Luna.RHI.Module.GetMainDevice();
        ValidateAdapters(createDevice: testCase == RhiCSharpTestCase.Empty);
        ValidateErrors();
        var queue = FindPresentQueue(device);
        var placement = device.GetTextureDataPlacementInfo(4, 4, 1, Format.Rgba8Unorm);
        if (placement.Size == 0 || placement.RowPitch == 0 || placement.SlicePitch == 0)
        {
            throw new InvalidOperationException("IDevice.GetTextureDataPlacementInfo returned unexpected data.");
        }

        using var swapChain = device.CreateSwapChain(queue, window, new SwapChainDesc(
            0,
            0,
            2,
            Format.Bgra8Unorm,
            verticalSynchronized: true));
        using var commandBuffer = device.CreateCommandBuffer(queue);
        commandBuffer.SetName($"RHICSharpTest.{testCase}.CommandBuffer");
        ValidateSwapChainWindow(swapChain, window);
        ValidateDeviceChild(commandBuffer, device, "IDeviceChild.Device");
        ValidateManagedDefaults();
        ValidateRhiUtilityContexts(device);
        ValidateDescriptorApi(device);

        switch (testCase)
        {
            case RhiCSharpTestCase.Empty:
                RunInteractiveLoop(window, swapChain, testCase, _ => RhiTestCases.RenderEmptyFrame(commandBuffer, swapChain));
                break;
            case RhiCSharpTestCase.Clear:
                RunInteractiveLoop(window, swapChain, testCase, _ => RhiTestCases.RenderClearFrame(commandBuffer, swapChain));
                break;
            case RhiCSharpTestCase.Triangle:
                RunTriangleCase(window, device, commandBuffer, swapChain);
                break;
            case RhiCSharpTestCase.Texture:
                RunTextureCase(window, device, commandBuffer, swapChain);
                break;
            case RhiCSharpTestCase.Box:
                RunBoxCase(window, device, commandBuffer, swapChain);
                break;
            default:
                throw new InvalidOperationException($"Unsupported RHI C# test case: {testCase}.");
        }

        Console.WriteLine($"RHICSharpTest {testCase} passed.");
    }

    private static void RunTriangleCase(IWindow window, IDevice device, ICommandBuffer commandBuffer, ISwapChain swapChain)
    {
        using var pipelineLayout = device.CreatePipelineLayout(new PipelineLayoutDesc
        {
            Flags = PipelineLayoutFlags.AllowInputAssemblerInputLayout |
                PipelineLayoutFlags.DenyPixelShaderAccess |
                PipelineLayoutFlags.DenyVertexShaderAccess
        });
        using var pipelineState = device.CreateGraphicsPipelineState(new GraphicsPipelineStateDesc
        {
            InputBindings = new[]
            {
                new InputBindingDesc(0, 24, InputRate.PerVertex)
            },
            InputAttributes = new[]
            {
                new InputAttributeDesc(0, 0, 0, Format.Rg32Float),
                new InputAttributeDesc(1, 0, 8, Format.Rgba32Float)
            },
            PipelineLayout = pipelineLayout,
            VertexShader = RhiTestShaders.LoadTriangleVertexShader(),
            PixelShader = RhiTestShaders.LoadTrianglePixelShader(),
            RasterizerState = new RasterizerDesc(depthClampEnable: true),
            DepthStencilState = new DepthStencilDesc(
                depthTestEnable: false,
                depthWriteEnable: false,
                stencilEnable: false,
                frontFace: new DepthStencilOpDesc(StencilOp.Replace, StencilOp.Keep, StencilOp.Replace, CompareFunction.Always),
                backFace: new DepthStencilOpDesc(StencilOp.Keep, StencilOp.Keep, StencilOp.Keep, CompareFunction.Always)),
            BlendState = CreateAlphaBlendState(),
            ColorFormats = new[] { Format.Bgra8Unorm }
        });
        var vertices = RhiTestAssets.CreateTriangleVertexData();
        using var vertexBuffer = device.CreateBuffer(MemoryType.Upload, new BufferDesc(BufferUsageFlags.VertexBuffer, (ulong)vertices.Length));
        vertexBuffer.Write(0, vertices);
        RunInteractiveLoop(
            window,
            swapChain,
            RhiCSharpTestCase.Triangle,
            framebufferSize => RhiTestCases.RenderTriangleFrame(commandBuffer, swapChain, framebufferSize, pipelineLayout, pipelineState, vertexBuffer, vertices));
    }

    private static void RunTextureCase(IWindow window, IDevice device, ICommandBuffer commandBuffer, ISwapChain swapChain)
    {
        var image = RhiTestAssets.LoadTextureTestImage();
        using var descriptorSetLayout = device.CreateDescriptorSetLayout(new DescriptorSetLayoutDesc
        {
            Bindings = new[]
            {
                DescriptorSetLayoutBinding.ReadTextureView(TextureViewType.Tex2D, 0, 1, ShaderVisibilityFlags.Pixel),
                DescriptorSetLayoutBinding.Sampler(1, 1, ShaderVisibilityFlags.Pixel)
            }
        });
        using var pipelineLayout = device.CreatePipelineLayout(new PipelineLayoutDesc
        {
            DescriptorSetLayouts = new[] { descriptorSetLayout },
            Flags = PipelineLayoutFlags.AllowInputAssemblerInputLayout
        });
        using var pipelineState = device.CreateGraphicsPipelineState(new GraphicsPipelineStateDesc
        {
            InputBindings = new[]
            {
                new InputBindingDesc(0, 16, InputRate.PerVertex)
            },
            InputAttributes = new[]
            {
                new InputAttributeDesc(0, 0, 0, Format.Rg32Float),
                new InputAttributeDesc(1, 0, 8, Format.Rg32Float)
            },
            PipelineLayout = pipelineLayout,
            VertexShader = RhiTestShaders.LoadTextureVertexShader(),
            PixelShader = RhiTestShaders.LoadTexturePixelShader(),
            DepthStencilState = new DepthStencilDesc(depthTestEnable: false, depthWriteEnable: false),
            ColorFormats = new[] { Format.Bgra8Unorm }
        });
        var framebufferSize = GetInitialFramebufferSize(window);
        var vertices = RhiTestAssets.CreateTexturedQuadVertexData(image.Desc.Width, image.Desc.Height, framebufferSize.Width, framebufferSize.Height);
        var indices = RhiTestAssets.CreateQuadIndexData();
        using var vertexBuffer = device.CreateBuffer(MemoryType.Local, new BufferDesc(BufferUsageFlags.VertexBuffer | BufferUsageFlags.CopyDestination, (ulong)vertices.Length));
        using var indexBuffer = device.CreateBuffer(MemoryType.Local, new BufferDesc(BufferUsageFlags.IndexBuffer | BufferUsageFlags.CopyDestination, (ulong)indices.Length));
        using var texture = CreateAndUploadTexture(device, commandBuffer, image);
        var textureRowPitch = checked(image.Desc.Width * ImageModule.PixelSize(image.Desc.Format));
        ValidateTextureReadback(device, commandBuffer, texture, image, textureRowPitch);
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.TextureResourceWriteContext");
        ValidateDeviceChild(writer, device, "IResourceWriteContext.Device");
        writer.Reset();
        writer.WriteBuffer(vertexBuffer, 0, vertices);
        writer.WriteBuffer(indexBuffer, 0, indices);
        writer.Commit(commandBuffer, submitAndWait: true);
        using var descriptorSet = device.CreateDescriptorSet(new DescriptorSetDesc(descriptorSetLayout));
        descriptorSet.SetReadTextureViews(0, 0, new[] { TextureViewDesc.Tex2D(texture) });
        descriptorSet.SetSamplers(1, 0, new[] { CreateLinearClampSampler() });
        RunInteractiveLoop(
            window,
            swapChain,
            RhiCSharpTestCase.Texture,
            currentFramebufferSize =>
            {
                if (currentFramebufferSize != framebufferSize)
                {
                    framebufferSize = currentFramebufferSize;
                    vertices = RhiTestAssets.CreateTexturedQuadVertexData(image.Desc.Width, image.Desc.Height, framebufferSize.Width, framebufferSize.Height);
                    writer.Reset();
                    writer.WriteBuffer(vertexBuffer, 0, vertices);
                    writer.Commit(commandBuffer, submitAndWait: true);
                }
                RhiTestCases.RenderTexturedQuadFrame(
                    commandBuffer,
                    swapChain,
                    framebufferSize,
                    descriptorSetLayout,
                    descriptorSet,
                    pipelineLayout,
                    pipelineState,
                    vertexBuffer,
                    indexBuffer,
                    texture,
                    vertices,
                    indices);
            });
    }

    private static void RunBoxCase(IWindow window, IDevice device, ICommandBuffer commandBuffer, ISwapChain swapChain)
    {
        var image = RhiTestAssets.LoadBoxTestImage();
        using var descriptorSetLayout = device.CreateDescriptorSetLayout(new DescriptorSetLayoutDesc
        {
            Bindings = new[]
            {
                DescriptorSetLayoutBinding.Sampler(15, 1, ShaderVisibilityFlags.Pixel),
                DescriptorSetLayoutBinding.UniformBufferView(0, 1, ShaderVisibilityFlags.Vertex),
                DescriptorSetLayoutBinding.ReadTextureView(TextureViewType.Tex2D, 8, 1, ShaderVisibilityFlags.Pixel)
            }
        });
        using var pipelineLayout = device.CreatePipelineLayout(new PipelineLayoutDesc
        {
            DescriptorSetLayouts = new[] { descriptorSetLayout },
            Flags = PipelineLayoutFlags.AllowInputAssemblerInputLayout
        });
        using var pipelineState = device.CreateGraphicsPipelineState(new GraphicsPipelineStateDesc
        {
            InputBindings = new[]
            {
                new InputBindingDesc(0, 20, InputRate.PerVertex)
            },
            InputAttributes = new[]
            {
                new InputAttributeDesc(0, 0, 0, Format.Rgb32Float),
                new InputAttributeDesc(1, 0, 12, Format.Rg32Float)
            },
            PipelineLayout = pipelineLayout,
            VertexShader = RhiTestShaders.LoadBoxVertexShader(),
            PixelShader = RhiTestShaders.LoadBoxPixelShader(),
            BlendState = new BlendDesc
            {
                Attachments = new[]
                {
                    new AttachmentBlendDesc(
                        blendEnable: false,
                        sourceBlendColor: BlendFactor.SourceAlpha,
                        destinationBlendColor: BlendFactor.OneMinusSourceAlpha,
                        blendOpColor: BlendOp.Add,
                        sourceBlendAlpha: BlendFactor.OneMinusSourceAlpha,
                        destinationBlendAlpha: BlendFactor.Zero,
                        blendOpAlpha: BlendOp.Add,
                        colorWriteMask: ColorWriteMask.All)
                }
            },
            RasterizerState = new RasterizerDesc(
                FillMode.Solid,
                CullMode.Back,
                0,
                0.0f,
                0.0f,
                frontCounterClockwise: false,
                depthClampEnable: true),
            DepthStencilState = new DepthStencilDesc(depthTestEnable: true, depthWriteEnable: true, depthFunction: CompareFunction.LessEqual),
            ColorFormats = new[] { Format.Bgra8Unorm },
            DepthStencilFormat = Format.D32Float
        });
        var vertices = RhiTestAssets.CreateBoxVertexData();
        var indices = RhiTestAssets.CreateBoxIndexData();
        var framebufferSize = GetInitialFramebufferSize(window);
        var cameraRotation = 0.0f;
        var uniformData = RhiTestAssets.CreateWorldToProjectionMatrix(framebufferSize.Width, framebufferSize.Height, cameraRotation);
        var uniformBufferSize = RhiTestAssets.AlignUp((ulong)uniformData.Length, device.CheckFeature(DeviceFeature.UniformBufferDataAlignment).AsUInt32);
        using var vertexBuffer = device.CreateBuffer(MemoryType.Local, new BufferDesc(BufferUsageFlags.VertexBuffer | BufferUsageFlags.CopyDestination, (ulong)vertices.Length));
        using var indexBuffer = device.CreateBuffer(MemoryType.Local, new BufferDesc(BufferUsageFlags.IndexBuffer | BufferUsageFlags.CopyDestination, (ulong)indices.Length));
        using var uniformBuffer = device.CreateBuffer(MemoryType.Upload, new BufferDesc(BufferUsageFlags.UniformBuffer, uniformBufferSize));
        uniformBuffer.Write(0, uniformData);
        using var texture = CreateAndUploadTexture(device, commandBuffer, image);
        var textureRowPitch = checked(image.Desc.Width * ImageModule.PixelSize(image.Desc.Format));
        ValidateTextureReadback(device, commandBuffer, texture, image, textureRowPitch);
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.BoxResourceWriteContext");
        ValidateDeviceChild(writer, device, "IResourceWriteContext.Device");
        writer.Reset();
        writer.WriteBuffer(vertexBuffer, 0, vertices);
        writer.WriteBuffer(indexBuffer, 0, indices);
        writer.Commit(commandBuffer, submitAndWait: true);
        var depthTexture = CreateDepthTexture(device, framebufferSize);
        using var descriptorSet = device.CreateDescriptorSet(new DescriptorSetDesc(descriptorSetLayout));
        descriptorSet.SetUniformBufferView(0, BufferViewDesc.UniformBuffer(uniformBuffer));
        descriptorSet.SetReadTextureViews(8, 0, new[] { TextureViewDesc.Tex2D(texture) });
        descriptorSet.SetSamplers(15, 0, new[] { CreateLinearClampSampler() });
        try
        {
            RunInteractiveLoop(
                window,
                swapChain,
                RhiCSharpTestCase.Box,
                currentFramebufferSize =>
                {
                    cameraRotation += 1.0f;
                    uniformData = RhiTestAssets.CreateWorldToProjectionMatrix(currentFramebufferSize.Width, currentFramebufferSize.Height, cameraRotation);
                    uniformBuffer.Write(0, uniformData);
                    if (currentFramebufferSize != framebufferSize)
                    {
                        framebufferSize = currentFramebufferSize;
                        depthTexture.Dispose();
                        depthTexture = CreateDepthTexture(device, framebufferSize);
                    }
                    RhiTestCases.RenderTexturedBoxFrame(
                        commandBuffer,
                        swapChain,
                        framebufferSize,
                        descriptorSetLayout,
                        descriptorSet,
                        pipelineLayout,
                        pipelineState,
                        vertexBuffer,
                        indexBuffer,
                        uniformBuffer,
                        texture,
                        depthTexture,
                        vertices,
                        indices);
                });
        }
        finally
        {
            depthTexture.Dispose();
        }
    }

    private static void RunInteractiveLoop(
        IWindow window,
        ISwapChain swapChain,
        RhiCSharpTestCase testCase,
        Action<Size2U> renderFrame)
    {
        Console.WriteLine($"RHICSharpTest {testCase}: close the window to finish this test.");
        var swapChainFramebufferSize = GetInitialFramebufferSize(window);
        while (true)
        {
            WindowModule.PollEvents();
            if (window.IsClosed)
            {
                break;
            }
            var framebufferSize = window.FramebufferSize;
            if (window.IsMinimized || framebufferSize.Width == 0 || framebufferSize.Height == 0)
            {
                WindowModule.PollEvents(waitEvents: true);
                continue;
            }
            if (swapChain.ResetSuggested || framebufferSize != swapChainFramebufferSize)
            {
                var desc = swapChain.Desc;
                swapChain.Reset(new SwapChainDesc(
                    framebufferSize.Width,
                    framebufferSize.Height,
                    desc.BufferCount,
                    desc.Format,
                    desc.VerticalSynchronized,
                    desc.ColorSpace));
                swapChainFramebufferSize = framebufferSize;
            }
            renderFrame(framebufferSize);
        }
    }

    private static Size2U GetInitialFramebufferSize(IWindow window)
    {
        var framebufferSize = window.FramebufferSize;
        return new Size2U(
            framebufferSize.Width == 0 ? 1u : framebufferSize.Width,
            framebufferSize.Height == 0 ? 1u : framebufferSize.Height);
    }

    private static uint FindPresentQueue(IDevice device)
    {
        uint? fallback = null;
        for (uint i = 0; i < device.CommandQueueCount; ++i)
        {
            var desc = device.GetCommandQueueDesc(i);
            if ((desc.Flags & CommandQueueFlags.Presenting) == 0)
            {
                continue;
            }
            if (desc.Type == CommandQueueType.Graphics)
            {
                return i;
            }
            fallback ??= i;
        }
        return fallback ?? throw new InvalidOperationException("The main RHI device does not expose a present-capable command queue.");
    }

    private static void ValidateAdapters(bool createDevice)
    {
        var adapters = Luna.RHI.Module.GetAdapters();
        try
        {
            if (adapters.Length == 0)
            {
                throw new InvalidOperationException("RHI did not report any adapters.");
            }
            foreach (var adapter in adapters)
            {
                if (string.IsNullOrWhiteSpace(adapter.Name))
                {
                    throw new InvalidOperationException("RHI returned an adapter with an empty name.");
                }
            }
            if (createDevice)
            {
                using var device = Luna.RHI.Module.CreateDevice(adapters[0]);
                if (device.CommandQueueCount == 0)
                {
                    throw new InvalidOperationException("Module.CreateDevice returned a device with no command queues.");
                }
            }
        }
        finally
        {
            foreach (var adapter in adapters)
            {
                adapter.Dispose();
            }
        }
    }

    private static void ValidateErrors()
    {
        if (!Errors.Category.IsValid)
        {
            throw new InvalidOperationException("Errors.Category is invalid.");
        }
        if (RuntimeErrors.GetCodeName(Errors.SwapChainOutOfDate) != "swap_chain_out_of_date")
        {
            throw new InvalidOperationException("Errors.SwapChainOutOfDate returned an unexpected error code.");
        }
    }

    private static void ValidateSwapChainWindow(ISwapChain swapChain, IWindow expectedWindow)
    {
        using var actualWindow = swapChain.Window;
        if (actualWindow.GetNativeHandle() != expectedWindow.GetNativeHandle())
        {
            throw new InvalidOperationException("ISwapChain.Window returned an unexpected window.");
        }
    }

    private static void ValidateDeviceChild(IDeviceChild child, IDevice expectedDevice, string name)
    {
        using var actualDevice = child.Device;
        if (actualDevice.GetNativeHandle() != expectedDevice.GetNativeHandle())
        {
            throw new InvalidOperationException($"{name} returned an unexpected device.");
        }
    }

    private static void ValidateTextureReadback(
        IDevice device,
        ICommandBuffer commandBuffer,
        ITexture texture,
        ImageData expectedImage,
        uint expectedRowPitch)
    {
        using var reader = Luna.RHIUtility.Module.CreateResourceReadContext(device);
        reader.SetName("RHICSharpTest.TextureResourceReadContext");
        ValidateDeviceChild(reader, device, "IResourceReadContext.Device");
        reader.Reset();
        var handle = reader.ReadTexture(
            texture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            expectedImage.Desc.Width,
            expectedImage.Desc.Height,
            1);
        reader.Commit(commandBuffer, submitAndWait: true);
        var readData = reader.GetTextureData(handle, expectedRowPitch, expectedImage.Desc.Height, 1);
        if (readData.RowPitch < expectedRowPitch)
        {
            throw new InvalidOperationException("Texture readback returned a row pitch smaller than requested.");
        }
        for (uint y = 0; y < expectedImage.Desc.Height; ++y)
        {
            var expectedOffset = checked((int)(y * expectedRowPitch));
            var actualOffset = checked((int)(y * readData.RowPitch));
            for (uint x = 0; x < expectedRowPitch; ++x)
            {
                var index = checked((int)x);
                if (readData.Data[actualOffset + index] != expectedImage.Data[expectedOffset + index])
                {
                    throw new InvalidOperationException("Texture readback did not match uploaded image data.");
                }
            }
        }
    }

    private static void ValidateRhiUtilityContexts(IDevice device)
    {
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.ValidateResourceWriteContext");
        ValidateDeviceChild(writer, device, "IResourceWriteContext.Device");

        using var reader = Luna.RHIUtility.Module.CreateResourceReadContext(device);
        reader.SetName("RHICSharpTest.ValidateResourceReadContext");
        ValidateDeviceChild(reader, device, "IResourceReadContext.Device");

        using var blitter = Luna.RHIUtility.Module.CreateBlitContext(device, Format.Rgba8Unorm);
        blitter.SetName("RHICSharpTest.ValidateBlitContext");
        ValidateDeviceChild(blitter, device, "IBlitContext.Device");

        using var mipmapGenerator = Luna.RHIUtility.Module.CreateMipmapGenerationContext(device);
        mipmapGenerator.SetName("RHICSharpTest.ValidateMipmapGenerationContext");
        ValidateDeviceChild(mipmapGenerator, device, "IMipmapGenerationContext.Device");
    }

    private static void ValidateManagedDefaults()
    {
        var rasterizer = RasterizerDesc.Default;
        if (rasterizer.FillMode != FillMode.Solid || rasterizer.CullMode != CullMode.Back || rasterizer.FrontCounterClockwise || rasterizer.DepthClampEnable)
        {
            throw new InvalidOperationException("RasterizerDesc.Default returned unexpected values.");
        }

        var depthStencil = DepthStencilDesc.Default;
        if (!depthStencil.DepthTestEnable || !depthStencil.DepthWriteEnable || depthStencil.DepthFunction != CompareFunction.Less || depthStencil.StencilEnable || depthStencil.StencilReadMask != 0xff || depthStencil.StencilWriteMask != 0xff)
        {
            throw new InvalidOperationException("DepthStencilDesc.Default returned unexpected values.");
        }

        var graphicsPipeline = new GraphicsPipelineStateDesc();
        if (graphicsPipeline.DepthStencilState.DepthFunction != CompareFunction.Less || !graphicsPipeline.DepthStencilState.DepthTestEnable || !graphicsPipeline.DepthStencilState.DepthWriteEnable)
        {
            throw new InvalidOperationException("GraphicsPipelineStateDesc default depth stencil state returned unexpected values.");
        }
    }

    private static void ValidateDescriptorApi(IDevice device)
    {
        using var descriptorBuffer = device.CreateBuffer(
            MemoryType.Local,
            new BufferDesc(BufferUsageFlags.ReadBuffer | BufferUsageFlags.ReadWriteBuffer, 256));
        using var descriptorTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            4,
            4,
            1,
            1,
            1,
            1,
            TextureUsageFlags.ReadTexture | TextureUsageFlags.ReadWriteTexture,
            ResourceFlags.None));
        using var descriptorSetLayout = device.CreateDescriptorSetLayout(new DescriptorSetLayoutDesc
        {
            Bindings = new[]
            {
                DescriptorSetLayoutBinding.ReadBufferView(0, 2, ShaderVisibilityFlags.Compute),
                DescriptorSetLayoutBinding.ReadWriteBufferView(2, 1, ShaderVisibilityFlags.Compute),
                DescriptorSetLayoutBinding.ReadTextureView(TextureViewType.Tex2D, 3, 2, ShaderVisibilityFlags.Pixel),
                DescriptorSetLayoutBinding.ReadWriteTextureView(TextureViewType.Tex2D, 5, 1, ShaderVisibilityFlags.Compute),
                DescriptorSetLayoutBinding.Sampler(6, 2, ShaderVisibilityFlags.Pixel)
            }
        });
        using var descriptorSet = device.CreateDescriptorSet(new DescriptorSetDesc(descriptorSetLayout));
        descriptorSet.SetReadBufferViews(0, 0, new[]
        {
            BufferViewDesc.StructuredBuffer(descriptorBuffer, 0, 4, 16),
            BufferViewDesc.StructuredBuffer(descriptorBuffer, 4, 4, 16)
        });
        descriptorSet.SetReadWriteBufferView(2, BufferViewDesc.StructuredBuffer(descriptorBuffer, 0, 4, 16));
        descriptorSet.SetReadTextureViews(3, 0, new[]
        {
            TextureViewDesc.Tex2D(descriptorTexture),
            TextureViewDesc.Tex2D(descriptorTexture)
        });
        descriptorSet.SetReadWriteTextureView(5, TextureViewDesc.Tex2D(descriptorTexture));
        descriptorSet.SetSamplers(6, 0, new[]
        {
            CreateLinearClampSampler(),
            new SamplerDesc(
                Filter.Nearest,
                Filter.Nearest,
                Filter.Nearest,
                TextureAddressMode.Repeat,
                TextureAddressMode.Repeat,
                TextureAddressMode.Repeat)
        });
    }

    private static BlendDesc CreateAlphaBlendState()
    {
        return new BlendDesc
        {
            Attachments = new[]
            {
                new AttachmentBlendDesc(
                    blendEnable: true,
                    sourceBlendColor: BlendFactor.SourceAlpha,
                    destinationBlendColor: BlendFactor.OneMinusSourceAlpha,
                    blendOpColor: BlendOp.Add,
                    sourceBlendAlpha: BlendFactor.One,
                    destinationBlendAlpha: BlendFactor.Zero,
                    blendOpAlpha: BlendOp.Add,
                    colorWriteMask: ColorWriteMask.All)
            }
        };
    }

    private static ITexture CreateDepthTexture(IDevice device, Size2U framebufferSize)
    {
        return device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.D32Float,
            framebufferSize.Width,
            framebufferSize.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.DepthStencilAttachment,
            ResourceFlags.None),
            ClearValue.ForDepthStencil(Format.D32Float, 1.0f));
    }

    private static ITexture CreateAndUploadTexture(IDevice device, ICommandBuffer commandBuffer, ImageData image)
    {
        var texture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            image.Desc.Width,
            image.Desc.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.CopySource | TextureUsageFlags.ReadTexture | TextureUsageFlags.CopyDestination,
            ResourceFlags.None));
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.TextureResourceWriteContext");
        ValidateDeviceChild(writer, device, "IResourceWriteContext.Device");
        writer.Reset();
        var textureRowPitch = checked(image.Desc.Width * ImageModule.PixelSize(image.Desc.Format));
        var textureSlicePitch = checked(textureRowPitch * image.Desc.Height);
        writer.WriteTexture(
            texture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            image.Desc.Width,
            image.Desc.Height,
            1,
            image.Data,
            textureRowPitch,
            textureSlicePitch,
            textureRowPitch);
        writer.Commit(commandBuffer, submitAndWait: true);
        return texture;
    }

    private static SamplerDesc CreateLinearClampSampler()
    {
        return new SamplerDesc(
            Filter.Linear,
            Filter.Linear,
            Filter.Linear,
            TextureAddressMode.Clamp,
            TextureAddressMode.Clamp,
            TextureAddressMode.Clamp);
    }
}
