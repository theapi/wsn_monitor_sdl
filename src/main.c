
#include <stdlib.h>

#include <SDL2/SDL.h>


#include "render/sensors.h"
#include "font.h"
#include "layout.h"
#include "counter.h"
#include "udp.h"


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

int main(int argc, char **argv) {
  SDL_Event event;
  SDL_Renderer *renderer;
  SDL_Window *window;
  int quit;
  uint32_t currentTime = 0;
  uint32_t lastFrameTime = 0;

  //SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

  UdpInit();
  FontInit();
  SensorsRenderInit();

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window,
                              &renderer);
  SDL_SetWindowTitle(window, "UDP Payload Monitor");
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED);

  quit = 0;
  while (!quit) {
    while (SDL_PollEvent(&event) == 1) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      }
    }

    // Check for UDP messages as often as possible.
    UdpListen();

    currentTime = SDL_GetTicks();
    if (currentTime > lastFrameTime + (1000 / 30)) {
      lastFrameTime = currentTime;

      LayoutRender(window, renderer);
      CounterRender(window, renderer);
      SensorsRender(window, renderer);

      SDL_RenderPresent(renderer);

      /* Prepare the back buffer for the next frame. */
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);
    }

    // Don't hog the cpu, this isn't embedded.
    SDL_Delay(30);
  }

  /* Clean up */
  FontQuit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
