Application as a Service (AaaS) is a programming pattern used to develop editor and tool applications with LunaSDK. Its core principles are:

1. Implement application capabilities in a **service** library before building the graphical interface.
2. Expose service operations as [[Frontend]] function resources with [[Variants|Variant]] parameters and results.
3. Keep the GUI application as an interface layer that translates user actions into Frontend calls and displays the results.
4. Reuse the same service operations for automation tools and AI agents by adding suitable protocol shells, such as a Model Context Protocol shell.

Frontend itself is a protocol-independent, in-process registry and invocation kernel. It does not serialize messages or access streams. When service operations must cross a process, machine, or language boundary, the application adds a shell that parses its chosen protocol, maps calls to Frontend resources, builds responses, performs serialization, and uses a transport such as TCP, a system pipe, or shared memory.

For example, a JSON-RPC 2.0 shell validates request IDs, handles batches, extracts `method` and `params`, calls `IFrontend::invoke`, and wraps the returned `R<Variant>` in a response object. JSON-RPC 2.0 is one possible shell protocol, not a message format understood directly by Frontend.

An AaaS application commonly uses one of these architectures:

1. **In-process service**: Compile the service into the GUI application. The GUI can invoke Frontend functions directly with `Variant` values, avoiding serialization. Optional shells can expose selected resources to external automation or AI agents.
2. **Out-of-process service**: Run the service and application interface as separate programs. A protocol shell and transport connect them. This adds communication overhead but permits remote access, cross-language clients, and isolation between service and interface failures.
