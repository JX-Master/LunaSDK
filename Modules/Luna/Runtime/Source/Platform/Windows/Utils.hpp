/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Utils.hpp
* @author JXMaster
* @date 2018/11/14
*/
#pragma once
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../../Unicode.hpp"
namespace Luna
{
    namespace Platform
    {
        inline wchar_t* utf8_to_wchar_buffered(const c8* src, usize* out_len = nullptr)
        {
            usize len = utf8_to_utf16_len(src) + 1;
            wchar_t* buf = (wchar_t*)memalloc(sizeof(wchar_t) * len);
            usize len2 = utf8_to_utf16((c16*)buf, len, src);
            if(out_len) *out_len = len2;
            return buf;
        }

        inline wchar_t* utf8_to_wchar_buffered(const c8* src, usize src_len, usize* out_len = nullptr)
        {
            usize len = utf8_to_utf16_len(src, src_len) + 1;
            wchar_t* buf = (wchar_t*)memalloc(sizeof(wchar_t) * len);
            usize len2 = utf8_to_utf16((c16*)buf, len, src);
            if(out_len) *out_len = len2;
            return buf;
        }
    }
}