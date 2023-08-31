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
#include "../VG.hpp"
#include "ShapeAtlas.hpp"
#include "FontAtlas.hpp"
#include "ShapeDrawList.hpp"
#include "ShapeRenderer.hpp"
#include "TextArranger.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
	namespace VG
	{
		RV init()
		{
			register_boxed_type<ShapeAtlas>();
			impl_interface_for_type<ShapeAtlas, IShapeAtlas>();
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

		void close()
		{
			deinit_render_resources();
		}

		LUNA_STATIC_REGISTER_MODULE(VG, "RHI;ShaderCompiler", init, close);
	}
}