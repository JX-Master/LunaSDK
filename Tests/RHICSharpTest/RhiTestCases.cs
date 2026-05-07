using Luna.RHI;
using Luna.Window;

internal static class RhiTestCases
{
    public static void RenderEmptyFrame(ICommandBuffer commandBuffer, ISwapChain swapChain)
    {
        commandBuffer.Reset();
        commandBuffer.BeginEvent("RHICSharpTest.EmptyFrame");
        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.AttachDeviceObject(backBuffer);
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.Present)
        });
        commandBuffer.EndEvent();
        commandBuffer.Submit(allowHostWaiting: true);
        commandBuffer.Wait();
        swapChain.Present();
    }

    public static void RenderClearFrame(ICommandBuffer commandBuffer, ISwapChain swapChain)
    {
        commandBuffer.Reset();
        commandBuffer.BeginEvent("RHICSharpTest.ClearFrame");
        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.AttachDeviceObject(backBuffer);
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.ColorAttachmentWrite,
                ResourceBarrierFlags.DiscardContent)
        });
        commandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(backBuffer, LoadOp.Clear, StoreOp.Store, new Color4(0.0f, 0.0f, 1.0f, 1.0f))
            }
        });
        commandBuffer.EndRenderPass();
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.Present)
        });
        commandBuffer.EndEvent();
        commandBuffer.Submit(allowHostWaiting: true);
        commandBuffer.Wait();
        swapChain.Present();
    }

    public static void RenderTriangleFrame(
        ICommandBuffer commandBuffer,
        ISwapChain swapChain,
        Size2U framebufferSize,
        IPipelineLayout pipelineLayout,
        IPipelineState pipelineState,
        IBuffer vertexBuffer,
        byte[] vertices)
    {
        commandBuffer.Reset();
        commandBuffer.BeginEvent("RHICSharpTest.TriangleFrame");
        commandBuffer.AttachDeviceObject(pipelineLayout);
        commandBuffer.AttachDeviceObject(pipelineState);
        commandBuffer.AttachDeviceObject(vertexBuffer);
        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.ResourceBarrier(
            new[]
            {
                new BufferBarrier(vertexBuffer, BufferStateFlags.Automatic, BufferStateFlags.VertexBuffer)
            },
            new[]
            {
                new TextureBarrier(
                    backBuffer,
                    SubresourceIndex.AllSubresources,
                    TextureStateFlags.Automatic,
                    TextureStateFlags.ColorAttachmentWrite,
                    ResourceBarrierFlags.DiscardContent)
            });
        commandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(backBuffer, LoadOp.Clear, StoreOp.Store, new Color4(1.0f, 1.0f, 0.0f, 1.0f))
            }
        });
        commandBuffer.SetGraphicsPipelineState(pipelineState);
        commandBuffer.SetGraphicsPipelineLayout(pipelineLayout);
        commandBuffer.SetVertexBuffers(0, new[]
        {
            new VertexBufferView(vertexBuffer, 0, (uint)vertices.Length, 24)
        });
        commandBuffer.SetScissorRect(new RectI(0, 0, checked((int)framebufferSize.Width), checked((int)framebufferSize.Height)));
        commandBuffer.SetViewport(new Viewport(0, 0, framebufferSize.Width, framebufferSize.Height, 0.0f, 1.0f));
        commandBuffer.Draw(3, 0);
        commandBuffer.EndRenderPass();
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.Present)
        });
        commandBuffer.EndEvent();
        commandBuffer.Submit(allowHostWaiting: true);
        commandBuffer.Wait();
        swapChain.Present();
    }

    public static void RenderTexturedQuadFrame(
        ICommandBuffer commandBuffer,
        ISwapChain swapChain,
        Size2U framebufferSize,
        IDescriptorSetLayout descriptorSetLayout,
        IDescriptorSet descriptorSet,
        IPipelineLayout pipelineLayout,
        IPipelineState pipelineState,
        IBuffer vertexBuffer,
        IBuffer indexBuffer,
        ITexture texture,
        byte[] vertices,
        byte[] indices)
    {
        commandBuffer.Reset();
        commandBuffer.BeginEvent("RHICSharpTest.TexturedQuadFrame");
        commandBuffer.AttachDeviceObject(descriptorSetLayout);
        commandBuffer.AttachDeviceObject(descriptorSet);
        commandBuffer.AttachDeviceObject(pipelineLayout);
        commandBuffer.AttachDeviceObject(pipelineState);
        commandBuffer.AttachDeviceObject(vertexBuffer);
        commandBuffer.AttachDeviceObject(indexBuffer);
        commandBuffer.AttachDeviceObject(texture);
        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.ResourceBarrier(
            new[]
            {
                new BufferBarrier(vertexBuffer, BufferStateFlags.Automatic, BufferStateFlags.VertexBuffer),
                new BufferBarrier(indexBuffer, BufferStateFlags.Automatic, BufferStateFlags.IndexBuffer)
            },
            new[]
            {
                new TextureBarrier(texture, new SubresourceIndex(0, 0), TextureStateFlags.Automatic, TextureStateFlags.ShaderReadPs),
                new TextureBarrier(
                    backBuffer,
                    SubresourceIndex.AllSubresources,
                    TextureStateFlags.Automatic,
                    TextureStateFlags.ColorAttachmentWrite,
                    ResourceBarrierFlags.DiscardContent)
            });
        commandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(backBuffer, LoadOp.Clear, StoreOp.Store, new Color4(0.0f, 0.0f, 0.0f, 1.0f))
            }
        });
        commandBuffer.SetGraphicsPipelineState(pipelineState);
        commandBuffer.SetGraphicsPipelineLayout(pipelineLayout);
        commandBuffer.SetGraphicsDescriptorSet(0, descriptorSet);
        commandBuffer.SetVertexBuffers(0, new[]
        {
            new VertexBufferView(vertexBuffer, 0, (uint)vertices.Length, 16)
        });
        commandBuffer.SetIndexBuffer(indexBuffer, 0, (uint)indices.Length, Format.R32Uint);
        commandBuffer.SetScissorRect(new RectI(0, 0, checked((int)framebufferSize.Width), checked((int)framebufferSize.Height)));
        commandBuffer.SetViewport(new Viewport(0, 0, framebufferSize.Width, framebufferSize.Height, 0.0f, 1.0f));
        commandBuffer.DrawIndexed(6, 0, 0);
        commandBuffer.EndRenderPass();
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.Present)
        });
        commandBuffer.EndEvent();
        commandBuffer.Submit(allowHostWaiting: true);
        commandBuffer.Wait();
        swapChain.Present();
    }

    public static void RenderTexturedBoxFrame(
        ICommandBuffer commandBuffer,
        ISwapChain swapChain,
        Size2U framebufferSize,
        IDescriptorSetLayout textureDescriptorSetLayout,
        IDescriptorSet textureDescriptorSet,
        IPipelineLayout texturePipelineLayout,
        IPipelineState texturePipelineState,
        IBuffer boxVertexBuffer,
        IBuffer boxIndexBuffer,
        IBuffer uniformBuffer,
        ITexture testTexture,
        ITexture depthTexture,
        byte[] boxVertices,
        byte[] boxIndices)
    {
        commandBuffer.Reset();
        commandBuffer.BeginEvent("RHICSharpTest.TexturedBoxFrame");
        commandBuffer.AttachDeviceObject(textureDescriptorSetLayout);
        commandBuffer.AttachDeviceObject(textureDescriptorSet);
        commandBuffer.AttachDeviceObject(texturePipelineLayout);
        commandBuffer.AttachDeviceObject(texturePipelineState);
        commandBuffer.AttachDeviceObject(boxVertexBuffer);
        commandBuffer.AttachDeviceObject(boxIndexBuffer);
        commandBuffer.AttachDeviceObject(uniformBuffer);
        commandBuffer.AttachDeviceObject(testTexture);
        commandBuffer.AttachDeviceObject(depthTexture);
        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.ResourceBarrier(
            new[]
            {
                new BufferBarrier(
                    uniformBuffer,
                    BufferStateFlags.Automatic,
                    BufferStateFlags.UniformBufferVs),
                new BufferBarrier(
                    boxVertexBuffer,
                    BufferStateFlags.Automatic,
                    BufferStateFlags.VertexBuffer),
                new BufferBarrier(
                    boxIndexBuffer,
                    BufferStateFlags.Automatic,
                    BufferStateFlags.IndexBuffer)
            },
            new[]
            {
                new TextureBarrier(
                    testTexture,
                    new SubresourceIndex(0, 0),
                    TextureStateFlags.Automatic,
                    TextureStateFlags.ShaderReadPs),
                new TextureBarrier(
                    backBuffer,
                    SubresourceIndex.AllSubresources,
                    TextureStateFlags.Automatic,
                    TextureStateFlags.ColorAttachmentWrite,
                    ResourceBarrierFlags.DiscardContent),
                new TextureBarrier(
                    depthTexture,
                    new SubresourceIndex(0, 0),
                    TextureStateFlags.Automatic,
                    TextureStateFlags.DepthStencilAttachmentWrite,
                    ResourceBarrierFlags.DiscardContent)
            });
        commandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(backBuffer, LoadOp.Clear, StoreOp.Store, new Color4(0.0f, 0.0f, 0.0f, 0.0f))
            },
            DepthStencilAttachment = new DepthStencilAttachment(depthTexture, readOnly: false, depthLoadOp: LoadOp.Clear, depthStoreOp: StoreOp.Store, depthClearValue: 1.0f)
        });
        commandBuffer.SetGraphicsPipelineState(texturePipelineState);
        commandBuffer.SetGraphicsPipelineLayout(texturePipelineLayout);
        commandBuffer.SetGraphicsDescriptorSet(0, textureDescriptorSet);
        commandBuffer.SetVertexBuffers(0, new[]
        {
            new VertexBufferView(boxVertexBuffer, 0, (uint)boxVertices.Length, 20)
        });
        commandBuffer.SetIndexBuffer(boxIndexBuffer, 0, (uint)boxIndices.Length, Format.R32Uint);
        commandBuffer.SetScissorRect(new RectI(0, 0, checked((int)framebufferSize.Width), checked((int)framebufferSize.Height)));
        commandBuffer.SetViewport(new Viewport(0, 0, framebufferSize.Width, framebufferSize.Height, 0.0f, 1.0f));
        commandBuffer.DrawIndexed(36, 0, 0);
        commandBuffer.EndRenderPass();
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.Present)
        });
        commandBuffer.EndEvent();
        commandBuffer.Submit(allowHostWaiting: true);
        commandBuffer.Wait();
        swapChain.Present();
    }
}
