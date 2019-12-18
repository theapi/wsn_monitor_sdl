#define _GNU_SOURCE /* For struct ip_mreq */

#include "udp.h"
#include "payload.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/*
Glabal as this size is too big for the stack.
see https://stackoverflow.com/questions/1847789/segmentation-fault-on-large-array-sizes
Probably time for malloc.
*/
static char udp_hex_buf[(MSGBUFSIZE * 3) + 1] = "-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --";
static char stats_buffer[64] = "Message id:   Battery:  ";
static uint8_t rx_buf[MSGBUFSIZE] = {0};

static struct sockaddr_in addr;
static int fd;

static void renderHex(SDL_Renderer *renderer, int x, int y, char *text,
                       TTF_Font *font) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 55, 0};
  SDL_Rect rect;
  SDL_Texture *texture;

  surface = TTF_RenderText_Blended(font, text, textColor);
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

static void renderStats(SDL_Renderer *renderer, int x, int y, char *text,
                       TTF_Font *font) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 255, 0};
  SDL_Rect rect;
  SDL_Texture *texture;

  surface = TTF_RenderText_Blended(font, text, textColor);
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

void UdpInit() {
  struct ip_mreq mreq;

  /* create what looks like an ordinary UDP socket */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  /* allow multiple sockets to use the same PORT number */
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    perror("Reusing ADDR failed");
    exit(1);
  }

  /* set up destination address */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
  addr.sin_port = htons(SERVER_PORT);

  /* Bind to receive address */
  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }

  /* Use setsockopt() to request that the kernel join a multicast group */
  mreq.imr_multiaddr.s_addr = inet_addr(SERVER_GROUP);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    perror("setsockopt");
    exit(1);
  }
}

void UdpListen() {
  int nbytes;
  socklen_t addrlen;
  addrlen = sizeof(addr);
  PAYLOAD_sensor_t payload = {};

  if ((nbytes = recvfrom(fd, rx_buf, MSGBUFSIZE, MSG_DONTWAIT,
                          (struct sockaddr*)&addr, &addrlen)) > 0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Received %d bytes: ", nbytes);
    /* pointer to the first item (0 index) of the output array */
    char *ptr = &udp_hex_buf[0];
    for (int i = 0; i < nbytes; i++) {
        ptr += sprintf (ptr, "%02X ", rx_buf[i]);
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s\n", udp_hex_buf);

    /* Unserialize the payload. */
    size_t len = sizeof(PAYLOAD_sensor_t);
    uint8_t sbuf[len];
    // Ignore the fist byte.
    for (int i = 0; i < len; i++) {
        sbuf[i] = rx_buf[i+1];
    }
    PAYLOAD_unserialize(&payload, sbuf);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Message id: %d\n", payload.message_id);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Batt: %d\n", payload.batt);
    sprintf(stats_buffer, "Message id: %d Battery: %d", payload.message_id, payload.batt);
  }
}

void UdpRender(SDL_Renderer *renderer, TTF_Font *font_small, TTF_Font *font_medium) {
  renderStats(renderer, 200, 10, stats_buffer, font_medium);
  renderHex(renderer, 10, 50, udp_hex_buf, font_small);
}
