
#include "counter.h"

static SDL_Rect rect;
static SDL_Texture *texture;

static void prepareFrame(SDL_Window *window, SDL_Renderer *renderer, char *text,
                       TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 55, 55, 0};

    int window_width, window_height;

  SDL_GetWindowSize(window, &window_width, &window_height);

  surface = TTF_RenderText_Solid(font, text, textColor);
  *texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect->x = 10;
  rect->y = window_height - 30;
  rect->w = text_width;
  rect->h = text_height;
}

void CounterStart(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font) {
  char countbuf[64] = {0};
  sprintf(countbuf, "Seconds: 0");
  prepareFrame(window, renderer, countbuf, font, &texture, &rect);
}

void CounterHandler(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font) {
  static uint32_t count = 0;
  static uint32_t lastTime = 0;

  // Update once per second
  uint32_t currentTime = SDL_GetTicks();
  if (currentTime > lastTime + 1000) {
    lastTime = currentTime;
    char countbuf[64] = {0};
    sprintf(countbuf, "Seconds: %d", count++);
    prepareFrame(window, renderer, countbuf, font, &texture, &rect);
  }
}

void CounterRenderCopy(SDL_Renderer *renderer) {
  SDL_RenderCopy(renderer, texture, NULL, &rect);
}

void CounterQuit() {
  SDL_DestroyTexture(texture);
}
