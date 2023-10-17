/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Adapter.hpp
* @author JXMaster
* @date 2023/10/16
*/
#pragma once
#include "../../Adapter.hpp"
#include "Common.hpp"

namespace Luna
{
    namespace AHI
    {
        struct Adapter : IAdapter
        {
            lustruct("AHI::Adapter", "{1bf1f33e-537c-4c34-98a6-b659378f734c}");
            luiimpl();

            ma_device_info m_info;

            virtual const c8* get_name() override { return m_info.name; }
            virtual bool is_primary() override { return m_info.isDefault != MA_FALSE; }
            virtual RV get_native_wave_formats(WaveFormat* out_formats, usize* num_formats) override;
        };
    }
}