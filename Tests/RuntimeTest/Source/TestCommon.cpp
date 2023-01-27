/*
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
}