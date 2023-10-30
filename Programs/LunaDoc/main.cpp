/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2023/10/19
*/
#include "Parser.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/StdIO.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VariantUtils/XML.hpp>
using namespace Luna;

RV print_help()
{
    const c8 help_text[] = R"(LunaDoc v0.0.1
Documentation converter for LunaSDK.
This program converts XML files generated by Doxygen to markdown files used by mkdocs to build LunaSDK-Docs site.
Usage: LunaDoc <task> [options]
Tasks:
    md, markdown   Generate markdown files.
    -h, --help      Print help message.
Task options (markdown):
    LunaDoc <-md|--markdown> [-o <./output>] [-s <./input_xml>]
    -o  Sets the output directory. Use "./markdown" if not specified.
    -i  Sets the input directory. Use current working directory if not specified.
    -v  Outputs verbose information for debugging.
)";
    auto io = get_std_io_stream();
    return io->write(help_text, sizeof(help_text));
}

RV gen_markdown(int argc, const char* argv[])
{
    lutry
    {
        Path input_dir = ".";
        Path output_dir = "./markdown";
        // Read command line arguments.
        {
            int argi = 2;
            while(argi < argc)
            {
                if(Name(argv[argi]) == "-o")
                {
                    ++argi;
                    if(argi >= argc) return set_error(BasicError::bad_arguments(), "Output path expected for -o");
                    output_dir = argv[argi];
                    ++argi;
                }
                else if(Name(argv[argi]) == "-i")
                {
                    ++argi;
                    if(argi >= argc) return set_error(BasicError::bad_arguments(), "Input path expected for -i");
                    input_dir = argv[argi];
                    ++argi;
                }
                else if(Name(argv[argi]) == "-v")
                {
                    ++argi;
                    set_log_to_platform_verbosity(LogVerbosity::verbose);
                }
                else
                {
                    return set_error(BasicError::bad_arguments(), "Unknown parameter: %s", argv[argi]);
                }
            }
        }
        Path current_dir;
        {
            u32 current_dir_len = get_current_dir(0, nullptr);
            Vector<c8> current_dir_str(current_dir_len, 0);
            get_current_dir(current_dir_len, current_dir_str.data());
            log_verbose("LunaDoc", "Current directory: %s", current_dir_str.data());
            current_dir = current_dir_str.data();
        }
        if(!test_flags(input_dir.flags(), PathFlag::absolute))
        {
            Path tmp = current_dir;
            tmp.append(input_dir);
            input_dir = tmp;
        }
        if(!test_flags(output_dir.flags(), PathFlag::absolute))
        {
            Path tmp = current_dir;
            tmp.append(output_dir);
            output_dir = tmp;
        }
        log_verbose("LunaDoc", "Input directory: %s", input_dir.encode().c_str());
        log_verbose("LunaDoc", "Output directory: %s", input_dir.encode().c_str());
        // Read source XML files.
        Vector<Variant> group_files;
        {
            lulet(iter, open_dir(input_dir.encode().c_str()));
            while(iter->is_valid())
            {
                auto name = iter->get_filename();
                if(strstr(name, "group__") == name)
                {
                    log_info("LunaDoc", "Read group file %s", name);
                    Path filename = input_dir.encode().c_str();
                    filename.append(name);
                    lulet(f, open_file(filename.encode().c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
                    lulet(group, VariantUtils::read_xml(f));
                    group_files.push_back(move(group));
                }
                iter->move_next();
            }
            if(group_files.empty())
            {
                log_info("LunaDoc", "No group found in %s", input_dir.encode().c_str());
            }
            Vector<Pair<Name, String>> groups;
            for(auto& g : group_files)
            {
                String group_md_text;
                Name group_filename;
                luexp(parse_group(g, group_md_text, group_filename));
                groups.push_back(make_pair(move(group_filename), move(group_md_text)));
            }
            // Create directory if not exist
            {
                auto dir = get_file_attribute(output_dir.encode().c_str());
                if (failed(dir))
                {
                    luexp(create_dir(output_dir.encode().c_str()));
                }
            }
            for(auto& g : groups)
            {
                Path path = output_dir;
                path.push_back(g.first);
                path.append_extension("md");
                lulet(f, open_file(path.encode().c_str(), FileOpenFlag::write, FileCreationMode::create_always));
                luexp(f->write(g.second.c_str(), g.second.size()));
            }
        }
    }
    lucatchret;
    return ok;
}

RV run(int argc, const char* argv[])
{
    lutry
    {
        _doxygen = "doxygen";
        _compounddef = "compounddef";
        _compoundname = "compoundname";
        _name = "name";
        _title = "title";
        _sectiondef = "sectiondef";
        _briefdescription = "briefdescription";
        _detaileddescription = "detaileddescription";
        _kind = "kind";
        _func = "func";
        _memberdef = "memberdef";
        _id = "id";
        _function = "function";
        _prot = "prot";
        _static = "static";
        _constexpr = "constexpr";
        _const = "const";
        _explicit = "explicit";
        _ninline = "inline";
        _virt = "virt";
        _no = "no";
        _yes = "yes";
        _type = "type";
        _definition = "definition";
        _argsstring = "argsstring";
        _qualifiedname = "qualifiedname";
        _param = "param";
        _declname = "declname";
        _para = "para";
        _parameterlist = "parameterlist";
        _simplesect = "simplesect";
        _return = "return";
        _parameternamelist = "parameternamelist";
        _parameterdescription = "parameterdescription";
        _parametername = "parametername";
        _parameteritem = "parameteritem";
        _computeroutput = "computeroutput";

        set_log_to_platform_enabled(true);
        set_log_to_platform_verbosity(LogVerbosity::info);
        auto io = get_std_io_stream();
        luexp(init_modules());
        if(argc < 2)
        {
            const c8 usage[] = "Usage: LunaDoc <task> [options]\nType \"LunaDoc --help\" for details.";
            luexp(io->write(usage, sizeof(usage)));
            return ok;
        }
        auto task = Name(argv[1]);
        if(task == "-h" || task == "--help")
        {
            luexp(print_help());
            return ok;
        }
        else if(task == "md" || task == "markdown")
        {
            luexp(gen_markdown(argc, argv));
        }
        else
        {
            return set_error(BasicError::bad_arguments(), "Invalid task: %s", argv[1]);
        }
    }
    lucatchret;
    return ok;
}

int main(int argc, const char* argv[])
{
    bool inited = Luna::init();
    if(!inited) return -1;
    auto r = run(argc, argv);
    if(failed(r))
    {
        log_error("LunaDoc", "%s", explain(r.errcode()));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}