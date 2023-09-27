/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SwapChain.hpp
* @author JXMaster
* @date 2019/9/20
*/
#pragma once
#include "Texture.hpp"
#include <Luna/Runtime/Waitable.hpp>
#include <Luna/Window/Window.hpp>

namespace Luna
{
	namespace RHI
	{
		struct SwapChainDesc
		{
			//! The width of the swap chain back buffer. 
			//! Specify 0 will determine the size from the bounding window's native size.
			u32 width;
			//! The width of the swap chain back buffer. 
			//! Specify 0 will determine the size from the bounding window's native size.
			u32 height;
			//! The number of back buffers in the swap chain.
			u32 buffer_count;
			//! The pixel format of the back buffer.
			Format format;
			//! Whether to synchronize frame image presentation to vertical blanks of the monitor.
			bool vertical_synchronized;

			SwapChainDesc() = default;
			SwapChainDesc(
				u32 width,
				u32 height,
				u32 buffer_count,
				Format format,
				bool vertical_synchronized
			) :
				width(width),
				height(height),
				buffer_count(buffer_count),
				format(format),
				vertical_synchronized(vertical_synchronized) {}
		};

		//! @interface ISwapChain
		struct ISwapChain : virtual IDeviceChild
		{
			luiid("{cc455fba-646d-4a64-83e4-149f004a5ea0}");

			//! Gets the window that this swap chain bounds to.
			virtual Window::IWindow* get_window() = 0;

			//! Gets the descriptor object.
			virtual SwapChainDesc get_desc() = 0;

			//! Gets the current back buffer that is available for rendering.
			//! @return Returns the current back buffer that is available for rendering.
			//! @remark The first call to `get_current_back_buffer` after `present` may block the current thread until 
			//! at least one back buffer is available for rendering, or until an error occurs. 
			//! After the first successful `get_current_back_buffer` call, all succeeding calls to `get_current_back_buffer` 
			//! return the same back buffer until another `present` call is issued. Every `present` call evicts the user access to the 
			//! current back buffer, and next `get_current_back_buffer` call will wait for another back buffer available for rendering.
			//! The returned back buffer resource should be released immediately after `present` is called.
			virtual R<ITexture*> get_current_back_buffer() = 0;

			//! Submits the current back buffer to the bounding queue for presenting.
			//! @remark This function only enqueues the presentation command to the command queue and 
			//! returns immediately after the command is successfully enqueued.
			//! The user must ensure that all writes to the current back buffer is completed before calling `present` to present the back buffer.
			virtual RV present() = 0;

			//! Resets the swap chain.
			//! @param[in] desc The new swap chain descriptor object.
			virtual RV reset(const SwapChainDesc& desc) = 0;
		};
	}
}