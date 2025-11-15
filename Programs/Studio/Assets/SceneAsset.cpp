/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneAsset.hpp
* @author JXMaster
* @date 2022/12/17
*/
#include "Scene.hpp"
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include "../StudioHeader.hpp"

namespace Luna
{
    Name get_scene_asset_type()
    {
        return "Scene";
    }

    static R<Variant> serialize_actor(typeinfo_t type, const void* inst)
    {
        const SceneActor& actor = *((const SceneActor*)inst);
        Variant ret;
        lutry
        {
            luset(ret["guid"], serialize(actor.guid));
            luset(ret["name"], serialize(actor.name));
            luset(ret["transform"], serialize(actor.transform));
            luset(ret["children"], serialize(actor.children));
            Variant data = Variant(VariantType::array);
            for (auto& i : actor.components)
            {
                Variant comp(VariantType::array);
                lulet(type, serialize<Guid>(get_type_guid(i.type())));
                lulet(value, serialize(i.type(), i.get()));
                comp.push_back(move(type));
                comp.push_back(move(value));
                data.push_back(move(comp));
            }
            ret["components"] = move(data);
        }
        lucatchret;
        return ret;
    }

    static RV deserialize_actor(typeinfo_t type, void* inst, const Variant& data)
    {
        SceneActor& actor = *((SceneActor*)inst);
        lutry
        {
            luexp(deserialize(actor.guid, data["guid"]));
            luexp(deserialize(actor.name, data["name"]));
            luexp(deserialize(actor.transform, data["transform"]));
            luexp(deserialize(actor.children, data["children"]));
            for (auto& comp : data["components"].values())
            {
                Guid type_guid;
                luexp(deserialize(type_guid, comp[(usize)0]));
                typeinfo_t type = get_type_by_guid(type_guid);
                if (!type) return BasicError::bad_data();
                object_t obj = object_alloc(type);
                construct_type(type, obj);
                ObjRef ref;
                ref.attach(obj);
                luexp(deserialize(type, obj, comp[1]));
                actor.components.push_back(move(ref));
            }
        }
        lucatchret;
        return ok;
    }
    void register_scene_asset_type()
    {
        register_struct_type<SceneActor>({
            luproperty(SceneActor, Guid, guid),
            luproperty(SceneActor, Name, name),
            luproperty(SceneActor, Transform, transform),
            luproperty(SceneActor, Vector<Guid>, children)
        });
        {
            SerializableTypeDesc desc;
            desc.serialize_func = serialize_actor;
            desc.deserialize_func = deserialize_actor;
            set_serializable<SceneActor>(&desc);
        }
        register_struct_type<Scene>({
            luproperty(Scene, SceneSettings, settings),
            luproperty(Scene, Vector<SceneActor>, actors)
        });
        set_serializable<Scene>();
        {
            Asset::AssetTypeDesc desc;
            desc.name = get_scene_asset_type();
            desc.on_load_asset = load_json_asset<Scene>;
            desc.on_save_asset = save_json_asset<Scene>;
            desc.on_load_asset_default_data = create_default_object<Scene>;
            desc.on_set_asset_data = nullptr;
            desc.userdata = nullptr;
            Asset::register_asset_type(desc);
        }
    }
}