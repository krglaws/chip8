#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include "chip8core.h"

/*
int update_display(SDL_Texture* texture, unsigned int screen, unsigned char display){
  for (int i = 0; i < HEIGHT; i++)
    for (int j = 0; j < WIDTH; j++)
      if (display[i][j])
        screen
}
*/

int main(int argc, char **argv){
  //unsigned int screen [DISP_WIDTH * DISP_HEIGHT];
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;

  //unsigned short instr;

  if (argc < 2){
    printf("Too few arguments supplied\n");
    exit(0);
  }

  // load rom and initialize hardware, RAM, registers, etc.
  if (init_hdw(argv[1]))
    return 0;

  if (SDL_Init(SDL_INIT_VIDEO) != 0){
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
  while (1)
  {

    SDL_PollEvent(&event);

    if (event.type == SDL_QUIT)
      break;

    //instr = fetch();

    /*if(decode(instr)){
      printf("exiting...");
      return 0;
    }*/
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
