#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sensor.h"

const uint8_t sensor_config_sensors[SENSOR_NUM][6] = {
  {0x5c, 0xcf, 0x7f, 0x80, 0xdf, 0xd2},
  {0x18, 0xfe, 0x34, 0xd3, 0x0d, 0x25}
};

#ifdef __cplusplus
}
#endif
#endif
