#ifndef FONT_H_
#define FONT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL_ttf.h>

/* The font is not part of the binary, so must point to a font on the system. */
#define FONT_MAIN "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
#define FONT_HEAD "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
#define FONT_NUM 8

void FontInit();
TTF_Font * FontGet(const char * path, int ptsize);

#ifdef __cplusplus
}
#endif
#endif
