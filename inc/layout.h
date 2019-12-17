#ifndef LAYOUT_H_
#define LAYOUT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void LayoutRender(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font);
void LayoutRenderCopy(SDL_Renderer *renderer);
void LayoutQuit();

#ifdef __cplusplus
}
#endif
#endif
