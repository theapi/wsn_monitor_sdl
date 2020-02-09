
#include "udp.h"
#include "font.h"
#include "sensor.h"
#include "render/sensors.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

static void sensorIgnore(Sensor_t *sensor) {
  sensor->hex[0] = 0;
}

static void renderHex(SDL_Renderer *renderer, int x, int y, char *text, uint8_t sensor_num) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 55, 0};
  SDL_Rect rect;
  SDL_Texture *texture;

  // Move down for each sensor.
  y = y + sensor_num * 80;

  TTF_Font *font = FontGet(FONT_MAIN, 14);
  surface = TTF_RenderText_Blended_Wrapped(font, text, textColor, 780);
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect.x = x;
  rect.y = y;
  rect.w = text_width;
  rect.h = text_height;

  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

static void renderStats(SDL_Renderer *renderer, int x, int y, char *text, uint8_t sensor_num) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 255, 0};
  SDL_Rect rect;
  SDL_Texture *texture;

  // Move down for each sensor.
  y = y + sensor_num * 80;

  TTF_Font *font = FontGet(FONT_HEAD, 16);
  surface = TTF_RenderText_Blended_Wrapped(font, text, textColor, 780);
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect.x = x;
  rect.y = y;
  rect.w = text_width;
  rect.h = text_height;

  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
}

void SensorsRender(SDL_Window *window, SDL_Renderer *renderer) {
  int32_t now = SDL_GetTicks();
  for (uint8_t i = 0; i < UDP_NUM_SENSORS; i++) {
    Sensor_t *sensor = SensorGetByNumber(i);
    // If longer than the delay time clear the led for that sensor.
    if (now - sensor->last > (sensor->payload.delay + UDP_DELAY_EXTRA) * 1000) {
      sensorIgnore(sensor);
    } else {
      if (sensor->hex[0] != 0) {
        renderStats(renderer, 10, 10, sensor->stats, i);
        renderHex(renderer, 10, 50, sensor->hex, i);
      }
    }
  }
}
