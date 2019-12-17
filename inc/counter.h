#ifndef COUNTER_H_
#define COUNTER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void CounterStart(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);
void CounterHandler(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);

#ifdef __cplusplus
}
#endif
#endif
