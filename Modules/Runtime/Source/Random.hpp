/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Random.hpp
* @author JXMaster
* @date 2019/3/24
*/
#pragma once
#include "../Random.hpp"
#include <random>

namespace Luna
{
	struct Random : IRandom
	{
		lustruct("Random", "{4f09c790-fa3c-4613-b511-2d0175e15582}");
		luiimpl();

		std::mt19937 m_engine;

		Random() {}

		virtual void set_seed(u32 seed) override
		{
			m_engine.seed(seed);
		}
		virtual u32 gen_u32() override
		{
			return m_engine();
		}
		virtual i32 gen_i32() override
		{
			return (i32)m_engine();
		}
		virtual u64 gen_u64() override
		{
			return ((u64)m_engine()) + (((u64)m_engine()) << 32);
		}
		virtual i64 gen_i64() override
		{
			return ((i64)m_engine()) + (((i64)m_engine()) << 32);
		}
		virtual f32 gen_f32(f32 range_begin, f32 range_end) override
		{
			std::uniform_real_distribution<f32> dis(range_begin, range_end);
			return dis(m_engine);
		}
		virtual f64 gen_f64(f64 range_begin, f64 range_end) override
		{
			std::uniform_real_distribution<f64> dis(range_begin, range_end);
			return dis(m_engine);
		}
		virtual Guid gen_guid() override
		{
			Guid guid;
			guid.low = gen_u64();
			guid.high = gen_u64();
			return guid;
		}
	};

	void random_init();
	void random_close();
}