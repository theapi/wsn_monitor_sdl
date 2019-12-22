#ifndef UDP_H_
#define UDP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define UDP_SERVER_PORT 12345
#define UDP_SERVER_GROUP "239.0.0.58"
#define UDP_MSGBUFSIZE 64
#define UDP_NUM_SENSORS 3
#define UDP_DELAY_EXTRA 5 // Number of seconds extra to wait to hear from the sensor again.

void UdpInit();
void UdpListen();
void UdpRender(SDL_Window *window, SDL_Renderer *renderer);

#ifdef __cplusplus
}
#endif
#endif
