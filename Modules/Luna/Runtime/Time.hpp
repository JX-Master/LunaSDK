/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Time.hpp
* @author JXMaster
* @date 2020/8/16
* @brief Runtime System Time APIs.
*/
#pragma once
#include "Result.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeTime Times
	//! @}

	//! @addtogroup RuntimeTime
	//! @{

	//! Queries the ticks of the high-resolution counter of CPU.
	//! @return Returns the current ticks of the CPU.
	LUNA_RUNTIME_API u64 get_ticks();

	//! Queries the resolution of high-resolution counter of CPU represented by
	//! number of ticks per second.
	//! @return Returns the number of ticks per second.
	LUNA_RUNTIME_API f64 get_ticks_per_second();

	struct DateTime
	{
		//! The year since 1 BC. 0 means 1 BC, -1 means 2 BC, 2022 means AD 2022.
		i16 year;
		//! The month [1-12]
		u8 month;
		//! The month of day [1-31]
		u8 day;
		//! The hour [0-23]
		u8 hour;
		//! The minute [0-59]
		u8 minute;
		//! The second [0-60]. Mostly [0-59], 60 for leap second.
		u8 second;
		//! The day of week (0: Sunday, 1: Monday, 2: Tuesday, 3: Wednesday, 4: Thursday, 5: Friday, 6: Saturday).
		u8 day_of_week;
	};

	//! Gets the UTC timestamp of the current time.
	//! @details The returned time is in UNIX time stamp format (number of seconds from Jan 1st, 1970, UTC).
	//! @return Returns the UTC timestamp of the current time.
	LUNA_RUNTIME_API i64 get_utc_timestamp();

	//! Gets the timestamp of the current time shiftted by the timezone setting of the current platform.
	//! @details The returned time is in UNIX time stamp format (number of seconds from Jan 1st, 1970, UTC).
	//! @return Returns the local timestamp.
	LUNA_RUNTIME_API i64 get_local_timestamp();

	//! Converts a local timestamp to a UTC timestamp based on the timezone setting of the current platform.
	//! @param[in] local_ts The local timestamp.
	//! @return Returns the converted UTC timestamp.
	LUNA_RUNTIME_API i64 local_timestamp_to_utc_timestamp(i64 local_ts);

	//! Converts a UTC timestamp to a local timestamp based on the timezone setting of the current platform.
	//! @param[in] utc_ts The UTC timestamp.
	//! @return Returns the converted local timestamp.
	LUNA_RUNTIME_API i64 utc_timestamp_to_local_timestamp(i64 utc_ts);

	//! Converts a timestamp to a calendar date time.
	//! @param[in] timestamp The timestamp to convert.
	//! @return The converted calendar date time.
	LUNA_RUNTIME_API DateTime timestamp_to_datetime(i64 timestamp);

	//! Converts a data time structure to a timestamp, without any timezone shift.
	//! @param[in] datetime The calendar date time to convert.
	//! @return Returns the converted timestamp.
	LUNA_RUNTIME_API i64 datetime_to_timestamp(const DateTime& datetime);

	//! @}
}