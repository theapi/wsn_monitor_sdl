
#include "render/counter.h"
#include "counter.h"
#include "font.h"

static void render(SDL_Window *window, SDL_Renderer *renderer, char *text) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 55, 55, 0};
  SDL_Rect rect;
  SDL_Texture *texture;
  int window_width, window_height;

  SDL_GetWindowSize(window, &window_width, &window_height);

  TTF_Font *font = FontGet(FONT_MAIN, 14);
  surface = TTF_RenderText_Solid(font, text, textColor);
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect.x = 10;
  rect.y = window_height - 30;
  rect.w = text_width;
  rect.h = text_height;

  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

void RenderCounter(SDL_Window *window, SDL_Renderer *renderer) {
  char countbuf[64] = {0};
  CounterUpdate(countbuf);
  render(window, renderer, countbuf);
}
