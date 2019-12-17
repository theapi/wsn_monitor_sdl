
#define _GNU_SOURCE /* For struct ip_mreq */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "counter.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

#define SERVER_PORT 12345
#define SERVER_GROUP "239.0.0.58"
#define MSGBUFSIZE 64

/*
Glabal as this size is too big for the stack.
see https://stackoverflow.com/questions/1847789/segmentation-fault-on-large-array-sizes
Probably time for malloc.
*/
char udp_hex_buf[(MSGBUFSIZE * 3) + 1] = {0};

uint8_t rx_buf[MSGBUFSIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
char countbuf[MSGBUFSIZE] = {0};

/*
- x, y: upper left corner.
- texture, rect: outputs.
*/
void get_text_and_rect(SDL_Renderer *renderer, int x, int y, char *text,
                       TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 55, 55, 0};

  surface = TTF_RenderText_Solid(font, text, textColor);
  *texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect->x = x;
  rect->y = y;
  rect->w = text_width;
  rect->h = text_height;
}

// void DisplayPrepareCounter(SDL_Renderer *renderer, int x, int y, char *text,
//                        TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
//   int text_width;
//   int text_height;
//   SDL_Surface *surface;
//   SDL_Color textColor = {55, 255, 55, 0};

//   surface = TTF_RenderText_Solid(font, text, textColor);
//   *texture = SDL_CreateTextureFromSurface(renderer, surface);
//   text_width = surface->w;
//   text_height = surface->h;
//   SDL_FreeSurface(surface);
//   rect->x = x;
//   rect->y = y;
//   rect->w = text_width;
//   rect->h = text_height;
// }

void DisplayPrepareUdpBuffer(SDL_Renderer *renderer, int x, int y, char *text,
                       TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
  int text_width;
  int text_height;
  SDL_Surface *surface;
  SDL_Color textColor = {55, 255, 55, 0};

  surface = TTF_RenderText_Solid(font, text, textColor);
  *texture = SDL_CreateTextureFromSurface(renderer, surface);
  text_width = surface->w;
  text_height = surface->h;
  SDL_FreeSurface(surface);
  rect->x = x;
  rect->y = y;
  rect->w = text_width;
  rect->h = text_height;
}

void printArray(const uint8_t *string, int size) {
  while (size) {
    printf("%02X ", *string++);
    size--;
  }
  //printf("\n");
}



int main(int argc, char **argv) {
  SDL_Event event;
  SDL_Rect rect1, rect2, rect3;
  SDL_Renderer *renderer;
  SDL_Texture *texture1, *texture2, *texture3;
  SDL_Window *window;
  char *font_path;
  int quit;
  int window_width, window_height;

  uint32_t count = 0;
  uint32_t lastTime = 0;
  uint32_t currentTime = 0;
  uint32_t lastFrameTime = 0;

  struct sockaddr_in addr;
  int fd, nbytes;
  socklen_t addrlen;
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

  if (argc == 1) {
    font_path = "FreeMono.ttf";
  } else if (argc == 2) {
    // ./text /usr/share/fonts/truetype/freefont/FreeMonoOblique.ttf
    font_path = argv[1];
  } else {
    fprintf(stderr, "error: too many arguments\n");
    exit(EXIT_FAILURE);
  }


  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window,
                              &renderer);
  SDL_SetWindowTitle(window, "UDP Payload Monitor");

  /* Inint TTF. */
  TTF_Init();
  TTF_Font *font_medium = TTF_OpenFont(font_path, 24);
  if (font_medium == NULL) {
    fprintf(stderr, "error: font not found\n");
    exit(EXIT_FAILURE);
  }
  TTF_Font *font_small = TTF_OpenFont(font_path, 14);
  if (font_small == NULL) {
    fprintf(stderr, "error: font not found\n");
    exit(EXIT_FAILURE);
  }

  sprintf(countbuf, "Count: %d", count++);
  CounterPrepare(renderer, 10, 10, countbuf, font_medium, &texture1, &rect1);

  SDL_GetWindowSize(window, &window_width, &window_height);
  get_text_and_rect(renderer, window_width - 145, window_height - 30,
                    "Proof of concept", font_small, &texture2, &rect2);

  quit = 0;
  while (!quit) {
    while (SDL_PollEvent(&event) == 1) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      }

      if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            // Keep the bottom left text in postition.
            get_text_and_rect(renderer, event.window.data1 - 145, event.window.data2 - 30,
                              "Proof of concept", font_small, &texture2, &rect2);
            break;
        }
      }
    }

    addrlen = sizeof(addr);
    if ((nbytes = recvfrom(fd, rx_buf, MSGBUFSIZE, MSG_DONTWAIT,
                           (struct sockaddr*)&addr, &addrlen)) > 0) {
      printf("Received %d bytes: ", nbytes);
      printArray(rx_buf, nbytes);
      printf("\n");

      /* pointer to the first item (0 index) of the output array */
      char *ptr = &udp_hex_buf[0];
      for (int i = 0; i < nbytes; i++) {
          ptr += sprintf (ptr, "%02X ", rx_buf[i]);
      }
      // printf ("%s\n", udp_hex_buf);
      DisplayPrepareUdpBuffer(renderer, 10, 50, udp_hex_buf, font_small, &texture3, &rect3);
    }

    // Update once per second
    currentTime = SDL_GetTicks();
    if (currentTime > lastTime + 1000) {
      lastTime = currentTime;
      sprintf(countbuf, "Count: %d", count++);
      CounterPrepare(renderer, 10, 10, countbuf, font_medium, &texture1, &rect1);
    }

    if (currentTime > lastFrameTime + (1000 / 30)) {
      lastFrameTime = currentTime;


      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);
      /* Use textures. */
      SDL_RenderCopy(renderer, texture1, NULL, &rect1);
      SDL_RenderCopy(renderer, texture2, NULL, &rect2);
      SDL_RenderCopy(renderer, texture3, NULL, &rect3);
      SDL_RenderPresent(renderer);
    }

    // Don't hog the cpu, this isn't embedded.
    SDL_Delay(20);
  }

  /* Deinit TTF. */
  SDL_DestroyTexture(texture1);
  SDL_DestroyTexture(texture2);
  TTF_Quit();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
