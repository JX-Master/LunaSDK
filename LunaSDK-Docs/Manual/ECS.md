ECS (entity-component-system) is a data structure that stores data using a data-oriented pattern. In ECS model, objects (we call entities) don't have any particular type, the behavior of one entity depends on what kind of data it has. This brings a great flexibility for defining entity behavior, it also brings better performance since we can group entities with similar components together for a more cache-friendly data accessing.

## The ECS Architecture
In LunaSDK, we use the following terms to describe one ECS database:
1. *world*: The world is the container for entities and their data. The world allocates memory to store all data for one ECS database instance, when the world is destroyed, all entities in the world will be destroyed, along with their data.
2. *entity*: An entity represents a data object in one specific world. An entity is represented by a unique 64-bit unsigned integer that can be allocated and freed. The entity itself does not contain any data, it acquires data by adding *components* to that entity.
3. *component*: A component is a typed structure that describes parts of the entity data. One entity can have unlimited components attached, but at most one component per type. Components can be added to or removed from one entity at any time.
4. *system*: The system is the procedure that manipulates entities and their components. In theory, systems are pure functions that does not have any state, but in practice, the system is usually used to describe user codes that doing CRUD operations on one ECS world.

## Archetypes and Clusters
Archetypes and clusters are how we store entities and their data in ECS module, we expose this internal data structure to the application so that user can maximize performance when manipulating entities.

An *archetype* is an implicit type described by a set of component types and tags. For example, one camera archetype may be described by one transform component and one camera component, while one static mesh archetype may be described by one transform component and one static mesh component. If two entities have the exact same kind of components, we say these two entities have the same archetype. ^dnm0tf

Archetypes are used to organize memory storage for entities. Every archetype that owns at least one entity will allocate memory storage for all entities of that archetype, which is called a *cluster*. Every entity will be placed in one specific cluster based on its archetype, and two entities are in the same cluster *if and only if they have exact the same kinds of components and tags*. The cluster that an entity belongs to and the position of the entity in that cluster are called the *address* of that entity. The world maintains a map from entity ID to the address of that entity, if any component is added to or removed from one entity, then the entity will be moved to a new cluster, causing its address being changed.

The memory allocated for one cluster is not continuous, it is composed by one or more memory chunks. The size of one chunk is usually aligned to system's memory page and is fixed. If the free size in one chunk is not enough to place new entities, one new chunk will be allocated to contain more entities. Removing entities from one chunk swaps the data with the last entity in the cluster, so that entities in one chunk is always continuous, and new entities are always allocated from the last chunk.

## Using Tags to Separate Entities to Different Clusters
Tags are untyped pointers that are used to mark entities. One entity can have unlimited number of tags, each of them must have a pointer different from others. Since [[ECS#^dnm0tf|tags are part of archetype definition]], if two entities have exact the same kinds of components, but with different set of tags, their archetypes are different and belongs to two different clusters.

The application can use this feature to separate entities to different clusters, so that they can be manipulated independently without interfering each other. For example, the application can define one tag for every loaded sub-level, they use the tag of one sub-level to filter out all clusters for that level, such clusters can them be removed directly to efficiently unloads that sub-level.

## Managing Entities
Every entity is identified by one *entity ID* that is unique in the current world context. Besides the entity ID, every entity also have one *entity address* that tells which *cluster* this entity belongs to and the position of the entity in that cluster. The entity address will change if the entity is moved to another cluster, or if any entity is removed from the cluster that contains the current entity. The application can use the entity address to read and write components of that entity.

Components and tags cannot be added to or removed from entities directly. Instead, the application should fetch the cluster that matches the specified components and tags firstly by calling `IWorld::get_cluster`, then calls `IWorld::set_entity_cluster` to move the entity to the target cluster. When moving entities between clusters, components are automatically constructed, destructed or moved.

To iterate over entities, the application should call `IWorld::find_clusters` to get all clusters that matches the user-defined filter function, then entities in every cluster can be iterated independently and concurrently. `IWorld::find_clusters` provides a more convenient version that accepts a set of component types and tags and returns all clusters that contains the specified component types and tags. Entities are always continuous in one cluster, if one entity is removed from the cluster, the place will be filled by swapping the last entity front. The application can also call `IWorld::delete_cluster` to delete one cluster and all entities in that cluster, this will be more efficient than deleting every entity in the cluster manually.

## Chunks
Entities in one cluster are stored using *chunks*. One chunk is a continuous memory storage that stores at most `CLUSTER_CHUNK_CAPACITY` entities, forming one entity array. Components in one chunk are stored using structure-of-array structure, every component has its own element array in one chunk, which can be fetched by calling `get_cluster_components_data`. Given a entity address with specified position `i`, the entity will be placed in chunk `i / CLUSTER_CHUNK_CAPACITY` at index `i % CLUSTER_CHUNK_CAPACITY`, so them component can be fetched like so:
```c++
ComponentType* component_array = (ComponentType*)get_cluster_components_data(addr.cluster, addr.index / CLUSTER_CHUNK_CAPACITY, typeof<ComponentType>());
ComponentType& data = component_array[addr.index % CLUSTER_CHUNK_CAPACITY];
```