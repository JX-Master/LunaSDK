/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderCompiler.hpp
* @author JXMaster
* @date 2022/10/15
*/
#pragma once
#ifdef LUNA_PLATFORM_WINDOWS
#include <dxc/Support/WinAdapter.h>
#include <dxc/Support/WinIncludes.h>
#include <dxc/dxcapi.h>
#else
#include <dxc/dxcapi.h>
#include <dxc/WinAdapter.h>
#endif
#include <Luna/Runtime/TSAssert.hpp>
#include "../ShaderCompiler.hpp"
#include <string>

namespace Luna
{
    namespace ShaderCompiler
    {
        template <typename _Ty>
        class ComPtr
        {
            _Ty* obj;

            void internal_addref()
            {
                if(obj) obj->AddRef();
            }
            void internal_clear()
            {
                if(obj) obj->Release();
            }
        public:
            ComPtr() : obj(nullptr) {}
            ComPtr(const ComPtr& rhs) : obj(rhs.obj) { internal_addref(); }
            ComPtr(ComPtr&& rhs) : obj(rhs.obj) { rhs.obj = nullptr; }
            ComPtr& operator=(const ComPtr& rhs)
            {
                internal_clear();
                obj = rhs.obj;
                internal_addref();
                return *this;
            }
            ComPtr& operator=(ComPtr&& rhs)
            {
                internal_clear();
                obj = rhs.obj;
                rhs.obj = nullptr;
                return *this;
            }
            ~ComPtr() { internal_clear(); }
            _Ty* get() const { return obj; }
            _Ty* operator->() const { return get(); }
            _Ty** get_address_of() {return &obj;}
            _Ty** operator&() { return get_address_of(); }
            void attach(_Ty* ptr)
            {
                internal_clear();
                obj = ptr;
            }
            _Ty* detach()
            {
                _Ty* r = obj;
                obj = nullptr;
                return r;
            }
            bool valid() const { return obj != nullptr; }
            operator bool() const { return valid(); }
            void reset() { internal_clear(); obj = nullptr; }
        };

        enum class DxcTargetType : u8
        {
            dxil = 0,
            spir_v = 1
        };
        enum class SpirvOutputType : u8
        {
            msl = 0
        };

        struct DxcCompileResult
        {
            ComPtr<IDxcResult> m_dxc_result;
            ComPtr<IDxcBlob> m_dxc_blob;
        };

        struct Compiler : public ICompiler
        {
            lustruct("ShaderCompiler::Compiler", "{E89511FE-424E-4076-8478-6BE1254714E0}");
            luiimpl();
            lutsassert_lock();

            // Context.
            const Path* m_source_file_path = nullptr;
            ComPtr<IDxcCompiler3> m_dxc_compiler;
            ComPtr<IDxcUtils> m_dxc_utils;
            ComPtr<IDxcIncludeHandler> m_default_include_handler;

            R<ShaderCompileResult> compile_none(const ShaderCompileParameters& params);
            R<DxcCompileResult> dxc_compile(const ShaderCompileParameters& params, DxcTargetType target_type);
            R<ShaderCompileResult> spirv_compile(const ShaderCompileParameters& params, SpirvOutputType output_type);
            virtual R<ShaderCompileResult> compile(const ShaderCompileParameters& params) override;
        };
    }
}
