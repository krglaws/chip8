#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define ROM_START 0x200
#define MEM_LEN 0x1000
#define MAX_STACK 0x10
#define DISP_HEIGHT 0x20
#define DISP_WIDTH 0x40
#define FONT_START 0x50


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

  return 0;
}


// clear display
int cls(){
  memset(display32x64, 0, DISP_WIDTH * DISP_HEIGHT * sizeof(int));
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

  if (sp == MEM_LEN - 1){
    printf("pop on empty stack\n");
    return 1;
  }

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
  V[0xF] = 0;

  // for each row
  for (int i = 0; i < rows && (i + y) < DISP_HEIGHT; i++){
    sprite = mem[I + i];
    // for each bit
    for (char j = 0; j < 8 && (j + x) < DISP_WIDTH; j++){
      pxl = (sprite >> (8 - (j + 1))) & 1;
      if (pxl){
        if (display32x64[((y+i) * DISP_WIDTH) + (x+j)])
          V[0xF] = 1;
        display32x64[((y+i) * DISP_WIDTH) + (x+j)] ^= 0xFFFFFFFF;
      }
    }
  }
  return 0;
}


int get_time(){
  //TODO
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
          printf("disp_clear()\n");
          return cls();

        case 0x00EE:
          printf("return\n");
          return ret();

        default:
          // ignore
          printf("call SYS func at 0x%X\n", (0x0FFF & instr));
          return 0;
      }

    case 1:
      printf("jmp 0x%X\n", (0x0FFF & instr));
      return jmp(instr);

    case 2:
      printf("call 0x%X\n", (0x0FFF & instr));
      return call(instr);

    case 3:
      printf("skip next instruction if (V%d == 0x%X)\n", nibble1, byte1);
      if (V[nibble1] == byte1) pc += 2;
      return 0;

    case 4:
      printf("skip next instruction if (V%d != 0x%X)\n", nibble1, byte1);
      if (V[nibble1] != byte1) pc += 2;
      return 0;

    case 5:
      if (nibble3 == 0){
        printf("skip next instruction if (V%d == V%d)\n", nibble1, nibble2);
        if (V[nibble1] == V[nibble2]) pc += 2;
        return 0;
      }
      else {
        printf("unrecognized instruction: 0x%X\n", instr);
        return 1;
      }

    case 6:
      printf("V%d = 0x%X\n", nibble1, byte1);
      V[nibble1] = byte1;
      return 0;

    case 7:
      printf("V%d += 0x%X\n", nibble1, byte1);
      V[nibble1] += byte1;
      return 0;

    case 8:
      switch(nibble3){

        case 0:
          printf("V%d = V%d\n", nibble1, nibble2);
          V[nibble1] = V[nibble2];
          return 0;

        case 1:
          printf("V%d |= V%d\n", nibble1, nibble2);
          V[nibble1] |= V[nibble2];
          return 0;

        case 2:
          printf("V%d &= V%d\n", nibble1, nibble2);
          V[nibble1] &= V[nibble2];
          return 0;

        case 3:
          printf("V%d ^= V%d\n", nibble1, nibble2);
          V[nibble1] ^= V[nibble2];
          return 0;

        case 4:
          printf("V%d += V%d (VF = 1 if there's a carry, else VF = 0)\n", nibble1, nibble2);
          if (V[nibble1] + V[nibble2] > 0xFF)
            V[0xF] = 1;
          else
            V[0xF] = 0;
          V[nibble1] += V[nibble2];
          return 0;

        case 5:
          printf("V%d -= V%d (VF = 0 if there's a borrow, else VF = 1)\n", nibble1, nibble2);
          if (V[nibble1] - V[nibble2] < 0)
            V[0xF] = 0;
          else
            V[0xF] = 1;
          return 0;

        case 6:
          printf("V%d >>= 1 (VF = least significant bit of V%d)\n", nibble1, nibble1);
          V[0xF] = V[nibble1] & 0x01;
          V[nibble1] >>= 1;
          return 0;

        case 7:
          printf("V%d = V%d - V%d (VF = 0 if there's a borrow, else VF = 1)\n", nibble1, nibble2, nibble1);
          if (V[nibble2] - V[nibble1] < 0)
            V[0xF] = 0;
          else
            V[0xF] = 1;
          V[nibble1] = V[nibble2] - V[nibble1];
          return 0;

        case 0xE:
          printf("V%d <<= 1 (VF = most significant bit of V%d)\n", nibble1, nibble1);
          V[0xF] = V[nibble1] & 0x80;
          V[nibble1] <<= 1;
          return 0;

        default:
          printf("unrecognized instruction: 0x%X\n", instr);
          return 1;
      }

    case 9:
      if (nibble3 == 0){
        printf("skip next instruction if (V%d != V%d)\n", nibble1, nibble2);
        if (V[nibble1] != V[nibble2]) pc += 2;
        return 0;
      }
      else {
        printf("unrecognized instruction: 0x%X\n", instr);
        return 1;
      }

    case 0xA:
      printf("I = 0x%X\n", (0x0FFF & instr));
      I = (0x0FFF & instr);
      return 0;

    case 0xB:
      printf("PC = V0 + 0x%X\n", (0x0FFF & instr));
      return jmp(V[0] + (0x0FFF & instr));

    case 0xC:
      printf("V%d = rand(0 : 255) & 0x%X\n", nibble1, byte1);
      V[nibble1] = rand() & byte1;
      return 0;

    case 0xD:
      printf("draw(V%d, V%d, 0x%X)\n", nibble1, nibble2, nibble3);
      return draw(instr);

    case 0xE:
      switch(byte1){
        case 0x9E:
          printf("skip next instruction if (key() == V%d)\n", nibble1);

          //TODO
          return 0;

        case 0xA1:
          printf("skip next instruction if (key() != V%d)\n", nibble1);

          //TODO
          return 0;

        default:
          printf("unrecognized instruction: 0x%X\n", instr);
          return 1;
      }

    case 0xF:
      switch(byte1){

        case 0x07:
          printf("V%d = get_delay()\n", nibble1);
          V[nibble1] = dt;
          return 0;

        case 0x0A:
          printf("V%d = get_key() (execution halted until key pressed)\n", nibble1);

          // TODO something like: while !key_pressed, pc -= 2;

          return 0;

        case 0x15:
          printf("delay_timer(V%d)\n", nibble1);
          dt = V[nibble1];
          return 0;

        case 0x18:
          printf("sound_time(V%d)\n", nibble1);
          st = V[nibble1];
          return 0;

        case 0x1E:
          printf("I += V%d\n", nibble1);
          I += V[nibble1];
          return 0;

        case 0x29:
          printf("I = sprite_val[V%d]\n", nibble1);
          I = V[nibble1];
          return 0;

        case 0x33:
          printf("set_BCD(V%d); *(I+0)=BCD(3); *(I+1)=BCD(2); *(I+2)=BCD(1);\n", nibble1);
          mem[I]     = V[(instr & 0x0F00) >> 8] / 100;
          mem[I + 1] = (V[(instr & 0x0F00) >> 8] / 10) % 10;
          mem[I + 2] = (V[(instr & 0x0F00) >> 8] % 100) % 10;
          //pc += 2;
          return 0;

        case 0x55:
          printf("reg_dump(V%d, &I)\n", nibble1);
          for (int i = 0, store = I; i < nibble1; i++)
            mem[store++] = V[i];
          return 0;

        case 0x65:
          printf("reg_load(V%d, &I)\n", nibble1);
          for (int i = 0, load = I; i < nibble1; i++)
            V[i] = mem[load++];
          return 0;

        default:
          printf("unrecognized instruction: 0x%X\n", instr);
          return 1;
      }

      default:
        printf("unrecognized instruction: 0x%X\n", instr);
        return 1;
  }

  // should be unreachable
  printf("U FOOKING WOT M8: 0x%X\n", instr);
  return 1;
}
