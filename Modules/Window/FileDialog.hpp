/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FileDialog.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Runtime/Result.hpp>
#include <Runtime/Path.hpp>
#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif
namespace Luna
{
	namespace Window
	{
		enum class FileOpenDialogFlag : u32
		{
			none = 0,
			//! Allows multiple files to be selected.
			multi_select = 0x01,
		};

		//! Displays one open file dialog and fetches the selecting results.
		//! @param[in] filter The filter string used by the open file dialog. The string contains multiple substrings, each of the substring ends with a NULL character (`\0`), and
		//! the last substring ends with two NULL characters. Every item in the filter contains a pair of substrings, the first substring is the text that describes the filter, and 
		//! the second substring describes the filter (like "*.la"). If the filter contains multiple patterns, they are separated by semicolon (like "*.la;*.lb").
		//! For example: "Image File\0*.jpg;*.jpeg;*.png;*.png;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic\0\0"
		//! @param[in] initial_dir The initial directory to set the file dialog to. If this is empty, the system decides the default directory. This path must be a platform native
		//! path if specified.
		//! @param[in] flags See FileOpenDialogFlag.
		//! @return Returns a list of selected file paths. All paths are platform native, absolute paths. This function returns failure if the user fails to select any file.
		LUNA_WINDOW_API R<Vector<Path>> open_file_dialog(const c8* filter, const c8* title = nullptr, const Path& initial_dir = Path(), FileOpenDialogFlag flags = FileOpenDialogFlag::none);

		//! Displays one save file dialog and fetches the selecting results.
		LUNA_WINDOW_API R<Path> save_file_dialog(const c8* filter, const c8* title = nullptr, const Path& initial_file_path = Path());

		//! Displays one open directory dialog and fetches the selecting results.
		LUNA_WINDOW_API R<Path> open_dir_dialog(const c8* title = nullptr, const Path& initial_dir = Path());
	}
}