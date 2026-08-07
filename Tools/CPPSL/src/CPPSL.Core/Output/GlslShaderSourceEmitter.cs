using System.Text;
using CPPSL.Core.Compiler;
using CPPSL.Core.ShaderModel;
using CPPSL.Core.Semantics;

namespace CPPSL.Core.Output;

internal sealed class GlslShaderSourceEmitter : CppslShaderSourceEmitterBase
{
    private Dictionary<string, string> _resourceAccessByPath = new(StringComparer.Ordinal);
    private Dictionary<string, CppslMethod> _methodsByDeclId = new(StringComparer.Ordinal);
    private Dictionary<string, CppslMethod> _uniqueMethodsByOwnerAndName = new(StringComparer.Ordinal);
    private Dictionary<string, int> _methodOverloadCounts = new(StringComparer.Ordinal);
    private Dictionary<string, string> _glslMethodNamesByDeclId = new(StringComparer.Ordinal);
    private HashSet<string>? _currentMethodFields;

    public string Emit(CppslCompileOptions options, CppslSemanticModel model, CppslShaderModel shaderModel)
    {
        BeginShaderModelEmission(shaderModel);
        var builder = new StringBuilder();
        builder.AppendLine("#version 450");
        builder.AppendLine("#extension GL_EXT_samplerless_texture_functions : require");
        WriteHeader(builder, "GLSL", options);
        var entryPoint = FindEntryPoint(options, model);
        _resourceAccessByPath = BuildResourceAccessMap(
            model,
            static global => global.ResourceKind is not null && global.AccessPath is not null,
            static global => global.Name);
        (_methodsByDeclId, _uniqueMethodsByOwnerAndName, _methodOverloadCounts) = BuildMethodMaps(model);
        _glslMethodNamesByDeclId = BuildGlslMethodNamesByDeclId(model, shaderModel);
        WriteStructs(builder, model, shaderModel);
        WriteMethods(builder, model, shaderModel);
        WriteComputeLayout(builder, entryPoint);
        WriteGroupSharedGlobals(builder, model);
        WriteResources(builder, model);
        WriteStageIo(builder, entryPoint, model, options.Stage);
        WriteFunctions(builder, entryPoint, model, shaderModel);
        WriteEntryPointHelper(builder, entryPoint, model, shaderModel);
        WriteMain(builder, entryPoint, model);
        var source = RewriteResidualResourceAccessPaths(builder.ToString(), model, static global => global.Name);
        _resourceAccessByPath = new Dictionary<string, string>(StringComparer.Ordinal);
        _methodsByDeclId = new Dictionary<string, CppslMethod>(StringComparer.Ordinal);
        _uniqueMethodsByOwnerAndName = new Dictionary<string, CppslMethod>(StringComparer.Ordinal);
        _methodOverloadCounts = new Dictionary<string, int>(StringComparer.Ordinal);
        _glslMethodNamesByDeclId = new Dictionary<string, string>(StringComparer.Ordinal);
        _currentMethodFields = null;
        EndShaderModelEmission();
        return source;
    }

    private void WriteFunctions(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslShaderModel shaderModel)
    {
        foreach (var function in model.Functions.Where(function => function.Name != entryPoint?.Name))
        {
            var shaderModelFunction = shaderModel.Functions.FirstOrDefault(candidate => candidate.DeclId == function.DeclId);
            if (shaderModelFunction?.Body is null)
            {
                continue;
            }

            builder.Append(MapValueType(function.ReturnType ?? "void"));
            builder.Append(' ');
            builder.Append(shaderModelFunction.EmittedName);
            builder.Append('(');
            builder.Append(string.Join(", ", function.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}")));
            builder.AppendLine(")");
            WriteFunctionBody(builder, function, model, shaderModelFunction.Body);
            builder.AppendLine();
        }
    }

    private void WriteStructs(StringBuilder builder, CppslSemanticModel model, CppslShaderModel shaderModel)
    {
        var descriptorSetLayouts = DescriptorSetLayoutStructNames(model);
        foreach (var structure in model.Structs.Where(structure => !descriptorSetLayouts.Contains(structure.Name)))
        {
            var shaderModelStruct = shaderModel.Structs.FirstOrDefault(candidate => candidate.DeclId == structure.DeclId);
            builder.AppendLine($"struct {shaderModelStruct?.EmittedName ?? structure.Name}");
            builder.AppendLine("{");
            foreach (var field in structure.Fields)
            {
                builder.AppendLine($"    {MapValueType(field.Type)} {field.Name};");
            }
            builder.AppendLine("};");
            builder.AppendLine();
        }
    }

    private void WriteMethods(StringBuilder builder, CppslSemanticModel model, CppslShaderModel shaderModel)
    {
        var descriptorSetLayouts = DescriptorSetLayoutStructNames(model);
        foreach (var structure in model.Structs.Where(structure => !descriptorSetLayouts.Contains(structure.Name)))
        {
            var shaderModelStruct = shaderModel.Structs.FirstOrDefault(candidate => candidate.DeclId == structure.DeclId);
            foreach (var method in structure.Methods)
            {
                var shaderModelMethod = shaderModelStruct?
                    .Methods
                    .FirstOrDefault(candidate => candidate.DeclId == method.DeclId);
                if (shaderModelMethod?.Body is null)
                {
                    continue;
                }

                builder.Append(MapValueType(method.ReturnType ?? "void"));
                builder.Append(' ');
                builder.Append(GlslMethodName(method));
                builder.Append('(');
                var parameters = new List<string>
                {
                    $"{(method.IsConst ? string.Empty : "inout ")}{shaderModelStruct?.EmittedName ?? structure.Name} self"
                };
                parameters.AddRange(method.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}"));
                builder.Append(string.Join(", ", parameters));
                builder.AppendLine(")");

                _currentMethodFields = structure.Fields.Select(static field => field.Name).ToHashSet(StringComparer.Ordinal);
                WriteMethodBody(builder, method, model, shaderModelMethod.Body);
                _currentMethodFields = null;
                builder.AppendLine();
            }
        }
    }

    private void WriteResources(StringBuilder builder, CppslSemanticModel model)
    {
        foreach (var global in model.Globals.Where(static global => global.ResourceKind is not null))
        {
            switch (global.ResourceKind)
            {
                case "constant_buffer":
                    WriteConstantBuffer(builder, GlslBufferLayout(global, "std140"), global, model);
                    break;
                case "texture":
                    builder.AppendLine($"{GlslResourceLayout(global)} uniform {GlslTextureType(global.Type)} {global.Name};");
                    break;
                case "rw_texture":
                    builder.AppendLine($"layout(set = {global.DescriptorSet}, binding = {global.Binding}, {GlslImageFormat(global)}) uniform {GlslImageType(global)} {global.Name};");
                    break;
                case "sampler":
                    builder.AppendLine($"{GlslResourceLayout(global)} uniform sampler {global.Name};");
                    break;
                default:
                    builder.AppendLine($"{GlslBufferLayout(global, "std430")} buffer {global.Name}_Block");
                    builder.AppendLine("{");
                    builder.AppendLine($"    {MapValueType(ResourceElementType(global))} {global.Name}[];");
                    builder.AppendLine("};");
                    break;
            }
        }

        if (model.Globals.Any(static global => global.ResourceKind is not null))
        {
            builder.AppendLine();
        }
    }

    private static string GlslResourceLayout(CppslGlobal global)
    {
        return $"layout(set = {global.DescriptorSet}, binding = {global.Binding})";
    }

    private static string GlslBufferLayout(CppslGlobal global, string packing)
    {
        return $"layout(set = {global.DescriptorSet}, binding = {global.Binding}, {packing}, column_major)";
    }

    private static void WriteComputeLayout(StringBuilder builder, CppslFunction? entryPoint)
    {
        if (entryPoint?.Attributes.FindAttribute("compute") is not { Arguments.Count: >= 3 } compute)
        {
            return;
        }

        builder.AppendLine($"layout(local_size_x = {compute.Arguments[0]}, local_size_y = {compute.Arguments[1]}, local_size_z = {compute.Arguments[2]}) in;");
        builder.AppendLine();
    }

    private void WriteGroupSharedGlobals(StringBuilder builder, CppslSemanticModel model)
    {
        foreach (var global in model.Globals.Where(static global => global.Attributes.FindAttribute("group_shared") is not null))
        {
            builder.AppendLine($"shared {FormatVariableDeclaration(MapValueType(global.Type), global.Name)};");
        }

        if (model.Globals.Any(static global => global.Attributes.FindAttribute("group_shared") is not null))
        {
            builder.AppendLine();
        }
    }

    private void WriteConstantBuffer(
        StringBuilder builder,
        string layout,
        CppslGlobal global,
        CppslSemanticModel model)
    {
        var elementType = ResourceElementType(global);
        var structure = model.Structs.FirstOrDefault(candidate => candidate.Name == elementType);
        builder.AppendLine($"{layout} uniform {global.Name}_Block");
        builder.AppendLine("{");
        if (structure is null)
        {
            builder.AppendLine($"    {MapValueType(elementType)} value;");
        }
        else
        {
            foreach (var field in structure.Fields)
            {
                builder.AppendLine($"    {MapValueType(field.Type)} {field.Name};");
            }
        }
        builder.AppendLine($"}} {global.Name};");
    }

    private void WriteStageIo(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        ShaderStage stage)
    {
        if (entryPoint is null)
        {
            return;
        }

        foreach (var parameter in entryPoint.Parameters)
        {
            var inputStruct = model.Structs.FirstOrDefault(structure => structure.Name == parameter.Type);
            if (inputStruct is null)
            {
                continue;
            }

            foreach (var field in inputStruct.Fields.Where(static field => field.Location is not null))
            {
                builder.AppendLine($"layout(location = {field.Location}) {StageInputQualifier(stage, field.Type)}in {MapValueType(field.Type)} cppsl_in_{field.Name};");
            }
        }

        var outputStruct = model.Structs.FirstOrDefault(structure => structure.Name == entryPoint.ReturnType);
        if (outputStruct is not null)
        {
            foreach (var field in outputStruct.Fields.Where(static field => field.Location is not null))
            {
                builder.AppendLine($"layout(location = {field.Location}) {StageOutputQualifier(stage, field.Type)}out {MapValueType(field.Type)} cppsl_out_{field.Name};");
            }
        }

        builder.AppendLine();
    }

    private static string StageInputQualifier(ShaderStage stage, string type)
    {
        return (stage == ShaderStage.Fragment || stage == ShaderStage.Pixel) && IsIntegerLikeStageIoType(type)
            ? "flat "
            : string.Empty;
    }

    private static string StageOutputQualifier(ShaderStage stage, string type)
    {
        return stage == ShaderStage.Vertex && IsIntegerLikeStageIoType(type)
            ? "flat "
            : string.Empty;
    }

    private static bool IsIntegerLikeStageIoType(string type)
    {
        return NormalizeShaderTypeName(type) switch
        {
            "bool" or "bool_t" or "bool2" or "bool3" or "bool4" => true,
            "int" or "int2" or "int3" or "int4" => true,
            "uint" or "uint2" or "uint3" or "uint4" => true,
            _ => false
        };
    }

    private static bool IsBoolVectorType(string? type)
    {
        return NormalizeShaderTypeName(type ?? string.Empty) is "bool2" or "bool3" or "bool4";
    }

    private void WriteEntryPointHelper(
        StringBuilder builder,
        CppslFunction? entryPoint,
        CppslSemanticModel model,
        CppslShaderModel shaderModel)
    {
        if (entryPoint is null)
        {
            return;
        }

        builder.Append(MapValueType(entryPoint.ReturnType ?? "void"));
        builder.Append(' ');
        builder.Append(entryPoint.Name);
        builder.Append('(');
        builder.Append(string.Join(", ", entryPoint.Parameters.Select(parameter => $"{MapValueType(parameter.Type)} {parameter.Name}")));
        builder.AppendLine(")");
        WriteFunctionBody(builder, entryPoint, model, shaderModel);
    }

    private void WriteMain(StringBuilder builder, CppslFunction? entryPoint, CppslSemanticModel model)
    {
        if (entryPoint is null)
        {
            return;
        }

        builder.AppendLine();
        builder.AppendLine("void main()");
        builder.AppendLine("{");
        var entryArguments = new List<string>();
        foreach (var parameter in entryPoint.Parameters)
        {
            var parameterName = parameter.Name;
            var inputStruct = model.Structs.FirstOrDefault(structure => structure.Name == parameter.Type);
            if (inputStruct is not null)
            {
                builder.AppendLine($"    {parameter.Type} {parameterName};");
                foreach (var field in inputStruct.Fields.Where(static field => field.Location is not null))
                {
                    builder.AppendLine($"    {parameterName}.{field.Name} = cppsl_in_{field.Name};");
                }
                entryArguments.Add(parameterName);
                continue;
            }

            if (TryGetBuiltinExpression(parameter, out var builtinExpression))
            {
                builder.AppendLine($"    {MapValueType(parameter.Type)} {parameterName} = {builtinExpression};");
                entryArguments.Add(parameterName);
                continue;
            }

            entryArguments.Add(parameterName);
        }

        if (entryPoint.ReturnType is not null && entryPoint.ReturnType != "void")
        {
            builder.AppendLine($"    {entryPoint.ReturnType} cppsl_output = {entryPoint.Name}({string.Join(", ", entryArguments)});");
            var outputStruct = model.Structs.FirstOrDefault(structure => structure.Name == entryPoint.ReturnType);
            if (outputStruct is not null)
            {
                foreach (var field in outputStruct.Fields)
                {
                    if (field.IsPosition)
                    {
                        builder.AppendLine($"    gl_Position = cppsl_output.{field.Name};");
                    }
                    else if (field.Location is not null)
                    {
                        builder.AppendLine($"    cppsl_out_{field.Name} = cppsl_output.{field.Name};");
                    }
                }
            }
        }
        else
        {
            builder.AppendLine($"    {entryPoint.Name}({string.Join(", ", entryArguments)});");
        }

        builder.AppendLine("}");
    }

    protected override string FormatFloatingLiteral(string spelling)
    {
        return spelling.EndsWith('f') || spelling.EndsWith('F')
            ? spelling[..^1]
            : spelling;
    }

    protected override string LowerExpression(CppslShaderModelNode node)
    {
        if (node.Kind == CppslShaderModelNodeKind.MemberExpression &&
            node.Children.Count == 0 &&
            node.DisplayName is { } fieldName &&
            _currentMethodFields?.Contains(fieldName) == true)
        {
            return $"self.{fieldName}";
        }

        if (node.Kind == CppslShaderModelNodeKind.CStyleCastExpression && node.Children.Count == 1)
        {
            var type = node.Type ?? node.TypeInfo?.Spelling ?? string.Empty;
            var expression = LowerExpression(node.Children[0]);
            return NormalizeShaderTypeName(type) == "void"
                ? expression
                : $"{MapValueType(type)}({expression})";
        }

        if (node.Kind == CppslShaderModelNodeKind.BinaryOperator &&
            node.DisplayName is "==" or "!=" &&
            node.Children.Count >= 2 &&
            IsBoolVectorType(node.Type ?? node.TypeInfo?.Spelling))
        {
            var functionName = node.DisplayName == "==" ? "equal" : "notEqual";
            return $"{functionName}({LowerExpression(node.Children[0])}, {LowerExpression(node.Children[1])})";
        }

        if (node.Kind == CppslShaderModelNodeKind.OperatorCallExpression &&
            node.DisplayName is "operator==" or "operator!=" &&
            node.Children.Count >= 3 &&
            IsBoolVectorType(node.Type ?? node.TypeInfo?.Spelling))
        {
            var functionName = node.DisplayName == "operator==" ? "equal" : "notEqual";
            return $"{functionName}({LowerExpression(node.Children[1])}, {LowerExpression(node.Children[2])})";
        }

        if (TryGetMappedResourceAccess(node, _resourceAccessByPath, out var resourceAccess))
        {
            return resourceAccess;
        }

        return base.LowerExpression(node);
    }

    protected override string LowerMemberCall(string receiver, string memberName, IReadOnlyList<string> arguments)
    {
        if (memberName == "Sample" && arguments.Count == 2)
        {
            return $"texture(sampler2D({receiver}, {arguments[0]}), {arguments[1]})";
        }
        if (memberName == "SampleLevel" && arguments.Count == 3)
        {
            return $"textureLod(sampler2D({receiver}, {arguments[0]}), {arguments[1]}, {arguments[2]})";
        }
        if (memberName == "Load" && arguments.Count == 1)
        {
            return $"texelFetch({receiver}, {GlslTexelCoordinate(2, arguments[0])}, 0)";
        }
        if (memberName == "Store" && arguments.Count == 2)
        {
            return $"imageStore({receiver}, {GlslTexelCoordinate(2, arguments[0])}, {arguments[1]})";
        }

        return base.LowerMemberCall(receiver, memberName, arguments);
    }

    protected override string LowerCallExpression(CppslShaderModelNode node)
    {
        if (TryGetMemberCall(node, out var receiverNode, out var receiver, out var memberName, out var memberArguments))
        {
            if (TryGetMethodForCall(receiverNode, memberName, node.DirectCalleeDeclId, out var method))
            {
                return $"{GlslMethodName(method)}({receiver}{(memberArguments.Count == 0 ? string.Empty : ", ")}{string.Join(", ", memberArguments)})";
            }
            if (memberName == "Sample" && memberArguments.Count == 2)
            {
                var dimension = TextureDimension(receiverNode);
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"texture({GlslSamplerConstructor(dimension, receiver, memberArguments[0])}, {memberArguments[1]})");
            }
            if (memberName == "SampleLevel" && memberArguments.Count == 3)
            {
                var dimension = TextureDimension(receiverNode);
                return AdaptTextureReadExpression(
                    receiverNode,
                    $"textureLod({GlslSamplerConstructor(dimension, receiver, memberArguments[0])}, {memberArguments[1]}, {memberArguments[2]})");
            }
            if (memberName == "Load" && memberArguments.Count == 1)
            {
                var dimension = TextureDimension(receiverNode);
                var coordinate = GlslTexelCoordinate(dimension, memberArguments[0]);
                if (IsStorageTexture(receiverNode))
                {
                    return AdaptTextureReadExpression(
                        receiverNode,
                        $"imageLoad({receiver}, {coordinate})");
                }

                return AdaptTextureReadExpression(
                    receiverNode,
                    $"texelFetch({receiver}, {coordinate}, 0)");
            }
            if (memberName == "Store" && memberArguments.Count == 2)
            {
                var dimension = TextureDimension(receiverNode);
                return $"imageStore({receiver}, {GlslTexelCoordinate(dimension, memberArguments[0])}, {AdaptTextureStoreValue(receiverNode, memberArguments[1], "vec4", "ivec4", "uvec4", "0.0")})";
            }
        }

        var name = node.DisplayName ?? node.Spelling;
        var arguments = node.Children
            .Skip(IsCalleeReference(node.Children.FirstOrDefault(), name) ? 1 : 0)
            .Select(LowerExpression)
            .ToArray();

        if (name == "GroupMemoryBarrierWithGroupSync" && arguments.Length == 0)
        {
            return "barrier()";
        }
        if (name == "discard_fragment" && arguments.Length == 0)
        {
            return "discard";
        }
        if (name == "InterlockedAdd" && arguments.Length == 2)
        {
            return $"atomicAdd({arguments[0]}, {arguments[1]})";
        }
        if (name == "lerp" && arguments.Length == 3)
        {
            return $"mix({arguments[0]}, {arguments[1]}, {arguments[2]})";
        }
        if (name == "atan2" && arguments.Length == 2)
        {
            return $"atan({arguments[0]}, {arguments[1]})";
        }
        if (name == "saturate" && arguments.Length == 1)
        {
            return $"clamp({arguments[0]}, 0.0, 1.0)";
        }

        return base.LowerCallExpression(node);
    }

    private static (Dictionary<string, CppslMethod> ByDeclId, Dictionary<string, CppslMethod> UniqueByOwnerAndName, Dictionary<string, int> OverloadCounts) BuildMethodMaps(CppslSemanticModel model)
    {
        var byDeclId = new Dictionary<string, CppslMethod>(StringComparer.Ordinal);
        var byOwnerAndNameGroups = new Dictionary<string, List<CppslMethod>>(StringComparer.Ordinal);
        foreach (var structure in model.Structs)
        {
            foreach (var method in structure.Methods)
            {
                if (!string.IsNullOrWhiteSpace(method.DeclId))
                {
                    byDeclId[method.DeclId] = method;
                }

                var key = MethodKey(structure.Name, method.Name);
                if (!byOwnerAndNameGroups.TryGetValue(key, out var group))
                {
                    group = new List<CppslMethod>();
                    byOwnerAndNameGroups.Add(key, group);
                }
                group.Add(method);
            }
        }

        var uniqueByOwnerAndName = new Dictionary<string, CppslMethod>(StringComparer.Ordinal);
        var overloadCounts = new Dictionary<string, int>(StringComparer.Ordinal);
        foreach (var (key, group) in byOwnerAndNameGroups)
        {
            overloadCounts[key] = group.Count;
            if (group.Count == 1)
            {
                uniqueByOwnerAndName[key] = group[0];
            }
        }

        return (byDeclId, uniqueByOwnerAndName, overloadCounts);
    }

    private static Dictionary<string, string> BuildGlslMethodNamesByDeclId(
        CppslSemanticModel model,
        CppslShaderModel shaderModel)
    {
        var result = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var structure in model.Structs)
        {
            var shaderModelStruct = shaderModel.Structs.FirstOrDefault(candidate => candidate.DeclId == structure.DeclId);
            var ownerName = shaderModelStruct?.EmittedName ?? structure.Name;
            foreach (var method in structure.Methods)
            {
                if (string.IsNullOrWhiteSpace(method.DeclId))
                {
                    continue;
                }

                var shaderModelMethod = shaderModelStruct?.Methods.FirstOrDefault(candidate => candidate.DeclId == method.DeclId);
                var emittedMemberName = shaderModelMethod?.EmittedName ?? method.Name;
                var baseName = $"{ownerName}_{emittedMemberName}";
                var overloadCount = structure.Methods.Count(candidate => candidate.Name == method.Name);
                result[method.DeclId] = !method.IsTemplateInstantiation &&
                    overloadCount > 1
                        ? $"{baseName}_ov_{StableIdentifierSuffix(method.DeclId ?? method.DisplayName ?? method.Name)}"
                        : baseName;
            }
        }
        return result;
    }

    private bool TryGetMethodForCall(
        CppslShaderModelNode receiverNode,
        string methodName,
        string? directCalleeDeclId,
        out CppslMethod method)
    {
        if (!string.IsNullOrWhiteSpace(directCalleeDeclId) &&
            _methodsByDeclId.TryGetValue(directCalleeDeclId, out method!))
        {
            return true;
        }

        foreach (var candidate in EnumerateNodeTypeCandidates(receiverNode).Select(NormalizeShaderTypeName))
        {
            if (_uniqueMethodsByOwnerAndName.TryGetValue(MethodKey(candidate, methodName), out method!))
            {
                return true;
            }
        }

        method = null!;
        return false;
    }

    private static string MethodKey(string ownerType, string methodName)
    {
        return $"{NormalizeShaderTypeName(ownerType)}.{methodName}";
    }

    private string GlslMethodName(CppslMethod method)
    {
        if (method.DeclId is not null && _glslMethodNamesByDeclId.TryGetValue(method.DeclId, out var emittedName))
        {
            return emittedName;
        }

        var emittedMemberName = MapMethodName(method.DeclId, method.Name);
        var baseName = $"{MapShaderTypeName(method.OwnerType)}_{emittedMemberName}";
        return !method.IsTemplateInstantiation &&
            _methodOverloadCounts.TryGetValue(MethodKey(method.OwnerType, method.Name), out var count) && count > 1
            ? $"{baseName}_ov_{StableIdentifierSuffix(method.DeclId ?? method.DisplayName ?? method.Name)}"
            : baseName;
    }

    private static string StableIdentifierSuffix(string value)
    {
        const uint offset = 2166136261u;
        const uint prime = 16777619u;
        var hash = offset;
        foreach (var ch in value)
        {
            hash ^= ch;
            hash *= prime;
        }
        return hash.ToString("x8");
    }

    protected override string MapValueType(string type)
    {
        var normalized = MapShaderTypeName(type);
        return normalized switch
        {
            "float2" => "vec2",
            "float3" => "vec3",
            "float4" => "vec4",
            "float4x4" => "mat4",
            "int2" => "ivec2",
            "int3" => "ivec3",
            "int4" => "ivec4",
            "bool2" => "bvec2",
            "bool3" => "bvec3",
            "bool4" => "bvec4",
            "uint2" => "uvec2",
            "uint3" => "uvec3",
            "uint4" => "uvec4",
            "uint" => "uint",
            "_Bool" => "bool",
            "bool_t" => "bool",
            _ => normalized
        };
    }

    protected override string DefaultStructValue(CppslStruct structure, CppslSemanticModel model)
    {
        return $"{structure.Name}({string.Join(", ", structure.Fields.Select(field => DefaultValue(field.Type, model)))})";
    }

    protected override string DefaultAggregateValue(string mappedType)
    {
        return $"{mappedType}(0.0)";
    }

    private static bool TryGetBuiltinExpression(CppslParameter parameter, out string expression)
    {
        expression = parameter.Attributes.FindAttribute("builtin")?.Arguments.FirstOrDefault() switch
        {
            "dispatch_thread_id" => "uvec3(gl_GlobalInvocationID)",
            "group_index" => "uint(gl_LocalInvocationIndex)",
            _ => string.Empty
        };
        return expression.Length != 0;
    }

    private static string GlslTextureType(string type)
    {
        return TextureDimension(type) switch
        {
            1 => "texture1D",
            3 => "texture3D",
            _ => "texture2D"
        };
    }

    private static string GlslImageType(CppslGlobal global)
    {
        var prefix = TextureElementType(global) switch
        {
            "int" or "int2" or "int3" or "int4" => "i",
            "uint" or "uint2" or "uint3" or "uint4" => "u",
            _ => string.Empty
        };
        return TextureDimension(global.Type) switch
        {
            1 => $"{prefix}image1D",
            3 => $"{prefix}image3D",
            _ => $"{prefix}image2D"
        };
    }

    private static string GlslImageFormat(CppslGlobal global)
    {
        return TextureElementType(global) switch
        {
            "float" => "r32f",
            "float2" => "rg32f",
            "float3" or "float4" => "rgba32f",
            "int" => "r32i",
            "int2" => "rg32i",
            "int3" or "int4" => "rgba32i",
            "uint" => "r32ui",
            "uint2" => "rg32ui",
            "uint3" or "uint4" => "rgba32ui",
            _ => "rgba32f"
        };
    }

    private static string TextureElementType(CppslGlobal global)
    {
        return global.Type.Contains('<', StringComparison.Ordinal)
            ? NormalizeShaderTypeName(UnwrapTemplateArgument(global.Type))
            : NormalizeShaderTypeName(global.Type);
    }

    private static int TextureDimension(CppslShaderModelNode node)
    {
        return TryGetTextureInfo(node, out var textureInfo) ? textureInfo.Dimension : 2;
    }

    private static int TextureDimension(string type)
    {
        return TryGetTextureInfo(type, out var textureInfo) ? textureInfo.Dimension : 2;
    }

    private static bool IsStorageTexture(CppslShaderModelNode node)
    {
        return TryGetTextureInfo(node, out var textureInfo) && textureInfo.IsStorage;
    }

    private static string GlslSamplerConstructor(int dimension, string texture, string sampler)
    {
        var samplerType = dimension switch
        {
            1 => "sampler1D",
            3 => "sampler3D",
            _ => "sampler2D"
        };
        return $"{samplerType}({texture}, {sampler})";
    }

    private static string GlslTexelCoordinate(int dimension, string coordinate)
    {
        return dimension switch
        {
            1 => $"int({coordinate})",
            3 => $"ivec3({coordinate})",
            _ => $"ivec2({coordinate})"
        };
    }

}
