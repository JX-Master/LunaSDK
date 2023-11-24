/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Time.cpp
* @author JXMaster
* @date 2020/8/16
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

#include <time.h>

namespace Luna
{
	namespace OS
	{
		LARGE_INTEGER g_ticks_per_second;
		void time_init()
		{
			::QueryPerformanceFrequency(&g_ticks_per_second);
		}
		u64 get_ticks()
		{
			LARGE_INTEGER i;
			::QueryPerformanceCounter(&i);
			return i.QuadPart;
		}
		f64 get_ticks_per_second()
		{
			return (f64)g_ticks_per_second.QuadPart;
		}
		i64 get_utc_timestamp()
		{
			i64 t = _time64(NULL);
			return t;
		}
		i64 get_local_timestamp()
		{
			i64 t = _time64(NULL);
			return OS::utc_timestamp_to_local_timestamp(t);
		}
		i64 local_timestamp_to_utc_timestamp(i64 local_ts)
		{
			tm dt;
			__time64_t t = (__time64_t)local_ts;
			_gmtime64_s(&dt, &t);
			return (i64)_mktime64(&dt);
		}
		i64 utc_timestamp_to_local_timestamp(i64 utc_ts)
		{
			i64 offset = local_timestamp_to_utc_timestamp(utc_ts);
			offset -= utc_ts;
			return utc_ts - offset;
		}
		DateTime timestamp_to_datetime(i64 timestamp)
		{
			tm dt;
			__time64_t t = (__time64_t)timestamp;
			_gmtime64_s(&dt, &t);
			DateTime dt_r;
			dt_r.year = dt.tm_year + 1900;
			dt_r.month = dt.tm_mon + 1;
			dt_r.day = dt.tm_mday;
			dt_r.hour = dt.tm_hour;
			dt_r.minute = dt.tm_min;
			dt_r.second = dt.tm_sec;
			dt_r.day_of_week = dt.tm_wday;
			return dt_r;
		}
		i64 datetime_to_timestamp(const DateTime& datetime)
		{
			tm dt;
			dt.tm_year = datetime.year - 1900;
			dt.tm_mon = datetime.month - 1;
			dt.tm_mday = datetime.day;
			dt.tm_hour = datetime.hour;
			dt.tm_min = datetime.minute;
			dt.tm_sec = datetime.second;
			dt.tm_isdst = 0;
			return utc_timestamp_to_local_timestamp((i64)_mktime64(&dt));
		}
	}
}