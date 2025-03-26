/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainSDL.inl
* @author JXMaster
* @date 2025/3/21
*/
#pragma once
#define SDL_MAIN_USE_CALLBACKS 1
#include "EventHandling.hpp"
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    Luna::Window::AppStatus res = Luna::app_init(appstate, argc, argv);
    switch(res)
    {
        case Luna::Window::AppStatus::running: return SDL_APP_CONTINUE;
        case Luna::Window::AppStatus::exiting: return SDL_APP_SUCCESS;
        case Luna::Window::AppStatus::failing: return SDL_APP_FAILURE;
        default: lupanic();
    }
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    Luna::Window::handle_sdl_event(*event);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Luna::Window::AppStatus res = Luna::app_update(appstate);
    switch(res)
    {
        case Luna::Window::AppStatus::running: return SDL_APP_CONTINUE;
        case Luna::Window::AppStatus::exiting: return SDL_APP_SUCCESS;
        case Luna::Window::AppStatus::failing: return SDL_APP_FAILURE;
        default: lupanic();
    }
    return SDL_APP_FAILURE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    Luna::Window::AppStatus res;
    switch(result)
    {
        case SDL_APP_CONTINUE: res = Luna::Window::AppStatus::running; break;
        case SDL_APP_SUCCESS: res = Luna::Window::AppStatus::exiting; break;
        case SDL_APP_FAILURE: res = Luna::Window::AppStatus::failing; break;
        default: lupanic(); res = Luna::Window::AppStatus::failing; break;
    }
    Luna::app_close(appstate, res);
}