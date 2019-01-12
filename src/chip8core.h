#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define ROM_START 0x200
#define MEM_LEN 0x1000
#define MAX_STACK 0x10
#define DISP_HEIGHT 0x20
#define DISP_WIDTH 0x40
#define FONT_START 0x0


int init_hdw(char*);
int cls();
int load_rom(char*);
int draw(unsigned short);
unsigned short fetch();
int decode(unsigned short);


// RAM
unsigned char mem[MEM_LEN];

// 8 bit registers
unsigned char V[0x10];

// pointer register
unsigned short I;

// delay timer
unsigned char dt;

// sound timer
unsigned char st;

// program counter
unsigned short pc;

// stack pointer
unsigned short sp;

// display grid 1
unsigned int display32x64[DISP_HEIGHT * DISP_WIDTH];

// screen update flag
char update_flag = 0;

// keyboard
unsigned char keys[0x10];

// font set
unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


int init_hdw(char * rom_file){

  // clear memory
  memset(mem, 0, MEM_LEN);

  // load fontset
  for (int i = FONT_START; i < FONT_START+80; i++)
    mem[i] = chip8_fontset[i-FONT_START];

  // clear registers
  memset(V, 0, 0x10);

  // load rom
  if(load_rom(rom_file))
    return 1;

  // clear special registers
  I  = 0;
  dt = 0;
  st = 0;
  pc = ROM_START;
  sp = MEM_LEN - 1;

  // clear screen
  cls();

  // clear keys
  memset(keys, 0, 0x10);

  return 0;
}


// clear display
int cls(){
  memset(display32x64, 0, DISP_WIDTH * DISP_HEIGHT * sizeof(int));
  update_flag = 1;
  return 0;
}


int load_rom(char * rom){

  FILE *fileptr;
  long filelen;

  fileptr = fopen(rom, "r");
  if (!fileptr){
    printf("ROM not found: %s\n", rom);
    return 1;
  }

  fseek(fileptr, 0, SEEK_END);
  filelen = ftell(fileptr);

  if (filelen + ROM_START > MEM_LEN){
    printf("ROM file size too large\n");
    return 1;
  }

  rewind(fileptr);
  for (int i = 0; i < filelen; i++)
    mem[ROM_START + i] = fgetc(fileptr);

  return 0;
}


unsigned short fetch(){

  if (pc > MEM_LEN - 1){
    printf("PC access out of bounds: %d\n", pc);
    exit(1);
  }

  unsigned short big_end;
  unsigned short little_end;
  unsigned short instr;

  big_end = mem[pc++];
  little_end = mem[pc++];
  instr = (big_end << 8) + little_end;

  return instr;
}


int jmp(unsigned short instr){

  unsigned short addr;

  addr = instr & 0x0FFF;

  pc = addr;

  return 0;
}


int push(unsigned short addr){

  if (sp < MEM_LEN - MAX_STACK){
    printf("stack overflow\n");
    return 1;
  }

  unsigned char lit = addr & 0x00FF;
  unsigned char big = (addr >> 8) & 0x00FF;

  mem[sp--] = lit; // little end
  mem[sp--] = big; // big end

  return 0;
}


int call(unsigned short instr){

  push(pc);

  jmp(instr);

  return 0;
}


int ret(){

  if (sp == MEM_LEN - 1)
    return 1;

  // pop
  unsigned char big = mem[++sp]; // big end
  unsigned char lit = mem[++sp]; // little end
  pc = (big << 8) + lit;

  return 0;
}


int draw(unsigned short opcode){

  unsigned char x = V[(opcode & 0x0F00) >> 8];
  unsigned char y = V[(opcode & 0x00F0) >> 4];
  unsigned char rows = opcode & 0x000F;
  unsigned char sprite;
  unsigned char pxl;
  int addr;
  update_flag = 1;
  V[0xF] = 0;

  // for each row
  for (int i = 0; i < rows; i++){
    sprite = mem[I + i];
    // for each bit
    for (char j = 0; j < 8; j++){
      addr = (((y+i) % DISP_HEIGHT) * DISP_WIDTH) + ((x+j) % DISP_WIDTH);
      pxl = (sprite >> (8 - (j + 1))) & 1;
      if (pxl){
        if (display32x64[addr]){
          V[0xF] = 1;
          display32x64[addr] = 0;
        }
        else display32x64[addr] ^= 0xFFFFFFFF;
      }
    }
  }
  return 0;
}


int decode(unsigned short instr){

  unsigned char nibble0 = (instr & 0xF000) >> 12;
  unsigned char nibble1 = (instr & 0x0F00) >> 8;
  unsigned char nibble2 = (instr & 0x00F0) >> 4;
  unsigned char nibble3 = (instr & 0x000F);
  unsigned char byte1   = (instr & 0x00FF);

  switch(nibble0){
    case 0:
      switch(instr){
        case 0x00E0:
          return cls();

        case 0x00EE:
          return ret();

        default:
          return 0;
      }

    case 1:
      return jmp(instr);

    case 2:
      return call(instr);

    case 3:
      if (V[nibble1] == byte1) pc += 2;
      return 0;

    case 4:
      if (V[nibble1] != byte1) pc += 2;
      return 0;

    case 5:
      if (nibble3 == 0)
      {
        if (V[nibble1] == V[nibble2]) pc += 2;
        return 0;
      }
      else return 1;

    case 6:
      V[nibble1] = byte1;
      return 0;

    case 7:
      V[nibble1] += byte1;
      return 0;

    case 8:
      switch(nibble3){
        case 0:
          V[nibble1] = V[nibble2];
          return 0;

        case 1:
          V[nibble1] |= V[nibble2];
          return 0;

        case 2:
          V[nibble1] &= V[nibble2];
          return 0;

        case 3:
          V[nibble1] ^= V[nibble2];
          return 0;

        case 4:
          if (V[nibble1] + V[nibble2] > 0xFF)
            V[0xF] = 1;
          else
            V[0xF] = 0;
          V[nibble1] += V[nibble2];
          return 0;

        case 5:
          if (V[nibble1] - V[nibble2] < 0)
            V[0xF] = 0;
          else
            V[0xF] = 1;
          V[nibble1] -= V[nibble2];
          return 0;

        case 6:
          V[0xF] = V[nibble1] & 0x01;
          V[nibble1] >>= 1;
          return 0;

        case 7:
          if (V[nibble2] - V[nibble1] < 0)
            V[0xF] = 0;
          else
            V[0xF] = 1;
          V[nibble1] = V[nibble2] - V[nibble1];
          return 0;

        case 0xE:
          V[0xF] = V[nibble1] & 0x80;
          V[nibble1] <<= 1;
          return 0;

        default:
          return 1;
      }

    case 9:
      if (nibble3 == 0){
        if (V[nibble1] != V[nibble2]) pc += 2;
        return 0;
      }
      else return 1;

    case 0xA:
      I = (0x0FFF & instr);
      return 0;

    case 0xB:
      return jmp(V[0] + (0x0FFF & instr));

    case 0xC:
      V[nibble1] = rand() & byte1;
      return 0;

    case 0xD:
      return draw(instr);

    case 0xE:
      switch(byte1){
        case 0x9E:
          if (keys[V[nibble1]]) pc += 2;
          return 0;

        case 0xA1:
          if (!keys[V[nibble1]]) pc += 2;
          return 0;

        default:
          return 1;
      }

    case 0xF:
      switch(byte1){

        case 0x07:
          V[nibble1] = dt/10;
          return 0;

        case 0x0A:
          {int pressed = 0;
          for (int i = 0; i < 0x10; i++)
            if (keys[i]){
              pressed = 1;
              V[nibble1] = i;
            }
          if (!pressed) pc -= 2;
          return 0;}

        case 0x15:
          dt = V[nibble1] * 60;
          return 0;

        case 0x18:
          st = V[nibble1];
          return 0;

        case 0x1E:
          I += V[nibble1];
          return 0;

        case 0x29:
          I = V[nibble1];
          return 0;

        case 0x33:
          mem[I]     = V[(instr & 0x0F00) >> 8] / 100;
          mem[I + 1] = (V[(instr & 0x0F00) >> 8] / 10) % 10;
          mem[I + 2] = (V[(instr & 0x0F00) >> 8] % 100) % 10;
          return 0;

        case 0x55:
          for (int i = 0, store = I; i <= nibble1; i++, store++)
            mem[store] = V[i];
          return 0;

        case 0x65:
          for (int i = 0, load = I; i <= nibble1; i++, load++)
            V[i] = mem[load];
          return 0;

        default:
          return 1;
      }
  }
  return 1;
}
