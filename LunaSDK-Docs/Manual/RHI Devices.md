A graphics device (`IDevice`) is an virtual representation of one graphics subsystem on the platform. Some platforms have only one graphics subsystem, while others have more. For example:

1. Desktop and laptop PCs usually have both integrated GPUs and dedicated graphics cards, so they will have two graphics subsystem listed. But this is not always the case, for example, Apple computers with Apple Silicon processors can only have one graphics subsystem.
1. Mobile devices (iOS, Andorid) has only one graphics subsystem that represents the GPU integrated in their processor.

We use the term **adapter** (`IAdapter`) to refer one graphics subsystem on the platform. The user can choose which adapter to use for the application, and creates one `IDevice` for that adapter. To determine which adapter to use for creating devices, the user can call `get_adapters` to get a list of adapters present on the current platform. After the user have decided which adapter to use, she can call `new_device` to create a new device from that adapter.

## Fetching the main device
The **main device** is the system-preferred device that is created during module initialization, and can be fetched by calling `get_main_device`. The module stores the reference to the main device so that every time `get_main_device` is called, the same device reference will be returned. If the user needs only one device (which is true for most applications), she can call `get_main_device` to fetch the main device directly rather than creating devices manually. Calls to `get_main_device` are efficient and can be used wherever you want to fetch the main device.

## Device objects
Most objects of RHI is **device objects**, such objects are attached to one specific device and cannot be used by other devices. Device objects inherit from `IDeviceChild`, which provides `get_device` function that can fetch the owner device of the object.

