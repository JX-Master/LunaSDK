# Rendering

LunaSDK provides a layered rendering stack. Applications can use the Rendering Hardware Interface (RHI) directly, describe work through the Render Graph (`RG`), or use focused utilities from `RHIUtility`. Shaders can be written in CPPSL and compiled by LunaBuild for the active graphics backend.

## RHI

RHI is a platform-independent abstraction over modern graphics devices and APIs. It exposes explicit resources, command queues and command buffers, synchronization, pipeline state, descriptor binding, and resource aliasing so applications can control GPU work without depending on one native graphics API.

- [[RHI Devices]]
- [[RHI Command Queues and Command Buffers]]
- [[RHI Resources]]
- [[RHI Pipeline Configuration]]
- [[Recording RHI Commands]]
- [[RHI Shader Resource Binding]]

## CPPSL

[[CPPSL Shader Language]] is the supported C++-like shader language. LunaBuild compiles CPPSL sources offline to the runtime shader format required by the selected backend.

## Render Graph

The `RG` module builds a dependency graph from declared render passes and resources. It culls unused passes, determines resource lifetimes, aliases compatible transient resources, and invokes enabled pass callbacks to record work into the command buffer supplied to `execute`. RG inserts aliasing barriers when a transient allocation is reused, but pass callbacks remain responsible for recording ordinary RHI state transitions required by their commands. The caller submits and synchronizes the command buffer. Include `<Luna/RG/RenderGraph.hpp>` when using the render graph API.

## RHIUtility

The `RHIUtility` module provides higher-level contexts for common staged operations, including resource upload, resource readback, mipmap generation, and texture blitting. These helpers still record work into RHI command buffers and therefore participate in the same queue submission and synchronization rules. Include the required header from `<Luna/RHIUtility/...>` for each utility context.
