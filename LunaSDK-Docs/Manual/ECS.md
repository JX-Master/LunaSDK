ECS (entity-component-system) stores application data in a data-oriented form. Objects, called *entities*, do not have a fixed class. Their data and behavior are determined by the components and tags assigned to them. Entities with the same component and tag set are stored together so systems can process their data efficiently.

## The ECS architecture

LunaSDK uses the following terms to describe an ECS database:

1. A *world* owns entities, clusters, and component storage for one independent ECS database. Destroying the world destroys all of its entities and component data. `IWorld` is not thread-safe; the application must synchronize structural changes.
2. An *entity* is an object in one world. It is represented by an opaque 64-bit `entity_id_t`. `NULL_ENTITY` is reserved as the invalid entity ID. An entity ID identifies an entity, but does not contain its component data.
3. A *component* is a reflected structure that stores one aspect of entity data. An entity can have at most one component of each type.
4. A *tag* is an untyped pointer used as an additional identity marker. Tags do not store component data, but they participate in archetype selection.
5. A *system* is application code that finds and processes entities and components. LunaSDK does not impose a system object type.

## Archetypes and clusters

An *archetype* is the exact set of component types and tags assigned to an entity. For example, a camera archetype might contain transform and camera components, while a static-mesh archetype might contain transform and static-mesh components. Two entities have the same archetype if and only if their component and tag sets are identical. ^dnm0tf

A *cluster* stores every entity of one archetype. The world creates or finds a cluster with `IWorld::get_cluster`. The combination of a cluster pointer and an index within that cluster is an `EntityAddress`.

Adding or removing a component or tag is a structural change. The application performs it by moving the entity to another cluster with `IWorld::set_entity_cluster`; shared components are moved, removed components are destroyed, and new components are default-constructed.

Cluster storage is dense and is split into fixed-capacity chunks. Deleting or moving an entity can leave a hole. The current implementation fills that hole by relocating the last entity in the source cluster. Therefore, the address of the affected entity and, when applicable, the relocated last entity changes; unrelated entity addresses do not all change merely because one entity was removed. Treat cached `EntityAddress` values as transient across structural changes and call `IWorld::get_entity_address` again when necessary.

## Using tags to separate entities

Tags are part of the [[ECS#^dnm0tf|archetype definition]]. Entities with identical components but different tag sets belong to different clusters.

This can be used to partition data without adding storage. For example, an application can use one stable tag pointer for each loaded sub-level, find all clusters carrying that tag, and delete those clusters to unload the sub-level efficiently. Tag values must remain stable and unique while they are used by the world; ECS treats them as opaque values and does not dereference them.

## Programming guide

### Declare and register components

ECS component types must be registered with Runtime reflection before they are passed to `typeof<T>()` or used in a cluster. Use LunaMetaTool metadata for new component types. For example:

```c++
// Position.hpp
#pragma once
#include <Luna/Runtime/Math/Vector.hpp>
#include "Position.generated.hpp"

struct [[Luna::struct("{FA5B32C3-3768-4905-82D0-0F214E8EE31E}")]] Position
{
    [[Luna::property]] Luna::Float3 value;
};
```

Add `Position.hpp` to `MetaHeaders(...)` in the target rules so LunaMetaTool generates `Position.generated.hpp`. If the target is named `MyGame`, include its target registration header in one source file and call the generated function after Runtime and the ECS module have been initialized:

```c++
#include "MyGame.meta.generated.hpp"

Luna::Meta::register_MyGame_types();
```

Every reflected type must use its own generated, globally unique GUID.

### Initialize ECS and create a world

```c++
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/ECS/ECS.hpp>
#include <Luna/ECS/World.hpp>
#include "Position.hpp"
#include "MyGame.meta.generated.hpp"

using namespace Luna;
using namespace Luna::ECS;

lupanic_if_failed(init());
lupanic_if_failed(add_modules({module_ecs()}));
lupanic_if_failed(init_modules());

Meta::register_MyGame_types();
Ref<IWorld> world = new_world();
```

`module_ecs()` registers the JobSystem module as a dependency, so it does not need to be added separately. Release all worlds and other ECS objects before calling `Luna::close()`.

### Create and access an entity

Create or find a cluster containing the desired component set, then create an entity in that cluster. The returned address can be used immediately to access its component arrays:

```c++
Cluster* position_cluster = world->get_cluster({typeof<Position>()}, {});

EntityAddress addr;
entity_id_t entity = world->new_entity(position_cluster, &addr);

usize chunk = addr.index / CLUSTER_CHUNK_CAPACITY;
usize index_in_chunk = addr.index % CLUSTER_CHUNK_CAPACITY;
Position* positions = get_cluster_components_data<Position>(addr.cluster, chunk);
positions[index_in_chunk].value = Float3(1.0f, 2.0f, 3.0f);
```

Each cluster stores at most `CLUSTER_CHUNK_CAPACITY` entities per chunk. Entity IDs occupy one dense array in each chunk, while each component type occupies its own dense array. `get_cluster_entities` returns the entity IDs in one chunk. `get_cluster_components`, `get_cluster_tags`, `get_cluster_num_entities`, and `get_cluster_num_chunks` expose the remaining metadata required for iteration.

### Change archetypes, find clusters, and delete data

Components and tags are changed by moving an entity to another cluster with `IWorld::set_entity_cluster`. Use `IWorld::find_clusters` to collect clusters that contain a required set of component types and tags, or supply a custom filter callback. Different clusters can be processed independently, but structural changes to the world must still be synchronized by the application.

Use `IWorld::delete_entity` to delete one entity, `IWorld::delete_cluster` to delete a cluster and all of its entities, or `IWorld::delete_all_entities` to clear the world. `delete_cluster` is more efficient than deleting the cluster's entities one at a time.
