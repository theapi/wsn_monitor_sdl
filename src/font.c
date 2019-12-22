
#include "font.h"

//#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/* The internal font loader struct. */
typedef struct {
  uint8_t ptsize;
  TTF_Font *font;
  const char * path;
} FontLoader_t;

static FontLoader_t fonts[FONT_NUM] = {0};

static int8_t fontIndex(const char * path, int ptsize) {
  // Look for the loaded font in the index.
  for (uint8_t i = 0; i < FONT_NUM; i++) {
    // Check for same point size first.
    if (fonts[i].ptsize == ptsize) {
      // Now check it's the same font.
      if (strcmp(fonts[i].path, path) == 0) {
        return i;
      }
    }
  }
  // Add the new size to the index.
  for (uint8_t i = 0; i < FONT_NUM; i++) {
    if (fonts[i].ptsize == 0) {
      fonts[i].ptsize = ptsize;
      fonts[i].path = path;
      return i;
    }
  }

  return 0;
}

void FontInit() {
  TTF_Init();
}

TTF_Font * FontGet(const char * path, int ptsize) {
  int8_t index = fontIndex(path, ptsize);
  if (fonts[index].font != NULL) {
    return fonts[index].font;
  }
  TTF_Font *font = TTF_OpenFont(path, ptsize);
  if (font == NULL) {
    fprintf(stderr, "error: font not found\n");
    exit(EXIT_FAILURE);
  }
  //SDL_Log("Opened font size %d from i: %d", ptsize, index);
  fonts[index].font = font;
  return font;
}
