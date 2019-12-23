#define _GNU_SOURCE /* For struct ip_mreq */

#include "udp.h"
#include "payload.h"
#include "font.h"

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

/* The internal sensor struct. */
typedef struct {
  PAYLOAD_sensor_t payload;
  char hex[(UDP_MSGBUFSIZE * 3) + 1]; /* Large enough for the string version of the payload. */
  char stats[1024];
  int32_t last;
} Sensor_t;

static Sensor_t sensors[UDP_NUM_SENSORS] = {0};

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
    if (compareMacs(sensors[i].payload.mac, payload->mac) == 0) {
      return i;
    }
  }

  // Not seen before, so add it to the array.
  for (uint8_t i = 0; i < UDP_NUM_SENSORS; i++) {
    if (sensors[i].payload.mac[0] == 0) {
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Adding: %d - %02X %02X %02X %02X %02X %02X", i,
               payload->mac[0], payload->mac[1],
               payload->mac[2], payload->mac[3],
               payload->mac[4], payload->mac[5]);
      return i;
    }
  }

  return -1;
}

static void sensorIgnore(uint8_t num) {
  sensors[num].hex[0] = 0;
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

    int8_t num = sensorNum(&payload);
    if (num == -1) {
      // Couldn't assign an led to the sensor.
      num = 0; // Override the first one for now.
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Sensor num: %d\n", num);

    // Check for new message or just a repeat of the last
    if (sensors[num].payload.message_id == payload.message_id) {
      // Same message so nothing to do.
      return;
    }
    sensors[num].payload = payload;

    // Remember when we last heard from this sensor.
    int32_t now = SDL_GetTicks();
    sensors[num].last = now;

    /* pointer to the first item (0 index) of the output array */
    char *ptr = &sensors[num].hex[0];
    for (int i = 0; i < nbytes; i++) {
        if (i > 0 && i%16 == 0) {
          ptr += sprintf(ptr, "\n");
        } else if (i > 0 && i%8 == 0) {
          ptr += sprintf(ptr, "  ");
        }
        ptr += sprintf(ptr, "%02X ", rx_buf[i]);

    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "HEX: %s\n", sensors[num].hex);

    sprintf(sensors[num].stats,
            "Sensor: %02X %02X %02X %02X %02X %02X\nMessage id: %d   Battery: %d  ADC: %d, %d, %d, %d, %d, %d, %d",
            sensors[num].payload.mac[0], sensors[num].payload.mac[1],
            sensors[num].payload.mac[2], sensors[num].payload.mac[3],
            sensors[num].payload.mac[4], sensors[num].payload.mac[5],
            sensors[num].payload.message_id, sensors[num].payload.batt,
            sensors[num].payload.adc[0], sensors[num].payload.adc[1], sensors[num].payload.adc[2],
            sensors[num].payload.adc[3], sensors[num].payload.adc[4], sensors[num].payload.adc[5],
            sensors[num].payload.adc[6]);
    //SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Stats: %s\n", sensors[num].stats);
  }
}

void UdpRender(SDL_Window *window, SDL_Renderer *renderer) {
  int32_t now = SDL_GetTicks();
  for (uint8_t i = 0; i < UDP_NUM_SENSORS; i++) {
    // If longer than the delay time clear the led for that sensor.
    if (now - sensors[i].last > (sensors[i].last + UDP_DELAY_EXTRA)) {
      sensorIgnore(i);
    } else {
      if (sensors[i].hex[0] != 0) {
        renderStats(renderer, 10, 10, sensors[i].stats, i);
        renderHex(renderer, 10, 50, sensors[i].hex, i);
      }
    }
  }
}
