using CPPSL.Core.Artifacts;
using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
using CPPSL.Core.IR;
using System.Text.RegularExpressions;

var repoRoot = FindRepoRoot(AppContext.BaseDirectory);
var fixturesRoot = Path.Combine(repoRoot, "Tools", "CPPSL", "tests", "fixtures");
var stdRoot = Path.Combine(repoRoot, "Tools", "CPPSL", "std");
var outputDir = Path.Combine(repoRoot, "build", "cppsl-smoke");
var compiler = new CppslCompiler();
var boxShader = Path.Combine(fixturesRoot, "valid", "box", "Box.cxx");
var textureBasicShader = Path.Combine(fixturesRoot, "valid", "texture_basic", "TextureBasic.cxx");
var texture1DSampleLevelShader = Path.Combine(fixturesRoot, "valid", "texture1d_sample_level", "Texture1DSampleLevel.cxx");
var textureScalarShader = Path.Combine(fixturesRoot, "valid", "texture_scalar", "TextureScalar.cxx");
var resourceAttributesShader = Path.Combine(fixturesRoot, "valid", "resource_attributes", "ResourceAttributes.cxx");
var descriptorSetShader = Path.Combine(fixturesRoot, "valid", "descriptor_set", "DescriptorSet.cxx");

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
    !File.Exists(result.Artifacts.IrPath) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Reflection)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Glsl)) ||
    !File.Exists(result.Artifacts.GetOutputPath(CppslOutputTarget.Msl)))
{
    Console.Error.WriteLine("error: expected CPPSL smoke artifacts were not generated.");
    return 1;
}

var irText = File.ReadAllText(result.Artifacts.IrPath);
var reflectionText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Reflection));
var hlslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Hlsl));
var glslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Glsl));
var mslText = File.ReadAllText(result.Artifacts.GetOutputPath(CppslOutputTarget.Msl));
if (!irText.Contains("main_vs", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendProvider\": \"Native\"", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendModelVersion\": 2", StringComparison.Ordinal) ||
    !irText.Contains("\"frontendAst\"", StringComparison.Ordinal) ||
    !irText.Contains("\"outputTargets\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"Function\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ProviderKind\": \"Function\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"Field\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ProviderKind\": \"Field\"", StringComparison.Ordinal) ||
    !irText.Contains("world_to_proj", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"Parameter\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ProviderKind\": \"ParmVar\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Range\": {", StringComparison.Ordinal) ||
    !irText.Contains("\"TypeInfo\": {", StringComparison.Ordinal) ||
    !irText.Contains("\"TemplateArguments\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Spelling\": \"Camera\"", StringComparison.Ordinal) ||
    !irText.Contains("\"ResultTypeInfo\": {", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"CompoundStatement\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"DeclarationStatement\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"LocalVariable\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Spelling\": \"output\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"CallExpression\"", StringComparison.Ordinal) ||
    !irText.Contains("\"DisplayName\": \"mul\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"MemberExpression\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"DeclRefExpression\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Kind\": \"ReturnStatement\"", StringComparison.Ordinal) ||
    !irText.Contains("cppslSemanticModel", StringComparison.Ordinal) ||
    !irText.Contains("cppslIr", StringComparison.Ordinal) ||
    !irText.Contains("\"Schema\": \"cppsl.ir\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Version\": 1", StringComparison.Ordinal) ||
    !irText.Contains("constant_buffer", StringComparison.Ordinal) ||
    !irText.Contains("\"IsEntryPoint\": true", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"desc_set\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"binding\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"cbuffer\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"location\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"position\"", StringComparison.Ordinal) ||
    !irText.Contains("\"Name\": \"vertex\"", StringComparison.Ordinal) ||
    !irText.Contains("\"DescriptorSet\": 0", StringComparison.Ordinal) ||
    !irText.Contains("\"Binding\": 0", StringComparison.Ordinal) ||
    !irText.Contains("\"Location\": 0", StringComparison.Ordinal) ||
    !irText.Contains("\"IsPosition\": true", StringComparison.Ordinal) ||
    !irText.Contains("\"DeclaredStage\": \"vertex\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected frontend, semantic, or IR facts were not written.");
    return 1;
}
if (!Regex.IsMatch(
        irText,
        "\"cppslIr\"\\s*:\\s*\\{.*?\"Version\"\\s*:\\s*1.*?\"EntryPoints\"\\s*:\\s*\\[.*?\"Name\"\\s*:\\s*\"main_vs\".*?\"Body\"\\s*:\\s*\\{.*?\"Kind\"\\s*:\\s*\"CompoundStatement\".*?\"Kind\"\\s*:\\s*\"LocalVariable\".*?\"DisplayName\"\\s*:\\s*\"mul\".*?\"Kind\"\\s*:\\s*\"ReturnStatement\"",
        RegexOptions.Singleline))
{
    Console.Error.WriteLine("error: expected entry point body AST skeleton was not written to CPPSL IR.");
    return 1;
}
if (!reflectionText.Contains("\"Schema\": \"cppsl.reflection\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Version\": 0", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"EntryPoint\": \"main_vs\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Descriptors\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Name\": \"camera\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Type\": \"Camera\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"ResourceKind\": \"constant_buffer\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Set\": 0", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Binding\": 0", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"StageInputs\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Struct\": \"VSInput\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"StageOutputs\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Struct\": \"VSOutput\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"IsPosition\": true", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected CPPSL reflection facts were not written.");
    return 1;
}
if (irText.Contains("\"Name\": \"output\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: local variables must not be classified as CPPSL globals.");
    return 1;
}
if (!Regex.IsMatch(irText, "\"Name\": \"input\".*?\"Attributes\": \\[\\s*\\]", RegexOptions.Singleline))
{
    Console.Error.WriteLine("error: function parameters must not inherit function attributes.");
    return 1;
}
if (!hlslText.Contains("struct VSInput", StringComparison.Ordinal) ||
    !hlslText.Contains("float4 position : SV_Position", StringComparison.Ordinal) ||
    !hlslText.Contains("ConstantBuffer<Camera> camera : register(b0, space0);", StringComparison.Ordinal) ||
    !hlslText.Contains("VSOutput main_vs(VSInput input)", StringComparison.Ordinal) ||
    !hlslText.Contains("output.position = mul(camera.world_to_proj, float4(input.position.x, input.position.y, input.position.z, 1.0f));", StringComparison.Ordinal) ||
    !hlslText.Contains("output.texcoord = input.texcoord;", StringComparison.Ordinal) ||
    !hlslText.Contains("return output;", StringComparison.Ordinal) ||
    !glslText.Contains("#version 450", StringComparison.Ordinal) ||
    !glslText.Contains("mat4 world_to_proj", StringComparison.Ordinal) ||
    !glslText.Contains("layout(set = 0, binding = 0) uniform camera_Block", StringComparison.Ordinal) ||
    !glslText.Contains("output.position = (camera.world_to_proj * vec4(input.position.x, input.position.y, input.position.z, 1.0));", StringComparison.Ordinal) ||
    !glslText.Contains("void main()", StringComparison.Ordinal) ||
    !glslText.Contains("gl_Position = cppsl_output.position;", StringComparison.Ordinal) ||
    !mslText.Contains("#include <metal_stdlib>", StringComparison.Ordinal) ||
    !mslText.Contains("float3 position [[attribute(0)]]", StringComparison.Ordinal) ||
    !mslText.Contains("float4 position [[position]]", StringComparison.Ordinal) ||
    !mslText.Contains("struct spvDescriptorSetBuffer0", StringComparison.Ordinal) ||
    !mslText.Contains("constant Camera* camera [[id(0)]]", StringComparison.Ordinal) ||
    !mslText.Contains("constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]]", StringComparison.Ordinal) ||
    !mslText.Contains("vertex VSOutput main_vs", StringComparison.Ordinal) ||
    !mslText.Contains("output.position = ((*spvDescriptorSet0.camera).world_to_proj * float4(input.position.x, input.position.y, input.position.z, 1.0f));", StringComparison.Ordinal) ||
    !mslText.Contains("return output;", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected CPPSL shader source targets were not emitted.");
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
if (!textureReflectionText.Contains("\"Name\": \"color_texture\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"ResourceKind\": \"texture\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"Name\": \"linear_sampler\"", StringComparison.Ordinal) ||
    !textureReflectionText.Contains("\"ResourceKind\": \"sampler\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected texture_basic reflection resources were not written.");
    return 1;
}
if (!textureHlslText.Contains("Texture2D<float4> color_texture : register(t0, space0);", StringComparison.Ordinal) ||
    !textureHlslText.Contains("SamplerState linear_sampler : register(s1, space0);", StringComparison.Ordinal) ||
    !textureGlslText.Contains("uniform texture2D color_texture", StringComparison.Ordinal) ||
    !textureGlslText.Contains("uniform sampler linear_sampler", StringComparison.Ordinal) ||
    !textureMslText.Contains("fragment PSOutput main_ps", StringComparison.Ordinal) ||
    !textureMslText.Contains("texture2d<float> color_texture [[id(0)]]", StringComparison.Ordinal) ||
    !textureMslText.Contains("sampler linear_sampler [[id(1)]]", StringComparison.Ordinal) ||
    !textureMslText.Contains("constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]]", StringComparison.Ordinal) ||
    !textureMslText.Contains("spvDescriptorSet0.color_texture.sample(spvDescriptorSet0.linear_sampler", StringComparison.Ordinal))
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
if (!texture1DSampleLevelMslText.Contains("texture1d<float> src_texture [[id(0)]]", StringComparison.Ordinal) ||
    !texture1DSampleLevelMslText.Contains("texture1d<float, access::write> dst_texture [[id(1)]]", StringComparison.Ordinal) ||
    !texture1DSampleLevelMslText.Contains("spvDescriptorSet0.src_texture.sample(spvDescriptorSet0.linear_sampler, uv)", StringComparison.Ordinal) ||
    texture1DSampleLevelMslText.Contains("src_texture.sample(spvDescriptorSet0.linear_sampler, uv, level(", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected Texture1D SampleLevel to lower to a Metal 1D sample without explicit level.");
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
if (!textureScalarMslText.Contains("float sampled = spvDescriptorSet0.src_tex.sample(spvDescriptorSet0.linear_sampler, uv, level(0.0f)).x;", StringComparison.Ordinal) ||
    !textureScalarMslText.Contains("float depth = spvDescriptorSet0.depth_tex.read(pixel).x;", StringComparison.Ordinal) ||
    !textureScalarMslText.Contains("spvDescriptorSet0.dst_tex.write(float4(sampled), pixel);", StringComparison.Ordinal) ||
    !textureScalarGlslText.Contains("float sampled = textureLod(sampler2D(src_tex, linear_sampler), uv, 0.0).x;", StringComparison.Ordinal) ||
    !textureScalarGlslText.Contains("float depth = texelFetch(sampler2D(depth_tex, sampler()), ivec2(pixel), 0).x;", StringComparison.Ordinal) ||
    !textureScalarGlslText.Contains("imageStore(dst_tex, ivec2(pixel), vec4(sampled));", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected scalar texture operations to lower with channel extraction/expansion.");
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
if (!resourceReflectionText.Contains("\"Name\": \"camera\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"ResourceKind\": \"constant_buffer\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"Name\": \"values\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"ResourceKind\": \"structured_buffer\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"Name\": \"output_values\"", StringComparison.Ordinal) ||
    !resourceReflectionText.Contains("\"ResourceKind\": \"rw_structured_buffer\"", StringComparison.Ordinal) ||
    !resourceHlslText.Contains("ConstantBuffer<Camera> camera : register(b0, space0);", StringComparison.Ordinal) ||
    !resourceHlslText.Contains("StructuredBuffer<float> values : register(t8, space0);", StringComparison.Ordinal) ||
    !resourceHlslText.Contains("RWStructuredBuffer<float> output_values : register(u15, space0);", StringComparison.Ordinal) ||
    !resourceGlslText.Contains("layout(set = 0, binding = 0) uniform camera_Block", StringComparison.Ordinal) ||
    !resourceGlslText.Contains("layout(set = 0, binding = 8) buffer values_Block", StringComparison.Ordinal) ||
    !resourceGlslText.Contains("layout(set = 0, binding = 15) buffer output_values_Block", StringComparison.Ordinal) ||
    !resourceMslText.Contains("constant Camera* camera [[id(0)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("device const float* values [[id(1)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("device float* output_values [[id(2)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]]", StringComparison.Ordinal) ||
    !resourceMslText.Contains("spvDescriptorSet0.output_values[0] = spvDescriptorSet0.values[0];", StringComparison.Ordinal))
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
    !descriptorSetHlslText.Contains("frame_output_values[0] = output.color.x;", StringComparison.Ordinal) ||
    descriptorSetGlslText.Contains("struct FrameSet", StringComparison.Ordinal) ||
    !descriptorSetGlslText.Contains("layout(set = 0, binding = 0) uniform frame_camera_Block", StringComparison.Ordinal) ||
    !descriptorSetGlslText.Contains("layout(set = 0, binding = 1) buffer frame_items_Block", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("constant Camera* frame_camera [[id(0)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("device const Item* frame_items [[id(1)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("device float* frame_output_values [[id(2)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("texture2d<float> frame_color_texture [[id(3)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("sampler frame_color_sampler [[id(4)]]", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("(*spvDescriptorSet0.frame_camera).world_to_proj", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("spvDescriptorSet0.frame_color_texture.sample(spvDescriptorSet0.frame_color_sampler", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("spvDescriptorSet0.frame_items[0].value", StringComparison.Ordinal) ||
    !descriptorSetMslText.Contains("spvDescriptorSet0.frame_output_values[0] = output.color.x;", StringComparison.Ordinal))
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
    new InvalidFixture("rwstructured_buffer_requires_mutable_pointer", "RWStructuredBufferRequiresMutablePointer.cxx", "main_vs", ShaderStage.Vertex, "must be declared as `T*`")
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
    !File.Exists(reflectionOnlyResult.Artifacts.IrPath) ||
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
