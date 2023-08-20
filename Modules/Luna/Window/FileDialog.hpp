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
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Path.hpp>
#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif
namespace Luna
{
	namespace Window
	{
		enum class FileDialogFlag : u32
		{
			none = 0,
			//! Allows multiple files to be selected.
			//! This is enabled only for `open_file_dialog` and is ignored in `save_file_dialog`, since the user is allowed to select 
			//! only one file in `save_file_dialog`.
			multi_select = 0x01,
			//! If the filter array is not empty and this is enabled, the file dialog allows the user to select one file with extension name 
			//! that is not in the filter list. If the filter array is empty, this flag is ignored.
			any_file = 0x02,
		};

		struct FileDialogFilter
		{
			//! The name of the filter. For example: "Image File".
			const c8* name;
			//! One array of extension names for this file. For example: {"jpg", "jpeg", "png"}
			Span<const c8*> extensions;
		};

		//! Displays one open file dialog and fetches the selecting results.
		//! @param[in] title The title of the dialog. If this is `nullptr`, the platform-specific default title will be used.
		//! @param[in] filters The filter string used by the open file dialog. If this is empty, the user can select any file types.
		//! @param[in] initial_dir The initial directory to set the file dialog to. If this is empty, the system decides the default directory. This path must be a platform native
		//! path if specified.
		//! @param[in] flags The additional flags.
		//! @return Returns a list of selected file paths. All paths are platform native, absolute paths. This function returns failure if the user fails to select any file.
		LUNA_WINDOW_API R<Vector<Path>> open_file_dialog(const c8* title = nullptr, Span<const FileDialogFilter> filters = {}, const Path& initial_dir = Path(), FileDialogFlag flags = FileDialogFlag::none);

		//! Displays one save file dialog and fetches the selecting results.
		LUNA_WINDOW_API R<Path> save_file_dialog(const c8* title = nullptr, Span<const FileDialogFilter> filters = {}, const Path& initial_file_path = Path(), FileDialogFlag flags = FileDialogFlag::none);

		//! Displays one open directory dialog and fetches the selecting results.
		LUNA_WINDOW_API R<Path> open_dir_dialog(const c8* title = nullptr, const Path& initial_dir = Path());
	}
}