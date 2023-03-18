/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneImporterGLTF.cpp
* @author JXMaster
* @date 2023/3/7
*/
#include <ObjLoader/ObjLoader.hpp>
#include "../StudioHeader.hpp"

namespace Luna
{
    struct SceneImporterGLTF : public IAssetEditor
    {
        lustruct("SceneImporterGLTF", "{{77e402e5-c0dc-4ef5-96b7-318af554c6f0}}");
        luiimpl();

        SceneImporterGLTF() {}

        bool m_open;

        virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
    };

    static RV load_gltf_file(const Path& path)
    {
        
    }
}