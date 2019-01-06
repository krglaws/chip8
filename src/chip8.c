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
    instr = fetch();

    if (decode(instr))
    {
      printf("exiting...\n");
      return 0;
    }

    //update_screen
    SDL_UpdateTexture(texture, NULL, display32x64, 4*DISP_WIDTH);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    //SDL_Delay(10);

    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
          quit = 1;
          break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          handle_keys(event.key.keysym.sym, event.type);
          break;
      }
    }
    if (dt > 0) dt--;
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}


void handle_keys(int key, int type)
{
  switch(key)
  {
    case SDLK_1:
      if (type == SDL_KEYDOWN) keys[0x1] = 1;
      else keys[0x1] = 0;
      break;
    case SDLK_2:
      if (type == SDL_KEYDOWN) keys[0x2] = 1;
      else keys[0x2] = 0;
      break;
    case SDLK_3:
      if (type == SDL_KEYDOWN) keys[0x3] = 1;
      else keys[0x3] = 0;
      break;
    case SDLK_4:
      if (type == SDL_KEYDOWN) keys[0xC] = 1;
      else keys[0xC] = 0;
      break;
    case SDLK_q:
      if (type == SDL_KEYDOWN) keys[0x4] = 1;
      else keys[0x4] = 0;
      break;
    case SDLK_w:
      if (type == SDL_KEYDOWN) keys[0x5] = 1;
      else keys[0x5] = 0;
      break;
    case SDLK_e:
      if (type == SDL_KEYDOWN) keys[0x6] = 1;
      else keys[0x6] = 0;
      break;
    case SDLK_r:
      if (type == SDL_KEYDOWN) keys[0xD] = 1;
      else keys[0xD] = 0;
      break;
    case SDLK_a:
      if (type == SDL_KEYDOWN) keys[0x7] = 1;
      else keys[0x7] = 0;
      break;
    case SDLK_s:
      if (type == SDL_KEYDOWN) keys[0x8] = 1;
      else keys[0x8] = 0;
      break;
    case SDLK_d:
      if (type == SDL_KEYDOWN) keys[0x9] = 1;
      else keys[0x9] = 0;
      break;
    case SDLK_f:
      if (type == SDL_KEYDOWN) keys[0xE] = 1;
      else keys[0xE] = 0;
      break;
    case SDLK_z:
      if (type == SDL_KEYDOWN) keys[0xA] = 1;
      else keys[0xA] = 0;
      break;
    case SDLK_x:
      if (type == SDL_KEYDOWN) keys[0x0] = 1;
      else keys[0x0] = 0;
      break;
    case SDLK_c:
      if (type == SDL_KEYDOWN) keys[0xB] = 1;
      else keys[0xB] = 0;
      break;
    case SDLK_v:
      if (type == SDL_KEYDOWN) keys[0xF] = 1;
      else keys[0xF] = 0;
      break;
  }
  return;
}
