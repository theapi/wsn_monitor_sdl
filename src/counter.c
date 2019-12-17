
#include "counter.h"

static void prepareFrame(SDL_Renderer *renderer, int x, int y, char *text,
                       TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 55, 0};

  surface = TTF_RenderText_Solid(font, text, textColor);
  *texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect->x = x;
  rect->y = y;
  rect->w = text_width;
  rect->h = text_height;
}

void CounterStart(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
  char countbuf[64] = {0};
  sprintf(countbuf, "Count: 0");
  prepareFrame(renderer, 10, 10, countbuf, font, texture, rect);
}

void CounterHandler(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
  static uint32_t count = 0;
  static uint32_t lastTime = 0;

  // Update once per second
  uint32_t currentTime = SDL_GetTicks();
  if (currentTime > lastTime + 1000) {
    lastTime = currentTime;
    char countbuf[64] = {0};
    sprintf(countbuf, "Count: %d", count++);
    prepareFrame(renderer, 10, 10, countbuf, font, texture, rect);
  }
}
