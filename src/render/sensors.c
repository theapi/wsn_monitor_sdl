
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

void SensorRenderPopulate(uint8_t raw[SENSOR_BUFFER_SIZE], uint8_t size, Sensor_t *sensor) {
      /* pointer to the first item (0 index) of the output array */
    char *ptr = &sensor->hex[0];
    for (int i = 0; i < size; i++) {
        if (i > 0 && i%16 == 0) {
          ptr += sprintf(ptr, "\n");
        } else if (i > 0 && i%8 == 0) {
          ptr += sprintf(ptr, "  ");
        }
        ptr += sprintf(ptr, "%02X ", raw[i]);

    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "MsgId: %d\n", sensor->payload.message_id);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "HEX: %s\n", sensor->hex);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "CRC: %d\n", sensor->payload.crc);
      sprintf(sensor->stats,
            "Sensor: %02X %02X %02X %02X %02X %02X    Payload type: %d    Interval: %d\nMessage id: %d   Battery: %d  ADC: %d, %d, %d, %d, %d, %d, %d",
            sensor->payload.mac[0], sensor->payload.mac[1],
            sensor->payload.mac[2], sensor->payload.mac[3],
            sensor->payload.mac[4], sensor->payload.mac[5],
            sensor->payload.payload_type, sensor->payload.delay, sensor->payload.message_id, sensor->payload.batt,
            sensor->payload.adc[0], sensor->payload.adc[1], sensor->payload.adc[2],
            sensor->payload.adc[3], sensor->payload.adc[4], sensor->payload.adc[5],
            sensor->payload.adc[6]);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Stats: %s\n", sensor->stats);
}
