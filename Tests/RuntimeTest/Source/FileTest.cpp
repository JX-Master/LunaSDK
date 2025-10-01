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

namespace Luna
{
    void file_test()
    {
        const char s[] = "Sample String";

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