#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include "chip8core.h"

void handle_keys();


int main(int argc, char **argv)
{

  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;

  unsigned short instr;

  if (argc < 2)
  {
    printf("Too few arguments supplied\n");
    exit(0);
  }

  // load rom and initialize hardware, RAM, registers, etc.
  if (init_hdw(argv[1]))
    return 0;

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("error initializing SDL: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("Chip8 Emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISP_WIDTH*15, DISP_HEIGHT*15, SDL_WINDOW_RESIZABLE);
  if(!window)
    printf("error creating window: %s\n", SDL_GetError());

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(!renderer)
  {
    printf("error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, DISP_WIDTH, DISP_HEIGHT);
  SDL_UpdateTexture(texture, NULL, display32x64, sizeof(int)*DISP_WIDTH);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  SDL_Event event;
  int quit = 0;
  while (!quit)
  {

    SDL_PollEvent(&event);

    switch(event.type)
    {
      case SDL_QUIT:
        quit = 1;
        break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        handle_keys(event.key.keysym.sym);
        break;
    }

    instr = fetch();

    if(decode(instr))
    {
      printf("exiting...\n");
      return 0;
    }

    //update_screen
    SDL_UpdateTexture(texture, NULL, display32x64, 4*DISP_WIDTH);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000/60);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}


void handle_keys(int key)
{
  switch(key)
  {
    case SDLK_1:
      if (keys[0x1]) printf("key '1' up\n");
      else printf("key '2' down\n");
      keys[0x1] = !keys[0x1];
      break;
    case SDLK_2:
      if (keys[0x2]) printf("key '2' up\n");
      else printf("key '2' down\n");
      keys[0x2] = !keys[0x2];
      break;
    case SDLK_3:
      if (keys[0x2]) printf("key '3' up\n");
      else printf("key '3' down\n");
      keys[0x3] = !keys[0x3];
      break;
    case SDLK_4:
      if (keys[0xC]) printf("key 'C' up\n");
      else printf("key 'C' down\n");
      keys[0xC] = !keys[0xC];
      break;
    case SDLK_q:
      if (keys[0x4]) printf("key '4' up\n");
      else printf("key '4' down\n");
      keys[0x4] = !keys[0x4];
      break;
    case SDLK_w:
      if (keys[0x5]) printf("key '5' up\n");
      else printf("key '5' down\n");
      keys[0x5] = !keys[0x5];
      break;
    case SDLK_e:
      if (keys[0x6]) printf("key '6' up\n");
      else printf("key '6' down\n");
      keys[0x6] = !keys[0x6];
      break;
    case SDLK_r:
      if (keys[0xD]) printf("key 'D' up\n");
      else printf("key 'D' down\n");
      keys[0xD] = !keys[0xD];
      break;
    case SDLK_a:
      if (keys[0x7]) printf("key '7' up\n");
      else printf("key '7' down\n");
      keys[0x7] = !keys[0x7];
      break;
    case SDLK_s:
      if (keys[0x8]) printf("key '8' up\n");
      else printf("key '8' down\n");
      keys[0x8] = !keys[0x8];
      break;
    case SDLK_d:
      if (keys[0x9]) printf("key '9' up\n");
      else printf("key '9' down\n");
      keys[0x9] = !keys[0x9];
      break;
    case SDLK_f:
      if (keys[0xE]) printf("key 'E' up\n");
      else printf("key 'E' down\n");
      keys[0xE] = !keys[0xE];
      break;
    case SDLK_z:
      if (keys[0xA]) printf("key 'A' up\n");
      else printf("key 'A' down\n");
      keys[0xA] = !keys[0xA];
      break;
    case SDLK_x:
      if (keys[0x0]) printf("key '0' up\n");
      else printf("key '0' down\n");
      keys[0x0] = !keys[0x0];
      break;
    case SDLK_c:
      if (keys[0xB]) printf("key 'B' up\n");
      else printf("key 'B' down\n");
      keys[0xB] = !keys[0xB];
      break;
    case SDLK_v:
      if (keys[0xF]) printf("key 'F' up\n");
      else printf("key 'F' down\n");
      keys[0xF] = !keys[0xF];
      break;
  }
}
