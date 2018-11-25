#include <stdlib.h>
#include <stdio.h>


void decode(unsigned short instr){

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
          break;

        case 0x00EE:
          printf("return\n");
          break;

        default:
          printf("call SYS func at 0x%X\n", (0x0FFF & instr));
      }
      break;

    case 1:
      printf("jmp 0x%X\n", (0x0FFF & instr));
      break;

    case 2:
      printf("call 0x%X\n", (0x0FFF & instr));
      break;

    case 3:
      printf("skip next instruction if (V%d == 0x%X)\n", nibble1, byte1);
      break;

    case 4:
      printf("skip next instruction if (V%d != 0x%X)\n", nibble1, byte1);
      break;

    case 5:
      if (nibble3 == 0)
        printf("skip next instruction if (V%d == V%d)\n", nibble1, nibble2);
      else
        printf("data: 0x%X\n", instr);
      break;

    case 6:
      printf("V%d = 0x%X\n", nibble1, byte1);
      break;

    case 7:
      printf("V%d += 0x%X\n", nibble1, byte1);
      break;

    case 8:
      switch(nibble3){

        case 0:
          printf("V%d = V%d\n", nibble1, nibble2);
          break;

        case 1:
          printf("V%d |= V%d\n", nibble1, nibble2);
          break;

        case 2:
          printf("V%d &= V%d\n", nibble1, nibble2);
          break;

        case 3:
          printf("V%d ^= V%d\n", nibble1, nibble2);
          break;

        case 4:
          printf("V%d += V%d (VF = 1 if there's a carry, else VF = 0)\n", nibble1, nibble2);
          break;

        case 5:
          printf("V%d -= V%d (VF = 0 if there's a borrow, else VF = 1)\n", nibble1, nibble2);
          break;

        case 6:
          printf("V%d >>= 1 (VF = least significant bit of V%d)\n", nibble1, nibble1);
          break;

        case 7:
          printf("V%d = V%d - V%d (VF = 0 if there's a borrow, else VF = 1)\n", nibble1, nibble2, nibble1);
          break;

        case 0xE:
          printf("V%d <<= 1 (VF = most significant bit of V%d)\n", nibble1, nibble1);
          break;

        default:
          printf("data: 0x%X\n", instr);
      }

    case 9:
      if (nibble3 == 0)
        printf("skip next instruction if (V%d != V%d)\n", nibble1, nibble2);
      else 
        printf("data: 0x%X\n", instr);
      break;

    case 0xA:
      printf("I = 0x%X\n", (0x0FFF & instr));
      break;

    case 0xB:
      printf("PC = V0 + 0x%X\n", (0x0FFF & instr));
      break;

    case 0xC:
      printf("V%d = rand(0 : 255) & 0x%X\n", nibble1, byte1);
      break;

    case 0xD:
      printf("draw(V%d, V%d, 0x%X)\n", nibble1, nibble2, nibble3);
      break;

    case 0xE:
      switch(byte1){
        case 0x9E:
            printf("skip next instruction if (key() == V%d)\n", nibble1);
            break;
        case 0xA1:
          printf("skip next instruction if (key() != V%d)\n", nibble1);
          break;
        default:
          printf("data: 0x%X\n", instr);
      }
      break;

    case 0xF:
      switch(byte1){

        case 0x07:
          printf("V%d = get_delay()\n", nibble1);
          break;

        case 0x0A:
          printf("V%d = get_key() (execution halted until key pressed)\n", nibble1);
          break;

        case 0x15:
          printf("delay_timer(V%d)\n", nibble1);
          break;

        case 0x18:
          printf("sound_time(V%d)\n", nibble1);
          break;

        case 0x1E:
          printf("I += V%d\n", nibble1);
          break;

        case 0x29:
          printf("I = sprite_val[V%d]\n", nibble1);
          break;

        case 0x33:
          printf("set_BCD(V%d); *(I+0)=BCD(3); *(I+1)=BCD(2); *(I+2)=BCD(1);\n", nibble1);
          break;

        case 0x55:
          printf("reg_dump(V%d, &I)\n", nibble1);
          break;

        case 0x65:
          printf("reg_load(V%d, &I)\n", nibble1);
          break;

        default:
          printf("data: 0x%X\n", instr);
          
      }
      break;

    default:
      printf("data: 0x%X\n", instr);
  }
}


int main(int argc, char** argv){

    FILE *romptr;
    long filelen;
    unsigned short big_end;
    unsigned short little_end;
    unsigned short instr;

    if (argc < 2) {
      printf("Too few arguments supplied\n");
      return 0;
    }

    romptr = fopen(argv[1], "r");
    if (!romptr){
        printf("ROM not found: %s\n", argv[1]);
        return 1;
    }

    fseek(romptr, 0, SEEK_END);
    filelen = ftell(romptr);
    rewind(romptr);

    for (int i = 0; i < filelen; i++){
        big_end = fgetc(romptr);
        little_end = fgetc(romptr);
        instr = (big_end << 8) + little_end;
        decode(instr);
    }

    fclose(romptr);
    return 0;
}
