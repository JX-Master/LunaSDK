target("ECSTest")
    set_luna_sdk_test()
    set_kind("binary")
    add_files("*.cpp")
    add_deps("Runtime", "JobSystem", "ECS")
target_end()