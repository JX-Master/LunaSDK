## Threads

```c++
#include <Luna/Runtime/Thread.hpp>
```

`new_thread` creates one system-level thread, which is represented by `IThread`. The user can wait for the thread to exit by calling `IThread::wait`, and check whether the thread is exited by calling `IThread::try_wait`. When the last reference to `IThread` is releasing, the system blocks the current thread until the thread quits.

Runtime records an `IThread` handle for the thread that initializes LunaSDK and for threads created by `new_thread`. `get_current_thread` returns that handle, or `nullptr` on a thread that Runtime does not track, such as a thread created directly by a third-party library. The main thread's handle is recorded and can be retrieved from any thread by `get_main_thread`. The user can delay the execution of the current thread by calling `sleep` or `fast_sleep`; the latter is more accurate and will not suspend the current thread if the requested time is smaller than several milliseconds.

The user can call `yield_current_thread` to yield the remain time slice of the current thread and let OS to schedule other threads. This is useful for reducing CPU cycles if the current thread is waiting for another operation to finish by hardware or another thread.

### Main Thread

LunaSDK recognizes main thread as the thread that initializes LunaSDK, which **does not need to be** the real main thread of the process. On some platforms like Android, the system requires the main thread to respond quickly to system events, or the application will be terminated by the system, so most developers only use main thread to forward system events, and use a separate thread for running application logic. In such case the main thread fetched from `get_main_thread` will be the thread that runs the application logic, not the real main thread of the process.

## Thread local storage (TLS)

```c++
#include <Luna/Runtime/Thread.hpp>
```

Thread local storage is a set of pointer-sized memory slots that contains unique data for every thread. This can be useful to store thread-local data and is efficient since reading such data does not require synchronization between threads.

Use `tls_alloc` to create a new thread local storage slot. The slot is available to every thread in the current process, including threads created later. The function returns `R<opaque_t>`, so allocation failure must be handled before the returned handle is passed to `tls_get`, `tls_set`, or `tls_free`.

`tls_set` stores one pointer-sized value for the calling thread, and `tls_get` retrieves the value for that same thread. A slot initially contains `nullptr` on every thread.

TLS slots do not accept destructor callbacks. Code that stores owned resources in a slot must release each thread's resource explicitly before that thread exits and before the slot is freed. `tls_free` discards the slot and does not release any pointer stored in it.

## Fibers and stackful coroutines

```c++
#include <Luna/Runtime/Fiber.hpp>
#include <Luna/Runtime/Coroutine.hpp>
```

A fiber is an execution context with its own stack that runs cooperatively on a thread. The current fiber implementation is safe only on Runtime-tracked threads: the thread that initializes Runtime and threads created by `new_thread`. Check that `get_current_thread()` is non-null before using the fiber or coroutine API; calling `convert_thread_to_fiber` from an unrelated third-party thread currently dereferences the missing Runtime thread record. On a tracked thread, `convert_thread_to_fiber` creates the calling thread's fiber context and returns the same context on repeated calls. After conversion, `new_fiber` creates another context and `switch_to_fiber` switches directly between contexts on that thread. `get_current_fiber` returns the active context, or `nullptr` if that tracked thread has not been converted. Call `convert_fiber_to_thread` only after execution has returned to the thread's original fiber context.

Direct fiber switching gives the caller complete control over transfers, but also makes it easy to bypass normal C++ scope exit. Prefer the stackful coroutine API when the work has a parent-child calling structure. Create a coroutine with `new_coroutine`, call `resume_coroutine` from a thread that has first been converted to a fiber, and call `yield_coroutine` inside the coroutine to return to its parent. `get_current_coroutine` returns `nullptr` outside a coroutine. Fiber and coroutine switching is cooperative: it does not create a new system thread or make work run in parallel.

Fiber local storage (FLS) follows the active fiber rather than only the current thread. Allocate a slot with `fls_alloc`, then access it with `fls_set` and `fls_get`. Unlike TLS, an FLS slot accepts a destructor callback for non-null values. The exact release point is platform-dependent: notably, the current POSIX backend does not invoke the callback immediately when `convert_fiber_to_thread` returns, so code must not use that call as a portable destruction boundary. Release timing-sensitive resources explicitly. Call `fls_free` only after all users of the slot have stopped accessing it.

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

1. A spin lock is implemented with user-mode atomic operations. `IMutex` uses the platform mutex primitive, which may also have an uncontended user-mode fast path and normally enters the operating system only when it must block. A spin lock can avoid sleeping and context-switch overhead under very short contention, but it is not unconditionally faster.
2. A waiting spin lock keeps checking in a busy loop and does not suspend the thread. A contended platform mutex can block the caller so another thread can use the processor. Use a spin lock only when the critical section is predictably short; use a mutex when the wait may be long, when the owner may be descheduled, or when CPU time should not be consumed by busy-waiting.
3. `SpinLock` and `RecursiveSpinLock` are value types that need no dynamic allocation, so they can be embedded directly in another object. `IMutex` is a reference-counted platform object created with `new_mutex`.

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

