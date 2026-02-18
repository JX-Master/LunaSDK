/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Time.cpp
* @author JXMaster
* @date 2020/12/10
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Platform/Time.hpp"
namespace Luna
{
    LUNA_RUNTIME_API u64 get_ticks()
    {
        return Platform::get_ticks();
    }
    LUNA_RUNTIME_API f64 get_ticks_per_second()
    {
        return Platform::get_ticks_per_second();
    }
    LUNA_RUNTIME_API i64 get_utc_timestamp()
    {
        return Platform::get_utc_timestamp();
    }
    LUNA_RUNTIME_API i64 get_local_timestamp()
    {
        return Platform::get_local_timestamp();
    }
    LUNA_RUNTIME_API i64 local_timestamp_to_utc_timestamp(i64 local_ts)
    {
        return Platform::local_timestamp_to_utc_timestamp(local_ts);
    }
    LUNA_RUNTIME_API i64 utc_timestamp_to_local_timestamp(i64 utc_ts)
    {
        return Platform::utc_timestamp_to_local_timestamp(utc_ts);
    }
    LUNA_RUNTIME_API DateTime timestamp_to_datetime(i64 timestamp)
    {
        return Platform::timestamp_to_datetime(timestamp);
    }
    LUNA_RUNTIME_API i64 datetime_to_timestamp(const DateTime& datetime)
    {
        return Platform::datetime_to_timestamp(datetime);
    }
}
