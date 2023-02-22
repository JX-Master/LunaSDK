/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Scene.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include <Runtime/TypeInfo.hpp>
#include <Runtime/HashMap.hpp>
#include <Runtime/Vector.hpp>
#include <Runtime/Ref.hpp>
#include <Runtime/Math/Quaternion.hpp>
#include <Runtime/Vector.hpp>
#include <Runtime/Math/Transform.hpp>
#include <Runtime/Result.hpp>

namespace Luna
{
	struct Entity
	{
		lustruct("Entity", "{183DB545-72F3-475C-9413-33F4B4E2BDBD}");

		Name name;
		Entity* parent = nullptr;
		Vector<Ref<Entity>> children;
		Float3 position = Float3::zero();
		Quaternion rotation = Quaternion::identity();
		Float3 scale = Float3::one();
		HashMap<typeinfo_t, ObjRef> components;

		void remove_child(usize index)
		{
			if (index < children.size())
			{
				children[index].get()->parent = nullptr;
				children.erase(children.begin() + index);
			}
		}
		void remove_child(Entity* child)
		{
			for (auto iter = children.begin(); iter != children.end(); ++iter)
			{
				if (*iter == child)
				{
					child->parent = nullptr;
					children.erase(iter);
					break;
				}
			}
		}
		void add_child(Entity* child, usize index)
		{
			Ref<Entity> e = child;
			if (e->parent)
			{
				e->parent->remove_child(e);
			}
			e->parent = this;
			if (index >= children.size())
			{
				children.push_back(e);
			}
			else
			{
				children.insert(children.begin() + index, e);
			}
		}
		Float4x4 this_to_parent_matrix() const
		{
			return AffineMatrix::make(position, rotation, scale);
		}
		Float4x4 parent_to_this_matrix() const
		{
			return inverse(this_to_parent_matrix());
		}
		Float4x4 local_to_world_matrix() const
		{
			if (parent)
			{
				return mul(this_to_parent_matrix(), parent->local_to_world_matrix());
			}
			else
			{
				return this_to_parent_matrix();
			}
		}
		Float4x4 world_to_local_matrix() const
		{
			if (parent)
			{
				return mul(parent->world_to_local_matrix(), parent->parent_to_this_matrix());
			}
			else
			{
				return parent_to_this_matrix();
			}
		}
		Float3 world_position() const
		{
			if (parent)
			{
				Float4x4 mat = parent->local_to_world_matrix();
				Float4 pos = mul(Float4(position.x, position.y, position.z, 1.0f), mat);
				return Float3(pos.x, pos.y, pos.z);
			}
			return position;
		}
		Quaternion world_rotation() const
		{
			if (parent)
			{
				return rotation * parent->world_rotation();
			}
			return rotation;
		}
		void set_world_position(const Float3& position)
		{
			Float4 pos(position.x, position.y, position.z, 1.0f);
			pos = mul(pos, world_to_local_matrix());
			this->position = Float3(pos.x, pos.y, pos.z);
		}
		void set_this_to_parent_matrix(const Float4x4& mat)
		{
			position = AffineMatrix::translation(mat);
			scale = mat.scale_factor();
			Float3 eular_angles = mat.rotation_matrix().euler_angles();
			rotation = Quaternion::from_euler_angles(Float3(eular_angles.x, eular_angles.y, eular_angles.z));
		}
		void set_parent_to_this_matrix(const Float4x4& mat)
		{
			set_this_to_parent_matrix(inverse(mat));
		}
		void set_local_to_world_matrix(const Float4x4& mat)
		{
			if (parent)
			{
				// We need to decompose the local-to-world matrix to this-to-parent matrix.
				set_this_to_parent_matrix(mul(mat, parent->world_to_local_matrix()));
			}
			else
			{
				set_this_to_parent_matrix(mat);
			}
		}
		void set_world_to_local_matrix(const Float4x4& mat)
		{
			if (parent)
			{
				set_parent_to_this_matrix(mul(parent->local_to_world_matrix(), mat));
			}
			else
			{
				set_parent_to_this_matrix(mat);
			}
		}
		template <typename _Ty>
		_Ty* get_component()
		{
			auto iter = components.find(typeof<_Ty>());
			if (iter != components.end())
			{
				return Ref<_Ty>(iter->second).get();
			}
			return nullptr;
		}
	};

	struct Scene
	{
		lustruct("Scene", "{7402c29e-780b-4bb8-8de4-ee83a006a3e8}");

		Vector<Ref<Entity>> root_entities;
		HashMap<typeinfo_t, ObjRef> scene_components;

	private:
		static Entity* find_entity(const Name& name, Vector<Ref<Entity>>& entities)
		{
			for (auto& e : entities)
			{
				if (e->name == name) return e;
				auto child = find_entity(name, e->children);
				if (child) return child;
			}
			return nullptr;
		}

	public:

		Entity* find_entity(const Name& name)
		{
			return find_entity(name, root_entities);
		}

		R<Entity*> add_entity(const Name& name, Entity* parent = nullptr)
		{
			auto ent = find_entity(name);
			if (ent) return BasicError::already_exists();
			Ref<Entity> entity = new_object<Entity>();
			entity->name = name;
			if (parent)
			{
				entity->parent = parent;
				parent->children.push_back(entity);
			}
			else
			{
				root_entities.push_back(entity);
			}
			return entity.get();
		}

		template <typename _Ty>
		_Ty* get_scene_component()
		{
			auto iter = scene_components.find(typeof<_Ty>());
			if (iter != scene_components.end())
			{
				return Ref<_Ty>(iter->second).get();
			}
			return nullptr;
		}

	};
}