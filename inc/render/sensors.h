#ifndef RENDER_SENSORS_H_
#define RENDER_SENSORS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"

#include <SDL2/SDL.h>

void SensorRenderPopulate(uint8_t raw[SENSOR_BUFFER_SIZE], uint8_t size, Sensor_t *sensor);
void SensorsRender(SDL_Window *window, SDL_Renderer *renderer);

#ifdef __cplusplus
}
#endif
#endif
