--[[
    Compiles one CPPSL shader entry file and emits a C++ shader header that
    depends only on RHI runtime types.

    The first integration target is Metal: CPPSL emits Metal source, xmake
    compiles it to metallib, and RHI consumes the metallib byte array.
]]

local function read_binary(file)
    local f = io.open(file, "rb")
    if not f then
        os.raise("failed to open file for reading: " .. file)
    end
    local data = f:read("*a")
    f:close()
    return data
end

local function write_text(file, data)
    local dir = path.directory(file)
    if not os.isdir(dir) then
        os.mkdir(dir)
    end
    local f = io.open(file, "wb")
    if not f then
        os.raise("failed to open file for writing: " .. file)
    end
    f:write(data)
    f:close()
end

local function cpp_byte_array(data)
    local bytes = {}
    for i = 1, #data do
        table.insert(bytes, tostring(string.byte(data, i)))
    end
    return table.concat(bytes, ",")
end

local function read_metal_numthreads(generated_dir, source_name)
    local reflection_file = path.join(generated_dir, source_name .. ".reflection.json")
    if not os.isfile(reflection_file) then
        return 0, 0, 0
    end
    local data = read_binary(reflection_file)
    local block = data:match('"WorkgroupSize"%s*:%s*%[(.-)%]')
    if not block then
        return 0, 0, 0
    end
    local values = {}
    for value in block:gmatch("%d+") do
        table.insert(values, tonumber(value))
    end
    return values[1] or 0, values[2] or 0, values[3] or 0
end

local function metal_sdk(configs)
    if configs and configs.metal_platform == "ios" then
        return "iphoneos"
    end
    return "macosx"
end

local function trim_tool_path(value)
    return (value or ""):gsub("^%s+", ""):gsub("%s+$", "")
end

local function metal_tool_paths(sdk)
    local metal_path = nil
    local ok = try {
        function ()
            metal_path = trim_tool_path(os.iorunv("xcrun", {"-sdk", sdk, "-find", "metal"}))
            return true
        end
    }
    if not ok or metal_path == "" then
        os.raise("Metal shader binary generation requires Apple's Metal command-line tool: xcrun metal. Install Xcode, select it with xcode-select, then run xcodebuild -runFirstLaunch.")
    end
    local metallib_path = nil
    ok = try {
        function ()
            metallib_path = trim_tool_path(os.iorunv("xcrun", {"-sdk", sdk, "-find", "metallib"}))
            return true
        end
    }
    if not ok or metallib_path == "" then
        local sibling = path.join(path.directory(metal_path), "metallib")
        if os.isfile(sibling) then
            metallib_path = sibling
        else
            os.raise("Metal shader binary generation requires Apple's Metal library linker: xcrun metallib. Install the full Xcode Metal toolchain, select it with xcode-select, then run xcodebuild -runFirstLaunch.")
        end
    end
    return metal_path, metallib_path
end

local function compile_metal_library(projectdir, source_file, generated_dir, source_name, configs)
    local sdk = metal_sdk(configs)
    local metal_path, metallib_path = metal_tool_paths(sdk)
    local module_cache = path.join(projectdir, "build", ".metal-module-cache")
    if not os.isdir(module_cache) then
        os.mkdir(module_cache)
    end
    local air_file = path.join(generated_dir, source_name .. ".air")
    local metallib_file = path.join(generated_dir, source_name .. ".metallib")
    os.runv(metal_path, {
        "-std=metal3.2",
        "-fmodules-cache-path=" .. module_cache,
        "-x",
        "metal",
        "-c",
        source_file,
        "-o",
        air_file
    })
    os.runv(metallib_path, {
        air_file,
        "-o",
        metallib_file
    })
    return metallib_file
end

local function glslang_stage(stage)
    if stage == "vertex" then
        return "vert"
    elseif stage == "fragment" or stage == "pixel" then
        return "frag"
    elseif stage == "compute" then
        return "comp"
    end
    os.raise("unsupported GLSL shader stage for glslang: " .. tostring(stage))
end

local function vulkan_tool_path(projectdir, executable)
    local bin_dir = ""
    if os.host() == "macosx" and os.arch() == "arm64" then
        bin_dir = path.join(projectdir, "SDKs", "vulkan-tools", "macosx", "arm64", "bin")
    elseif os.host() == "windows" then
        bin_dir = path.join(projectdir, "SDKs", "vulkan-tools", "windows", "x64", "bin")
        if not executable:endswith(".exe") then
            executable = executable .. ".exe"
        end
    end
    if bin_dir == "" then
        os.raise("CPPSL Vulkan shader binary generation is not supported on this host yet: " .. os.host() .. "." .. os.arch())
    end

    local tool = path.join(bin_dir, executable)
    if not os.isfile(tool) then
        os.raise("CPPSL Vulkan shader binary generation requires " .. executable .. " from the Vulkan SDK at: " .. tool)
    end
    return tool
end

local function compile_glsl_to_spirv(projectdir, source_file, generated_dir, source_name, stage)
    local spirv_file = path.join(generated_dir, source_name .. ".spv")
    os.runv(vulkan_tool_path(projectdir, "glslang"), {
        "-V",
        "--target-env",
        "vulkan1.4",
        "--spirv-val",
        "-S",
        glslang_stage(stage),
        "-e",
        "main",
        "-o",
        spirv_file,
        source_file
    })
    return spirv_file
end

local function write_shader_header(source_name, source_file, target_format, entry_point, output_file, metal_numthreads_x, metal_numthreads_y, metal_numthreads_z)
    local data = read_binary(source_file)
    local header = [[// Autogenerated by cppslc, do not modify.
#pragma once
#include <Luna/RHI/CppslShaderHelper.hpp>

namespace Luna
{
    constexpr u8 SHADER_DATA_]] .. source_name .. "[] = {" .. cpp_byte_array(data) .. [[};
    constexpr usize SHADER_DATA_SIZE_]] .. source_name .. " = sizeof(SHADER_DATA_" .. source_name .. [[);
    constexpr RHI::ShaderDataFormat SHADER_DATA_FORMAT_]] .. source_name .. " = RHI::ShaderDataFormat::" .. target_format .. [[;
    constexpr c8 SHADER_ENTRY_POINT_]] .. source_name .. "[] = \"" .. entry_point .. [[";
    constexpr u32 SHADER_METAL_NUMTHREADS_X_]] .. source_name .. " = " .. tostring(metal_numthreads_x or 0) .. [[;
    constexpr u32 SHADER_METAL_NUMTHREADS_Y_]] .. source_name .. " = " .. tostring(metal_numthreads_y or 0) .. [[;
    constexpr u32 SHADER_METAL_NUMTHREADS_Z_]] .. source_name .. " = " .. tostring(metal_numthreads_z or 0) .. [[;
}]]
    write_text(output_file, header)
end

local function cppslc_executable_name()
    local executable = "cppslc"
    if is_host("windows") then
        executable = "cppslc.exe"
    end
    return executable
end

local function cppsl_tool_platform_dir(projectdir)
    if os.host() == "windows" and os.arch() == "x64" then
        return path.join(projectdir, "SDKs", "CPPSL", "windows", "x64", "bin")
    elseif os.host() == "macosx" then
        if os.arch() == "arm64" then
            return path.join(projectdir, "SDKs", "CPPSL", "macosx", "arm64", "bin")
        elseif os.arch() == "x86_64" then
            return path.join(projectdir, "SDKs", "CPPSL", "macosx", "x86_64", "bin")
        end
    end
    os.raise("CPPSL tools are not supported on the current platform: " .. os.host() .. "." .. os.arch())
end

local function cppslc_path(projectdir)
    local tool = nil
    if has_config("build_cppsl_tools") then
        tool = path.join(projectdir, "Tools", "CPPSL", "src", "CPPSL.Cli", "bin", "Debug", "net9.0", cppslc_executable_name())
    else
        tool = path.join(cppsl_tool_platform_dir(projectdir), cppslc_executable_name())
    end
    if os.isfile(tool) then
        return tool
    end
    if has_config("build_cppsl_tools") then
        os.raise("cppslc was not built: " .. tool .. "\nBuild the CPPSL target before compiling CPPSL shaders.")
    end
    os.raise("CPPSL SDK tool is missing: " .. tool .. "\nRun setup to install the SDK package, or configure with --build_cppsl_tools=true and build the CPPSL target.")
end

local function native_extractor_executable_name()
    local executable = "cppsl-native-extractor"
    if is_host("windows") then
        executable = executable .. ".exe"
    end
    return executable
end

local function native_extractor_path(projectdir)
    local tool = nil
    if has_config("build_cppsl_tools") then
        tool = path.join(projectdir, "Tools", "CPPSL", "native", "bin", native_extractor_executable_name())
    else
        tool = path.join(cppsl_tool_platform_dir(projectdir), native_extractor_executable_name())
    end
    if os.isfile(tool) then
        return tool
    end
    if has_config("build_cppsl_tools") then
        os.raise("CPPSL native extractor was not built: " .. tool .. "\nBuild the CPPSL target before compiling CPPSL shaders.")
    end
    os.raise("CPPSL SDK tool is missing: " .. tool .. "\nRun setup to install the SDK package, or configure with --build_cppsl_tools=true and build the CPPSL target.")
end

local function cppsl_target_from_format(target_format)
    if target_format == "dxil" then
        return "hlsl"
    elseif target_format == "spir_v" then
        return "glsl"
    elseif target_format == "msl" then
        return "msl"
    end
    os.raise("unsupported CPPSL target format: " .. tostring(target_format))
end

local function cppsl_source_extension_from_format(target_format, cppsl_target)
    if target_format == "msl" then
        return "metal"
    end
    return cppsl_target
end

local function stage_from_type(shader_type)
    if shader_type == "pixel" then
        return "fragment"
    end
    return shader_type or "vertex"
end

function compile_cppsl(shader_file, configs)
    local projectdir = vformat("$(projectdir)")
    local output_file = configs.output
    if not output_file then
        os.raise("compile_cppsl requires configs.output")
    end

    local target_format = configs.target_format or "msl"
    local cppsl_target = cppsl_target_from_format(target_format)
    local stage = configs.stage or stage_from_type(configs.type)
    local entry_point = configs.entry_point or "main"
    local source_name = path.basename(shader_file)
    local generated_dir = path.join(path.directory(output_file), ".cppsl", source_name)

    local tool = cppslc_path(projectdir)
    local native_extractor = native_extractor_path(projectdir)

    local requested_target = cppsl_target
    if target_format == "msl" then
        requested_target = cppsl_target .. ",reflection"
    end

    local args = {
        "compile",
        shader_file,
        "--stage",
        stage,
        "--entry",
        entry_point,
        "--include",
        path.join(projectdir, "Tools", "CPPSL", "std"),
        "--include",
        path.directory(shader_file),
        "--out",
        generated_dir,
        "--target",
        requested_target,
        "--native-extractor",
        native_extractor
    }
    os.runv(tool, args)

    local generated_source = path.join(generated_dir, source_name .. "." .. cppsl_source_extension_from_format(target_format, cppsl_target))
    if target_format == "msl" then
        local generated_library = compile_metal_library(projectdir, generated_source, generated_dir, source_name, configs)
        local nx, ny, nz = read_metal_numthreads(generated_dir, source_name)
        write_shader_header(source_name, generated_library, "metallib", entry_point, output_file, nx, ny, nz)
        return
    end

    if target_format == "dxil" then
        import("compile_shader")
        local compiled_shader = path.join(generated_dir, source_name .. ".dxil")
        local hlsl_configs = table.clone(configs)
        hlsl_configs.cpp_output = false
        hlsl_configs.output = compiled_shader
        compile_shader.compile_shader(generated_source, hlsl_configs)
        write_shader_header(source_name, compiled_shader, "dxil", entry_point, output_file, 0, 0, 0)
        return
    end

    if target_format == "spir_v" then
        local compiled_shader = compile_glsl_to_spirv(projectdir, generated_source, generated_dir, source_name, stage)
        write_shader_header(source_name, compiled_shader, "spirv", "main", output_file, 0, 0, 0)
        return
    end

    os.raise("unsupported CPPSL target format: " .. tostring(target_format))
end
