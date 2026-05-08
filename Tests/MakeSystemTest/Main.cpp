/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.cpp
* @author JXMaster
* @date 2026/5/8
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/MakeSystem/MakeSystem.hpp>
#include <cstdio>
#include <cstring>

using namespace Luna;
using namespace Luna::MakeSystem;

#define lutest luassert_always

namespace
{
    static RV write_text_file(const Path& path, const String& data)
    {
        lutry
        {
            lulet(file, open_file(path.encode().c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(file->write(data.data(), data.size()));
        }
        lucatchret;
        return ok;
    }

    static R<String> read_text_file(const Path& path)
    {
        lutry
        {
            lulet(file, open_file(path.encode().c_str(), FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
            lulet(data, load_file_data(file));
            return String((const c8*)data.data(), data.size());
        }
        lucatchret;
        return BasicError::failure();
    }

    static Ref<IMakeSystem> checked_new_make_system(const Path& build_dir, u32 max_num_parallel_tasks)
    {
        auto r = new_make_system(build_dir, max_num_parallel_tasks);
        if(!r.valid())
        {
            printf("new_make_system failed: %llu %s\n", (u64)r.errcode().code, get_error().message.c_str());
            fflush(stdout);
        }
        lutest(r.valid());
        return r.get();
    }

    struct WriteFileCommand : IMakeCommand
    {
        lustruct("MakeSystemTest::WriteFileCommand", "{d112916e-514a-4c23-92bc-9d9606e7b1c8}");
        luiimpl();

        Path output;
        String content;
        Path depfile;
        String depfile_content;
        Path side_output;
        String side_content;
        usize* counter = nullptr;
        bool write_output = true;
        bool fail = false;

        virtual RV execute(LogHandler&) override
        {
            if(counter)
            {
                ++(*counter);
            }
            if(fail)
            {
                return BasicError::failure();
            }
            lutry
            {
                if(write_output)
                {
                    luexp(write_text_file(output, content));
                }
                if(!side_output.empty())
                {
                    luexp(write_text_file(side_output, side_content));
                }
                if(!depfile.empty())
                {
                    luexp(write_text_file(depfile, depfile_content));
                }
            }
            lucatchret;
            return ok;
        }
    };

    static Path test_build_dir()
    {
        const c8* cwd = get_current_dir();
        Path ret(cwd);
        release_current_dir(cwd);
        ret.push_back("build");
        String name;
        strprintf(name, "MakeSystemTest-%llu", (u64)get_ticks());
        ret.push_back(name);
        return ret;
    }

    static Ref<WriteFileCommand> new_write_command(const Path& output, const String& content, usize* counter)
    {
        auto command = new_object<WriteFileCommand>();
        command->output = output;
        command->content = content;
        command->counter = counter;
        return command;
    }
}

static void incremental_and_action_key_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    Path input = build_dir;
    input.push_back("input.txt");
    Path output = build_dir;
    output.push_back("output.txt");
    lutest(succeeded(write_text_file(input, "one")));

    usize command_count = 0;
    auto command = new_write_command(output, "generated", &command_count);

    MakeNode input_node;
    input_node.path = input;
    input_node.kind = MakeNodeKind::file;

    MakeNode output_node;
    output_node.path = output;
    output_node.display_info = " write output.txt";
    output_node.kind = MakeNodeKind::file;
    output_node.command = command;
    output_node.action.command = "write\nmode=debug";
    output_node.dependencies.push_back(&input_node);

    MakeNode* targets[] = {&output_node};
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
    lutest(!strcmp(read_text_file(output).get().c_str(), "generated"));

    auto reloaded_make_system = checked_new_make_system(build_dir, 2);
    lutest(succeeded(reloaded_make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);

    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);

    output_node.action.command = "write\nmode=release";
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 2);

    sleep(1100);
    lutest(succeeded(write_text_file(input, "one plus more bytes")));
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 3);
}

static void depfile_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    Path source = build_dir;
    source.push_back("source.txt");
    Path header = build_dir;
    header.push_back("header.txt");
    Path output = build_dir;
    output.push_back("with_depfile.txt");
    Path depfile = build_dir;
    depfile.push_back("with_depfile.d");

    lutest(succeeded(write_text_file(source, "source")));
    lutest(succeeded(write_text_file(header, "header")));

    usize command_count = 0;
    auto command = new_write_command(output, "depfile-output", &command_count);
    command->depfile = depfile;
    String depfile_content;
    strprintf(depfile_content, "%s: %s %s\n", output.encode().c_str(), source.encode().c_str(), header.encode().c_str());
    command->depfile_content = depfile_content;

    MakeNode source_node;
    source_node.path = source;
    source_node.kind = MakeNodeKind::file;

    MakeNode header_node;
    header_node.path = header;
    header_node.kind = MakeNodeKind::file;

    MakeNode depfile_node;
    depfile_node.path = depfile;
    depfile_node.kind = MakeNodeKind::file;

    MakeNode output_node;
    output_node.path = output;
    output_node.display_info = " write depfile output";
    output_node.kind = MakeNodeKind::file;
    output_node.command = command;
    output_node.action.command = "write_depfile";
    output_node.dependencies.push_back(&source_node);
    output_node.depfiles.push_back(&depfile_node);

    MakeNode* targets[] = {&output_node};
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);

    sleep(1100);
    lutest(succeeded(write_text_file(header, "header changed with different size")));
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 2);
}

static void graph_validation_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    MakeNode a;
    MakeNode b;
    a.path = Path("duplicate");
    b.path = Path("duplicate");
    a.kind = MakeNodeKind::phony;
    b.kind = MakeNodeKind::phony;
    MakeNode* duplicate_targets[] = {&a, &b};
    lutest(failed(make_system->make(Span<MakeNode*>(duplicate_targets, 2))));

    MakeNode c;
    MakeNode d;
    c.path = Path("cycle-c");
    d.path = Path("cycle-d");
    c.kind = MakeNodeKind::phony;
    d.kind = MakeNodeKind::phony;
    c.dependencies.push_back(&d);
    d.dependencies.push_back(&c);
    MakeNode* cycle_targets[] = {&c};
    lutest(failed(make_system->make(Span<MakeNode*>(cycle_targets, 1))));
}

static void missing_input_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    Path missing_input = build_dir;
    missing_input.push_back("missing.txt");
    Path output = build_dir;
    output.push_back("missing_input_output.txt");

    usize command_count = 0;
    auto command = new_write_command(output, "should-not-run", &command_count);

    MakeNode input_node;
    input_node.path = missing_input;
    input_node.kind = MakeNodeKind::file;

    MakeNode output_node;
    output_node.path = output;
    output_node.display_info = " missing input";
    output_node.kind = MakeNodeKind::file;
    output_node.command = command;
    output_node.action.command = "missing_input";
    output_node.dependencies.push_back(&input_node);

    MakeNode* targets[] = {&output_node};
    lutest(failed(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 0);
}

static void missing_output_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    Path input = build_dir;
    input.push_back("input.txt");
    Path output = build_dir;
    output.push_back("missing_output.txt");
    lutest(succeeded(write_text_file(input, "input")));

    usize command_count = 0;
    auto command = new_write_command(output, "not-written", &command_count);
    command->write_output = false;

    MakeNode input_node;
    input_node.path = input;
    input_node.kind = MakeNodeKind::file;

    MakeNode output_node;
    output_node.path = output;
    output_node.display_info = " missing output";
    output_node.kind = MakeNodeKind::file;
    output_node.command = command;
    output_node.action.command = "missing_output";
    output_node.dependencies.push_back(&input_node);

    MakeNode* targets[] = {&output_node};
    lutest(failed(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
}

static void command_failure_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    Path input = build_dir;
    input.push_back("input.txt");
    Path output = build_dir;
    output.push_back("failed_output.txt");
    lutest(succeeded(write_text_file(input, "input")));

    usize command_count = 0;
    auto command = new_write_command(output, "not-written", &command_count);
    command->fail = true;

    MakeNode input_node;
    input_node.path = input;
    input_node.kind = MakeNodeKind::file;

    MakeNode output_node;
    output_node.path = output;
    output_node.display_info = " command failure";
    output_node.kind = MakeNodeKind::file;
    output_node.command = command;
    output_node.action.command = "command_failure";
    output_node.dependencies.push_back(&input_node);

    MakeNode* targets[] = {&output_node};
    lutest(failed(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
}

static void phony_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    Path input = build_dir;
    input.push_back("input.txt");
    Path output = build_dir;
    output.push_back("phony_output.txt");
    lutest(succeeded(write_text_file(input, "input")));

    usize command_count = 0;
    auto command = new_write_command(output, "phony-output", &command_count);

    MakeNode input_node;
    input_node.path = input;
    input_node.kind = MakeNodeKind::file;

    MakeNode output_node;
    output_node.path = output;
    output_node.display_info = " phony output";
    output_node.kind = MakeNodeKind::file;
    output_node.command = command;
    output_node.action.command = "phony_output";
    output_node.dependencies.push_back(&input_node);

    MakeNode phony_node;
    phony_node.path = Path("all");
    phony_node.kind = MakeNodeKind::phony;
    phony_node.dependencies.push_back(&output_node);

    MakeNode* targets[] = {&phony_node};
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
}

static void side_output_test()
{
    Path build_dir = test_build_dir();
    auto make_system = checked_new_make_system(build_dir, 2);

    Path input = build_dir;
    input.push_back("input.txt");
    Path output = build_dir;
    output.push_back("primary.txt");
    Path side_output = build_dir;
    side_output.push_back("side.txt");
    lutest(succeeded(write_text_file(input, "input")));

    usize command_count = 0;
    auto command = new_write_command(output, "primary", &command_count);
    command->side_output = side_output;
    command->side_content = "side";

    MakeNode input_node;
    input_node.path = input;
    input_node.kind = MakeNodeKind::file;

    MakeNode side_output_node;
    side_output_node.path = side_output;
    side_output_node.kind = MakeNodeKind::file;

    MakeNode output_node;
    output_node.path = output;
    output_node.display_info = " side output";
    output_node.kind = MakeNodeKind::file;
    output_node.command = command;
    output_node.action.command = "side_output";
    output_node.dependencies.push_back(&input_node);
    output_node.outputs.push_back(&side_output_node);

    MakeNode* targets[] = {&output_node};
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
    lutest(!strcmp(read_text_file(output).get().c_str(), "primary"));
    lutest(!strcmp(read_text_file(side_output).get().c_str(), "side"));
    lutest(succeeded(make_system->make(Span<MakeNode*>(targets, 1))));
    lutest(command_count == 1);
}

int main()
{
    init();
    lupanic_if_failed(add_module(module_make_system()));
    lupanic_if_failed(init_modules());
    register_boxed_type<WriteFileCommand>();
    impl_interface_for_type<WriteFileCommand, IMakeCommand>();
    incremental_and_action_key_test();
    depfile_test();
    graph_validation_test();
    missing_input_test();
    missing_output_test();
    command_failure_test();
    phony_test();
    side_output_test();
    close();
    return 0;
}
