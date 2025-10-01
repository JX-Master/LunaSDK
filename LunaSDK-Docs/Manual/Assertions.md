```c++
#include <Luna/Runtime/Assertions.hpp>
```

Assertions are used to detect programming mistakes (or "bugs") when developing complex systems and modules. Unlike [[Error Handling]] that is used for run-time errors, assertions are used for errors that should never happen on correctly behaved systems and are expected to interrupt the program immediately when such error occurs, so the developer can get into the code to check the error quickly when an debugger is attached to the process. After the application is fully tested, all assertions should be disabled to increase performance.

In LunaSDK, we separate programming mistakes to internal programming mistakes and external programming mistakes, and use different macros to handle them.

> Prefer assertions to throwing error codes when possible, since it simplifies the implementation, reduces the runtime cost and makes the error obvious. Use error codes only when the error cannot be completely solved during the development period.

## Assertions for internal programming mistakes

Internal programming mistakes are mistakes caused by incorrectly implementing functions. Such mistakes should never happen if the function is correctly used by the user.

`luassert` can be used to handle such internal programming mistakes. `luassert` firstly evaluates the value of the given expression, if the evaluated value is `0`, it calls `assert_fail` to report one error message then interrupts the program. `luassert_msg` is similar to `luassert`, but it allows the user to specify the message reported by `assert_fail`.

`lupanic` is equal to `luassert(false)`, it interrupts the program immediately when being executed. The user can use `lupanic` to mark one position in the code that should never be reached in normal condition. `lupanic_msg` is similar to `lupanic`, but it allows the user to specify the message reported by `assert_fail`.

All those four assertion macros take effect only in debug version of the program or library (controlled by `LUNA_DEBUG` macro, which is configured by xmake). If you want these macros to work in profile and release builds, use macros with `_always` suffix, like `luassert_always`, `lupanic_always`, `luassert_msg_always` and `lupanic_msg_always`.

## Assertions for external programming mistakes

External programming mistakes are mistakes caused by programmers who use the function. Most functions have constraints on arguments and calling time, calling such functions with bad arguments (like out-of-range index) or at improper time (using one service before it is initialized) will result in undefined behavior that is hard to debug. Unlike internal programming mistakes, these mistakes are caused by programmers who uses the function, thus is impossible to be solved when implementing the function.

For such case, `lucheck` and `lucheck_msg` can be used to check external programming mistakes. `lucheck` and `lucheck_msg` behave the same as `luassert` and `luassert_msg`. However, these two macros are not controlled by `LUNA_DEBUG`, but another macro called `LUNA_ENABLE_CONTRACT_ASSERTION`, which can be enabled by specifying `contract_assertion` when building the module with xmake. The module developer can place `lucheck` and `lucheck_msg` at the beginning of the function implementation to validate function arguments and calling time, and interrupts the program if the function is improperly called. With these macros, the developer of the module may ship two versions of libraries to the user, one for developing with the module, and another for releasing with the final product. The development version of the module can be compiled on release mode with `contract_assertion` switched on, so all `luassert` assertions get removed, but `lucheck` assertions are retained for the user, while the release version of the module will remove both `luassert` and `lucheck` for maximum performance.