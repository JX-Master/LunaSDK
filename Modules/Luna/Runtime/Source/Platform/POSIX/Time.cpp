/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Time.hpp
* @author JXMaster
* @date 2020/9/23
*/
#include "../../OS.hpp"
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#ifdef LUNA_PLATFORM_MACOS
#include <mach/mach_time.h>
#endif

namespace Luna
{
    namespace OS
    {
#ifdef LUNA_PLATFORM_MACOS
		f64 g_ticks_per_second;
#endif
        u64 g_start_ticks;
    
		void time_init()
		{
#ifdef LUNA_PLATFORM_MACOS
            mach_timebase_info_data_t tb_info;
            luassert_msg_always(mach_timebase_info(&tb_info) == KERN_SUCCESS, "mach_timebase_info failed.");
            g_ticks_per_second = (1000000000.0 * (f64)tb_info.denom) / (f64)tb_info.numer;
#endif
            g_start_ticks = get_ticks();
		}
		u64 get_ticks()
		{
#ifdef LUNA_PLATFORM_MACOS
            return mach_absolute_time();
#else
            timespec        spec;
            clock_gettime(CLOCK_MONOTONIC, &spec);
            return (i64(spec.tv_sec) * 1000000000U + spec.tv_nsec);
#endif
		}
		f64 get_ticks_per_second()
		{
#ifdef LUNA_PLATFORM_MACOS
			return g_ticks_per_second;
#else
			return 1000000000.0;
#endif
		}
        i64 get_utc_timestamp()
        {
            time_t t = time(nullptr);
            return (i64)t;
        }
	    i64 get_local_timestamp()
        {
            time_t t = time(nullptr);
            return utc_timestamp_to_local_timestamp((i64)t);
        }
	    i64 local_timestamp_to_utc_timestamp(i64 local_ts)
        {
            tm dt;
            time_t t = (time_t)local_ts;
            // gmtime_r does not shift timestamp by timezone, so we get local date time here.
            gmtime_r(&t, &dt);
            // mktime returns utc timestamp from local date time, which is what we need here.
            return (i64)mktime(&dt);
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
            time_t t = (time_t)timestamp;
            gmtime_r(&t, &dt);
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
            return utc_timestamp_to_local_timestamp((i64)mktime(&dt));
        }
    }
}