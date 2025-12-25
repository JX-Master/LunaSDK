--[[
    configs: 
        output: Sets the output file.
            [default]: Outputs to $source$.cso on the current working directory.
        target_format: <dxil|spir_v|msl> Sets the target format.
            dxil: [Windows only] Outputs DirectX Intermediate Language for shader model 6.0 and newer.
            spir_v: Outputs SPIR-V for Vulkan API.
            msl: Outputs Metal Shading Lauguage.
            [default]: `dxil` on Windows, `msl` on macOS.
        type: <vertex|pixel|compute> Sets the shader type.
            vertex: Compiles vertex shader.
            pixel: Compiles pixel/fragment shader.
            compute: Compiles compute shader.
            [default]: vertex
        entry_point: Sets the entry point function name.
            [default]: main
        optimize: <none|1|2> Sets the shader optimization level.
            none: Do not perform any optimization. This can be used if 
                you want to debug shader code using shader debugging tools.
            1: Specifies shader optimization level 1.
            2: Specifies shader optimization level 2.
            [default]: 2
        shader_model: <X_X> Sets the HLSL shader model version used when compiling HLSL source code.
            [default]: 6_0
        include_paths: Sets include search paths. This can be a table of multiple paths.
        definitions: Sets definitions (<DEF|DEF=VALUE>). This can be a table of multiple paths.
        debug: <true|false> Whether to keep debug information in the compiled shader.
            [default]: false
        skip_validation: <true|false> Whether to skip shader validation.
            [default]: false
        matrix_pack: <column|row> Sets the matrix pack mode.
            column: Use column major pack mode.
            row: Use row major pack mode.
            [default]: column
        metal_platform: <macos|ios> Sets the intended running platform for one metal shader.
            macos: The shader is intended to be running on macOS.
            ios: The shader is intended to be running on iOS/iPadOS.
            [default]: macos
        cpp_output: <true|false> Converts the shader output to C++ byte array, so that it can be embedded to the program directly.)";
            [default]: false
]]
function compile_shader(shader_file, configs)
    local args = {}
    table.insert(args, shader_file)
    for k, v in pairs(configs) do
        if k == "output" then
            table.insert(args, "-o")
            table.insert(args, v)
        elseif k == "target_format" then
            table.insert(args, "-f")
            table.insert(args, v)
        elseif k == "type" then
            table.insert(args, "-t")
            table.insert(args, v)
        elseif k == "entry_point" then
            table.insert(args, "-e")
            table.insert(args, v)
        elseif k == "optimize" then
            table.insert(args, "--optimize")
            table.insert(args, v)
        elseif k == "shader_model" then
            table.insert(args, "-sm")
            table.insert(args, v)
        elseif k == "include_paths" then
            for _, p in ipairs(v) do
                table.insert(args, "-i")
                table.insert(args, p)
            end
        elseif k == "definitions" then
            for _, d in ipairs(v) do
                table.insert(args, "-d")
                table.insert(args, d)
            end
        elseif k == "debug" then
            if v == true then
                table.insert(args, "--debug")
            end
        elseif k == "skip_validation" then
            if v == true then
                table.insert(args, "--skip_validation")
            end
        elseif k == "matrix_pack" then
            table.insert(args, "--matrix_pack")
            table.insert(args, v)
        elseif k == "metal_platform" then
            table.insert(args, "--metal_platform")
            table.insert(args, v)
        elseif k == "cpp_output" then
            if v == true then
                table.insert(args, "--cpp_output")
            end
        end
    end
    local bin_path = ""
    if os.host() == "windows" then
        bin_path = path.join(vformat("$(projectdir)"), "SDKs", "LunaShader", "windows", "x64", "LunaShader.exe")
    elseif os.host() == "macosx" then
        if os.arch() == "x86_64" then
            bin_path = path.join(vformat("$(projectdir)"), "SDKs", "LunaShader", "macosx", "x86_64", "LunaShader")
        elseif os.arch() == "arm64" then
            bin_path = path.join(vformat("$(projectdir)"), "SDKs", "LunaShader", "macosx", "arm64", "LunaShader")
        end
    end
    if bin_path == "" then
        os.raise("LunaShader is not supported on the current platform. (" .. os.host() .. "." .. os.arch() .. ")")
    end
    os.runv(bin_path, args)
end

function shader_to_cpp(file, shader_type)
    local shader_path = path.join(vformat("$(scriptdir)"), file)
    local target_path = path.join(vformat("$(buildir)"), file)
    local configs = {
        output = target_path,
        target_format = "dxil",
        type = shader_type,
        optimize = "2",
        cpp_output = true
    }
    compile_shader(shader_path, configs)
end