Input Method Editor represents a operating system level feature that allows the user to input characters that are not directly mapped to the keyboard (like Chinese, Japanese and Korean characters). On devices without physical keyboard, this is also used to let the system display a virtual keyboard on screen so that the user can use that keyboard to input characters.

All IME functions work in a per window basis, different windows can have different IME settings.

## Enabling / Disabling IME

IME is disabled by default. Call `IWindow::begin_text_input` to enable IME, and `IWindow::end_text_input` to disable IME. On some platforms, enabling IME will bring up one virtual keyboard on screen to let the user input characters.

## Set IME Text Input Area

IME may draw input box and overlay on the area where character input happens, calls `IWindow::set_text_input_area` to notify the system about that area, so that IME overlay can be displayed correctly.