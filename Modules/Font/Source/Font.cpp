/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontSystem.cpp
* @author JXMaster
* @date 2019/10/7
*/
#include "FontFileTTF.hpp"
#include "DefaultFont.hpp"
#include <Runtime/Module.hpp>
namespace Luna
{
	namespace Font
	{
		Ref<IFontFile> g_default_font;
		void deinit()
		{
			g_default_font = nullptr;
		}
		RV init()
		{
			register_boxed_type<FontFileTTF>();
			impl_interface_for_type<FontFileTTF, IFontFile>();
			auto r = load_font_file((const byte_t*)opensans_regular_ttf, (usize)opensans_regular_ttf_size, FontFileFormat::ttf);
			if (failed(r))
			{
				return r.errcode();
			}
			g_default_font = r.get();
			return ok;
		}
		StaticRegisterModule m("Font", "", init, deinit);
		LUNA_FONT_API R<Ref<IFontFile>> load_font_file(const byte_t* data, usize data_size, FontFileFormat format)
		{
			switch (format)
			{
			case FontFileFormat::ttf:
			{
				Ref<FontFileTTF> f = new_object<FontFileTTF>();
				lutry
				{
					luexp(f->init(data, data_size));
				}
				lucatchret;
				return f;
			}
			default:
				lupanic();
				break;
			}
			return BasicError::not_supported();
		}
		LUNA_FONT_API IFontFile* get_default_font()
		{
			return g_default_font;
		}
	}
}