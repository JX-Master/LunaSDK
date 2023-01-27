/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneAsset.hpp
* @author JXMaster
* @date 2022/12/17
*/
#include "Scene.hpp"
#include <VFS/VFS.hpp>
#include <Runtime/VariantJSON.hpp>
#include <Runtime/Serialization.hpp>
#include "../StudioHeader.hpp"

namespace Luna
{
	Name get_scene_asset_type()
	{
		return "Scene";
	}

	R<Variant> serialize_entity(const Entity* entity)
	{
		Variant ret;
		lutry
		{
			lulet(data, serialize(entity->name));
			ret["name"] = move(data);
			luset(data, serialize(entity->position));
			ret["position"] = move(data);
			luset(data, serialize(entity->rotation));
			ret["rotation"] = move(data);
			luset(data, serialize(entity->scale));
			ret["scale"] = move(data);
			data = Variant(Variant::Type::array);
			for (auto& i : entity->children)
			{
				lulet(child, serialize_entity(i.get()));
				data.push_back(move(child));
			}
			ret["children"] = move(data);
			data = Variant(Variant::Type::array);
			for (auto& i : entity->components)
			{
				Variant comp(Variant::Type::array);
				lulet(type, serialize<Guid>(get_type_guid(i.first)));
				lulet(value, serialize(i.first, i.second.get()));
				comp.push_back(move(type));
				comp.push_back(move(value));
				data.push_back(move(comp));
			}
			ret["components"] = move(data);
		}
		lucatchret;
		return ret;
	}
	RV deserialize_entity(Entity* entity, const Variant& data)
	{
		lutry
		{
			luexp(deserialize(entity->name, data["name"]));
			luexp(deserialize(entity->position, data["position"]));
			luexp(deserialize(entity->rotation, data["rotation"]));
			luexp(deserialize(entity->scale, data["scale"]));
			for (auto& child : data["children"].values())
			{
				Ref<Entity> ch = new_object<Entity>();
				luexp(deserialize_entity(ch.get(), child));
				ch->parent = entity;
				entity->children.push_back(ch);
			}
			for (auto& comp : data["components"].values())
			{
				Guid type_guid;
				luexp(deserialize(type_guid, comp[(usize)0]));
				typeinfo_t type = get_type_by_guid(type_guid);
				if (!type) return BasicError::bad_data();
				object_t obj = object_alloc(type);
				ObjRef ref;
				ref.attach(obj);
				luexp(deserialize(type, obj, comp[1]));
				entity->components.insert(make_pair(type, ref));
			}
		}
		lucatchret;
		return ok;
	}
	void register_scene_asset_type()
	{
		register_struct_type<Entity>({});
		register_struct_type<Scene>({});
		{
			SerializableTypeDesc desc;
			desc.serialize_func = [](typeinfo_t type, const void* inst)->R<Variant> {
				Variant ret;
				lutry
				{
					const Scene * scene = (const Scene*)inst;
					Variant root_entities(Variant::Type::array);
					for (auto& i : scene->root_entities)
					{
						lulet(data, serialize_entity(i.get()));
						root_entities.push_back(move(data));
					}
					ret["root_entities"] = move(root_entities);
					Variant scene_components(Variant::Type::array);
					for (auto& i : scene->scene_components)
					{
						Variant comp(Variant::Type::array);
						lulet(type, serialize<Guid>(get_type_guid(i.first)));
						lulet(value, serialize(i.first, i.second.get()));
						comp.push_back(move(type));
						comp.push_back(move(value));
						scene_components.push_back(move(comp));
					}
					ret["scene_components"] = move(scene_components);
				}
				lucatchret;
				return ret;
			};
			desc.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data)->RV {
				lutry
				{
					Scene * scene = (Scene*)inst;
					for (auto& i : data["root_entities"].values())
					{
						Ref<Entity> ent = new_object<Entity>();
						luexp(deserialize_entity(ent.get(), i));
						scene->root_entities.push_back(ent);
					}
					for (auto& i : data["scene_components"].values())
					{
						Guid type_guid;
						luexp(deserialize(type_guid, i[(usize)0]));
						typeinfo_t type = get_type_by_guid(type_guid);
						if (!type) return BasicError::bad_data();
						object_t obj = object_alloc(type);
						construct_type(type, obj);
						ObjRef ref;
						ref.attach(obj);
						luexp(deserialize(type, obj, i[1]));
						scene->scene_components.insert(make_pair(type, ref));
					}
				}
				lucatchret;
				return ok;
			};
			set_serializable<Scene>(&desc);
		}
		{
			Asset::AssetTypeDesc desc;
			desc.name = get_scene_asset_type();
			desc.on_load_asset = load_json_asset<Scene>;
			desc.on_save_asset = save_json_asset<Scene>;
			desc.on_set_asset_data = nullptr;
			desc.userdata = nullptr;
			Asset::register_asset_type(desc);
		}
	}
}