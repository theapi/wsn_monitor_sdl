
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "layout.h"
#include "counter.h"
#include "udp.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

int main(int argc, char **argv) {
  SDL_Event event;
  SDL_Rect rect1, rect2, rect3;
  SDL_Renderer *renderer;
  SDL_Texture *texture1, *texture2, *texture3;
  SDL_Window *window;
  char *font_path;
  int quit;
  uint32_t currentTime = 0;
  uint32_t lastFrameTime = 0;

  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

  if (argc == 1) {
    // The font is not part of the binary, so must point to a font on the system.
    font_path = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";
  } else if (argc == 2) {
    // ./monitor /usr/share/fonts/truetype/freefont/FreeMonoOblique.ttf
    font_path = argv[1];
  } else {
    fprintf(stderr, "error: too many arguments\n");
    exit(EXIT_FAILURE);
  }

  UdpInit();

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window,
                              &renderer);
  SDL_SetWindowTitle(window, "UDP Payload Monitor");

  /* Inint TTF. */
  TTF_Init();
  TTF_Font *font_medium = TTF_OpenFont(font_path, 24);
  if (font_medium == NULL) {
    fprintf(stderr, "error: font not found\n");
    exit(EXIT_FAILURE);
  }
  TTF_Font *font_small = TTF_OpenFont(font_path, 14);
  if (font_small == NULL) {
    fprintf(stderr, "error: font not found\n");
    exit(EXIT_FAILURE);
  }

  LayoutDraw(window, renderer, font_small, &texture2, &rect2);
  CounterStart(renderer, font_medium, &texture1, &rect1);

  quit = 0;
  while (!quit) {
    while (SDL_PollEvent(&event) == 1) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      }

      if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            // Keep the bottom left text in postition.
            LayoutDraw(window, renderer, font_small, &texture2, &rect2);
            break;
        }
      }
    }

    CounterHandler(renderer, font_medium, &texture1, &rect1);
    UdpHandler(renderer, font_small, &texture3, &rect3);

    currentTime = SDL_GetTicks();
    if (currentTime > lastFrameTime + (1000 / 30)) {
      lastFrameTime = currentTime;

      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);
      /* Use textures. */
      SDL_RenderCopy(renderer, texture1, NULL, &rect1);
      SDL_RenderCopy(renderer, texture2, NULL, &rect2);
      SDL_RenderCopy(renderer, texture3, NULL, &rect3);
      SDL_RenderPresent(renderer);
    }

    // Don't hog the cpu, this isn't embedded.
    SDL_Delay(10);
  }

  /* Deinit */
  SDL_DestroyTexture(texture1);
  SDL_DestroyTexture(texture2);
  SDL_DestroyTexture(texture3);
  TTF_Quit();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
