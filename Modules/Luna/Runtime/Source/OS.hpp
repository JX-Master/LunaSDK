/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file OS.hpp
* @author JXMaster
* @date 2020/8/20
* @brief This file contains the Operation System Independent Layer (or simply OS layer), 
* which is an OS-agnostic programming interface between the underlying platform/OS and the Luna Runtime that lists the 
* minimum requirements for Luna Runtime to run.
* 
* For every platform/OS that wants to run Luna Runtime, all functions in the interface layer should be implemented.
*/
#pragma once
#include "../Result.hpp"
#include "../Time.hpp"
#include "../Thread.hpp"
#include "../File.hpp"
#include "../Log.hpp"
namespace Luna
{
	namespace OS
	{
		//! Initializaes the os/platform layer. This is called by the Luna Runtime when Luna::init is called.
		//! All SDK services cannot be used in this call, including memory allocation/deallocation.
		void init();

		//! Closes the os/platform layer. This is called by the Luna Runtime when Luna::close is called.
		//! All SDKrvices cannot be used in this call, including memory allocation/deallocation.
		void close();

		//! Reports an assertion failure information to the underlying OS or CRT.
		//! This function works in all builds, and can be called even if the runtime is not initialized.
		//! The behavior of this function depends on the OS/CRT implementation, but in general it will 
		//! present an error message box and then terminate the program.
		//! 
		//! If the underlying platform does not support assertions, it can simply terminate the program,
		//! or pause the program at where the assertion fails.
		//! @param[in] msg The UTF-8 error message to show.
		//! @param[in] file The UTF-8 name of the file that causes the panic.
		//! @param[in] line The code line the assertion is placed.
		void assert_fail(const c8* msg, const c8* file, u32 line);

		//! Triggers a debug break, pauses the program and attaches the debugger to the program.
		//! This only works in debug build. In release build, this function returns directly. If the underlying
		//! platform does not support debug break, this function is skipped and returned directly.
		void debug_break();

		//! Logs one message to the platform.
		//! If logging is not available on the current build, this function does nothing.
		//! @param[in] verbosity The log verbosity.
		//! @param[in] tag The log tag. Used by the implementation to filter logs.
		//! @param[in] tag_len The log tag length, not including the null terminator.
		//! @param[in] message The log message encoded in UTF-8. The implementation should insert newline character (`\n`) in its implementation.
		//! @param[in] message_len The log message length, not including the null terminator.
		void log(LogVerbosity verbosity, const c8* tag, usize tag_len, const c8* message, usize message_len);

		//! Allocates memory blocks from system heap.
		//! @param[in] size The number of bytes to allocate. If this is 0, no memory will be allocated and the return value will be `nullptr`.
		//! @param[in] alignment Optional. The required alignment of the allocated memory block. 
		//! 
		//! If this is 0 (default), then the memory is allocated with no additional alignment requirement. In such case, the memory address is 
		//! aligned to times of 8 in 32-bit platform, and times of 16 in 64-bit platform. 
		//! 
		//! If this is not 0, then the memory is allocated with the specified alignment requirement satisfied. The alignment value must be powers
		//! of 2 (32, 64, 128, 256, etc).
		//! 
		//! Note that you shall use the same alignment value for the same memory block in `allocate`, `free`, `reallocate` and `size` function.
		//! 
		//! @return Returns a pointer to the first available memory address allocated, or `nullptr` if failed to allocate one from this allocator.
		void* memalloc(usize size, usize alignment = 0);

		//! Frees memory blocks allocated by `OS::memalloc` or `OS::memrealloc`.
		//! @param[in] ptr The pointer returned by `OS::memalloc` or `OS::memrealloc` method. If this is `nullptr`, this function does nothing.
		//! @param[in] alignment Optional. The alignment requirement specified when allocating the memory block. Default is 0.
		void memfree(void* ptr, usize alignment = 0);

		//! Gets the allocated size of the memory block allocated by `OS::memalloc` or `OS::memrealloc`. 
		//! The returned size is the size that is available for the user to use. 
		//! Note that the allocated size may be bigger than the size required to specify alignment and padding requirements.
		//! @param[in] ptr The pointer returned by `OS::memalloc` or `OS::memrealloc`.
		//! @param[in] alignment Optional. The alignment requirement specified for the memory block during allocation. Default is 0. 
		//! @return The size of bytes of the memory block. If `ptr` is `nullptr`, the returned value is 0.
		usize memsize(void* ptr, usize alignment = 0);

		//! Global object creation function.
		template <typename _Ty, typename... _Args>
		_Ty* memnew(_Args&&... args)
		{
			_Ty* o = reinterpret_cast<_Ty*>(OS::memalloc(sizeof(_Ty), alignof(_Ty)));
			if (o)
			{
				new (o) _Ty(forward<_Args>(args)...);
				return o;
			}
			return nullptr;
		}

		//! Global object deleting function.
		template <typename _Ty>
		void memdelete(_Ty* o)
		{
			o->~_Ty();
			OS::memfree(o, alignof(_Ty));
		}

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

		using thread_callback_func_t = void(void* params);

		//! Create a new system thread and make it run the callback function.
		//! @param[in] callback The callback function to be called by the new thread.
		//! @param[in] params The parameter object passed to callback function as parameter.
		//! @param[in] stack_size The stack size for new thread's call stack.
		//! @return Returns an interface representing the new created thread, or `nullptr` if failed.
		opaque_t new_thread(thread_callback_func_t* callback, void* params, const c8* name, usize stack_size);

		//! Sets the thread schedule priority.
		//! @param[in] thread The thread handle.
		//! @param[in] priority The priority to set.
		void set_thread_priority(opaque_t thread, ThreadPriority priority);

		//! Waits for the thread to finish.
		void wait_thread(opaque_t thread);

		//! Tries to wait for the thread to finish.
		bool try_wait_thread(opaque_t thread);

		//! Closes the thread handle. 
		void detach_thread(opaque_t thread);

		//! Gets the current thread handle.
		opaque_t get_current_thread_handle();

		//! Suspends current thread for a specific period of time. The actual suspended time may be longer than required.
		//! @param[in] time_milliseconds The time, in milliseconds, that this thread needs to suspend.
		void sleep(u32 time_milliseconds);

		//! Delay the execution of this thread for a very shout time by yielding this thread several times.
		//! This is more accurate to `sleep` method and will not suspend current thread unless the specified time is larger than several milliseconds.
		//! @param[in] time_microseconds The time, in microseconds, that this thread needs to delay.
		void fast_sleep(u32 time_microseconds);

		//! Yields the remain time slice of the current thread and let OS to schedule other threads.
		//! There is no way to resume a thread from user mode, since threads are scheduled by OS automatically.
		//! 
		//! If multi-thread is not supported on the target platform, this function does nothing and returns immediately
		void yield_current_thread();

		//! Allocates one thread local storage (TLS) slot for every thread running in this process, including the thread that is currently not being 
		//! created yet. After the handle is returned, every thread can set a thread-local value to this slot using this handle.
		//! 
		//! The allocated slot is large enough to store one pointer to the real thread-local data. The pointer is `nullptr` for every thread before it 
		//! is firstly modified by that thread.
		//! @param[in] destructor The optional destructor to use for this TLS slot. If this is not `nullptr`, this destructor will be called on one thread
		//! when that thread exits and the TLS pointer value of this slot for that thread is not `nullptr`.
		//! 
		//! When the destructor is called, the system resets the TLS value to `nullptr` and passes the original TLS value to the destructor. This process may
		//! be repeated several times until the TLS value is `nullptr` after the destructor returns.
		//! 
		//! If multi-thread is not supported on the target platform, TLS can be mimicked using one global TLS table
		//! for the only one thread.
		//! 
		//! @return The handle to the TLS slot, or error code if failed.
		opaque_t tls_alloc(void (*destructor)(void*));

		//! Free the TLS slot allocated by `tls_alloc`. The handle will be invalid after this call and the pointer stored for every 
		//! thread will be discarded.
		//! 
		//! Make sure to free all resources bound to the specified slot manually before calling this, or they will never be freed.
		//! @param[in] handle The handle returned by `tls_alloc`.
		//! @remark Note that calling `tls_free` will not call the destructor registered for this slot on any thread. After `tls_free` is called, the 
		//! destructor will be cleared and will not be called any more.
		void tls_free(opaque_t handle);

		//! Set the data bound to the current thread's TLS slot specified by `handle`.
		//! @param[in] handle The handle of the slot specified.
		//! @param[in] ptr The pointer value to set to this slot.
		void tls_set(opaque_t handle, void* ptr);

		//! Get the value bound to the TLS slot of current thread.
		//! @param[in] handle The handle of the slot to query.
		//! @return The pointer set, or `nullptr` if no pointer is set to this slot.
		void* tls_get(opaque_t handle);

		//! Creates one signal object.
		opaque_t new_signal(bool manual_reset);

		//! Destroys one signal object.
		void delete_signal(opaque_t signal);

		//! Waits for the signal to be triggered.
		void wait_signal(opaque_t signal);

		//! Try to wait for the signal is triggered, returns immediately if the signal is not triggered.
		bool try_wait_signal(opaque_t signal);

		//! Triggers the signal.
		void trigger_signal(opaque_t signal);

		//! Resets the signal to non-triggered state. 
		//! This operation is valid only for the signal with `manual_reset` set to `true` when creating the signal.
		void reset_signal(opaque_t signal);

		//! Creates one mutex object.
		opaque_t new_mutex();

		//! Destroys one mutex object.
		void delete_mutex(opaque_t mutex);

		//! Locks the mutex object.
		void lock_mutex(opaque_t mutex);

		//! Tries to lock the mutex object, returns immediately if the mutex is already locked by another thread.
		bool try_lock_mutex(opaque_t mutex);

		//! Releases the mutex object.
		void unlock_mutex(opaque_t mutex);

		//! Creates one semaphore object.
		opaque_t new_semaphore(i32 initial_count, i32 max_count);

		//! Destroys one semaphore object.
		void delete_semaphore(opaque_t sema);

		//! Acquires one semaphore object.
		void acquire_semaphore(opaque_t sema);

		//! Tries to acquire one semaphore object.
		bool try_acquire_semaphore(opaque_t sema);

		//! Releases the semaphore acquired.
		void release_semaphore(opaque_t sema);

		//! Initializes read write lock object.
		//! The read write lock object have three modes: unlocked mode, read mode or write mode.
		//! 
		//! On unlocked mode, both read and write request will succeed, changing the lock to read or write mode.
		//! 
		//! On read mode, only read request will succeed. Every read request increases the read count of the lock, and the lock will be changed back to 
		//! unlocked mode when the last read lock is released.
		//! 
		//! On write mode, both read and write request will fail. The read write lock is not recursive, so only one write lock can be acquired.
		//! The lock will be changed back to unlocked mode when the write lock is released.
		opaque_t new_read_write_lock();

		//! Destroys read write lock object.
		void delete_read_write_lock(opaque_t lock);

		//! Acquires read lock of the read write lock object.
		//! This call blocks the current thread until the lock is acquired.
		void acquire_read_lock(opaque_t lock);

		//! Acquires write lock of the read write lock object.
		//! This call blocks the current thread until the lock is acquired.
		void acquire_write_lock(opaque_t lock);

		//! Tries to acquire read lock of the read write lock object.
		//! This call does not block, it returns `false` if the lock cannot be acquired.
		bool try_acquire_read_lock(opaque_t lock);

		//! Tries to acquire write lock of the read write lock object.
		//! This call does not block, it returns `false` if the lock cannot be acquired.
		bool try_acquire_write_lock(opaque_t lock);

		//! Releases the read lock acquired.
		void release_read_lock(opaque_t lock);

		//! Releases the write lock acquired.
		void release_write_lock(opaque_t lock);

		//! Opens unbuffered file.
		//! @param[in] path The path of the file.
		//! @param[in] flags The file open flags.
		//! @param[in] creation Specify whether to create a file if the file does not exist.
		//! @return The new opened file handle if succeeds. Returns one error code if failed.
		//! Possible errors:
		//! * BasicError::bad_arguments
		//! * BasicError::access_denied
		//! * BasicError::not_found
		//! * BasicError::bad_platform_call for all errors that cannot be identified.
		R<opaque_t> open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation);

		//! Closes one file opened by `open_file`.
		//! @param[in] file The file handle opened by `open_file`.
		void close_file(opaque_t file);

		//! Reads data from the current position the cursor is pointing to and offsets the cursor back. If the data to be
		//! read is not ready, the platform suspends this thread until the data is ready.
		//! @param[in] file The file handle opened by `open_file`.
		//! @param[in] buffer The buffer used to store the read data.
		//! @param[in] size The size, in bytes, of the data to read.
		//! @param[out] read_bytes If this is not `nullptr`, the system sets the actual size of bytes being read to the buffer
		//! to this parameter.
		//! The actual size of bytes being read may be smaller than the size of bytes required to be read if the cursor
		//! reaches the end of the stream, but this is NOT an error. Specially, if one read operation is performed when
		//! the cursor is beyond or at the end of the stream, the read operation succeeds with 0 bytes being read. This 
		//! can be considered as an EOF symbol in stdlib.
		//! @return Returns success on success, returns the following error code on failure:
		//! * BasicError::not_supported
		RV read_file(opaque_t file, void* buffer, usize size, usize* read_bytes = nullptr);

		//! Writes data to the current position the cursor is pointing to and offsets the cursor back. This call returns after
		//! all data have been written.
		//! @param[in] file The file handle opened by `open_file`.
		//! @param[in] buffer The buffer that holds the data to be written.
		//! @param[in] size The size, in bytes, of the data to write.
		//! @param[out] write_bytes If not `nullptr`, the system sets the actual size of bytes being written to this parameter.
		//! Mostly, if the cursor goes beyond the end of the stream buffer while writing data, the stream will be expanded so
		//! the succeeding data can be written, so unless an error occurs, the size of bytes written will always equal to the 
		//! size of bytes required by the user to write. However, if an error occurs while writing data, some of the data may have 
		//! already be written while others are not, in such case the `write_bytes` reported by system may not be equal to `size` 
		//! specified by the user.
		RV write_file(opaque_t file, const void* buffer, usize size, usize* write_bytes = nullptr);

		//! Gets the size of the file in bytes.
		//! @param[in] file The file handle opened by `open_file`.
		//! @return The size of the file, or error code on failure.
		u64 get_file_size(opaque_t file);

		//! Sets the size of the file in bytes.
		//! If the current file size is smaller than the size to set and this call succeeded, the stream will be extended to the size specified
		//! with data between the last size and current size be uninitialized. If the current file size is greater than the size to set and this 
		//! call succeeded, the stream will be truncated and the data between the last size and current size will be discarded.
		//! @param[in] file The file handle opened by `open_file`.
		//! @param[in] size The size to set, in bytes.
		RV set_file_size(opaque_t file, u64 size);

		//! Gets the current position of the stream cursor. The position is number of bytes relative to the beginning of the 
		//! stream.
		//! @param[in] file The file handle opened by `open_file`.
		R<u64> get_file_cursor(opaque_t file);

		//! Moves the cursor to a new position.
		//! @param[in] file The file handle opened by `open_file`.
		//! @param[in] offset Number of bytes to move relative to the position specified by `mode`.
		//! @param[in] mode The seek mode to use.
		//! @remark The cursor position is not bound to the size of the stream. If you move the cursor beyond the end of the stream, the cursor
		//! is still valid with the following behaviors:
		//! 1. All read operations succeed with 0 byte being read, just like EOF behaviors.
		//! 2. All write operations extends the size of the stream before the actual write operation performed.
		//! 
		//! The cursor value cannot be negative, if the new cursor position goes below 0, the seek operation fails.
		RV set_file_cursor(opaque_t file, i64 offset, SeekMode mode);

		//! Clears all buffers for this file and causes all buffered data to be written to the underlying device.
		//! @param[in] file The file handle opened by `open_file`.
		void flush_file(opaque_t file);

		//! Gets the attribute/status of one file or directory.
		//! @param[in] path The path of the file to get.
		//! @return The file attribute structure if succeeded, returns error code if failed.
		//! Possible errors:
		//! * BasicError::access_denied
		//! * BasicError::not_found
		//! * BasicError::bad_platform_call for all errors that cannot be identified.
		R<FileAttribute> get_file_attribute(const c8* path);

		//! Copies the file from the source path to the destination path.
		//! Refer to docs in `File.hpp`.
		RV	copy_file(const c8* from_path, const c8* to_path, FileCopyFlag flags);

		//! Moves the file from the source path to the destination path. This call can also be used to rename a file.
		//! Refer to docs in `File.hpp`.
		RV	move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags);

		//! Deletes the specified file or directory.
		//! Refer to docs in `File.hpp`.
		RV	delete_file(const c8* path);

		//! Creates a file iterator handle that can be used to iterate all files in the specified directory. 
		//! File placeholders for current and parent directory ("." and "..") should not be included in the iterator entry.
		//! @param[in] path The directory path to open.
		//! @return Returns a file iterator handle if succeeded. Returns one of the following error codes if failed:
		//! * BasicError::access_denied
		//! * BasicError::not_found
		//! * BasicError::bad_platform_call for all errors that cannot be identified.
		R<opaque_t> open_dir(const c8* path);

		//! Closes the file iterator handle.
		void close_dir(opaque_t dir_iter);

		//! Checks if this iterator points to a valid file item in the directory stream.
		//! @param[in] dir_iter The directory iterator handle.
		bool dir_iterator_is_valid(opaque_t dir_iter);

		//! Returns the name of the file the iterator currently points to.
		//! Returns `nullptr` if the file iterator is invalid.
		//! @param[in] dir_iter The directory iterator handle.
		const c8* dir_iterator_get_filename(opaque_t dir_iter);

		//! Returns the file attribute of the file the iterator currently points to.
		//! Returns EFileAttributeFlag::none if the file iterator is invalid.
		//! @param[in] dir_iter The directory iterator handle.
		FileAttributeFlag dir_iterator_get_attributes(opaque_t dir_iter);

		//! Moves the file iterator to the next file in the directory.
		//! @param[in] dir_iter The directory iterator handle.
		//! @return Returns `true` if the file iterator is valid after this call, 
		//! returns `false` otherwise.
		//! If the return value is `false`, then the iterator has reached the end of the 
		//! directory stream.
		bool dir_iterator_move_next(opaque_t dir_iter);

		//! Creates one directory.
		//! @param[in] path The path of the directory to create.
		//! @return Returns on of the following error codes on error:
		//! * BasicError::already_exists
		//! * BasicError::not_found
		//! * BasicError::bad_platform_call for all errors that cannot be identified.
		RV	create_dir(const c8* path);

		//! Get the current working directory path for the underlying system.
		//! The default current working directory is set to the path that contains the executable file.
		//! @param[in] buffer_length The length of the buffer for the current directory string, including the null terminator.
		//! @param[in] A pointer to the buffer that receives the current directory string. To gets the required buffer size, specify
		//! `buffer` to `nullptr` and `buffer_length` to 0.
		//! @return The number of `c8` characters copied into the buffer, including the null terminator. If `buffer_length` is 0 and
		//! `buffer` is `nullptr`, returns the required buffer size to fetch the current directory, including the null terminator. 
		u32 get_current_dir(u32 buffer_length, c8* buffer);

		//! Set the current working directory path for the underlying system. The current directory will be set for the process scope.
		//! @param[in] path The path to set. This must be a null-terminated string.
		//! @return Returns `s_ok` on success, returns one of the following error codes on failure:
		//! * BasicError::bad_platform_call
		RV set_current_dir(const c8* path);

		//! Get the application executable file's absolute directory, ended with application executable name.
		//! @return The process path. The string is static and valid until the SDK closed.
		const c8* get_process_path();

		//! Returns the number of logical processors on the platform.
		u32 get_num_processors();

		//! Reads one string from the standard input.
		//! @param[in] buffer The buffer used to accept the input stream.
		//! @param[in] size The number of `c8` characters to read from the input stream.
		//! @remark The stream shall be encoded in UTF-8 format. The input process shall be terminated when
		//! the buffer is full, or one new line (`\n`) or EOF is reached. The string stored in `buffer` shall
		//! be null-terminated, so that at most `buffer.size() - 1` characters can be accepted. If the last 
		//! input character is a multi-bytes character in UTF-8, it must either be fully written or not be written.
		//! 
		//! If the input process is terminated by one new line character, the new line character shall not be discarded
		//! and not written to the buffer.
		//! 
		//! This function must be thread-safe.
 		RV std_input(c8* buffer, usize size, usize* read_bytes);

		//! Writes one string to the standard output.
		//! @param[in] buffer The buffer that contains the string to be written.
		//! @param[in] size The size of `c8` characters to write.
		//! @remark The stream is assumed to be encoded in UTF-8 format. The output process shall be terminated when
		//! one null terminator is reached, or the buffer size limit is reached. The input string is not guaranteed to
		//! be null-terminated. If one character in the buffer is a multi-bytes character in UTF-8, it should either be fully
		//! outputted or not outputted.
		//! The the output process is terminated by one null terminator, the null terminator should not be outputted.
		//! 
		//! This function must be thread-safe.
		RV std_output(const c8* buffer, usize size, usize* write_bytes);

		//! Captures function call stack information of the current thread.
		//! @param[out] frames One buffer that receives captured frames. Every frame is represented by 
		//! one opaque handle in the buffer.
		//! @return The number of captured frames written to `frames`.
		u32 stack_backtrace(Span<opaque_t> frames);

		//! Gets symbolic names for frames returned by @ref stack_backtrace.
		//! @param[in] frames One buffer that contains frames to query.
		//! @return Returns one array of strings that store symbolic names for frames. Strings are stored in 
		//! the same order as `frames`. If the symbolic name of one frame is not found, `nullptr` will be written.
		//! @par Valid Usage
		//! 1. All frames in `frames` must be valid frames returned by @ref stack_backtrace. In particular, if the return
		//! value of @ref stack_backtrace is smaller than the size of the frame buffer passed to @ref stack_backtrace, only 
		//! valid frames, not the whole buffer, shall be specified in this call.
		const c8** stack_backtrace_symbols(Span<const opaque_t> frames);

		//! Frees symbols returned by @ref stack_backtrace_symbols.
    	//! @param[in] symbols The symbol array returned by @ref stack_backtrace_symbols.
		void free_backtrace_symbols(const c8** symbols);

		//! Loads the specified library to the process's address space. This call may load additional libraries
		//! required by the specified library.
		//! @param[in] path The path of the library file. It can be one `.dll` or `.exe` file on Windows,
		//! or one `.so` file on POSIX systems.
		//! @return Returns one handle that represents the loaded library.
		R<opaque_t> load_library(const c8* path);

		//! Unloads the specified library.
		//! @param[in] handle The library handle returned by @ref load_library.
		//! @remark The library handle is reference counted: every call to  @ref load_library for the same library
		//! file increases the reference counter, and every @ref free_library for the same library handle decreases the 
		//! reference counter. The library will be removed from the process's address space when the reference counter drop to 0.
		//! 
		//! When one library is removed from the process's address space, it will decrease reference counters for all its dependent 
		//! libraries, and removes them as well if their reference counters drop to 0.
		void free_library(opaque_t handle);

		//! Gets the function address (function pointer) of one function in the library from its symbol name.
		//! @param[in] handle The library handle returned by @ref load_library.
		//! @param[in] symbol The function's symbol name.
		//! @return The function address of the specified function.
		R<void*> get_library_function(opaque_t handle, const c8* symbol);
	}

	//! The allocator that allocates memory from OS directly.
	class OSAllocator
	{
	public:
		template <typename _Ty>
		_Ty* allocate(usize n = 1)
		{
			return (_Ty*)OS::memalloc(sizeof(_Ty) * n, alignof(_Ty));
		}
		template <typename _Ty>
		void deallocate(_Ty* ptr, usize n = 1)
		{
			OS::memfree(ptr, alignof(_Ty));
		}
		bool operator==(const OSAllocator&)
		{
			return true;
		}
		bool operator!=(const OSAllocator&)
		{
			return false;
		}
	};

	struct OSMutexGuard
	{
		opaque_t m_handle;

		OSMutexGuard(opaque_t h) :
			m_handle(h)
		{
			OS::lock_mutex(m_handle);
		}
		~OSMutexGuard()
		{
			OS::unlock_mutex(m_handle);
		}
	};
}
