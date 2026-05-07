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
        ValidateRhiDeviceFacilities(device);
        ValidateRhiUtilityOperations(device, queue);
        ValidateRhiCommandOperations(device, queue);

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
                RunTextureCase(window, device, queue, commandBuffer, swapChain);
                break;
            case RhiCSharpTestCase.Box:
                RunBoxCase(window, device, queue, commandBuffer, swapChain);
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

    private static void RunTextureCase(IWindow window, IDevice device, uint queue, ICommandBuffer commandBuffer, ISwapChain swapChain)
    {
        var image = RhiTestAssets.LoadTextureTestImage();
        using var uploadCommandBuffer = device.CreateCommandBuffer(queue);
        using var readbackCommandBuffer = device.CreateCommandBuffer(queue);
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
        using var texture = CreateAndUploadTexture(device, uploadCommandBuffer, image);
        var textureRowPitch = checked(image.Desc.Width * ImageModule.PixelSize(image.Desc.Format));
        ValidateTextureReadback(device, readbackCommandBuffer, texture, image, textureRowPitch);
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.TextureResourceWriteContext");
        ValidateDeviceChild(writer, device, "IResourceWriteContext.Device");
        writer.Reset();
        writer.WriteBuffer(vertexBuffer, 0, vertices);
        writer.WriteBuffer(indexBuffer, 0, indices);
        writer.Commit(uploadCommandBuffer, submitAndWait: true);
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
                    writer.Commit(uploadCommandBuffer, submitAndWait: true);
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

    private static void RunBoxCase(IWindow window, IDevice device, uint queue, ICommandBuffer commandBuffer, ISwapChain swapChain)
    {
        var image = RhiTestAssets.LoadBoxTestImage();
        using var uploadCommandBuffer = device.CreateCommandBuffer(queue);
        using var readbackCommandBuffer = device.CreateCommandBuffer(queue);
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
        using var texture = CreateAndUploadTexture(device, uploadCommandBuffer, image);
        var textureRowPitch = checked(image.Desc.Width * ImageModule.PixelSize(image.Desc.Format));
        ValidateTextureReadback(device, readbackCommandBuffer, texture, image, textureRowPitch);
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.BoxResourceWriteContext");
        ValidateDeviceChild(writer, device, "IResourceWriteContext.Device");
        writer.Reset();
        writer.WriteBuffer(vertexBuffer, 0, vertices);
        writer.WriteBuffer(indexBuffer, 0, indices);
        writer.Commit(uploadCommandBuffer, submitAndWait: true);
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
        if (!Luna.RHI.Errors.Category.IsValid)
        {
            throw new InvalidOperationException("Errors.Category is invalid.");
        }
        if (RuntimeErrors.GetCodeName(Luna.RHI.Errors.SwapChainOutOfDate) != "swap_chain_out_of_date")
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
        if (!Enum.IsDefined(swapChain.SurfaceTransform))
        {
            throw new InvalidOperationException("ISwapChain.SurfaceTransform returned an unexpected transform value.");
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
        SubresourceIndex subresource,
        ImageData expectedImage,
        uint expectedRowPitch)
    {
        using var reader = Luna.RHIUtility.Module.CreateResourceReadContext(device);
        reader.SetName("RHICSharpTest.TextureResourceReadContext");
        ValidateDeviceChild(reader, device, "IResourceReadContext.Device");
        reader.Reset();
        var handle = reader.ReadTexture(
            texture,
            subresource,
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

    private static void ValidateTextureReadback(
        IDevice device,
        ICommandBuffer commandBuffer,
        ITexture texture,
        ImageData expectedImage,
        uint expectedRowPitch)
    {
        ValidateTextureReadback(device, commandBuffer, texture, new SubresourceIndex(0, 0), expectedImage, expectedRowPitch);
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

    private static void ValidateRhiUtilityOperations(IDevice device, uint queue)
    {
        using var uploadCommandBuffer = device.CreateCommandBuffer(queue);
        uploadCommandBuffer.SetName("RHICSharpTest.RHIUtility.UploadCommandBuffer");
        using var readbackCommandBuffer = device.CreateCommandBuffer(queue);
        readbackCommandBuffer.SetName("RHICSharpTest.RHIUtility.ReadbackCommandBuffer");
        using var graphicsCommandBuffer = device.CreateCommandBuffer(queue);
        graphicsCommandBuffer.SetName("RHICSharpTest.RHIUtility.GraphicsCommandBuffer");
        using var computeCommandBuffer = device.CreateCommandBuffer(queue);
        computeCommandBuffer.SetName("RHICSharpTest.RHIUtility.ComputeCommandBuffer");

        ValidateBufferRoundtrip(device, uploadCommandBuffer, readbackCommandBuffer);
        ValidateBlitRoundtrip(device, uploadCommandBuffer, readbackCommandBuffer, graphicsCommandBuffer);
        ValidateMipmaps(device, uploadCommandBuffer, readbackCommandBuffer, computeCommandBuffer);
    }

    private static void ValidateRhiCommandOperations(IDevice device, uint queue)
    {
        using var uploadCommandBuffer = device.CreateCommandBuffer(queue);
        uploadCommandBuffer.SetName("RHICSharpTest.RHI.UploadCommandBuffer");
        using var graphicsCommandBuffer = device.CreateCommandBuffer(queue);
        graphicsCommandBuffer.SetName("RHICSharpTest.RHI.GraphicsCommandBuffer");
        using var copySignalCommandBuffer = device.CreateCommandBuffer(queue);
        copySignalCommandBuffer.SetName("RHICSharpTest.RHI.CopySignalCommandBuffer");
        using var copyWaitCommandBuffer = device.CreateCommandBuffer(queue);
        copyWaitCommandBuffer.SetName("RHICSharpTest.RHI.CopyWaitCommandBuffer");
        using var computeCommandBuffer = device.CreateCommandBuffer(queue);
        computeCommandBuffer.SetName("RHICSharpTest.RHI.ComputeCommandBuffer");
        using var readbackCommandBuffer = device.CreateCommandBuffer(queue);
        readbackCommandBuffer.SetName("RHICSharpTest.RHI.ReadbackCommandBuffer");

        ValidateGraphicsCommands(device, uploadCommandBuffer, graphicsCommandBuffer, readbackCommandBuffer);
        ValidateCopyCommands(device, uploadCommandBuffer, copySignalCommandBuffer, copyWaitCommandBuffer, readbackCommandBuffer);
        ValidateComputeCommands(device, computeCommandBuffer, readbackCommandBuffer);
    }

    private static void ValidateRhiDeviceFacilities(IDevice device)
    {
        for (uint i = 0; i < device.CommandQueueCount; ++i)
        {
            var frequency = device.GetCommandQueueTimestampFrequency(i);
            if (frequency <= 0.0)
            {
                throw new InvalidOperationException("IDevice.GetCommandQueueTimestampFrequency returned a non-positive value.");
            }
        }

        using var fence = device.CreateFence();
        fence.SetName("RHICSharpTest.ValidateFence");
        ValidateDeviceChild(fence, device, "IFence.Device");

        using var timestampQueryHeap = device.CreateQueryHeap(new QueryHeapDesc(QueryType.Timestamp, 4));
        timestampQueryHeap.SetName("RHICSharpTest.ValidateTimestampQueryHeap");
        ValidateDeviceChild(timestampQueryHeap, device, "IQueryHeap.Device");
        if (timestampQueryHeap.Desc.Type != QueryType.Timestamp || timestampQueryHeap.Desc.Count != 4)
        {
            throw new InvalidOperationException("IQueryHeap.Desc returned unexpected timestamp heap data.");
        }

        using var occlusionQueryHeap = device.CreateQueryHeap(new QueryHeapDesc(QueryType.Occlusion, 2));
        occlusionQueryHeap.SetName("RHICSharpTest.ValidateOcclusionQueryHeap");
        if (occlusionQueryHeap.Desc.Type != QueryType.Occlusion || occlusionQueryHeap.Desc.Count != 2)
        {
            throw new InvalidOperationException("IQueryHeap.Desc returned unexpected occlusion heap data.");
        }

        var aliasingBufferDesc = new BufferDesc(BufferUsageFlags.CopySource | BufferUsageFlags.CopyDestination, 256);
        if (!device.IsResourcesAliasingCompatible(MemoryType.Local, new[] { aliasingBufferDesc }, Array.Empty<TextureDesc>()))
        {
            throw new InvalidOperationException("IDevice.IsResourcesAliasingCompatible unexpectedly rejected a single buffer descriptor.");
        }
        using var aliasingBufferMemory = device.AllocateMemory(MemoryType.Local, new[] { aliasingBufferDesc }, Array.Empty<TextureDesc>());
        if (aliasingBufferMemory.MemoryType != MemoryType.Local || aliasingBufferMemory.Size < aliasingBufferDesc.Size)
        {
            throw new InvalidOperationException("IDeviceMemory returned unexpected buffer allocation properties.");
        }
        using var aliasingBuffer = device.CreateAliasingBuffer(aliasingBufferMemory, aliasingBufferDesc);
        if (aliasingBuffer.Desc.Size != aliasingBufferDesc.Size)
        {
            throw new InvalidOperationException("CreateAliasingBuffer returned an unexpected descriptor.");
        }
        using (var actualMemory = aliasingBuffer.Memory)
        {
            if (actualMemory.GetNativeHandle() != aliasingBufferMemory.GetNativeHandle())
            {
                throw new InvalidOperationException("Aliasing buffer did not report the expected backing memory.");
            }
        }

        var aliasingTextureDesc = new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            4,
            4,
            1,
            1,
            1,
            1,
            TextureUsageFlags.CopySource | TextureUsageFlags.CopyDestination | TextureUsageFlags.ReadTexture,
            ResourceFlags.None);
        if (!device.IsResourcesAliasingCompatible(MemoryType.Local, Array.Empty<BufferDesc>(), new[] { aliasingTextureDesc }))
        {
            throw new InvalidOperationException("IDevice.IsResourcesAliasingCompatible unexpectedly rejected a single texture descriptor.");
        }
        using var aliasingTextureMemory = device.AllocateMemory(MemoryType.Local, Array.Empty<BufferDesc>(), new[] { aliasingTextureDesc });
        if (aliasingTextureMemory.MemoryType != MemoryType.Local || aliasingTextureMemory.Size == 0)
        {
            throw new InvalidOperationException("IDeviceMemory returned unexpected texture allocation properties.");
        }
        using var aliasingTexture = device.CreateAliasingTexture(aliasingTextureMemory, aliasingTextureDesc);
        if (aliasingTexture.Desc.Width != aliasingTextureDesc.Width || aliasingTexture.Desc.Height != aliasingTextureDesc.Height)
        {
            throw new InvalidOperationException("CreateAliasingTexture returned an unexpected descriptor.");
        }
        using (var actualMemory = aliasingTexture.Memory)
        {
            if (actualMemory.GetNativeHandle() != aliasingTextureMemory.GetNativeHandle())
            {
                throw new InvalidOperationException("Aliasing texture did not report the expected backing memory.");
            }
        }
    }

    private static void ValidateBufferRoundtrip(IDevice device, ICommandBuffer uploadCommandBuffer, ICommandBuffer readbackCommandBuffer)
    {
        var sourceData = new byte[64];
        for (var i = 0; i < sourceData.Length; ++i)
        {
            sourceData[i] = unchecked((byte)(i * 3 + 1));
        }

        using var buffer = device.CreateBuffer(
            MemoryType.Local,
            new BufferDesc(BufferUsageFlags.CopyDestination | BufferUsageFlags.CopySource, (ulong)sourceData.Length));
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.RHIUtility.BufferWriter");
        writer.Reset();
        writer.WriteBuffer(buffer, 0, sourceData);
        writer.Commit(uploadCommandBuffer, submitAndWait: true);

        using var reader = Luna.RHIUtility.Module.CreateResourceReadContext(device);
        reader.SetName("RHICSharpTest.RHIUtility.BufferReader");
        reader.Reset();
        var handle = reader.ReadBuffer(buffer, 0, (ulong)sourceData.Length);
        reader.Commit(readbackCommandBuffer, submitAndWait: true);
        var actualData = reader.GetBufferData(handle, (ulong)sourceData.Length);
        if (!actualData.AsSpan().SequenceEqual(sourceData))
        {
            throw new InvalidOperationException("RHIUtility buffer roundtrip mismatch.");
        }
    }

    private static void ValidateBlitRoundtrip(
        IDevice device,
        ICommandBuffer uploadCommandBuffer,
        ICommandBuffer readbackCommandBuffer,
        ICommandBuffer graphicsCommandBuffer)
    {
        var sourceImage = CreateQuadrantImageData();
        var expectedRowPitch = checked(sourceImage.Desc.Width * ImageModule.PixelSize(sourceImage.Desc.Format));

        using var sourceTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.CopySource | TextureUsageFlags.CopyDestination | TextureUsageFlags.ReadTexture,
            ResourceFlags.None));
        using var destinationTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.ColorAttachment | TextureUsageFlags.CopySource,
            ResourceFlags.None));
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.RHIUtility.TextureWriter");
        writer.Reset();
        var writeInfo = writer.WriteTexture(
            sourceTexture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1,
            sourceImage.Data,
            expectedRowPitch,
            checked(expectedRowPitch * sourceImage.Desc.Height),
            expectedRowPitch);
        if (writeInfo.RowPitch < expectedRowPitch)
        {
            throw new InvalidOperationException("RHIUtility texture upload returned an unexpected row pitch.");
        }
        writer.Commit(uploadCommandBuffer, submitAndWait: true);

        using var blitter = Luna.RHIUtility.Module.CreateBlitContext(device, Format.Rgba8Unorm);
        blitter.SetName("RHICSharpTest.RHIUtility.Blitter");
        blitter.Reset();
        blitter.Blit(
            destinationTexture,
            new SubresourceIndex(0, 0),
            TextureViewDesc.Tex2D(sourceTexture),
            CreateLinearClampSampler(),
            new BlitPoint(0.0f, 0.0f),
            new BlitPoint(sourceImage.Desc.Width, 0.0f),
            new BlitPoint(0.0f, sourceImage.Desc.Height),
            new BlitPoint(sourceImage.Desc.Width, sourceImage.Desc.Height));
        blitter.Commit(graphicsCommandBuffer, submitAndWait: true);

        ValidateTextureReadback(device, readbackCommandBuffer, destinationTexture, sourceImage, expectedRowPitch);
    }

    private static void ValidateMipmaps(
        IDevice device,
        ICommandBuffer uploadCommandBuffer,
        ICommandBuffer readbackCommandBuffer,
        ICommandBuffer computeCommandBuffer)
    {
        var baseImage = CreateQuadrantImageData();
        var mipImage = CreateQuadrantMipImageData();
        var baseRowPitch = checked(baseImage.Desc.Width * ImageModule.PixelSize(baseImage.Desc.Format));
        var mipRowPitch = checked(mipImage.Desc.Width * ImageModule.PixelSize(mipImage.Desc.Format));

        using var texture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            baseImage.Desc.Width,
            baseImage.Desc.Height,
            1,
            1,
            3,
            1,
            TextureUsageFlags.CopySource | TextureUsageFlags.CopyDestination | TextureUsageFlags.ReadTexture | TextureUsageFlags.ReadWriteTexture,
            ResourceFlags.None));
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.RHIUtility.MipmapWriter");
        writer.Reset();
        writer.WriteTexture(
            texture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            baseImage.Desc.Width,
            baseImage.Desc.Height,
            1,
            baseImage.Data,
            baseRowPitch,
            checked(baseRowPitch * baseImage.Desc.Height),
            baseRowPitch);
        writer.Commit(uploadCommandBuffer, submitAndWait: true);

        using var generator = Luna.RHIUtility.Module.CreateMipmapGenerationContext(device);
        generator.SetName("RHICSharpTest.RHIUtility.MipmapGenerator");
        generator.Reset();
        generator.GenerateMipmaps(texture);
        generator.Commit(computeCommandBuffer, submitAndWait: true);

        ValidateTextureReadback(device, readbackCommandBuffer, texture, new SubresourceIndex(1, 0), mipImage, mipRowPitch);
    }

    private static void ValidateCopyCommands(
        IDevice device,
        ICommandBuffer uploadCommandBuffer,
        ICommandBuffer copySignalCommandBuffer,
        ICommandBuffer copyWaitCommandBuffer,
        ICommandBuffer readbackCommandBuffer)
    {
        var bufferData = new byte[64];
        for (var i = 0; i < bufferData.Length; ++i)
        {
            bufferData[i] = unchecked((byte)(0x30 + i));
        }

        using var sourceBuffer = device.CreateBuffer(
            MemoryType.Local,
            new BufferDesc(BufferUsageFlags.CopySource | BufferUsageFlags.CopyDestination, (ulong)bufferData.Length));
        using var middleBuffer = device.CreateBuffer(
            MemoryType.Local,
            new BufferDesc(BufferUsageFlags.CopySource | BufferUsageFlags.CopyDestination, (ulong)bufferData.Length));
        using var destinationBuffer = device.CreateBuffer(
            MemoryType.Local,
            new BufferDesc(BufferUsageFlags.CopySource | BufferUsageFlags.CopyDestination, (ulong)bufferData.Length));
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.RHI.CopyWriter");
        writer.Reset();
        writer.WriteBuffer(sourceBuffer, 0, bufferData);
        writer.Commit(uploadCommandBuffer, submitAndWait: true);

        using var fence = device.CreateFence();
        fence.SetName("RHICSharpTest.RHI.CopyFence");
        using var timestampQueryHeap = device.CreateQueryHeap(new QueryHeapDesc(QueryType.Timestamp, 2));
        timestampQueryHeap.SetName("RHICSharpTest.RHI.CopyTimestampQueryHeap");

        copySignalCommandBuffer.Reset();
        copySignalCommandBuffer.BeginEvent("RHICSharpTest.CopySignal");
        copySignalCommandBuffer.BeginCopyPass(new CopyPassDesc
        {
            TimestampQueryHeap = timestampQueryHeap,
            TimestampQueryBeginPassWriteIndex = 0,
            TimestampQueryEndPassWriteIndex = 1
        });
        copySignalCommandBuffer.CopyBuffer(middleBuffer, 0, sourceBuffer, 0, (ulong)bufferData.Length);
        copySignalCommandBuffer.EndCopyPass();
        copySignalCommandBuffer.EndEvent();
        copySignalCommandBuffer.Submit(Array.Empty<IFence>(), new[] { fence }, allowHostWaiting: false);

        copyWaitCommandBuffer.Reset();
        copyWaitCommandBuffer.BeginEvent("RHICSharpTest.CopyWait");
        copyWaitCommandBuffer.BeginCopyPass(new CopyPassDesc());
        copyWaitCommandBuffer.CopyResource(destinationBuffer, middleBuffer);
        copyWaitCommandBuffer.EndCopyPass();
        copyWaitCommandBuffer.EndEvent();
        copyWaitCommandBuffer.Submit(new[] { fence }, Array.Empty<IFence>(), allowHostWaiting: true);
        copyWaitCommandBuffer.Wait();
        if (!copyWaitCommandBuffer.TryWait())
        {
            throw new InvalidOperationException("Copy command buffer should report completion after Wait.");
        }

        ValidateTimestampValues(timestampQueryHeap, "copy");

        using var reader = Luna.RHIUtility.Module.CreateResourceReadContext(device);
        reader.SetName("RHICSharpTest.RHI.CopyReader");
        reader.Reset();
        var bufferHandle = reader.ReadBuffer(destinationBuffer, 0, (ulong)bufferData.Length);
        reader.Commit(readbackCommandBuffer, submitAndWait: true);
        var actualBufferData = reader.GetBufferData(bufferHandle, (ulong)bufferData.Length);
        if (!actualBufferData.AsSpan().SequenceEqual(bufferData))
        {
            throw new InvalidOperationException("CopyBuffer/CopyResource roundtrip mismatch.");
        }

        var sourceImage = CreateQuadrantImageData();
        var expectedRowPitch = checked(sourceImage.Desc.Width * ImageModule.PixelSize(sourceImage.Desc.Format));
        var placement = device.GetTextureDataPlacementInfo(sourceImage.Desc.Width, sourceImage.Desc.Height, 1, Format.Rgba8Unorm);
        using var sourceTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.CopySource | TextureUsageFlags.CopyDestination | TextureUsageFlags.ReadTexture,
            ResourceFlags.None));
        using var destinationTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.CopySource | TextureUsageFlags.CopyDestination | TextureUsageFlags.ReadTexture,
            ResourceFlags.None));
        using var copiedTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.CopySource | TextureUsageFlags.CopyDestination | TextureUsageFlags.ReadTexture,
            ResourceFlags.None));
        using var copyBuffer = device.CreateBuffer(
            MemoryType.Local,
            new BufferDesc(BufferUsageFlags.CopySource | BufferUsageFlags.CopyDestination, placement.Size));

        writer.Reset();
        writer.WriteTexture(
            sourceTexture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1,
            sourceImage.Data,
            expectedRowPitch,
            checked(expectedRowPitch * sourceImage.Desc.Height),
            expectedRowPitch);
        writer.Commit(uploadCommandBuffer, submitAndWait: true);

        copySignalCommandBuffer.Reset();
        copySignalCommandBuffer.BeginEvent("RHICSharpTest.TextureCopy");
        copySignalCommandBuffer.BeginCopyPass(new CopyPassDesc());
        copySignalCommandBuffer.CopyTexture(
            destinationTexture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            sourceTexture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1);
        copySignalCommandBuffer.CopyTextureToBuffer(
            copyBuffer,
            0,
            (uint)placement.RowPitch,
            (uint)placement.SlicePitch,
            destinationTexture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1);
        copySignalCommandBuffer.CopyBufferToTexture(
            copiedTexture,
            new SubresourceIndex(0, 0),
            0,
            0,
            0,
            copyBuffer,
            0,
            (uint)placement.RowPitch,
            (uint)placement.SlicePitch,
            sourceImage.Desc.Width,
            sourceImage.Desc.Height,
            1);
        copySignalCommandBuffer.EndCopyPass();
        copySignalCommandBuffer.EndEvent();
        copySignalCommandBuffer.Submit(allowHostWaiting: true);
        copySignalCommandBuffer.Wait();

        reader.Reset();
        bufferHandle = reader.ReadBuffer(copyBuffer, 0, placement.Size);
        reader.Commit(readbackCommandBuffer, submitAndWait: true);
        var copiedTextureBytes = reader.GetBufferData(bufferHandle, placement.Size);
        ValidateTextureBufferBytes(copiedTextureBytes, (uint)placement.RowPitch, sourceImage);
        ValidateTextureReadback(device, readbackCommandBuffer, copiedTexture, sourceImage, expectedRowPitch);
    }

    private static void ValidateGraphicsCommands(
        IDevice device,
        ICommandBuffer uploadCommandBuffer,
        ICommandBuffer graphicsCommandBuffer,
        ICommandBuffer readbackCommandBuffer)
    {
        ValidateTriangleGraphicsCommands(device, graphicsCommandBuffer, readbackCommandBuffer);
        ValidateTexturedQuadGraphicsCommands(device, uploadCommandBuffer, graphicsCommandBuffer, readbackCommandBuffer);
    }

    private static void ValidateTriangleGraphicsCommands(
        IDevice device,
        ICommandBuffer graphicsCommandBuffer,
        ICommandBuffer readbackCommandBuffer)
    {
        using var colorTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            16,
            16,
            1,
            1,
            1,
            1,
            TextureUsageFlags.ColorAttachment | TextureUsageFlags.CopySource,
            ResourceFlags.None));
        using var occlusionQueryHeap = device.CreateQueryHeap(new QueryHeapDesc(QueryType.Occlusion, 1));
        occlusionQueryHeap.SetName("RHICSharpTest.RHI.GraphicsOcclusionQueryHeap");
        IQueryHeap? pipelineStatisticsQueryHeap = null;
        try
        {
            if (Luna.RHI.Module.BackendType != BackendType.Metal)
            {
                pipelineStatisticsQueryHeap = device.CreateQueryHeap(new QueryHeapDesc(QueryType.PipelineStatistics, 1));
                pipelineStatisticsQueryHeap.SetName("RHICSharpTest.RHI.GraphicsPipelineStatisticsQueryHeap");
            }

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
                DepthStencilState = new DepthStencilDesc(depthTestEnable: false, depthWriteEnable: false),
                BlendState = CreateAlphaBlendState(),
                ColorFormats = new[] { Format.Rgba8Unorm }
            });
            var vertices = RhiTestAssets.CreateTriangleVertexData();
            using var vertexBuffer = device.CreateBuffer(MemoryType.Upload, new BufferDesc(BufferUsageFlags.VertexBuffer, (ulong)vertices.Length));
            vertexBuffer.Write(0, vertices);

            graphicsCommandBuffer.Reset();
            graphicsCommandBuffer.BeginEvent("RHICSharpTest.GraphicsTriangle");
            graphicsCommandBuffer.AttachDeviceObject(colorTexture);
            graphicsCommandBuffer.AttachDeviceObject(occlusionQueryHeap);
            if (pipelineStatisticsQueryHeap is not null)
            {
                graphicsCommandBuffer.AttachDeviceObject(pipelineStatisticsQueryHeap);
            }
            graphicsCommandBuffer.AttachDeviceObject(pipelineLayout);
            graphicsCommandBuffer.AttachDeviceObject(pipelineState);
            graphicsCommandBuffer.AttachDeviceObject(vertexBuffer);
            graphicsCommandBuffer.ResourceBarrier(
                new[]
                {
                    new BufferBarrier(vertexBuffer, BufferStateFlags.Automatic, BufferStateFlags.VertexBuffer)
                },
                new[]
                {
                    new TextureBarrier(
                        colorTexture,
                        SubresourceIndex.AllSubresources,
                        TextureStateFlags.Automatic,
                        TextureStateFlags.ColorAttachmentWrite,
                        ResourceBarrierFlags.DiscardContent)
                });
            graphicsCommandBuffer.BeginRenderPass(new RenderPassDesc
            {
                ColorAttachments = new[]
                {
                    new ColorAttachment(colorTexture, LoadOp.Clear, StoreOp.Store, new Color4(0.0f, 0.0f, 0.0f, 1.0f))
                },
                OcclusionQueryHeap = occlusionQueryHeap,
                PipelineStatisticsQueryHeap = pipelineStatisticsQueryHeap,
                PipelineStatisticsQueryWriteIndex = pipelineStatisticsQueryHeap is null ? uint.MaxValue : 0u
            });
            graphicsCommandBuffer.SetGraphicsPipelineState(pipelineState);
            graphicsCommandBuffer.SetGraphicsPipelineLayout(pipelineLayout);
            graphicsCommandBuffer.SetVertexBuffer(0, vertexBuffer, 0, (uint)vertices.Length, 24);
            graphicsCommandBuffer.SetScissorRects(new[]
            {
                new RectI(0, 0, 16, 16)
            });
            graphicsCommandBuffer.SetViewports(new[]
            {
                new Viewport(0, 0, 16, 16, 0.0f, 1.0f)
            });
            graphicsCommandBuffer.SetBlendFactor(new Color4(1.0f, 1.0f, 1.0f, 1.0f));
            graphicsCommandBuffer.SetStencilRef(0);
            graphicsCommandBuffer.BeginOcclusionQuery(OcclusionQueryMode.Counting, 0);
            graphicsCommandBuffer.DrawInstanced(3, 2, 0, 0);
            graphicsCommandBuffer.EndOcclusionQuery(0);
            graphicsCommandBuffer.EndRenderPass();
            graphicsCommandBuffer.EndEvent();
            graphicsCommandBuffer.Submit(allowHostWaiting: true);
            graphicsCommandBuffer.Wait();

            var occlusionValues = occlusionQueryHeap.GetOcclusionValues(0, 1);
            if (occlusionValues.Length != 1 || occlusionValues[0] == 0)
            {
                throw new InvalidOperationException("Occlusion query did not report any rendered samples.");
            }

            if (pipelineStatisticsQueryHeap is not null)
            {
                var statistics = pipelineStatisticsQueryHeap.GetPipelineStatisticsValues(0, 1);
                if (statistics.Length != 1 ||
                    statistics[0].VertexShaderInvocations == 0 ||
                    statistics[0].RenderedPrimitives == 0 ||
                    statistics[0].PixelShaderInvocations == 0)
                {
                    throw new InvalidOperationException("Pipeline statistics query returned unexpected graphics values.");
                }
            }

            ValidateTextureContainsColor(device, readbackCommandBuffer, colorTexture, 16, 16);
        }
        finally
        {
            pipelineStatisticsQueryHeap?.Dispose();
        }
    }

    private static void ValidateTexturedQuadGraphicsCommands(
        IDevice device,
        ICommandBuffer uploadCommandBuffer,
        ICommandBuffer graphicsCommandBuffer,
        ICommandBuffer readbackCommandBuffer)
    {
        var image = CreateQuadrantImageData();
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
            ColorFormats = new[] { Format.Rgba8Unorm }
        });
        var vertices = RhiTestAssets.CreateTexturedQuadVertexData(image.Desc.Width, image.Desc.Height, image.Desc.Width, image.Desc.Height);
        var indices = RhiTestAssets.CreateQuadIndexData();
        using var vertexBuffer = device.CreateBuffer(MemoryType.Local, new BufferDesc(BufferUsageFlags.VertexBuffer | BufferUsageFlags.CopyDestination, (ulong)vertices.Length));
        using var indexBuffer = device.CreateBuffer(MemoryType.Local, new BufferDesc(BufferUsageFlags.IndexBuffer | BufferUsageFlags.CopyDestination, (ulong)indices.Length));
        using var sourceTexture = CreateAndUploadTexture(device, uploadCommandBuffer, image);
        using var colorTexture = device.CreateTexture(MemoryType.Local, new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            image.Desc.Width,
            image.Desc.Height,
            1,
            1,
            1,
            1,
            TextureUsageFlags.ColorAttachment | TextureUsageFlags.CopySource,
            ResourceFlags.None));
        using var writer = Luna.RHIUtility.Module.CreateResourceWriteContext(device);
        writer.SetName("RHICSharpTest.RHI.GraphicsQuadWriter");
        writer.Reset();
        writer.WriteBuffer(vertexBuffer, 0, vertices);
        writer.WriteBuffer(indexBuffer, 0, indices);
        writer.Commit(uploadCommandBuffer, submitAndWait: true);
        using var descriptorSet = device.CreateDescriptorSet(new DescriptorSetDesc(descriptorSetLayout));
        descriptorSet.SetReadTextureViews(0, 0, new[] { TextureViewDesc.Tex2D(sourceTexture) });
        descriptorSet.SetSamplers(1, 0, new[] { CreateNearestClampSampler() });

        graphicsCommandBuffer.Reset();
        graphicsCommandBuffer.BeginEvent("RHICSharpTest.GraphicsTexturedQuad");
        graphicsCommandBuffer.AttachDeviceObject(descriptorSetLayout);
        graphicsCommandBuffer.AttachDeviceObject(descriptorSet);
        graphicsCommandBuffer.AttachDeviceObject(pipelineLayout);
        graphicsCommandBuffer.AttachDeviceObject(pipelineState);
        graphicsCommandBuffer.AttachDeviceObject(vertexBuffer);
        graphicsCommandBuffer.AttachDeviceObject(indexBuffer);
        graphicsCommandBuffer.AttachDeviceObject(sourceTexture);
        graphicsCommandBuffer.AttachDeviceObject(colorTexture);
        graphicsCommandBuffer.ResourceBarrier(
            new[]
            {
                new BufferBarrier(vertexBuffer, BufferStateFlags.Automatic, BufferStateFlags.VertexBuffer),
                new BufferBarrier(indexBuffer, BufferStateFlags.Automatic, BufferStateFlags.IndexBuffer)
            },
            new[]
            {
                new TextureBarrier(sourceTexture, new SubresourceIndex(0, 0), TextureStateFlags.Automatic, TextureStateFlags.ShaderReadPs),
                new TextureBarrier(colorTexture, SubresourceIndex.AllSubresources, TextureStateFlags.Automatic, TextureStateFlags.ColorAttachmentWrite, ResourceBarrierFlags.DiscardContent)
            });
        graphicsCommandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(colorTexture, LoadOp.Clear, StoreOp.Store, new Color4(0.0f, 0.0f, 0.0f, 1.0f))
            }
        });
        graphicsCommandBuffer.SetGraphicsPipelineState(pipelineState);
        graphicsCommandBuffer.SetGraphicsPipelineLayout(pipelineLayout);
        graphicsCommandBuffer.SetGraphicsDescriptorSets(0, new[] { descriptorSet });
        graphicsCommandBuffer.SetVertexBuffers(0, new[]
        {
            new VertexBufferView(vertexBuffer, 0, (uint)vertices.Length, 16)
        });
        graphicsCommandBuffer.SetIndexBuffer(indexBuffer, 0, (uint)indices.Length, Format.R32Uint);
        graphicsCommandBuffer.SetViewport(new Viewport(0, 0, image.Desc.Width, image.Desc.Height, 0.0f, 1.0f));
        graphicsCommandBuffer.SetScissorRect(new RectI(0, 0, checked((int)image.Desc.Width), checked((int)image.Desc.Height)));
        graphicsCommandBuffer.DrawIndexedInstanced(6, 1, 0, 0, 0);
        graphicsCommandBuffer.EndRenderPass();
        graphicsCommandBuffer.EndEvent();
        graphicsCommandBuffer.Submit(allowHostWaiting: true);
        graphicsCommandBuffer.Wait();

        var expectedRowPitch = checked(image.Desc.Width * ImageModule.PixelSize(image.Desc.Format));
        ValidateTextureReadback(device, readbackCommandBuffer, colorTexture, image, expectedRowPitch);
    }

    private static void ValidateComputeCommands(IDevice device, ICommandBuffer computeCommandBuffer, ICommandBuffer readbackCommandBuffer)
    {
        using var outputBuffer = device.CreateBuffer(
            MemoryType.Local,
            new BufferDesc(BufferUsageFlags.ReadWriteBuffer | BufferUsageFlags.CopySource, 4u * sizeof(uint)));
        using var descriptorSetLayout = device.CreateDescriptorSetLayout(new DescriptorSetLayoutDesc
        {
            Bindings = new[]
            {
                DescriptorSetLayoutBinding.ReadWriteBufferView(0, 1, ShaderVisibilityFlags.Compute)
            }
        });
        using var descriptorSet = device.CreateDescriptorSet(new DescriptorSetDesc(descriptorSetLayout));
        descriptorSet.SetReadWriteBufferView(0, BufferViewDesc.StructuredBuffer(outputBuffer, 0, 4, sizeof(uint)));
        using var pipelineLayout = device.CreatePipelineLayout(new PipelineLayoutDesc
        {
            DescriptorSetLayouts = new[] { descriptorSetLayout }
        });
        using var pipelineState = device.CreateComputePipelineState(new ComputePipelineStateDesc
        {
            PipelineLayout = pipelineLayout,
            ComputeShader = RhiTestShaders.LoadComputeShader(),
            MetalNumThreadsX = 4,
            MetalNumThreadsY = 1,
            MetalNumThreadsZ = 1
        });
        using var timestampQueryHeap = device.CreateQueryHeap(new QueryHeapDesc(QueryType.Timestamp, 2));
        timestampQueryHeap.SetName("RHICSharpTest.RHI.ComputeTimestampQueryHeap");

        computeCommandBuffer.Reset();
        computeCommandBuffer.BeginEvent("RHICSharpTest.Compute");
        computeCommandBuffer.BeginComputePass(new ComputePassDesc
        {
            TimestampQueryHeap = timestampQueryHeap,
            TimestampQueryBeginPassWriteIndex = 0,
            TimestampQueryEndPassWriteIndex = 1
        });
        computeCommandBuffer.SetComputePipelineLayout(pipelineLayout);
        computeCommandBuffer.SetComputePipelineState(pipelineState);
        computeCommandBuffer.SetComputeDescriptorSets(0, new[] { descriptorSet });
        computeCommandBuffer.Dispatch(1, 1, 1);
        computeCommandBuffer.EndComputePass();
        computeCommandBuffer.EndEvent();
        computeCommandBuffer.Submit(allowHostWaiting: true);
        computeCommandBuffer.Wait();
        if (!computeCommandBuffer.TryWait())
        {
            throw new InvalidOperationException("Compute command buffer should report completion after Wait.");
        }

        computeCommandBuffer.Reset();
        computeCommandBuffer.BeginEvent("RHICSharpTest.ComputeSingleDescriptorSet");
        computeCommandBuffer.BeginComputePass(new ComputePassDesc());
        computeCommandBuffer.SetComputePipelineLayout(pipelineLayout);
        computeCommandBuffer.SetComputePipelineState(pipelineState);
        computeCommandBuffer.SetComputeDescriptorSet(0, descriptorSet);
        computeCommandBuffer.Dispatch(1, 1, 1);
        computeCommandBuffer.EndComputePass();
        computeCommandBuffer.EndEvent();
        computeCommandBuffer.Submit(allowHostWaiting: true);
        computeCommandBuffer.Wait();

        ValidateTimestampValues(timestampQueryHeap, "compute");

        var expectedValues = new uint[] { 5u, 22u, 39u, 56u };
        var expectedBytes = new byte[expectedValues.Length * sizeof(uint)];
        Buffer.BlockCopy(expectedValues, 0, expectedBytes, 0, expectedBytes.Length);

        using var reader = Luna.RHIUtility.Module.CreateResourceReadContext(device);
        reader.SetName("RHICSharpTest.RHI.ComputeReader");
        reader.Reset();
        var handle = reader.ReadBuffer(outputBuffer, 0, (ulong)expectedBytes.Length);
        reader.Commit(readbackCommandBuffer, submitAndWait: true);
        var actualBytes = reader.GetBufferData(handle, (ulong)expectedBytes.Length);
        if (!actualBytes.AsSpan().SequenceEqual(expectedBytes))
        {
            throw new InvalidOperationException("Compute pipeline output mismatch.");
        }
    }

    private static void ValidateTimestampValues(IQueryHeap queryHeap, string passName)
    {
        var values = queryHeap.GetTimestampValues(0, 2);
        if (values.Length != 2)
        {
            throw new InvalidOperationException($"Timestamp query heap returned an unexpected value count for {passName} pass.");
        }
        if (values[1] < values[0])
        {
            throw new InvalidOperationException($"Timestamp query heap returned a decreasing timestamp range for {passName} pass.");
        }
    }

    private static void ValidateTextureBufferBytes(byte[] data, uint rowPitch, ImageData expectedImage)
    {
        var expectedRowPitch = checked(expectedImage.Desc.Width * ImageModule.PixelSize(expectedImage.Desc.Format));
        for (uint y = 0; y < expectedImage.Desc.Height; ++y)
        {
            var expectedOffset = checked((int)(y * expectedRowPitch));
            var actualOffset = checked((int)(y * rowPitch));
            for (uint x = 0; x < expectedRowPitch; ++x)
            {
                var index = checked((int)x);
                if (data[actualOffset + index] != expectedImage.Data[expectedOffset + index])
                {
                    throw new InvalidOperationException("Texture buffer copy data mismatch.");
                }
            }
        }
    }

    private static void ValidateTextureContainsColor(
        IDevice device,
        ICommandBuffer readbackCommandBuffer,
        ITexture texture,
        uint width,
        uint height)
    {
        using var reader = Luna.RHIUtility.Module.CreateResourceReadContext(device);
        reader.SetName("RHICSharpTest.RHI.ColorTextureReader");
        reader.Reset();
        var handle = reader.ReadTexture(texture, new SubresourceIndex(0, 0), 0, 0, 0, width, height, 1);
        reader.Commit(readbackCommandBuffer, submitAndWait: true);
        var rowPitch = checked(width * (uint)ImageModule.PixelSize(ImageFormat.Rgba8Unorm));
        var data = reader.GetTextureData(handle, rowPitch, height, 1);
        var hasNonBlackPixel = false;
        for (var i = 0; i < data.Data.Length; i += 4)
        {
            if (data.Data[i] != 0 || data.Data[i + 1] != 0 || data.Data[i + 2] != 0)
            {
                hasNonBlackPixel = true;
                break;
            }
        }
        if (!hasNonBlackPixel)
        {
            throw new InvalidOperationException("Offscreen graphics pass did not write any visible color.");
        }
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
        using var descriptorUniformBuffer = device.CreateBuffer(
            MemoryType.Upload,
            new BufferDesc(BufferUsageFlags.UniformBuffer, 256));
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
                DescriptorSetLayoutBinding.Sampler(6, 2, ShaderVisibilityFlags.Pixel),
                DescriptorSetLayoutBinding.UniformBufferView(7, 2, ShaderVisibilityFlags.Vertex),
                DescriptorSetLayoutBinding.ReadBufferView(8, 1, ShaderVisibilityFlags.Compute),
                DescriptorSetLayoutBinding.ReadWriteBufferView(9, 2, ShaderVisibilityFlags.Compute),
                DescriptorSetLayoutBinding.ReadTextureView(TextureViewType.Tex2D, 10, 1, ShaderVisibilityFlags.Pixel),
                DescriptorSetLayoutBinding.ReadWriteTextureView(TextureViewType.Tex2D, 11, 2, ShaderVisibilityFlags.Compute),
                DescriptorSetLayoutBinding.Sampler(12, 1, ShaderVisibilityFlags.Pixel)
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
        descriptorSet.SetUniformBufferViews(7, 0, new[]
        {
            BufferViewDesc.UniformBuffer(descriptorUniformBuffer, 0, 64),
            BufferViewDesc.UniformBuffer(descriptorUniformBuffer, 64, 64)
        });
        descriptorSet.SetReadBufferView(8, BufferViewDesc.StructuredBuffer(descriptorBuffer, 0, 4, 16));
        descriptorSet.SetReadWriteBufferViews(9, 0, new[]
        {
            BufferViewDesc.StructuredBuffer(descriptorBuffer, 0, 4, 16),
            BufferViewDesc.StructuredBuffer(descriptorBuffer, 4, 4, 16)
        });
        descriptorSet.SetReadTextureView(10, TextureViewDesc.Tex2D(descriptorTexture));
        descriptorSet.SetReadWriteTextureViews(11, 0, new[]
        {
            TextureViewDesc.Tex2D(descriptorTexture),
            TextureViewDesc.Tex2D(descriptorTexture)
        });
        descriptorSet.SetSampler(12, CreateNearestClampSampler());
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

    private static SamplerDesc CreateNearestClampSampler()
    {
        return new SamplerDesc(
            Filter.Nearest,
            Filter.Nearest,
            Filter.Nearest,
            TextureAddressMode.Clamp,
            TextureAddressMode.Clamp,
            TextureAddressMode.Clamp);
    }

    private static ImageData CreateQuadrantImageData()
    {
        return new ImageData(
            new byte[]
            {
                255, 0, 0, 255, 255, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
                255, 0, 0, 255, 255, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
                0, 0, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                0, 0, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
            },
            new ImageDesc(ImageFormat.Rgba8Unorm, 4, 4));
    }

    private static ImageData CreateQuadrantMipImageData()
    {
        return new ImageData(
            new byte[]
            {
                255, 0, 0, 255, 0, 255, 0, 255,
                0, 0, 255, 255, 255, 255, 255, 255
            },
            new ImageDesc(ImageFormat.Rgba8Unorm, 2, 2));
    }
}
