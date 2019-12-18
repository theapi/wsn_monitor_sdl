#ifndef UDP_H_
#define UDP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SERVER_PORT 12345
#define SERVER_GROUP "239.0.0.58"
#define MSGBUFSIZE 64

void UdpInit();
void UdpListen();
void UdpRender(SDL_Renderer *renderer, TTF_Font *font_small, TTF_Font *font_medium);

#ifdef __cplusplus
}
#endif
#endif
