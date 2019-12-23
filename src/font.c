
#include "font.h"

//#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/* The internal font loader struct. */
typedef struct {
  uint8_t ptsize;
  TTF_Font *font;
  const char * name;
} FontLoader_t;

static FontLoader_t fonts[FONT_NUM] = {0};

static char* resource_dir = NULL;

void initializeResourcePath()
{
    const char subPath[] = "resources/";
    size_t length;
    int base_path_len;
    char *base_path = SDL_GetBasePath();

    if (resource_dir) {
        free(resource_dir);
    }

    // Ignore the "bin/" at the end
    base_path_len = strlen(base_path) - 4;
    length = base_path_len + sizeof(subPath);
    resource_dir = (char*)malloc(length);
    snprintf(resource_dir, length, "%.*s%s", base_path_len, base_path, subPath);
    free(base_path);
}

static int8_t fontIndex(const char * name, int ptsize) {
  // Look for the loaded font in the index.
  for (uint8_t i = 0; i < FONT_NUM; i++) {
    // Check for same point size first.
    if (fonts[i].ptsize == ptsize) {
      // Now check it's the same font.
      if (strcmp(fonts[i].name, name) == 0) {
        return i;
      }
    }
  }
  // Add the new size to the index.
  for (uint8_t i = 0; i < FONT_NUM; i++) {
    if (fonts[i].ptsize == 0) {
      fonts[i].ptsize = ptsize;
      fonts[i].name = name;
      return i;
    }
  }

  return 0;
}

void FontInit() {
  TTF_Init();
  initializeResourcePath();
  SDL_Log("resource_dir: %s", resource_dir);
}

TTF_Font * FontGet(const char * name, int ptsize) {
  const char subPath[] = "fonts/";
  size_t length;
  char *path = NULL;

  int8_t index = fontIndex(name, ptsize);
  if (fonts[index].font != NULL) {
    return fonts[index].font;
  }

  length = strlen(resource_dir) + sizeof(subPath) + strlen(name);
  path = (char*)malloc(length);
  snprintf(path, length, "%s%s%s", resource_dir, subPath, name);
  SDL_Log("font file: %s", path);

  TTF_Font *font = TTF_OpenFont(path, ptsize);
  free(path);
  if (font == NULL) {
    fprintf(stderr, "error: font not found\n");
    exit(EXIT_FAILURE);
  }
  //SDL_Log("Opened font size %d from i: %d", ptsize, index);
  fonts[index].font = font;
  return font;
}

void FontQuit() {
  TTF_Quit();
  free(resource_dir);
}
