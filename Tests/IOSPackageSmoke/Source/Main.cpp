#include <Luna/Runtime/Module.hpp>
#include <Luna/Window/AppMain.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/Window/Window.hpp>

int luna_main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;
    if(Luna::failed(Luna::init()))
    {
        return 1;
    }
    Luna::RV result = Luna::add_modules({ Luna::module_window() });
    if(Luna::failed(result))
    {
        Luna::close();
        return 2;
    }
    result = Luna::init_modules();
    if(Luna::failed(result))
    {
        Luna::close();
        return 3;
    }
    Luna::Window::IWindow* window = Luna::Window::get_system_window();
    if(!window)
    {
        Luna::close();
        return 4;
    }
    while(!window->is_closed())
    {
        Luna::Window::poll_events();
        Luna::sleep(16);
    }
    Luna::close();
    return 0;
}
