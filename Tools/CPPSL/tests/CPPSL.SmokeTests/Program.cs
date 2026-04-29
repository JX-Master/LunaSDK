using CPPSL.Core.Artifacts;
using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.Frontend;
using CPPSL.Core.ShaderModel;
using System.Text.RegularExpressions;

var repoRoot = FindRepoRoot(AppContext.BaseDirectory);
var fixturesRoot = Path.Combine(repoRoot, "Tools", "CPPSL", "tests", "fixtures");
var stdRoot = Path.Combine(repoRoot, "Tools", "CPPSL", "std");
var outputDir = Path.Combine(repoRoot, "build", "cppsl-smoke");
var compiler = new CppslCompiler();
var boxShader = Path.Combine(fixturesRoot, "valid", "box", "Box.cxx");
var textureBasicShader = Path.Combine(fixturesRoot, "valid", "texture_basic", "TextureBasic.cxx");
var texture1DSampleLevelShader = Path.Combine(fixturesRoot, "valid", "texture1d_sample_level", "Texture1DSampleLevel.cxx");
var textureComponentInitShader = Path.Combine(fixturesRoot, "valid", "texture_component_init", "TextureComponentInit.cxx");
var textureScalarShader = Path.Combine(fixturesRoot, "valid", "texture_scalar", "TextureScalar.cxx");
var structuredBufferAccessShader = Path.Combine(fixturesRoot, "valid", "structured_buffer_access", "StructuredBufferAccess.cxx");
var helperResourceAccessShader = Path.Combine(fixturesRoot, "valid", "helper_resource_access", "HelperResourceAccess.cxx");
var resourceAttributesShader = Path.Combine(fixturesRoot, "valid", "resource_attributes", "ResourceAttributes.cxx");
var descriptorSetShader = Path.Combine(fixturesRoot, "valid", "descriptor_set", "DescriptorSet.cxx");
var vectorSwizzleShader = Path.Combine(fixturesRoot, "valid", "vector_swizzle", "VectorSwizzle.cxx");
var vectorConstructorShader = Path.Combine(fixturesRoot, "valid", "vector_constructor", "VectorConstructor.cxx");
var structMethodShader = Path.Combine(fixturesRoot, "valid", "struct_method", "StructMethod.cxx");
var nonConstStructMethodShader = Path.Combine(fixturesRoot, "valid", "non_const_struct_method", "NonConstStructMethod.cxx");
var schemaV3Shader = Path.Combine(fixturesRoot, "valid", "schema_v3", "SchemaV3.cxx");
var defaultArgumentsOverloadShader = Path.Combine(fixturesRoot, "valid", "default_arguments_overload", "DefaultArgumentsOverload.cxx");
var constexprEvaluationShader = Path.Combine(fixturesRoot, "valid", "constexpr_evaluation", "ConstexprEvaluation.cxx");
var functionTemplateShader = Path.Combine(fixturesRoot, "valid", "function_template", "FunctionTemplate.cxx");
var templateMemberFunctionShader = Path.Combine(fixturesRoot, "valid", "template_member_function", "TemplateMemberFunction.cxx");
var templateStructShader = Path.Combine(fixturesRoot, "valid", "template_struct", "TemplateStruct.cxx");

if (!ExpectSchemaV3Facts(schemaV3Shader, stdRoot))
{
    return 1;
}

var constexprEvaluationResult = compiler.Compile(new CppslCompileOptions(
    constexprEvaluationShader,
    Path.Combine(outputDir, "constexpr-evaluation"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));
if (!constexprEvaluationResult.Succeeded || constexprEvaluationResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected constexpr_evaluation fixture to compile.");
    foreach (var diagnostic in constexprEvaluationResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var constexprEvaluationHlslText = File.ReadAllText(constexprEvaluationResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var constexprEvaluationGlslText = File.ReadAllText(constexprEvaluationResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var constexprEvaluationMslText = File.ReadAllText(constexprEvaluationResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!AssertNotContainsAny(
        constexprEvaluationHlslText + constexprEvaluationGlslText + constexprEvaluationMslText,
        new[] { "kSteps", "kBias", "kUseBias", "999.0" },
        "expected constexpr globals and discarded branches to be folded away."))
{
    return 1;
}
if (!AssertContainsAll(
        constexprEvaluationGlslText,
        new[] { "float ApplyConstexpr(float value)", "return ((value + 1.25) + 4.0);", "vec4(ApplyConstexpr(v.weight), 4.0, 0.625, 1.0)" },
        "expected GLSL constexpr values to be folded into scalar literals."))
{
    return 1;
}
if (!AssertNotContainsAny(
        constexprEvaluationHlslText + constexprEvaluationGlslText + constexprEvaluationMslText,
        new[] { "local_scale" },
        "expected constexpr local declarations to be folded away."))
{
    return 1;
}

var defaultArgumentsOverloadResult = compiler.Compile(new CppslCompileOptions(
    defaultArgumentsOverloadShader,
    Path.Combine(outputDir, "default-arguments-overload"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));
if (!defaultArgumentsOverloadResult.Succeeded || defaultArgumentsOverloadResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected default_arguments_overload fixture to compile.");
    foreach (var diagnostic in defaultArgumentsOverloadResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var defaultArgumentsOverloadHlslText = File.ReadAllText(defaultArgumentsOverloadResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var defaultArgumentsOverloadGlslText = File.ReadAllText(defaultArgumentsOverloadResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var defaultArgumentsOverloadMslText = File.ReadAllText(defaultArgumentsOverloadResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!AssertContainsAll(
        defaultArgumentsOverloadHlslText,
        new[] { "float MixValue(float value, float bias)", "float2 MixValue(float2 value, float bias)", "float Apply(float value)", "float3 Apply(float3 value)" },
        "expected HLSL overload declarations to be emitted."))
{
    return 1;
}
if (!AssertContainsAll(
        defaultArgumentsOverloadMslText,
        new[] { "float MixValue(float value, float bias)", "float2 MixValue(float2 value, float bias)", "float Apply(float value)", "float3 Apply(float3 value)" },
        "expected Metal overload declarations to be emitted."))
{
    return 1;
}
if (!AssertContainsAll(
        defaultArgumentsOverloadGlslText,
        new[] { "float MixValue(float value, float bias)", "vec2 MixValue(vec2 value, float bias)", "float Mixer_Apply_ov_", "vec3 Mixer_Apply_ov_", "MixValue(v.weight, 0.25" },
        "expected GLSL default arguments and method overload lowering to be emitted."))
{
    return 1;
}

var functionTemplateResult = compiler.Compile(new CppslCompileOptions(
    functionTemplateShader,
    Path.Combine(outputDir, "function-template"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));
if (!functionTemplateResult.Succeeded || functionTemplateResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected function_template fixture to compile.");
    foreach (var diagnostic in functionTemplateResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var functionTemplateShaderModelText = File.ReadAllText(functionTemplateResult.Artifacts.ShaderModelPath);
var functionTemplateHlslText = File.ReadAllText(functionTemplateResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var functionTemplateGlslText = File.ReadAllText(functionTemplateResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var functionTemplateMslText = File.ReadAllText(functionTemplateResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!AssertContainsAll(
        functionTemplateShaderModelText,
        new[] { "\"IsTemplateInstantiation\": true", "\"EmittedName\": \"AddBias_tpl_float\"", "\"EmittedName\": \"AddBias_tpl_float3\"", "\"EmittedName\": \"SelectValue_tpl_float\"" },
        "expected shader model to include concrete function template instantiations."))
{
    return 1;
}
if (!AssertContainsAll(
        functionTemplateHlslText,
        new[] { "float AddBias_tpl_float(float value, float bias)", "float3 AddBias_tpl_float3(float3 value, float3 bias)", "float SelectValue_tpl_float(bool use_left, float left, float right)", "AddBias_tpl_float(v.weight, 0.25f)", "AddBias_tpl_float3(v.position, float3(0.5f, 0.5f, 0.5f))", "SelectValue_tpl_float(true, scalar, v.weight)" },
        "expected HLSL function template specializations and rewritten calls to be emitted."))
{
    return 1;
}
if (!AssertContainsAll(
        functionTemplateGlslText,
        new[] { "float AddBias_tpl_float(float value, float bias)", "vec3 AddBias_tpl_float3(vec3 value, vec3 bias)", "float SelectValue_tpl_float(bool use_left, float left, float right)", "AddBias_tpl_float(v.weight, 0.25)", "AddBias_tpl_float3(v.position, vec3(0.5, 0.5, 0.5))", "SelectValue_tpl_float(true, scalar, v.weight)" },
        "expected GLSL function template specializations and rewritten calls to be emitted."))
{
    return 1;
}
if (!AssertContainsAll(
        functionTemplateMslText,
        new[] { "float AddBias_tpl_float(float value, float bias)", "float3 AddBias_tpl_float3(float3 value, float3 bias)", "float SelectValue_tpl_float(bool use_left, float left, float right)", "AddBias_tpl_float(v.weight, 0.25f)", "AddBias_tpl_float3(v.position, float3(0.5f, 0.5f, 0.5f))", "SelectValue_tpl_float(true, scalar, v.weight)" },
        "expected MSL function template specializations and rewritten calls to be emitted."))
{
    return 1;
}
var combineTemplateNames = Regex.Matches(functionTemplateHlslText, "\\bCombine_tpl_float_[0-9a-f]{8}\\b")
    .Select(static match => match.Value)
    .Distinct(StringComparer.Ordinal)
    .ToArray();
if (combineTemplateNames.Length != 2 ||
    combineTemplateNames.Any(name =>
        !functionTemplateShaderModelText.Contains($"\"EmittedName\": \"{name}\"", StringComparison.Ordinal) ||
        !functionTemplateHlslText.Contains($"float {name}(", StringComparison.Ordinal) ||
        !functionTemplateGlslText.Contains($"float {name}(", StringComparison.Ordinal) ||
        !functionTemplateMslText.Contains($"float {name}(", StringComparison.Ordinal)))
{
    Console.Error.WriteLine("error: expected overloaded function template instantiations to receive stable unique names.");
    foreach (var name in combineTemplateNames)
    {
        Console.Error.WriteLine($"  observed: {name}");
    }
    return 1;
}
if (!AssertNotContainsAny(
        functionTemplateHlslText + functionTemplateGlslText + functionTemplateMslText,
        new[] { "template <", "template<" },
        "target shader sources must contain only concrete function template specializations."))
{
    return 1;
}

var templateMemberFunctionResult = compiler.Compile(new CppslCompileOptions(
    templateMemberFunctionShader,
    Path.Combine(outputDir, "template-member-function"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));
if (!templateMemberFunctionResult.Succeeded || templateMemberFunctionResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected template_member_function fixture to compile.");
    foreach (var diagnostic in templateMemberFunctionResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var templateMemberFunctionShaderModelText = File.ReadAllText(templateMemberFunctionResult.Artifacts.ShaderModelPath);
var templateMemberFunctionHlslText = File.ReadAllText(templateMemberFunctionResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var templateMemberFunctionGlslText = File.ReadAllText(templateMemberFunctionResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var templateMemberFunctionMslText = File.ReadAllText(templateMemberFunctionResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!AssertContainsAll(
        templateMemberFunctionShaderModelText,
        new[] { "\"IsTemplateInstantiation\": true", "\"EmittedName\": \"Add_tpl_float\"", "\"EmittedName\": \"Add_tpl_float3\"" },
        "expected shader model to include concrete method template instantiations."))
{
    return 1;
}
if (!AssertContainsAll(
        templateMemberFunctionHlslText,
        new[] { "float Add_tpl_float(float value, float bias)", "float3 Add_tpl_float3(float3 value, float3 bias)", "biaser.Add_tpl_float(v.weight, biaser.base)", "biaser.Add_tpl_float3(v.position, float3(0.5f, 0.5f, 0.5f))" },
        "expected HLSL method template specializations and rewritten member calls to be emitted."))
{
    return 1;
}
if (!AssertContainsAll(
        templateMemberFunctionGlslText,
        new[] { "float Biaser_Add_tpl_float(Biaser self, float value, float bias)", "vec3 Biaser_Add_tpl_float3(Biaser self, vec3 value, vec3 bias)", "Biaser_Add_tpl_float(biaser, v.weight, biaser.base)", "Biaser_Add_tpl_float3(biaser, v.position, vec3(0.5, 0.5, 0.5))" },
        "expected GLSL method template specializations to lower to free functions and rewritten calls."))
{
    return 1;
}
if (!AssertContainsAll(
        templateMemberFunctionMslText,
        new[] { "float Add_tpl_float(float value, float bias)", "float3 Add_tpl_float3(float3 value, float3 bias)", "biaser.Add_tpl_float(v.weight, biaser.base)", "biaser.Add_tpl_float3(v.position, float3(0.5f, 0.5f, 0.5f))" },
        "expected MSL method template specializations and rewritten member calls to be emitted."))
{
    return 1;
}
var combineMethodNames = Regex.Matches(templateMemberFunctionHlslText, "\\bCombine_tpl_float_[0-9a-f]{8}\\b")
    .Select(static match => match.Value)
    .Distinct(StringComparer.Ordinal)
    .ToArray();
if (combineMethodNames.Length != 2 ||
    combineMethodNames.Any(name =>
        !templateMemberFunctionShaderModelText.Contains($"\"EmittedName\": \"{name}\"", StringComparison.Ordinal) ||
        !templateMemberFunctionHlslText.Contains($"float {name}(", StringComparison.Ordinal) ||
        !templateMemberFunctionHlslText.Contains($"biaser.{name}(", StringComparison.Ordinal) ||
        !templateMemberFunctionGlslText.Contains($"float Biaser_{name}(", StringComparison.Ordinal) ||
        !templateMemberFunctionGlslText.Contains($"Biaser_{name}(biaser", StringComparison.Ordinal) ||
        !templateMemberFunctionMslText.Contains($"float {name}(", StringComparison.Ordinal) ||
        !templateMemberFunctionMslText.Contains($"biaser.{name}(", StringComparison.Ordinal)))
{
    Console.Error.WriteLine("error: expected overloaded method template instantiations to receive stable unique names.");
    foreach (var name in combineMethodNames)
    {
        Console.Error.WriteLine($"  observed: {name}");
    }
    return 1;
}
if (!AssertNotContainsAny(
        templateMemberFunctionHlslText + templateMemberFunctionGlslText + templateMemberFunctionMslText,
        new[] { "template <", "template<", "biaser.Add(" },
        "target shader sources must contain only concrete method template specializations."))
{
    return 1;
}

var templateStructResult = compiler.Compile(new CppslCompileOptions(
    templateStructShader,
    Path.Combine(outputDir, "template-struct"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));
if (!templateStructResult.Succeeded || templateStructResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected template_struct fixture to compile.");
    foreach (var diagnostic in templateStructResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var templateStructShaderModelText = File.ReadAllText(templateStructResult.Artifacts.ShaderModelPath);
var templateStructHlslText = File.ReadAllText(templateStructResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var templateStructGlslText = File.ReadAllText(templateStructResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var templateStructMslText = File.ReadAllText(templateStructResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!AssertContainsAll(
        templateStructShaderModelText,
        new[] { "\"EmittedName\": \"Pair_tpl_float\"", "\"EmittedName\": \"Pair_tpl_float3\"", "\"EmittedName\": \"Box_tpl_float\"", "\"IsTemplateInstantiation\": true" },
        "expected shader model to include concrete template struct/class instantiations."))
{
    return 1;
}
if (!AssertContainsAll(
        templateStructHlslText,
        new[] { "struct Pair_tpl_float", "float left;", "float Sum()", "struct Pair_tpl_float3", "float3 left;", "float3 Sum()", "struct Box_tpl_float", "float value;", "float Twice()", "Pair_tpl_float scalar;", "Box_tpl_float boxed;", "Pair_tpl_float3 vector;", "float combined = (scalar.Sum() + boxed.Twice());", "float3 shifted = vector.Sum();" },
        "expected HLSL template struct/class specializations and concrete variable types to be emitted."))
{
    return 1;
}
if (!AssertContainsAll(
        templateStructGlslText,
        new[] { "struct Pair_tpl_float", "float left;", "float Pair_tpl_float_Sum(Pair_tpl_float self)", "struct Pair_tpl_float3", "vec3 left;", "vec3 Pair_tpl_float3_Sum(Pair_tpl_float3 self)", "struct Box_tpl_float", "float value;", "float Box_tpl_float_Twice(Box_tpl_float self)", "Pair_tpl_float scalar;", "Box_tpl_float boxed;", "Pair_tpl_float3 vector;", "float combined = (Pair_tpl_float_Sum(scalar) + Box_tpl_float_Twice(boxed));", "vec3 shifted = Pair_tpl_float3_Sum(vector);" },
        "expected GLSL template struct/class specializations and lowered method calls to be emitted."))
{
    return 1;
}
if (!AssertContainsAll(
        templateStructMslText,
        new[] { "struct Pair_tpl_float", "float left;", "float Sum()", "struct Pair_tpl_float3", "float3 left;", "float3 Sum()", "struct Box_tpl_float", "float value;", "float Twice()", "Pair_tpl_float scalar;", "Box_tpl_float boxed;", "Pair_tpl_float3 vector;", "float combined = (scalar.Sum() + boxed.Twice());", "float3 shifted = vector.Sum();" },
        "expected MSL template struct/class specializations and concrete variable types to be emitted."))
{
    return 1;
}
if (!AssertNotContainsAny(
        templateStructHlslText + templateStructGlslText + templateStructMslText,
        new[] { "template <", "template<", "Pair<float", "Pair <float" },
        "target shader sources must contain only concrete template struct specializations."))
{
    return 1;
}

var result = compiler.Compile(new CppslCompileOptions(
    boxShader,
    outputDir,
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));

foreach (var diagnostic in result.Diagnostics)
{
    Console.Error.WriteLine(diagnostic.ToDisplayString());
}

if (!result.Succeeded)
{
    return 1;
}

if (result.Artifacts is null ||
    !File.Exists(result.Artifacts.ShaderModelPath) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Reflection)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Glsl)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Msl)))
{
    Console.Error.WriteLine("error: expected CPPSL smoke artifacts were not generated.");
    return 1;
}

var shaderModelText = File.ReadAllText(result.Artifacts.ShaderModelPath);
var reflectionText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Reflection));
var hlslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var glslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var mslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!AssertContainsAll(
        shaderModelText,
        new[]
        {
            "main_vs",
            "\"frontendProvider\": \"Native\"",
            "\"frontendModelVersion\": 3",
            "\"frontendAst\"",
            "\"outputTargets\"",
            "\"Kind\": \"Function\"",
            "\"ProviderKind\": \"Function\"",
            "\"DeclId\":",
            "\"CanonicalDeclId\":",
            "\"Kind\": \"Field\"",
            "\"ProviderKind\": \"Field\"",
            "world_to_proj",
            "\"Kind\": \"Parameter\"",
            "\"ProviderKind\": \"ParmVar\"",
            "\"OwnerDeclId\":",
            "\"Range\": {",
            "\"TypeInfo\": {",
            "\"TemplateArguments\"",
            "\"Spelling\": \"Camera\"",
            "\"ResultTypeInfo\": {",
            "\"Kind\": \"CompoundStatement\"",
            "\"Kind\": \"DeclarationStatement\"",
            "\"Kind\": \"LocalVariable\"",
            "\"Spelling\": \"o\"",
            "\"Kind\": \"CallExpression\"",
            "\"DisplayName\": \"mul\"",
            "\"DirectCalleeDeclId\":",
            "\"Kind\": \"MemberExpression\"",
            "\"ReferencedDeclId\":",
            "\"Kind\": \"DeclRefExpression\"",
            "\"ConstantValue\":",
            "\"Kind\": \"ReturnStatement\"",
            "cppslSemanticModel",
            "cppslShaderModel",
            "\"Schema\": \"cppsl.shader_model\"",
            "\"Version\": 1",
            "constant_buffer",
            "\"IsEntryPoint\": true",
            "\"Name\": \"desc_set\"",
            "\"Name\": \"binding\"",
            "\"Name\": \"cbuffer\"",
            "\"Name\": \"location\"",
            "\"Name\": \"position\"",
            "\"Name\": \"vertex\"",
            "\"DescriptorSet\": 0",
            "\"Binding\": 0",
            "\"Location\": 0",
            "\"IsPosition\": true",
            "\"DeclaredStage\": \"vertex\""
        },
        "expected frontend, semantic, or shader model facts were not written."))
{
    return 1;
}
if (!Regex.IsMatch(
        shaderModelText,
        "\"cppslShaderModel\"\\s*:\\s*\\{.*?\"Version\"\\s*:\\s*1.*?\"EntryPoints\"\\s*:\\s*\\[.*?\"Name\"\\s*:\\s*\"main_vs\".*?\"Body\"\\s*:\\s*\\{.*?\"Kind\"\\s*:\\s*\"CompoundStatement\".*?\"Kind\"\\s*:\\s*\"LocalVariable\".*?\"DisplayName\"\\s*:\\s*\"mul\".*?\"Kind\"\\s*:\\s*\"ReturnStatement\"",
        RegexOptions.Singleline))
{
    Console.Error.WriteLine("error: expected entry point body was not written to CPPSL shader model.");
    return 1;
}
if (!AssertNotContainsAny(
        shaderModelText,
        new[] { "cppsl" + "Ir", "cppsl." + "ir", "\"phase\"", "phase " + "0", "CPPSL " + "IR" },
        "shader model artifact must not contain legacy intermediate-form or prototype metadata."))
{
    return 1;
}
if (!AssertContainsAll(
        reflectionText,
        new[]
        {
            "\"Schema\": \"cppsl.reflection\"",
            "\"Version\": 0",
            "\"EntryPoint\": \"main_vs\"",
            "\"Descriptors\"",
            "\"Name\": \"frame_camera\"",
            "\"Type\": \"Camera\"",
            "\"ResourceKind\": \"constant_buffer\"",
            "\"Set\": 0",
            "\"Binding\": 0",
            "\"StageInputs\"",
            "\"Struct\": \"VSInput\"",
            "\"StageOutputs\"",
            "\"Struct\": \"VSOutput\"",
            "\"IsPosition\": true"
        },
        "expected CPPSL reflection facts were not written."))
{
    return 1;
}
if (shaderModelText.Contains("\"Name\": \"output\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: local variables must not be classified as CPPSL globals.");
    return 1;
}
if (!Regex.IsMatch(shaderModelText, "\"Name\": \"v\".*?\"Attributes\": \\[\\s*\\]", RegexOptions.Singleline))
{
    Console.Error.WriteLine("error: function parameters must not inherit function attributes.");
    return 1;
}
if (!AssertContainsAll(
        hlslText,
        new[]
        {
            "struct VSInput",
            "#pragma pack_matrix(column_major)",
            "float4 position : SV_Position",
            "ConstantBuffer<Camera> frame_camera : register(b0, space0);",
            "VSOutput main_vs(VSInput v)",
            "o.position = mul(frame_camera.world_to_proj, float4(v.position.x, v.position.y, v.position.z, 1.0f));",
            "o.texcoord = v.texcoord;",
            "return o;"
        },
        "expected HLSL Box source facts were not emitted.") ||
    !AssertContainsAll(
        glslText,
        new[]
        {
            "#version 450",
            "mat4 world_to_proj",
            "layout(set = 0, binding = 0, std140, column_major) uniform frame_camera_Block",
            "o.position = (frame_camera.world_to_proj * vec4(v.position.x, v.position.y, v.position.z, 1.0));",
            "void main()",
            "gl_Position = cppsl_output.position;"
        },
        "expected GLSL Box source facts were not emitted.") ||
    !AssertContainsAll(
        mslText,
        new[]
        {
            "#include <metal_stdlib>",
            "float3 position [[attribute(0)]]",
            "float4 position [[position]]",
            "struct spvDescriptorSetBuffer0",
            "constant Camera* frame_camera [[id(0)]]",
            "constant spvDescriptorSetBuffer0* constant spvDescriptorSet0 [[buffer(0)]]",
            "vertex VSOutput main_vs",
            "o.position = ((*(*spvDescriptorSet0).frame_camera).world_to_proj * float4(v.position.x, v.position.y, v.position.z, 1.0f));",
            "return o;"
        },
        "expected MSL Box source facts were not emitted."))
{
    return 1;
}

var vectorSwizzleResult = compiler.Compile(new CppslCompileOptions(
    vectorSwizzleShader,
    Path.Combine(outputDir, "vector-swizzle"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));

if (!vectorSwizzleResult.Succeeded || vectorSwizzleResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected vector_swizzle fixture to compile.");
    foreach (var diagnostic in vectorSwizzleResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var vectorSwizzleShaderModelText = File.ReadAllText(vectorSwizzleResult.Artifacts.ShaderModelPath);
var vectorSwizzleHlslText = File.ReadAllText(vectorSwizzleResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var vectorSwizzleGlslText = File.ReadAllText(vectorSwizzleResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var vectorSwizzleMslText = File.ReadAllText(vectorSwizzleResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!vectorSwizzleShaderModelText.Contains("\"DisplayName\": \"xyz\"", StringComparison.Ordinal) ||
    !vectorSwizzleShaderModelText.Contains("\"DisplayName\": \"rgb\"", StringComparison.Ordinal) ||
    !vectorSwizzleShaderModelText.Contains("\"DisplayName\": \"zwx\"", StringComparison.Ordinal) ||
    !vectorSwizzleHlslText.Contains("float3 xyz = v.color.xyz;", StringComparison.Ordinal) ||
    !vectorSwizzleHlslText.Contains("float3 rgb = v.color.rgb;", StringComparison.Ordinal) ||
    !vectorSwizzleHlslText.Contains("float3 zwx = v.color.zwx;", StringComparison.Ordinal) ||
    !vectorSwizzleHlslText.Contains("float2 zw = v.color.zw;", StringComparison.Ordinal) ||
    !vectorSwizzleHlslText.Contains("float4 reversed = v.color.wzyx;", StringComparison.Ordinal) ||
    !vectorSwizzleHlslText.Contains("float4 repeated = v.normal.xyyz;", StringComparison.Ordinal) ||
    !vectorSwizzleGlslText.Contains("vec3 xyz = v.color.xyz;", StringComparison.Ordinal) ||
    !vectorSwizzleGlslText.Contains("vec3 rgb = v.color.rgb;", StringComparison.Ordinal) ||
    !vectorSwizzleGlslText.Contains("vec3 zwx = v.color.zwx;", StringComparison.Ordinal) ||
    !vectorSwizzleMslText.Contains("float3 xyz = v.color.xyz;", StringComparison.Ordinal) ||
    !vectorSwizzleMslText.Contains("float3 rgb = v.color.rgb;", StringComparison.Ordinal) ||
    !vectorSwizzleMslText.Contains("float3 zwx = v.color.zwx;", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected vector swizzle members to type-check and lower as source swizzles.");
    return 1;
}

var vectorConstructorResult = compiler.Compile(new CppslCompileOptions(
    vectorConstructorShader,
    Path.Combine(outputDir, "vector-constructor"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));

if (!vectorConstructorResult.Succeeded || vectorConstructorResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected vector_constructor fixture to compile.");
    foreach (var diagnostic in vectorConstructorResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var vectorConstructorHlslText = File.ReadAllText(vectorConstructorResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var vectorConstructorGlslText = File.ReadAllText(vectorConstructorResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var vectorConstructorMslText = File.ReadAllText(vectorConstructorResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!vectorConstructorHlslText.Contains("o.rgba = float4(v.color.rgb, v.weight);", StringComparison.Ordinal) ||
    !vectorConstructorHlslText.Contains("o.xgba = float4(v.color.x, v.color.gba);", StringComparison.Ordinal) ||
    !vectorConstructorHlslText.Contains("o.xyzw = float4(v.color.xy, v.color.zw);", StringComparison.Ordinal) ||
    !vectorConstructorHlslText.Contains("o.splat = float4(v.weight);", StringComparison.Ordinal) ||
    !vectorConstructorGlslText.Contains("o.rgba = vec4(v.color.rgb, v.weight);", StringComparison.Ordinal) ||
    !vectorConstructorGlslText.Contains("o.xgba = vec4(v.color.x, v.color.gba);", StringComparison.Ordinal) ||
    !vectorConstructorGlslText.Contains("o.xyzw = vec4(v.color.xy, v.color.zw);", StringComparison.Ordinal) ||
    !vectorConstructorMslText.Contains("o.rgba = float4(v.color.rgb, v.weight);", StringComparison.Ordinal) ||
    !vectorConstructorMslText.Contains("o.xgba = float4(v.color.x, v.color.gba);", StringComparison.Ordinal) ||
    !vectorConstructorMslText.Contains("o.xyzw = float4(v.color.xy, v.color.zw);", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected vector constructors to type-check and lower as shader constructors.");
    return 1;
}

var structMethodResult = compiler.Compile(new CppslCompileOptions(
    structMethodShader,
    Path.Combine(outputDir, "struct-method"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));

if (!structMethodResult.Succeeded || structMethodResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected struct_method fixture to compile.");
    foreach (var diagnostic in structMethodResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var structMethodShaderModelText = File.ReadAllText(structMethodResult.Artifacts.ShaderModelPath);
var structMethodHlslText = File.ReadAllText(structMethodResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var structMethodGlslText = File.ReadAllText(structMethodResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var structMethodMslText = File.ReadAllText(structMethodResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!structMethodShaderModelText.Contains("\"Methods\": [", StringComparison.Ordinal) ||
    !structMethodShaderModelText.Contains("\"Name\": \"Apply\"", StringComparison.Ordinal) ||
    !structMethodHlslText.Contains("float3 Apply(float3 color)", StringComparison.Ordinal) ||
    !structMethodHlslText.Contains("return ((color * exposure) + float3(0.25f, 0.25f, 0.25f));", StringComparison.Ordinal) ||
    !structMethodHlslText.Contains("o.color = float4(mapper.Apply(v.color.rgb), 1.0f);", StringComparison.Ordinal) ||
    !structMethodMslText.Contains("float3 Apply(float3 color)", StringComparison.Ordinal) ||
    !structMethodMslText.Contains("return ((color * exposure) + float3(0.25f, 0.25f, 0.25f));", StringComparison.Ordinal) ||
    !structMethodMslText.Contains("o.color = float4(mapper.Apply(v.color.rgb), 1.0f);", StringComparison.Ordinal) ||
    !structMethodGlslText.Contains("vec3 ToneMapper_Apply(ToneMapper self, vec3 color)", StringComparison.Ordinal) ||
    !structMethodGlslText.Contains("return ((color * self.exposure) + vec3(0.25, 0.25, 0.25));", StringComparison.Ordinal) ||
    !structMethodGlslText.Contains("o.color = vec4(ToneMapper_Apply(mapper, v.color.rgb), 1.0);", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected struct methods to stay as methods in HLSL/MSL and lower to free functions in GLSL.");
    return 1;
}

var nonConstStructMethodResult = compiler.Compile(new CppslCompileOptions(
    nonConstStructMethodShader,
    Path.Combine(outputDir, "non-const-struct-method"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));

if (!nonConstStructMethodResult.Succeeded || nonConstStructMethodResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected non_const_struct_method fixture to compile.");
    foreach (var diagnostic in nonConstStructMethodResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var nonConstStructMethodShaderModelText = File.ReadAllText(nonConstStructMethodResult.Artifacts.ShaderModelPath);
var nonConstStructMethodHlslText = File.ReadAllText(nonConstStructMethodResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var nonConstStructMethodGlslText = File.ReadAllText(nonConstStructMethodResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var nonConstStructMethodMslText = File.ReadAllText(nonConstStructMethodResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!nonConstStructMethodShaderModelText.Contains("\"Name\": \"Add\"", StringComparison.Ordinal) ||
    !nonConstStructMethodShaderModelText.Contains("\"IsConst\": false", StringComparison.Ordinal) ||
    !nonConstStructMethodShaderModelText.Contains("\"Name\": \"Read\"", StringComparison.Ordinal) ||
    !nonConstStructMethodShaderModelText.Contains("\"IsConst\": true", StringComparison.Ordinal) ||
    !nonConstStructMethodHlslText.Contains("void Add(float delta)", StringComparison.Ordinal) ||
    !nonConstStructMethodHlslText.Contains("value += delta;", StringComparison.Ordinal) ||
    !nonConstStructMethodHlslText.Contains("accumulator.Add(v.value);", StringComparison.Ordinal) ||
    !nonConstStructMethodMslText.Contains("void Add(float delta)", StringComparison.Ordinal) ||
    !nonConstStructMethodMslText.Contains("value += delta;", StringComparison.Ordinal) ||
    !nonConstStructMethodMslText.Contains("accumulator.Add(v.value);", StringComparison.Ordinal) ||
    !nonConstStructMethodGlslText.Contains("void Accumulator_Add(inout Accumulator self, float delta)", StringComparison.Ordinal) ||
    !nonConstStructMethodGlslText.Contains("self.value += delta;", StringComparison.Ordinal) ||
    !nonConstStructMethodGlslText.Contains("float Accumulator_Read(Accumulator self)", StringComparison.Ordinal) ||
    !nonConstStructMethodGlslText.Contains("Accumulator_Add(accumulator, v.value);", StringComparison.Ordinal) ||
    !nonConstStructMethodGlslText.Contains("o.value = Accumulator_Read(accumulator);", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected non-const struct methods to lower to GLSL inout self and stay as methods in HLSL/MSL.");
    return 1;
}

var textureResult = compiler.Compile(new CppslCompileOptions(
    textureBasicShader,
    Path.Combine(outputDir, "texture-basic"),
    new[] { stdRoot },
    "main_ps",
    ShaderStage.Fragment));

if (!textureResult.Succeeded || textureResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected texture_basic fixture to compile.");
    foreach (var diagnostic in textureResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var textureReflectionText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Reflection));
var textureHlslText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var textureGlslText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var textureMslText = File.ReadAllText(textureResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!textureReflectionText.Contains("\"Name\": \"textures_color_texture\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"ResourceKind\": \"texture\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"Name\": \"textures_linear_sampler\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"ResourceKind\": \"sampler\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected texture_basic reflection resources were not written.");
    return 1;
}
if (!textureHlslText.Contains("Texture2D<float4> textures_color_texture : register(t0, space0);", StringComparison.Ordinal) ||
    !textureHlslText.Contains("SamplerState textures_linear_sampler : register(s1, space0);", StringComparison.Ordinal) ||
    !textureGlslText.Contains("uniform texture2D textures_color_texture", StringComparison.Ordinal) ||
    !textureGlslText.Contains("uniform sampler textures_linear_sampler", StringComparison.Ordinal) ||
    !textureMslText.Contains("fragment PSOutput main_ps", StringComparison.Ordinal) ||
    !textureMslText.Contains("texture2d<float> textures_color_texture [[id(0)]]", StringComparison.Ordinal) ||
    !textureMslText.Contains("sampler textures_linear_sampler [[id(1)]]", StringComparison.Ordinal) ||
    !textureMslText.Contains("constant spvDescriptorSetBuffer0* constant spvDescriptorSet0 [[buffer(0)]]", StringComparison.Ordinal) ||
    !textureMslText.Contains("(*spvDescriptorSet0).textures_color_texture.sample((*spvDescriptorSet0).textures_linear_sampler", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected texture_basic shader source resources were not emitted.");
    return 1;
}

var texture1DSampleLevelResult = compiler.Compile(new CppslCompileOptions(
    texture1DSampleLevelShader,
    Path.Combine(outputDir, "texture1d-sample-level"),
    new[] { stdRoot },
    "main_cs",
    ShaderStage.Compute));

if (!texture1DSampleLevelResult.Succeeded || texture1DSampleLevelResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected texture1d_sample_level fixture to compile.");
    foreach (var diagnostic in texture1DSampleLevelResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var texture1DSampleLevelMslText = File.ReadAllText(texture1DSampleLevelResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!texture1DSampleLevelMslText.Contains("texture1d<float> textures_src_texture [[id(0)]]", StringComparison.Ordinal) ||
    !texture1DSampleLevelMslText.Contains("texture1d<float, access::read_write> textures_dst_texture [[id(1)]]", StringComparison.Ordinal) ||
    !texture1DSampleLevelMslText.Contains("(*spvDescriptorSet0).textures_src_texture.sample((*spvDescriptorSet0).textures_linear_sampler, uv)", StringComparison.Ordinal) ||
    texture1DSampleLevelMslText.Contains("textures_src_texture.sample((*spvDescriptorSet0).textures_linear_sampler, uv, level(", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected Texture1D SampleLevel to lower to a Metal 1D sample without explicit level.");
    return 1;
}

var textureComponentInitResult = compiler.Compile(new CppslCompileOptions(
    textureComponentInitShader,
    outputDir,
    new[] { stdRoot },
    "main_ps",
    ShaderStage.Pixel,
    new[] { CppslOutputTarget.Msl }));
if (!textureComponentInitResult.Succeeded || textureComponentInitResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected texture_component_init fixture to compile.");
    return 1;
}

var textureComponentInitMslText = File.ReadAllText(textureComponentInitResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!textureComponentInitMslText.Contains("float sampled = (*spvDescriptorSet0).textures_src_tex.sample((*spvDescriptorSet0).textures_linear_sampler, v.uv).x;", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected texture component initializer to keep the sampled expression in MSL.");
    return 1;
}

var textureScalarResult = compiler.Compile(new CppslCompileOptions(
    textureScalarShader,
    Path.Combine(outputDir, "texture-scalar"),
    new[] { stdRoot },
    "main_cs",
    ShaderStage.Compute));

if (!textureScalarResult.Succeeded || textureScalarResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected texture_scalar fixture to compile.");
    foreach (var diagnostic in textureScalarResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var textureScalarGlslText = File.ReadAllText(textureScalarResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var textureScalarMslText = File.ReadAllText(textureScalarResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!textureScalarMslText.Contains("float sampled = (*spvDescriptorSet0).textures_src_tex.sample((*spvDescriptorSet0).textures_linear_sampler, uv, level(0.0f)).x;", StringComparison.Ordinal) ||
    !textureScalarMslText.Contains("depth2d<float> textures_depth_tex [[id(1)]]", StringComparison.Ordinal) ||
    !textureScalarMslText.Contains("float depth = (*spvDescriptorSet0).textures_depth_tex.read(pixel);", StringComparison.Ordinal) ||
    !textureScalarMslText.Contains("(*spvDescriptorSet0).textures_dst_tex.write(float4(sampled), pixel);", StringComparison.Ordinal) ||
    !textureScalarGlslText.Contains("float sampled = textureLod(sampler2D(textures_src_tex, textures_linear_sampler), uv, 0.0).x;", StringComparison.Ordinal) ||
    !textureScalarGlslText.Contains("float depth = texelFetch(textures_depth_tex, ivec2(pixel), 0).x;", StringComparison.Ordinal) ||
    !textureScalarGlslText.Contains("imageStore(textures_dst_tex, ivec2(pixel), vec4(sampled));", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected scalar texture operations to lower with channel extraction/expansion.");
    return 1;
}

var structuredBufferAccessResult = compiler.Compile(new CppslCompileOptions(
    structuredBufferAccessShader,
    Path.Combine(outputDir, "structured-buffer-access"),
    new[] { stdRoot },
    "main_cs",
    ShaderStage.Compute));

if (!structuredBufferAccessResult.Succeeded || structuredBufferAccessResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected structured_buffer_access fixture to compile.");
    foreach (var diagnostic in structuredBufferAccessResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var structuredBufferAccessMslText = File.ReadAllText(structuredBufferAccessResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!structuredBufferAccessMslText.Contains("packed_float3 direction;", StringComparison.Ordinal) ||
    !structuredBufferAccessMslText.Contains("packed_float3 position;", StringComparison.Ordinal) ||
    !structuredBufferAccessMslText.Contains("-normalize((*spvDescriptorSet0).light_set_lights[index].direction)", StringComparison.Ordinal) ||
    !structuredBufferAccessMslText.Contains("-normalize(world_position - (*spvDescriptorSet0).light_set_lights[index].position)", StringComparison.Ordinal) ||
    !structuredBufferAccessMslText.Contains("mix(direction, to_light, 0.5f)", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected structured buffer resource accesses and lerp lowering to emit correctly in MSL.");
    return 1;
}

var helperResourceAccessResult = compiler.Compile(new CppslCompileOptions(
    helperResourceAccessShader,
    Path.Combine(outputDir, "helper-resource-access"),
    new[] { stdRoot },
    "main_cs",
    ShaderStage.Compute));

if (!helperResourceAccessResult.Succeeded || helperResourceAccessResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected helper_resource_access fixture to compile.");
    foreach (var diagnostic in helperResourceAccessResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var helperResourceAccessMslText = File.ReadAllText(helperResourceAccessResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!helperResourceAccessMslText.Contains("float4 helper_sample(float2 uv)", StringComparison.Ordinal) ||
    !helperResourceAccessMslText.Contains("(*spvDescriptorSet0).main_set_src_tex.sample((*spvDescriptorSet0).main_set_linear_sampler, uv, level(0.0f))", StringComparison.Ordinal) ||
    !helperResourceAccessMslText.Contains("if ((color.x < (*(*spvDescriptorSet0).main_set_params).threshold))", StringComparison.Ordinal) ||
    !helperResourceAccessMslText.Contains("float4 color = helper_sample(uv);", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected helper function resource accesses to receive forwarded descriptor set parameters in MSL.");
    return 1;
}

var resourceAttributesResult = compiler.Compile(new CppslCompileOptions(
    resourceAttributesShader,
    Path.Combine(outputDir, "resource-attributes"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));

if (!resourceAttributesResult.Succeeded || resourceAttributesResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected resource_attributes fixture to compile.");
    foreach (var diagnostic in resourceAttributesResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var resourceReflectionText = File.ReadAllText(resourceAttributesResult.Artifacts.GetOutputPath(CppslOutputTarget.Reflection));
var resourceHlslText = File.ReadAllText(resourceAttributesResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var resourceGlslText = File.ReadAllText(resourceAttributesResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var resourceMslText = File.ReadAllText(resourceAttributesResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!resourceReflectionText.Contains("\"Name\": \"resources_camera\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"ResourceKind\": \"constant_buffer\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"Name\": \"resources_values\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"ResourceKind\": \"structured_buffer\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"Name\": \"resources_output_values\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"ResourceKind\": \"rw_structured_buffer\"", StringComparison.Ordinal) ||
    !resourceHlslText.Contains("ConstantBuffer<Camera> resources_camera : register(b0, space0);", StringComparison.Ordinal) ||
    !resourceHlslText.Contains("StructuredBuffer<float> resources_values : register(t8, space0);", StringComparison.Ordinal) ||
    !resourceHlslText.Contains("RWStructuredBuffer<float> resources_output_values : register(u15, space0);", StringComparison.Ordinal) ||
    !resourceGlslText.Contains("layout(set = 0, binding = 0, std140, column_major) uniform resources_camera_Block", StringComparison.Ordinal) ||
    !resourceGlslText.Contains("layout(set = 0, binding = 8, std430, column_major) buffer resources_values_Block", StringComparison.Ordinal) ||
    !resourceGlslText.Contains("layout(set = 0, binding = 15, std430, column_major) buffer resources_output_values_Block", StringComparison.Ordinal) ||
    !resourceMslText.Contains("constant Camera* resources_camera [[id(0)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("device const float* resources_values [[id(1)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("device float* resources_output_values [[id(2)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("constant spvDescriptorSetBuffer0* constant spvDescriptorSet0 [[buffer(0)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("(*spvDescriptorSet0).resources_output_values[0] = (*spvDescriptorSet0).resources_values[0];", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected CPPSL resource attribute declarations to be emitted.");
    return 1;
}

var descriptorSetResult = compiler.Compile(new CppslCompileOptions(
    descriptorSetShader,
    Path.Combine(outputDir, "descriptor-set"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex));

if (!descriptorSetResult.Succeeded || descriptorSetResult.Artifacts is null)
{
    Console.Error.WriteLine("error: expected descriptor_set fixture to compile.");
    foreach (var diagnostic in descriptorSetResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

var descriptorSetReflectionText = File.ReadAllText(descriptorSetResult.Artifacts.GetOutputPath(CppslOutputTarget.Reflection));
var descriptorSetHlslText = File.ReadAllText(descriptorSetResult.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var descriptorSetGlslText = File.ReadAllText(descriptorSetResult.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var descriptorSetMslText = File.ReadAllText(descriptorSetResult.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!descriptorSetReflectionText.Contains("\"Name\": \"frame_camera\"", StringComparison.Ordinal) ||
    !descriptorSetReflectionText.Contains("\"Name\": \"frame_items\"", StringComparison.Ordinal) ||
    !descriptorSetReflectionText.Contains("\"Name\": \"frame_output_values\"", StringComparison.Ordinal) ||
    !descriptorSetReflectionText.Contains("\"Name\": \"frame_color_texture\"", StringComparison.Ordinal) ||
    descriptorSetHlslText.Contains("struct FrameSet", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("ConstantBuffer<Camera> frame_camera : register(b0, space0);", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("StructuredBuffer<Item> frame_items : register(t1, space0);", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("RWStructuredBuffer<float> frame_output_values : register(u2, space0);", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("Texture2D<float4> frame_color_texture : register(t3, space0);", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("SamplerState frame_color_sampler : register(s4, space0);", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("frame_camera.world_to_proj", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("frame_color_texture.Sample(frame_color_sampler", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("frame_items[0].value", StringComparison.Ordinal) ||
    !descriptorSetHlslText.Contains("frame_output_values[0] = o.color.x;", StringComparison.Ordinal) ||
    descriptorSetGlslText.Contains("struct FrameSet", StringComparison.Ordinal) ||
    !descriptorSetGlslText.Contains("layout(set = 0, binding = 0, std140, column_major) uniform frame_camera_Block", StringComparison.Ordinal) ||
    !descriptorSetGlslText.Contains("layout(set = 0, binding = 1, std430, column_major) buffer frame_items_Block", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("constant Camera* frame_camera [[id(0)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("device const Item* frame_items [[id(1)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("device float* frame_output_values [[id(2)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("texture2d<float> frame_color_texture [[id(3)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("sampler frame_color_sampler [[id(4)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("(*(*spvDescriptorSet0).frame_camera).world_to_proj", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("(*spvDescriptorSet0).frame_color_texture.sample((*spvDescriptorSet0).frame_color_sampler", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("(*spvDescriptorSet0).frame_items[0].value", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("(*spvDescriptorSet0).frame_output_values[0] = o.color.x;", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected descriptor set resources and accesses to be emitted.");
    return 1;
}

var invalidFixtures = new[]
{
    new InvalidFixture("forbidden_include", "ForbiddenInclude.cxx", "main", ShaderStage.Vertex, "include"),
    new InvalidFixture("stage_mismatch", "StageMismatch.cxx", "main_vs", ShaderStage.Vertex, "declares `compute`"),
    new InvalidFixture("missing_binding", "MissingBinding.cxx", "main_vs", ShaderStage.Vertex, "binding"),
    new InvalidFixture("duplicate_location", "DuplicateLocation.cxx", "main_vs", ShaderStage.Vertex, "duplicate location"),
    new InvalidFixture("unknown_attribute", "UnknownAttribute.cxx", "main_vs", ShaderStage.Vertex, "unknown CPPSL attribute"),
    new InvalidFixture("duplicate_binding", "DuplicateBinding.cxx", "main_vs", ShaderStage.Vertex, "duplicate resource binding"),
    new InvalidFixture("invalid_attribute_target", "InvalidAttributeTarget.cxx", "main_vs", ShaderStage.Vertex, "cannot be used on field"),
    new InvalidFixture("structured_buffer_requires_const_pointer", "StructuredBufferRequiresConstPointer.cxx", "main_vs", ShaderStage.Vertex, "must be declared as `const T*`"),
    new InvalidFixture("rwstructured_buffer_requires_mutable_pointer", "RWStructuredBufferRequiresMutablePointer.cxx", "main_vs", ShaderStage.Vertex, "must be declared as `T*`"),
    new InvalidFixture("legacy_resource_global", "LegacyResourceGlobal.cxx", "main_vs", ShaderStage.Vertex, "resource global"),
    new InvalidFixture("reserved_parameter_name", "ReservedParameterName.cxx", "main_vs", ShaderStage.Vertex, "parameter name `input` is reserved"),
    new InvalidFixture("reserved_local_name", "ReservedLocalName.cxx", "main_cs", ShaderStage.Compute, "local variable name `output` is reserved")
};

foreach (var fixture in invalidFixtures)
{
    if (!ExpectFixtureFailure(compiler, fixturesRoot, outputDir, stdRoot, fixture))
    {
        return 1;
    }
}

var reflectionOnlyDir = Path.Combine(outputDir, "reflection-only");
var reflectionOnlyResult = compiler.Compile(new CppslCompileOptions(
    boxShader,
    reflectionOnlyDir,
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex,
    new[] { CppslOutputTarget.Reflection }));

if (!reflectionOnlyResult.Succeeded ||
    reflectionOnlyResult.Artifacts is null ||
    !File.Exists(reflectionOnlyResult.Artifacts.ShaderModelPath) ||
    !File.Exists(reflectionOnlyResult.Artifacts.GetOutputPath(CppslOutputTarget.Reflection)) ||
    reflectionOnlyResult.Artifacts.Outputs.ContainsKey(CppslOutputTarget.Hlsl) ||
    reflectionOnlyResult.Artifacts.Outputs.ContainsKey(CppslOutputTarget.Glsl) ||
    reflectionOnlyResult.Artifacts.Outputs.ContainsKey(CppslOutputTarget.Msl))
{
    Console.Error.WriteLine("error: expected CPPSL reflection-only target emission to work.");
    return 1;
}

var nativeSyntaxErrorShader = Path.Combine(fixturesRoot, "invalid", "syntax_error", "SyntaxError.cxx");
var nativeSyntaxErrorResult = compiler.Compile(new CppslCompileOptions(
    nativeSyntaxErrorShader,
    Path.Combine(outputDir, "native-syntax-error"),
    new[] { stdRoot },
    "main_vs",
    ShaderStage.Vertex,
    new[] { CppslOutputTarget.Reflection }));

if (nativeSyntaxErrorResult.Succeeded ||
    !nativeSyntaxErrorResult.Diagnostics.Any(static diagnostic =>
        diagnostic.Severity == DiagnosticSeverity.Error &&
        diagnostic.Message.Contains("expected", StringComparison.OrdinalIgnoreCase) &&
        diagnostic.File is not null &&
        diagnostic.Line is not null &&
        diagnostic.Column is not null))
{
    Console.Error.WriteLine("error: expected native extractor frontend to report structured Clang diagnostics.");
    foreach (var diagnostic in nativeSyntaxErrorResult.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }
    return 1;
}

Console.WriteLine("CPPSL smoke tests passed.");
return 0;

static bool AssertContainsAll(string text, IEnumerable<string> fragments, string message)
{
    var missing = fragments
        .Where(fragment => !text.Contains(fragment, StringComparison.Ordinal))
        .ToArray();
    if (missing.Length == 0)
    {
        return true;
    }

    Console.Error.WriteLine($"error: {message}");
    foreach (var fragment in missing)
    {
        Console.Error.WriteLine($"  missing: {fragment}");
    }
    return false;
}

static bool AssertNotContainsAny(string text, IEnumerable<string> fragments, string message)
{
    var unexpected = fragments
        .Where(fragment => text.Contains(fragment, StringComparison.Ordinal))
        .ToArray();
    if (unexpected.Length == 0)
    {
        return true;
    }

    Console.Error.WriteLine($"error: {message}");
    foreach (var fragment in unexpected)
    {
        Console.Error.WriteLine($"  unexpected: {fragment}");
    }
    return false;
}

static bool ExpectFixtureFailure(
    CppslCompiler compiler,
    string fixturesRoot,
    string outputDir,
    string stdRoot,
    InvalidFixture fixture)
{
    var shaderPath = Path.Combine(fixturesRoot, "invalid", fixture.Directory, fixture.FileName);
    var result = compiler.Compile(new CppslCompileOptions(
        shaderPath,
        Path.Combine(outputDir, "invalid", fixture.Directory),
        new[] { stdRoot },
        fixture.EntryPoint,
        fixture.Stage));

    if (result.Succeeded ||
        !result.Diagnostics.Any(d => d.Severity == DiagnosticSeverity.Error))
    {
        Console.Error.WriteLine($"error: expected fixture `{fixture.Directory}` to fail CPPSL validation.");
        return false;
    }

    if (!result.Diagnostics.Any(d => d.Message.Contains(fixture.ExpectedDiagnosticFragment, StringComparison.OrdinalIgnoreCase)))
    {
        Console.Error.WriteLine($"error: fixture `{fixture.Directory}` did not report expected diagnostic fragment `{fixture.ExpectedDiagnosticFragment}`.");
        foreach (var diagnostic in result.Diagnostics)
        {
            Console.Error.WriteLine(diagnostic.ToDisplayString());
        }
        return false;
    }

    return true;
}

static bool ExpectSchemaV3Facts(string shaderPath, string stdRoot)
{
    var frontend = new NativeExtractorFrontend();
    var result = frontend.Parse(new CppslFrontendOptions(shaderPath, new[] { stdRoot }));
    foreach (var diagnostic in result.Diagnostics)
    {
        Console.Error.WriteLine(diagnostic.ToDisplayString());
    }

    if (!result.Succeeded || result.ModelVersion != NativeExtractorFrontend.ModelVersion)
    {
        Console.Error.WriteLine("error: expected native extractor schema v3 fixture to parse.");
        return false;
    }

    var nodes = Flatten(result.AstNodes).ToArray();
    if (!nodes.Any(node => node.Kind == CppslAstNodeKind.GlobalVariable &&
                           node.Spelling == "kBias" &&
                           node.IsConstexpr &&
                           !string.IsNullOrWhiteSpace(node.ConstantValue)))
    {
        Console.Error.WriteLine("error: expected constexpr declarations to carry evaluated constant values.");
        return false;
    }

    if (!nodes.Any(node => node.Kind == CppslAstNodeKind.CallExpression &&
                           node.DisplayName == "AddDefault" &&
                           !string.IsNullOrWhiteSpace(node.DirectCalleeDeclId) &&
                           node.Children.Any(static child => child.UsesDefaultArgument)))
    {
        Console.Error.WriteLine("error: expected default-argument calls to expose direct callee IDs and default argument nodes.");
        return false;
    }

    if (!nodes.Any(node => node.Kind == CppslAstNodeKind.CallExpression &&
                           node.DisplayName == "Identity" &&
                           node.IsTemplateInstantiation &&
                           !string.IsNullOrWhiteSpace(node.TemplatePatternDeclId) &&
                           node.TemplateArguments.Count != 0))
    {
        Console.Error.WriteLine("error: expected template calls to expose pattern IDs and template arguments.");
        return false;
    }

    if (!nodes.Any(node => node.Kind == CppslAstNodeKind.MemberExpression &&
                           node.DisplayName == "bias" &&
                           !string.IsNullOrWhiteSpace(node.ReferencedDeclId)))
    {
        Console.Error.WriteLine("error: expected member expressions to expose referenced declaration IDs.");
        return false;
    }

    return true;
}

static IEnumerable<CppslAstNode> Flatten(IEnumerable<CppslAstNode> nodes)
{
    foreach (var node in nodes)
    {
        yield return node;
        foreach (var child in Flatten(node.Children))
        {
            yield return child;
        }
    }
}

static string FindRepoRoot(string start)
{
    var dir = new DirectoryInfo(start);
    while (dir is not null)
    {
        if (File.Exists(Path.Combine(dir.FullName, "xmake.lua")) &&
            Directory.Exists(Path.Combine(dir.FullName, "Tools")))
        {
            return dir.FullName;
        }
        dir = dir.Parent;
    }
    throw new InvalidOperationException("Could not find LunaSDK repository root.");
}

internal sealed record InvalidFixture(
    string Directory,
    string FileName,
    string EntryPoint,
    ShaderStage Stage,
    string ExpectedDiagnosticFragment);
