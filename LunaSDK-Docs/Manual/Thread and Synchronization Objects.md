## Threads

```c++
#include <Luna/Runtime/Thread.hpp>
```

`new_thread` creates one system-level thread, which is represented by `IThread`. The user can wait for the thread to exit by calling `IThread::wait`, and check whether the thread is exited by calling `IThread::try_wait`. When the last reference to `IThread` is releasing, the system blocks the current thread until the thread quits.

Every thread uses a thread-local variable to record the current thread's handle, which can be retrieved by `get_current_thread`. The main thread's handle is also recorded and can be retrieved from any thread by `get_main_thread`. The user can delay the execution of the current thread by calling `sleep` or `fast_sleep`, the second function is more accurate and will not suspend the current thread if the time specified is smaller than several milliseconds.

The user can call `yield_current_thread` to yield the remain time slice of the current thread and let OS to schedule other threads. This is useful for reducing CPU cycles if the current thread is waiting for another operation to finish by hardware or another thread.

## Thread local storage (TLS)

```c++
#include <Luna/Runtime/Thread.hpp>
```

Thread local storage is a set of pointer-sized memory slots that contains unique data for every thread. This can be useful to store thread-local data and is efficient since reading such data does not require synchronization between threads.

Use `tls_alloc` to create a new thread local storage slot. The slot is allocated for every thread in the current process, including threads that are not yet created. Every thread local storage slot may accept an optional destructor function, which will be called to clean up the thread local object when one thread with one non-zero thread local value on the specified slot is exiting.

> The TLS destructor function works only for threads created by `new_thread` on Windows.

`tls_alloc` returns one `opaque_t`-typed handle, which will be used to get the pointer stored in the thread local storage by `tls_get`, and set the pointer stored in the thread local storage by `tls_set`. The stored pointer will be set to `0` by system before it is set by the user for the first time on one particular thread.

`tls_free` frees one thread local storage slot allocated by `tls_alloc`. Note that freeing one TLS slot will not call destructors registered by `tls_alloc`, so make sure to clean up such resources manually.

## Signals

```c++
#include <Luna/Runtime/Signal.hpp>
```

Signal (`ISignal`) is a synchronization object for execution synchronization between threads. Every signal has two states: triggered and untriggered. When one signal is in untriggered state, all threads that wait for the signal will be blocked until the signal is switched to triggered state. When one signal is in triggered state, all threads that wait for the signal will be resumed.

One signal can be created by `new_signal`, the signal is in untriggered state when created. One signal can be monitored by `ISignal::wait` and `ISignal::try_wait`, the second form returns `false` instead of blocking the current thread if the signal is in untriggered state. One signal can be triggered by `ISignal::trigger`, which transfers the signal to triggered state. One signal can be reset back to untriggered state manually or automatically, which is specified by `manual_reset` when creating the signal. If `manual_reset` is `true`, one `ISignal::trigger` call will resume all threads waiting for the signal, and the signal stays in triggered state until `ISignal::reset` is called; if `manual_reset` is `false`, every `ISignal::trigger` call will only resume exact one thread waiting for the signal, and the signal will be reset back to untriggered state automatically. The resuming order of threads waiting for the signal is unspecified in both modes.

## Mutex

```c++
#include <Luna/Runtime/Mutex.hpp>
```

Mutex (`IMutex`) is a synchronization object for granting exclusive access of one entity to at most one thread. Every mutex have two states: locked and unlocked. When one mutex is in unlocked state, the first thread that tries to acquire the lock succeeds and transfers the mutex to locked state. When one mutex is in locked state, all other threads that try to acquire the mutex will get blocked until the mutex is released by its owning thread and is transferred to unlocked state. The mutex lock is recursive, acquiring the lock multiple times from the same thread is allowed, but the user should release the lock the same times as she acquires the lock to finally release the lock.

One mutex can be created by `new_mutex`, the mutex is in unlocked state when created. One mutex can be locked by `IMutex::wait` and `IMutex::try_wait`, the second form returns `false` instead of blocking the current thread if failed to acquire the lock. One mutex can be unlocked by `IMutex::unlock`. The user can use `MutexGuard` helper object to lock one mutex in one function scope and release it automatically when `MutexGuard` is expired.

## Spin lock

```c++
#include <Luna/Runtime/SpinLock.hpp>
```

A spin lock (`SpinLock` or `RecursiveSpinLock`) is a light-weight version of `IMutex` with the following differences:

1. The spin lock is implemented purely in user-mode by C++, while the mutex is implemented by the underlying platform/OS and is usually implemented in kernel-mode as an OS component, which means locking and releasing one spin lock is much faster than locking and releasing one mutex, since the later is usually performed through a system call.
2. The spin lock will never suspend one thread, nor will it yield the time slice of the waiting thread. If one spin lock is already locked, the waiting thread will keep checking (busy-waiting) until it obtains the lock. In the other side, the mutex will usually suspends or yields the current thread if the mutex is already locked to let other threads use the processor. This makes the spin lock suitable for locking the resource for a very short period of time (hundreds or thousands of CPU-cycles), but not suitable if the lock will be obtained for a long time (>100us).
3. Creating one spin lock creation consumes much less memory than creating one mutex (only 4 bytes for non-recursive spin lock). Meanwhile, creating one spin lock does not need to allocate any dynamic memory, just declare and use it, which makes it suitable for embedding into other objects.

One spin lock can be acquired by `lock` and `try_lock`, and can be released by `unlock`. Recursive locking from the same thread is supported only by `RecursiveSpinLock`, not `SpinLock`. The user can use `LockGuard` helper object to acquire one spin lock in one function scope and release it automatically when `LockGuard` is expired. `LockGuard` works for both `SpinLock`  and `RecursiveSpinLock`.

## Semaphore

```c++
#include <Luna/Runtime/Semaphore.hpp>
```

Semaphore (`ISemaphore`) is a synchronization object which allows at most `max_count` number of threads to access the same resource. Every semaphore maintains one counter value between `0` and`max_count` , when the semaphore is acquired by one thread, its counter value is decreased by one; when the semaphore is released by one thread, its counter value is increased by one. If the counter value is `0` when one thread wants to acquire the semaphore, the thread will be blocked until another thread releases the semaphore to increase the counter value. The counter value of one semaphore will never go below `0`.

One semaphore can be created by `new_semaphore`. When creating the semaphore, the user can specify the initial counter value and maximum counter value of the semaphore. One semaphore can be acquired by `ISemaphore::wait` and `ISemaphore::try_wait`, the second form returns `false` instead of blocking the current thread if failed to acquire the semaphore. One semaphore can be released by `ISemaphore::release`.

## Read write lock

```c++
#include <Luna/Runtime/ReadWriteLock.hpp>
```

A read write lock (`IReadWriteLock`) is a special mutex that allows unlimited number of read locks, but only one write lock at the same time. Every read write lock have three states: unlocked, read locked and write locked. When the read write lock is in unlocked state, the user can acquire both read and write lock from the object, which transfers the object into read locked or write locked state. When the read write lock is in read locked state, only read locks can be acquired, which increases the internal read count of the lock. The read locked state will be transferred back to unlocked state when all read locks are released. When the object is in write locked state, neither read lock nor write lock can be acquired. The write locked state will be transferred back to unlocked state when the unique write lock is released.

One read write lock can be acquired by `new_read_write_lock`. The read lock of one read write lock can be acquired by `acquire_read` and `try_acquire_read`, and can be released by `release_read`. The write lock of one read write lock can be acquired by `acquire_write` and `try_acquire_write`, and can be released by `release_write`. `try_acquire_read` and `try_acquire_write` return `false` instead of blocking the current thread if failed to acquire the lock.





