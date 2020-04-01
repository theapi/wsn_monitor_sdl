
#include "counter.h"
#include <stdint.h>
#include <SDL2/SDL.h>

static uint32_t count = 0;
static uint32_t lastTime = 0;

void CounterReset() {
  count = 0;
  lastTime = 0;
}

void CounterUpdate(char buf[64]) {
  // Update once per second
  uint32_t currentTime = SDL_GetTicks();
  if (currentTime > lastTime + 1000) {
    lastTime = currentTime;
    count++;
  }
  sprintf(buf, "Seconds: %d", count);
}
