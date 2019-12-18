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
static char udp_hex_buf[UDP_NUM_SENSORS][(UDP_MSGBUFSIZE * 3) + 1] = {"-- ", "-- ", "-- "};
static char stats_buffer[UDP_NUM_SENSORS][64] = {"Message id:   Battery:  ", " - ", " - "};
static uint8_t rx_buf[UDP_MSGBUFSIZE] = {0};

static struct sockaddr_in addr;
static int fd;

// Identified by their mac address: 5ccf7f80dfd2
static uint8_t sensors[UDP_NUM_SENSORS][6] = {};
static int32_t sensors_last[UDP_NUM_SENSORS] = {};
static int32_t sensors_delay[UDP_NUM_SENSORS] = {};

/**
 * Compares mac addresses.
 *
 * returns zero if they are “equal”
 * see https://www.gnu.org/software/libc/manual/html_node/Comparison-Functions.html
 *
 */
static int8_t compareMacs(uint8_t a[6], uint8_t b[6]) {
  for (int i = 0; i < 6; i++) {
    if (a[i] != b[i]) {
      return 1;
    }
  }
  return 0;
}

static int8_t sensorNum(PAYLOAD_sensor_t *payload) {
  for (uint8_t i = 0; i < UDP_NUM_SENSORS; i++) {
    if (compareMacs(sensors[i], payload->mac) == 0) {
      // Update the delay in case it changed.
      sensors_delay[i] = payload->delay;
      return i;
    }
  }

  // Not seen before, so add it to the array.
  for (uint8_t i = 0; i < UDP_NUM_SENSORS; i++) {
    if (sensors[i][0] == 0) {
      for (uint8_t x = 0; x < 6; x++) {
        sensors[i][x] = payload->mac[x];
      }
      sensors_delay[i] = payload->delay;
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Added: %d - %02X %02X %02X %02X %02X %02X", i,
               sensors[i][0], sensors[i][1], sensors[i][2], sensors[i][3],
               sensors[i][4], sensors[i][5]);
      return i;
    }
  }

  return -1;
}

static void sensorClear(uint8_t num) {

}

static void renderHex(SDL_Renderer *renderer, int x, int y, char *text, uint8_t sensor,
                       TTF_Font *font) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 55, 0};
  SDL_Rect rect;
  SDL_Texture *texture;

  // Move down for each sensor.
  y = y + sensor * 60;

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

static void renderStats(SDL_Renderer *renderer, int x, int y, char *text, uint8_t sensor,
                       TTF_Font *font) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 255, 0};
  SDL_Rect rect;
  SDL_Texture *texture;

  // Move down for each sensor.
  y = y + sensor * 60;

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
  PAYLOAD_sensor_t payload = {};

  if ((nbytes = recvfrom(fd, rx_buf, UDP_MSGBUFSIZE, MSG_DONTWAIT,
                          (struct sockaddr*)&addr, &addrlen)) > 0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Received %d bytes: ", nbytes);

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

    int8_t sensor = sensorNum(&payload);
    if (sensor == -1) {
      // Couldn't assign an led to the sensor.
      sensor = 0; // Override the first one for now.
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Sensor: %d\n", sensor);

    // Remember when we last heard from this sensor.
    int32_t now = SDL_GetTicks();
    sensors_last[sensor] = now;

    /* pointer to the first item (0 index) of the output array */
    char *ptr = &udp_hex_buf[sensor][0];
    for (int i = 0; i < nbytes; i++) {
        ptr += sprintf (ptr, "%02X ", rx_buf[i]);
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s\n", udp_hex_buf[sensor]);

    sprintf(stats_buffer[sensor], "Message id: %d Battery: %d", payload.message_id, payload.batt);
  }
}

void UdpRender(SDL_Renderer *renderer, TTF_Font *font_small, TTF_Font *font_medium) {
  int32_t now = SDL_GetTicks();
  for (uint8_t i = 0; i < UDP_NUM_SENSORS; i++) {
    // If longer than the delay time clear the led for that sensor.
    if (now - sensors_last[i] > (sensors_delay[i] + UDP_DELAY_EXTRA) * 1000) {
      sensorClear(i);
    } else {
      renderStats(renderer, 200, 10, stats_buffer[i], i, font_medium);
      renderHex(renderer, 10, 50, udp_hex_buf[i], i, font_small);
    }
  }
}
