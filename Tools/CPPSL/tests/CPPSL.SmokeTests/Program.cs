using CPPSL.Core.Artifacts;
using CPPSL.Core.Compiler;
using CPPSL.Core.Diagnostics;
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
if (!shaderModelText.Contains("main_vs", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"frontendProvider\": \"Native\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"frontendModelVersion\": 2", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"frontendAst\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"outputTargets\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"Function\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"ProviderKind\": \"Function\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"Field\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"ProviderKind\": \"Field\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("world_to_proj", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"Parameter\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"ProviderKind\": \"ParmVar\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Range\": {", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"TypeInfo\": {", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"TemplateArguments\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Spelling\": \"Camera\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"ResultTypeInfo\": {", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"CompoundStatement\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"DeclarationStatement\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"LocalVariable\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Spelling\": \"o\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"CallExpression\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"DisplayName\": \"mul\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"MemberExpression\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"DeclRefExpression\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Kind\": \"ReturnStatement\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("cppslSemanticModel", StringComparison.Ordinal) ||
    !shaderModelText.Contains("cppslShaderModel", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Schema\": \"cppsl.shader_model\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Version\": 1", StringComparison.Ordinal) ||
    !shaderModelText.Contains("constant_buffer", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"IsEntryPoint\": true", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Name\": \"desc_set\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Name\": \"binding\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Name\": \"cbuffer\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Name\": \"location\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Name\": \"position\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Name\": \"vertex\"", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"DescriptorSet\": 0", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Binding\": 0", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"Location\": 0", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"IsPosition\": true", StringComparison.Ordinal) ||
    !shaderModelText.Contains("\"DeclaredStage\": \"vertex\"", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected frontend, semantic, or shader model facts were not written.");
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
if (shaderModelText.Contains("cppsl" + "Ir", StringComparison.Ordinal) ||
    shaderModelText.Contains("cppsl." + "ir", StringComparison.Ordinal) ||
    shaderModelText.Contains("\"phase\"", StringComparison.OrdinalIgnoreCase) ||
    shaderModelText.Contains("phase " + "0", StringComparison.OrdinalIgnoreCase) ||
    shaderModelText.Contains("CPPSL " + "IR", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: shader model artifact must not contain legacy intermediate-form or prototype metadata.");
    return 1;
}
if (!reflectionText.Contains("\"Schema\": \"cppsl.reflection\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Version\": 0", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"EntryPoint\": \"main_vs\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Descriptors\"", StringComparison.Ordinal) ||
    !reflectionText.Contains("\"Name\": \"frame_camera\"", StringComparison.Ordinal) ||
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
if (!hlslText.Contains("struct VSInput", StringComparison.Ordinal) ||
    !hlslText.Contains("#pragma pack_matrix(column_major)", StringComparison.Ordinal) ||
    !hlslText.Contains("float4 position : SV_Position", StringComparison.Ordinal) ||
    !hlslText.Contains("ConstantBuffer<Camera> frame_camera : register(b0, space0);", StringComparison.Ordinal) ||
    !hlslText.Contains("VSOutput main_vs(VSInput v)", StringComparison.Ordinal) ||
    !hlslText.Contains("o.position = mul(frame_camera.world_to_proj, float4(v.position.x, v.position.y, v.position.z, 1.0f));", StringComparison.Ordinal) ||
    !hlslText.Contains("o.texcoord = v.texcoord;", StringComparison.Ordinal) ||
    !hlslText.Contains("return o;", StringComparison.Ordinal) ||
    !glslText.Contains("#version 450", StringComparison.Ordinal) ||
    !glslText.Contains("mat4 world_to_proj", StringComparison.Ordinal) ||
    !glslText.Contains("layout(set = 0, binding = 0, std140, column_major) uniform frame_camera_Block", StringComparison.Ordinal) ||
    !glslText.Contains("o.position = (frame_camera.world_to_proj * vec4(v.position.x, v.position.y, v.position.z, 1.0));", StringComparison.Ordinal) ||
    !glslText.Contains("void main()", StringComparison.Ordinal) ||
    !glslText.Contains("gl_Position = cppsl_output.position;", StringComparison.Ordinal) ||
    !mslText.Contains("#include <metal_stdlib>", StringComparison.Ordinal) ||
    !mslText.Contains("float3 position [[attribute(0)]]", StringComparison.Ordinal) ||
    !mslText.Contains("float4 position [[position]]", StringComparison.Ordinal) ||
    !mslText.Contains("struct spvDescriptorSetBuffer0", StringComparison.Ordinal) ||
    !mslText.Contains("constant Camera* frame_camera [[id(0)]]", StringComparison.Ordinal) ||
    !mslText.Contains("constant spvDescriptorSetBuffer0* constant spvDescriptorSet0 [[buffer(0)]]", StringComparison.Ordinal) ||
    !mslText.Contains("vertex VSOutput main_vs", StringComparison.Ordinal) ||
    !mslText.Contains("o.position = ((*(*spvDescriptorSet0).frame_camera).world_to_proj * float4(v.position.x, v.position.y, v.position.z, 1.0f));", StringComparison.Ordinal) ||
    !mslText.Contains("return o;", StringComparison.Ordinal))
{
    Console.Error.WriteLine("error: expected CPPSL shader source targets were not emitted.");
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
