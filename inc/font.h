#ifndef FONT_H_
#define FONT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL_ttf.h>

/* The font is not part of the binary, it is in the resources directory. */
#define FONT_MAIN "FreeMono.ttf"
#define FONT_HEAD "FreeSans.ttf"
#define FONT_NUM 8

void FontInit();
void FontQuit();
TTF_Font * FontGet(const char * path, int ptsize);

#ifdef __cplusplus
}
#endif
#endif
