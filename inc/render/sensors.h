#ifndef RENDER_SENSORS_H_
#define RENDER_SENSORS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"
#include "udp.h"

#include <SDL2/SDL.h>

// Data for the render.
typedef struct {
  char hex[(UDP_MSGBUFSIZE * 3) + 1]; /* Large enough for the string version of the payload. */
  char stats[1024];
} SensorsRender_data_t;

void RenderSensorsInit();
void RenderSensorsPopulate(uint8_t raw[SENSOR_BUFFER_SIZE], uint8_t size, Sensor_t *sensor);
void RenderSensors(SDL_Window *window, SDL_Renderer *renderer);

#ifdef __cplusplus
}
#endif
#endif
