/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VariantTest.cpp
* @author JXMaster
* @date 2020/12/7
*/
#include "TestCommon.hpp"
#include <Runtime/Variant.hpp>

namespace Luna
{
	void variant_test()
	{
		{
			Variant v(VariantType::array);
			for (usize i = 0; i < U16_MAX; ++i)
			{
				v.push_back(Variant(i));
			}
			lutest(v.size() == U16_MAX);
			for (usize i = 0; i < U16_MAX; ++i)
			{
				lutest(v[i].unum() == i);
			}
		}

		{
			Variant v(VariantType::array);
			for (usize i = 0; i < U16_MAX * 2; ++i)
			{
				v.push_back(Variant(i));
			}
			lutest(v.size() == U16_MAX * 2);
			for (usize i = 0; i < U16_MAX * 2; ++i)
			{
				lutest(v[i].unum() == i);
			}
		}

		{
			Variant v(VariantType::object);
			c8 name_buf[32];
			for (usize i = 0; i < U8_MAX; ++i)
			{
				snprintf(name_buf, 32, "name%d", (i32)i);
				v.insert(name_buf, Variant(i));
			}
			lutest(v.size() == U8_MAX);
			for (usize i = 0; i < U8_MAX; ++i)
			{
				snprintf(name_buf, 32, "name%d", (i32)i);
				auto var = v.find(name_buf);
				lutest(var.unum() == i);
			}
		}

		{
			Variant v(VariantType::object);
			c8 name_buf[32];
			for (usize i = 0; i < 512; ++i)
			{
				snprintf(name_buf, 32, "name%d", (i32)i);
				v.insert(name_buf, Variant(i));
			}
			lutest(v.size() == 512);
			for (usize i = 0; i < 512; ++i)
			{
				snprintf(name_buf, 32, "name%d", (i32)i);
				auto var = v.find(name_buf);
				lutest(var.unum() == i);
			}
		}

		// BUG 20220627: The existance of `null` property in `object` type does not affect comparision.
		{
			Variant a(VariantType::object);
			Variant b(VariantType::object);

			a["k1"] = "Sample";
			a["k2"] = Variant();

			b["k1"] = "Sample";

			lutest(a == b);
		}
	}
}
