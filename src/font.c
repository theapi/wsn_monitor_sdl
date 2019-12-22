
#include "font.h"

#include <SDL2/SDL_ttf.h>

/* The internal font loader struct. */
typedef struct {
  uint8_t ptsize;
  TTF_Font *font;
} FontLoader_t;

static FontLoader_t font_main[FONT_MAIN_NUM_SIZES] = {0};

static int8_t fontIndex(int ptsize) {
  // Look for the font size in the index.
  for (uint8_t i = 0; i < FONT_MAIN_NUM_SIZES; i++) {
    if (font_main[i].ptsize == ptsize) {
      return i;
    }
  }
  // Add the new size to the index.
  for (uint8_t i = 0; i < FONT_MAIN_NUM_SIZES; i++) {
    if (font_main[i].ptsize == 0) {
      font_main[i].ptsize = ptsize;
      return i;
    }
  }

  return 0;
}

void FontInit() {
  TTF_Init();
}

TTF_Font * FontMain(int ptsize) {
  int8_t index = fontIndex(ptsize);
  if (font_main[index].font != NULL) {
    return font_main[index].font;
  }
  TTF_Font *font = TTF_OpenFont(FONT_MAIN, ptsize);
  if (font == NULL) {
    fprintf(stderr, "error: font not found\n");
    exit(EXIT_FAILURE);
  }

  font_main[index].font = font;
  return font;
}
