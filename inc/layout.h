#ifndef LAYOUT_H_
#define LAYOUT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void LayoutDraw(SDL_Window * window, SDL_Renderer *renderer, TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);

#ifdef __cplusplus
}
#endif
#endif
