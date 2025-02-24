#include "stdafx.h"
#include "sdl2.h"
#include "ImGui/imgui_impl_sdl.h"

int sdl_loop(Example_DiscreteData &application)
{
    SDL_Event event;

    bool quit = false;
    while (!quit)
    {
        SDL_PumpEvents();
        application.Run();

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    quit = true;
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    application.SetWindow(application.window);
                    break;
                default:
                    break;
                }
            default:
                break;
            }

            wi::input::sdlinput::ProcessEvent(event);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{

    Example_DiscreteData application;

#ifdef WickedEngine_SHADER_DIR
    wi::renderer::SetShaderSourcePath(WickedEngine_SHADER_DIR);
#endif

    application.infoDisplay.active = true;
    application.infoDisplay.watermark = true;
    application.infoDisplay.resolution = true;
    application.infoDisplay.fpsinfo = true;

    sdl2::sdlsystem_ptr_t system = sdl2::make_sdlsystem(SDL_INIT_EVERYTHING | SDL_INIT_EVENTS);
    sdl2::window_ptr_t window = sdl2::make_window(
        "Template",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1000, 1000,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Event event;

    application.SetWindow(window.get());

    int ret = sdl_loop(application);

    SDL_Quit();

    return ret;
}
