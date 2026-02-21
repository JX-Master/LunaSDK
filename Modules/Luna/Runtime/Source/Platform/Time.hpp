/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Time.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Time.hpp"

namespace Luna
{
    namespace Platform
    {
        //! Queries the ticks of the high-performance counter of CPU.
        //! @return The current ticks of the CPU.
        u64 get_ticks();

        //! Queries the resolution of high-performance counter of CPU represented by
        //! number of ticks per second.
        //! @return The number of ticks per second.
        f64 get_ticks_per_second();

        //! Gets the timestamp of the current time.
        //! The returned time is in UNIX time stamp format (number of seconds from Jan 1st, 1970, UTC).
        i64 get_utc_timestamp();

        //! Gets the timestamp of the current time shiftted by the timezone setting of the current platform.
        //! The returned time is in UNIX time stamp format (number of seconds from Jan 1st, 1970, UTC).
        i64 get_local_timestamp();

        //! Converts a local timestamp to a UTC timestamp based on the timezone setting of the current platform.
        i64 local_timestamp_to_utc_timestamp(i64 local_ts);

        //! Converts a UTC timestamp to a local timestamp based on the timezone setting of the current platform.
        i64 utc_timestamp_to_local_timestamp(i64 utc_ts);

        //! Converts a timestamp to a calendar date time structure.
        DateTime timestamp_to_datetime(i64 timestamp);

        //! Converts a data time structure to a timestamp, without any timezone shift.
        i64 datetime_to_timestamp(const DateTime& datetime);
    }
}