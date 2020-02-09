#define _GNU_SOURCE /* For struct ip_mreq */

#include "udp.h"
#include "payload.h"
#include "font.h"
#include "sensor.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


static uint8_t rx_buf[UDP_MSGBUFSIZE] = {0};

static struct sockaddr_in addr;
static int fd;

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
  addr.sin_port = htons(UDP_SERVER_PORT);

  /* Bind to receive address */
  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }

  /* Use setsockopt() to request that the kernel join a multicast group */
  mreq.imr_multiaddr.s_addr = inet_addr(UDP_SERVER_GROUP);
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

  if ((nbytes = recvfrom(fd, rx_buf, UDP_MSGBUFSIZE, MSG_DONTWAIT,
                          (struct sockaddr*)&addr, &addrlen)) > 0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Received %d bytes", nbytes);

    Sensor_t *sensor = SensorPopulate((uint8_t*) rx_buf, nbytes, SDL_GetTicks());

    /* pointer to the first item (0 index) of the output array */
    char *ptr = &sensor->hex[0];
    for (int i = 0; i < nbytes; i++) {
        if (i > 0 && i%16 == 0) {
          ptr += sprintf(ptr, "\n");
        } else if (i > 0 && i%8 == 0) {
          ptr += sprintf(ptr, "  ");
        }
        ptr += sprintf(ptr, "%02X ", rx_buf[i]);

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
}


void UdpRender(SDL_Window *window, SDL_Renderer *renderer) {
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
