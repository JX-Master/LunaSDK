/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Signal.hpp
* @author JXMaster
* @date 2019/3/14
* @brief Signal interface represents a signal object.
*/
#pragma once
#include "Waitable.hpp"
#include "Ref.hpp"

namespace Luna
{
	//! @addtogroup RuntimeThread
	//! @{
	
	//! @interface ISignal
	//! Represents a system-level signal object.
	//! @threadsafe
	struct ISignal : virtual IWaitable
	{
		luiid("{79648c17-4685-41e0-a625-6228b0a06509}");

		//! Set this signal to trigged state.
		//! @details If this signal is not a manual reset signal, all threads that are waiting for this signal will
		//! be waken up and continue to run.
		//! 
		//! If this signal is a manual reset signal, only one thread that waits for this signal will be waken up, and the 
		//! signal will be reset to untriggered state automatically. If no thread is waiting for this signal when the signal
		//! is triggered, the signal stays in triggered state until one thread calls `wait` or `try_wait` to reset the signal.
		virtual void trigger() = 0;
		//! Resets this signal to untriggered state for manual reset signals.
		virtual void reset() = 0;
	};

	//! Create a new signal object.
	//! @param[in] manual_reset If we need to manually reset the trigger state of the signal object.
	//! If set to `false`, the signal will be automatically reset to not triggered state when a single thread that waiting for this
	//! signal is passed.
	//! If set to `true`, user needs to manually reset the state by calling @ref ISignal::reset.
	//! @return Returns the new created signal object.
	LUNA_RUNTIME_API Ref<ISignal> new_signal(bool manual_reset);

	//! @}
}