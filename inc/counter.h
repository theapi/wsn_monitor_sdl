#ifndef COUNTER_H_
#define COUNTER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void CounterPrepare(SDL_Renderer *renderer, int x, int y, char *text,
                    TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);

#ifdef __cplusplus
}
#endif
#endif /* DISPLAY_H_ */
