### Built-in resource functions
The frontend comes with some built-in functions to let client operate resources more conveniently.

#### get_type
Returns the type of one resource. 
**Parameters**
1. `url`: The resource to check.
**Return value**
The return value is a string of the following possible values:
2. `function`: If this is a function.
3. `data`: If this is a `Variant`.
4. `userdata`: If this is a opaque user data.
5. `null`: if this does not exist.

#### get
Returns the data of one resource if it is a `Variant`, or returns error if it is not.
**Parameters**
1. `url`: The resource to get.
**Return value**
Returns the data of the resource.

#### set
Sets the data of one resource. 
**Parameters**
1. `url`: The resource to set.
2. `data`: The data to set.
3. `overwrite`: A Boolean value specifies whether this operation can overwrite a existing resource.
The resource is a `Variant` type after set.

#### delete
Removes one resource.
**Parameters**
1. `url`: The resource to remove.

#### list
Lists all resources in the specific path.
**Parameters**
1. `url` The directory path to search for.
**Return value**
Returns one array that contains URLs of all resources queried.
## Nested requests
The Frontend module provides support for nested requests. When specifying parameters for one request object, instead of providing the parameter value directly, the client can ask the server to call another function and uses the result as parameter
of the parent function, such secondary function call are called **nested requests**, and are represented by **nested request messages**. A **nested request message** has the following properties:
1. `method`: Same as `method` in normal request messages.
2. `params`: Same as `params` in normal request messages, which can contain more nested request messages.
3. `catch`: Optional. A value to use or a nested request to invoke if the nest request fails.

When processing request messages, the Frontend automatically detects all nested requests in request parameters, and processes such nested requests firstly. The Frontend detects one object as nested request if it has `method` and `params` properties. The result value of nested requests will then replace the nested request object as parameters to the parent calls. 

Using nested request can reduce the use of temporary variables and the number of communication times between the server and the client. For example, if you need to invoke two functions, while the parameter of second function depends on the result value of the first function. In normal cases, the client should:
1. Send request message to call the first function.
2. Get response message and extract result.
3. Send request message to call the second function.
4. Get response message and extract result.

Now with nested requests, the client can:
1. Send request message to call the second function, with a nested request message that calls the first function.
2. Get response message and extract result.

This reduces two communication times and one temporary variable on client side. As the request logic begins complex, this can further save communication and memory costs.

### Error handling in nested requests
If the nested request fails and `catch` is not set, the Frontend will return the error object directly and will NOT call the parent function at all. If this behavior is not desired, the client can set `catch` property of one nested request object.

The `catch` property can be set to a value or another nested request:
1. If `catch` is set to a value, the error is suppressed and `catch` is returned to the parent as the result value.
2. If `catch` is set to a nested request, the nested request will be called, with error object set as the `error` parameter. The client can specify additional parameters in the nested request, just like normal function calls. The result of `catch` function call will be returned to the parent as the result value, if `catch` also fails, the Frontend will return the error object directly and will NOT call the parent function at all.

## Scripts
The Frontend module provides several built-in request objects to provide full functionality for executing scripts. By using scripts, the client can batch a series of function calls to be executed by the server at once. Such scripts are usually not written by human, but generated from AI agents or third-party tools, and uses a grammar similar to abstract syntax tree (AST) instead of most programming languages.

### `sequence`
The `sequence` request object executes nested functions in order.
1. `method`: must be `sequence`.
2. `params`:
	1. `body`: An array that contains all nested functions to call.
**Return value**
Returns the return value of the last executed function. If no function is executed, returns `null`.

### `if`
The `if` request object executes one of two provided nested functions or values according to the condition value.
1. `method`: must be `if`.
2. `params`:
	1. `cond`: A Boolean value to control which branch to call.
	2. `then`: Optional. A nested function to call if `cond` is `true`.
	3. `else`: Optional. A nested function to call if `cond` is `false`.
**Return value**
Returns the return value of the executed branch. If no branch is executed, returns `null`.

### `switch`
The `switch` request object executes one of the provided nested functions based on the condition value.
1. `method`: must be `switch`.
2. `params`: 
	1. `cond`: A value that can be compared with the candidate value.
	2. `body`: An array of key-value pairs that takes form of \[k1, v1, k2, v2, ...].
	3. `default`: Optional. The value to use if no key in `body` is matched.
**Return value**
Returns the return value of the executed branch. If no branch is executed, returns `null`.

### `while`
The `while` request object executes one function repeatedly until the condition is not satisfied.
1. `method`: must be `while`.
2. `params`:
	1. `cond`: A nested function that returns one Boolean value.
	2. `body`: The function to call when `cond` is `true`.


