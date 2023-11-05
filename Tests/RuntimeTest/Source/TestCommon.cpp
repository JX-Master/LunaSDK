/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TestCommon.cpp
* @author JXMaster
* @date 2020/2/16
*/
#include "TestCommon.hpp"

namespace Luna
{
	i64 TestObject::g_count = 0;
	i64 TestObject::g_ctor_count = 0;
	i64 TestObject::g_dtor_count = 0;
	i64 TestObject::g_default_ctor_count = 0;
	i64 TestObject::g_arg_ctor_count = 0;
	i64 TestObject::g_copy_ctor_count = 0;
	i64 TestObject::g_move_ctor_count = 0;
	i64 TestObject::g_copy_assign_count = 0;
	i64 TestObject::g_move_assign_count = 0;
	i32 TestObject::g_magic_error_count = 0;
	usize g_allocated_memory = 0;

	usize get_allocated_memory()
	{
		return g_allocated_memory;
	}
	void memory_profiler_callback(const ProfilerEvent& event)
	{
		switch (event.id)
		{
			case ProfilerEventId::MEMORY_ALLOCATE:
			{
				ProfilerEventData::MemoryAllocate* data = (ProfilerEventData::MemoryAllocate*)event.data;
				g_allocated_memory += data->size;
				break;
			}
			case ProfilerEventId::MEMORY_REALLOCATE:
			{
				ProfilerEventData::MemoryReallocate* data = (ProfilerEventData::MemoryReallocate*)event.data;
				g_allocated_memory += data->new_size;
				g_allocated_memory -= data->size;
				break;
			}
			case ProfilerEventId::MEMORY_DEALLOCATE:
			{
				ProfilerEventData::MemoryDeallocate* data = (ProfilerEventData::MemoryDeallocate*)event.data;
				g_allocated_memory -= data->size;
				break;
			}
			default: break;
		}
	}
}