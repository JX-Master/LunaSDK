function compile_shader(hlsl_file, options)
    options = options or {}
    options.type = options.type or "ps"
    options.shading_model = options.shading_model or "6_0"
    options.output_path = options.output_path or vformat("$(projectdir)/$(buildir)")
    options.entry_point = options.entry_point or "main"
    options.optimization_level = options.optimization_level or 3

    if not os.exists(hlsl_file) then
        raise("Shader file " .. hlsl_file .. " does not exist.")
    end
    
    local filename = path.basename(hlsl_file)
    local opt = {}
    if options.envs then
        opt.envs = options.envs
    end
    local program = "dxc"
    import("lib.detect.find_program")
    if tonumber(string.sub(options.shading_model, 1, 1)) >= 6 then
        program = find_program("dxc", opt)
        os.execv(program, {"/T", (options.type .. "_" .. options.shading_model), hlsl_file, "/Fo", (options.output_path .. "/" .. filename .. ".cso"), "/E", options.entry_point, ("/O" .. tostring(options.optimization_level))})
    else
        opt.check = function(program) end -- bypass --version checking, since fxc does not implement this.
        program = find_program("fxc", opt)
        os.execv(program, {"-T", (options.type .. "_" .. options.shading_model), "-Fo", (options.output_path .. "/" .. filename .. ".cso"), "-E", options.entry_point, ("-O" .. tostring(options.optimization_level)), hlsl_file})
    end
end