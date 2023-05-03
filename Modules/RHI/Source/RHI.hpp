/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.hpp
* @author JXMaster
* @date 2019/7/10
* @brief D3D12 implementation of GraphicSystem
*/
#pragma once
#include "../RHI.hpp"

namespace Luna
{
	namespace RHI
	{
		//! Implemented by the rendering API to initialize the rendering infrastructure.
		RV render_api_init();
		//! Implemented by the rendering API to clean up the rendering infrastructure.
		void render_api_close();
	}
}