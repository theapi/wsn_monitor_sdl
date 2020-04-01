
#include "render/layout.h"
#include "font.h"

/*
- x, y: upper left corner.
- texture, rect: outputs.
*/
static void prepareFrame(SDL_Renderer *renderer, int x, int y, char *text) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 55, 55, 0};
  SDL_Rect rect;
  SDL_Texture *texture;

  TTF_Font *font = FontGet(FONT_MAIN, 14);
  surface = TTF_RenderText_Solid(font, text, textColor);
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect.x = x;
  rect.y = y;
  rect.w = text_width;
  rect.h = text_height;

  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

void RenderLayout(SDL_Window *window, SDL_Renderer *renderer) {
  int window_width, window_height;

  SDL_GetWindowSize(window, &window_width, &window_height);
  prepareFrame(renderer, window_width - 145, window_height - 30, "Proof of concept");
}

