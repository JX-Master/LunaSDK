The Frontend module provides a stream-based API for LunaSDK appliactions, this is used to implement multiple functions in LunaSDK, including:
1. Remote Procedual Call used for web applications and multi-process applications.
2. Model Context Protocol used for LLM agents.
3. Command-based user actions used in GUI applications (that supports undo/redo).

The main design purpose of Frontend module is to transfer messages, events, function calls and function call results across multiple domains, so that different application parts can call functions of each other, even if they are written in different programming language, or running on different hosts. When using Frontend, we typically have one part of the application (the **server**) that provides the function, and another part of the application (the **client**) that uses the function. Frontend works like this:
1. Server registers functions it provides to the Frontend.
2. Client wants to call one function of the server, it encodes the function call to one **message**, and sends it to the server.
3. The Frontend module of the server decodes the message, if it accepts the message, it calls the corresponding callback function of the server.
4. The server performs the function call, and returns the result to Frontend.
5. Frontend encodes the function result into another **message**, and sends it back to the client.
6. The client decodes the function result.
Note that in some cases, both side of the communication can send and handle function calls. The server and client role is only meaningful for one specific call: the client is the side that sends the **request message** (call parameters), while the server is the side that sends the **response message** (call result).

## Message
The format of **messages** of the Frontend is similar to JSON-RPC 2.0 format, it is represented by one `Variant` object, and can be easily encoded into text stream (like JSON) or binary stream (like BSON). If the client and the server are in the same process, the client can pass `Variant` object directly to the server without any serialization/deserialization. In Frontend, we have two types of messages: request message and response message, the former carries the function call information and the later carries the function call result information.

### Request message
The request message is sent from client to server. The request message contains the following attributes:
1. `method`: Specify the function to call. The function is represented by a URL, see [[Frontend#Resource|Resource]] for details. 
2. `params`: A `Variant` object that contains all parameters of the function call. The handler of the method is responsible for interpreting this object. The type of this `Variant` must be `object`, with keys identifying the name of every parameter. This can be `null` if the message does not have any parameter.

### Response message
The response message is sent from server to client. The response message contains the following attributes:
1. `result`: If the function call is successful, this is return value of the function call. The type of this `Variant` is defined by the callee function. If the function call is failed, this must not exist.
2. `error`: If the function call is failed, this is the error information of the function call. The error information is represented by one error object described below. If the function call is successful, this must not exist.

Every request message will only have one response message.

### Error object
The error object is a `object` typed `Variant` with the following attributes:
1. `category`: One string that identifies the category of the error, for example `BasicError`.
2. `code`: One string that identifies the name of the error, for example `bad_platform_call`.
3. `message`: Optional message that describes the error.
4. `data`: Optional additional information of the error.

### Message batch
The `Variant` sent to the Frontend can be an `object` or an `array`. If the `Variant` is an `object`, it represents one single message described as above. If the `Variant` is an `array`, it will be a sequence of messages that being sent in the array order. One message sequence can contain both request messages and response messages. Batching multiple messages into one array and send them at once can help improve performance and reduce latency in some cases.

## Resource
The client **cannot** access files, objects, memory and data in server directly, instead,  the server must expose such entities as **resources**, so that the client can refer them in function calls. On client side, one resource is simply one URL (uniform resource locator) string that identifies the resource, the URL string is passed to the server along the function call, and it is the server's responsibility to find the resource from the URL. The URL can be any non-empty string, but as a convention, we use absolute path separated by slashes (`/Path/To/Resouce`) as URL format.

### Resource Registry
The Frontend provides a **resource registry**, which can be used to store URL-to-resource map for the server. All functions exposed to Frontend must be registered to this resource registry, so that the Frontend can dispatch function calls correctly. The resource registered in registry can be one of the following types:
1. Function: A function that can be called from client.
2. `Variant`: An arbitrary data represented by a `Variant` node.
3. Userdata: A memory block of any size. The memory block is managed by the resource registry, once it is freed, a special destructor callback can be called to clean up the object in the memory block.
4. Null: A special type that infers one resource does not exist.

Resource registry does not have any reference tracking mechanism, it behaves like a file on the file system: once the user removes one resource, it is gone, and all references to that resource is invalidated.

