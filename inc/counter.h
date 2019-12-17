#ifndef COUNTER_H_
#define COUNTER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void CounterStart(SDL_Renderer *renderer, TTF_Font *font);
void CounterHandler(SDL_Renderer *renderer, TTF_Font *font);
void CounterRenderCopy(SDL_Renderer *renderer);
void CounterQuit();

#ifdef __cplusplus
}
#endif
#endif
