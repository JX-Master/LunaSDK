/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2020/4/20
*/
#include "StudioHeader.hpp"
#include <Runtime/Runtime.hpp>
#include <Runtime/File.hpp>
#include "ProjectSelector.hpp"
#include "MainEditor.hpp"
#include <Runtime/Module.hpp>
#include <Runtime/Log.hpp>

namespace Luna
{
	AppEnv* g_env = nullptr;

	RV init_env()
	{
		lutry
		{
			g_env = memnew<AppEnv>();
			luset(g_env->graphics_queue, RHI::get_main_device()->new_command_queue(RHI::CommandQueueType::graphic));
			luset(g_env->async_compute_queue, RHI::get_main_device()->new_command_queue(RHI::CommandQueueType::compute));
		}
		lucatchret;
		return ok;
	}

	void run_editor()
	{
		set_log_std_enabled(true);
		auto r = init_modules();
		if (failed(r))
		{
			log_error("App", explain(r.errcode()));
			return;
		}
		if (failed(init_env())) return;

		// Run project selector.
		auto project = select_project();
		if (failed(project))
		{
			return;
		}

		// Run main editor.
		run_main_editor(project.get());

		memdelete(g_env);
		g_env = nullptr;

		return;
	}

	void set_current_dir_to_process_path()
	{
		Path p = get_process_path();
		p.pop_back();
		luassert_always(succeeded(set_current_dir(p.encode().c_str())));
	}
}

using namespace Luna;

int main()
{
	luassert_always(Luna::init());
	set_current_dir_to_process_path();
	run_editor();
	Luna::close();
	return 0;
}