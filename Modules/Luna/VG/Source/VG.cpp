/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VG.cpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "FontAtlas.hpp"
#include "ShapeDrawList.hpp"
#include "ShapeRenderer.hpp"
#include "TextArranger.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>

namespace Luna
{
	namespace VG
	{
		struct VGModule : public Module
		{
			virtual const c8* get_name() override { return "VG"; }
			virtual RV on_register() override
			{
				return add_dependency_modules(this, {module_rhi(), module_shader_compiler()});
			}
			virtual RV on_init() override
			{
				register_boxed_type<FontAtlas>();
				impl_interface_for_type<FontAtlas, IFontAtlas>();
				register_boxed_type<ShapeDrawList>();
				impl_interface_for_type<ShapeDrawList, IShapeDrawList>();
				register_boxed_type<FillShapeRenderer>();
				impl_interface_for_type<FillShapeRenderer, IShapeRenderer>();
				register_boxed_type<TextArranger>();
				impl_interface_for_type<TextArranger, ITextArranger>();
				return init_render_resources();
			}
			virtual void on_close() override
			{
				deinit_render_resources();
			}
		};
	}

	LUNA_VG_API Module* module_vg()
	{
		static VG::VGModule m;
		return &m;
	}
}