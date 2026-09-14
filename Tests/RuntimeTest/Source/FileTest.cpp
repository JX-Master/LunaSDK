/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FileTest.cpp
* @author JXMaster
* @date 2020/2/20
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/String.hpp>
#ifndef LUNA_PLATFORM_WINDOWS
#include <unistd.h>
#endif

namespace Luna
{
    static void write_test_file(const c8* path, const c8* text)
    {
        auto file = open_file(path, FileOpenFlag::write, FileCreationMode::create_new);
        lupanic_if_failed(file);
        lupanic_if_failed(file.get()->write(text, strlen(text)));
    }

    static void expect_file_text(const c8* path, const c8* text)
    {
        auto file = open_file(path, FileOpenFlag::read, FileCreationMode::open_existing);
        lupanic_if_failed(file);
        auto data = load_file_data(file.get());
        lupanic_if_failed(data);
        lutest(data.get().size() == strlen(text));
        lutest(!memcmp(data.get().data(), text, data.get().size()));
    }

    static void expect_file_error(RV result, ResultCode code)
    {
        lutest(failed(result));
        lutest(unwrap_errcode(result.errcode()) == code);
    }

    static void expect_missing_file(const c8* path)
    {
        auto result = get_file_attribute(path);
        lutest(failed(result));
        lutest(unwrap_errcode(result.errcode()) == E_NOT_FOUND);
    }

    static void file_move_test()
    {
        c8 guid[GUID_STRING_LENGTH + 1] = {};
        lupanic_if_failed(encode_guid(random_guid(), guid, GUID_STRING_LENGTH));
        String root("RuntimeFileMoveTest-");
        root.append(guid);
        lupanic_if_failed(create_dir(root.c_str()));
        String source(root);
        source.append("/source");
        String destination(root);
        destination.append("/destination");
        const FileMoveFlag options[] = {
            FileMoveFlag::none,
            FileMoveFlag::no_copy,
            FileMoveFlag::allow_overwrite,
            FileMoveFlag::allow_overwrite | FileMoveFlag::no_copy
        };
        for(auto flags : options)
        {
            write_test_file(source.c_str(), "new contents");
            write_test_file(destination.c_str(), "original contents");
            if(test_flags(flags, FileMoveFlag::allow_overwrite))
            {
                lupanic_if_failed(move_file(source.c_str(), destination.c_str(), flags));
                expect_file_text(destination.c_str(), "new contents");
                expect_missing_file(source.c_str());
                expect_file_error(move_file(source.c_str(), destination.c_str(), flags), E_NOT_FOUND);
                expect_file_text(destination.c_str(), "new contents");
                // Also allow replacement mode when the destination does not exist yet.
                lupanic_if_failed(move_file(destination.c_str(), source.c_str(), flags));
            }
            else
            {
                auto result = flags == FileMoveFlag::none ? move_file(source.c_str(), destination.c_str()) :
                    move_file(source.c_str(), destination.c_str(), flags);
                expect_file_error(result, E_ALREADY_EXISTS);
                expect_file_text(source.c_str(), "new contents");
                expect_file_text(destination.c_str(), "original contents");
                lupanic_if_failed(delete_file(destination.c_str()));
            }
            // Moving into an absent destination works with every option combination.
            lupanic_if_failed(move_file(source.c_str(), destination.c_str(), flags));
            expect_missing_file(source.c_str());
            expect_file_text(destination.c_str(), "new contents");
            lupanic_if_failed(delete_file(destination.c_str()));

            // Overwrite permission never allows replacing directories, but does allow renaming them.
            lupanic_if_failed(create_dir(source.c_str()));
            lupanic_if_failed(create_dir(destination.c_str()));
            String child(source);
            child.append("/child");
            write_test_file(child.c_str(), "child contents");
            ResultCode directory_error = test_flags(flags, FileMoveFlag::allow_overwrite) ? E_IS_DIRECTORY : E_ALREADY_EXISTS;
            expect_file_error(move_file(source.c_str(), destination.c_str(), flags), directory_error);
            expect_file_error(move_file(child.c_str(), destination.c_str(), flags), directory_error);
            expect_file_text(child.c_str(), "child contents");
            lupanic_if_failed(delete_file(destination.c_str()));
            write_test_file(destination.c_str(), "original contents");
            expect_file_error(move_file(source.c_str(), destination.c_str(), flags), directory_error);
            expect_file_text(destination.c_str(), "original contents");
            lupanic_if_failed(delete_file(destination.c_str()));
            lupanic_if_failed(move_file(source.c_str(), destination.c_str(), flags));
            expect_missing_file(source.c_str());
            child = destination;
            child.append("/child");
            expect_file_text(child.c_str(), "child contents");
            lupanic_if_failed(delete_file(child.c_str()));
            lupanic_if_failed(delete_file(destination.c_str()));
        }

        write_test_file(source.c_str(), "preserved contents");
        expect_file_error(move_file(nullptr, destination.c_str()), E_BAD_ARGUMENTS);
        expect_file_error(move_file(source.c_str(), ""), E_BAD_ARGUMENTS);
        expect_file_error(move_file(source.c_str(), destination.c_str(), (FileMoveFlag)0x04), E_BAD_ARGUMENTS);
        expect_file_error(move_file(source.c_str(), source.c_str(), FileMoveFlag::allow_overwrite), E_BAD_ARGUMENTS);
        expect_file_text(source.c_str(), "preserved contents");
#ifndef LUNA_PLATFORM_WINDOWS
        // A dangling symbolic link is still an existing destination.
        lutest(::symlink("missing", destination.c_str()) == 0);
        expect_file_error(move_file(source.c_str(), destination.c_str()), E_ALREADY_EXISTS);
        expect_file_text(source.c_str(), "preserved contents");
        lupanic_if_failed(move_file(source.c_str(), destination.c_str(), FileMoveFlag::allow_overwrite | FileMoveFlag::no_copy));
        expect_missing_file(source.c_str());
        expect_file_text(destination.c_str(), "preserved contents");
        lupanic_if_failed(delete_file(destination.c_str()));
#else
        lupanic_if_failed(delete_file(source.c_str()));
#endif
        lupanic_if_failed(delete_file(root.c_str()));
    }

    void file_test()
    {
        file_move_test();
        const char s[] = "Sample String";

        {
            auto attribute = get_file_attribute("MissingFileAttributeTest.file");
            lutest(failed(attribute));
            lutest(unwrap_errcode(attribute.errcode()) == E_NOT_FOUND);
        }

        {
            // Try to open one file from vfs and writes to it.
            auto file = open_file("SampleFile.txt",
                FileOpenFlag::write, FileCreationMode::create_always).get();
            lutest(succeeded(file->write(s, sizeof(s) - sizeof(char))));
            file = nullptr;
        }

        {
            // try to open file from platform directly to make sure it is successfully written.
            auto file = open_file("SampleFile.txt", 
                FileOpenFlag::read, FileCreationMode::open_existing).get();
            char str[32];
            lutest(succeeded(file->read(str, 13 * sizeof(char))));
            str[13] = 0;
            lutest(!strcmp(s, str));
            file = nullptr;

            // Clean up.
            lutest(succeeded(delete_file("SampleFile.txt")));
        }
    }
}
