#ifndef RENDER_COUNTER_H_
#define RENDER_COUNTER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>

void RenderCounter(SDL_Window *window, SDL_Renderer *renderer);
void CounterReset();

#ifdef __cplusplus
}
#endif
#endif
