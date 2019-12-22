#ifndef COUNTER_H_
#define COUNTER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void CounterRender(SDL_Window *window, SDL_Renderer *renderer);

#ifdef __cplusplus
}
#endif
#endif
