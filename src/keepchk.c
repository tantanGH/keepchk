#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <doslib.h>

#define MAX_EYE_CATCH_LEN (32)

int32_t main(int32_t argc, uint8_t* argv[]) {

  // default return code
  int rc = -1;

  // argument check
  if (argc < 2) {
    printf("usage: keepchk [-d] <eye-catch>\n");
    goto exit;
  }

  static uint8_t eye_catch[ MAX_EYE_CATCH_LEN + 1];
  eye_catch[0] = '\0';

  for (int16_t i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'd' && argc > (i+1)) {
        // dump mode
        uint8_t* exec_file = argv[i+1];
        FILE* fp = fopen(exec_file,"rb");
        if (fp == NULL) {
          printf("error: cannot open (%s).\n", exec_file);
          goto exit;
        }
        fseek(fp, 64, SEEK_CUR);
        fread(eye_catch, 1, MAX_EYE_CATCH_LEN, fp);
        fclose(fp);
        for (int16_t i = 0; i < MAX_EYE_CATCH_LEN; i++) {
          if (i > 0) printf(" ");
          printf("%02X", eye_catch[i]);
        }
        printf("\n");
        for (int16_t i = 0; i < MAX_EYE_CATCH_LEN; i++) {
          if (i > 0) printf(" ");
          if (eye_catch[i] >= 0x20 && eye_catch[i] <= 0x7f) {
            printf("%c ", eye_catch[i]);
          } else {
            printf("* ");
          }
        }
        printf("\n");
        rc = 0;
        goto exit;
      } else {
        printf("error: unknown option.\n");
        goto exit;
      }
    } else {
      if (argv[i][0] == '0' && argv[i][1] == 'x') {
        for (int16_t j = 0; j < (strlen(argv[i]) - 2)/2; j++) {
          uint8_t c1 = argv[i][2+j*2];
          uint8_t c2 = argv[i][2+j*2+1];
          if (c1 >= '0' && c1 <= '9') c1 -= '0';
          else if (c1 >= 'a' && c1 <= 'f') c1 -= 'a' - 10;
          else if (c1 >= 'A' && c1 <= 'F') c1 -= 'A' - 10;
          else {
            printf("error: bad format.\n");
            goto exit;
          }
          if (c2 >= '0' && c2 <= '9') c2 -= '0';
          else if (c2 >= 'a' && c2 <= 'f') c2 -= 'a' - 10;
          else if (c2 >= 'A' && c2 <= 'F') c2 -= 'A' - 10;
          else {
            printf("error: bad format.\n");
            goto exit;
          }
          eye_catch[j] = c1 * 16 + c2;
          eye_catch[j+1] = '\0';
        }
      } else {
        if (strlen(argv[i]) > MAX_EYE_CATCH_LEN) {
          printf("error: too long eye catch.\n");
          goto exit;
        }
        strcpy(eye_catch, argv[i]);
      }
    }
  }

  if (eye_catch[0] == '\0') {
    printf("error: no eye catch is specified.\n");
    goto exit;
  }

  int32_t eye_catch_len = strlen(eye_catch);
  if (eye_catch_len > MAX_EYE_CATCH_LEN) {
    printf("error: too long eye catch.\n");
    goto exit;
  }

  // supervisor mode
  SUPER(0);

  // get the current process PSP address by using PDB address
  uint8_t* psp = (uint8_t*)GETPDB() - 16;

  // PSP (Memory Management Pointer) 16 bytes
  //  offset 0x00 ... PREVMEM (Previous memory block address)
  //  offset 0x04 ... MOTHER  (Parent process memory block address)
  //  offset 0x08 ... MEMEND  (Last address of this memory block)
  //  offset 0x0c ... NEXTMEM (Next memory block address)
  //  the following 240 bytes are defined as struct PDBADR in XC library

  // find the root process
  for (;;) {
    uint32_t parent = *((uint32_t*)(psp + 4));
    if (parent == 0) {
      // no more parent process (most likely this is Human68k)
      break;
    }
    //printf("parent=%08X\n", parent);
    psp = (uint8_t*)parent;
  }

  // check memory blocks in the link
  for (;;) {
    //printf("checking process at %08X\n",psp);
    if (psp[4] == 0xff) {   // is this a KEEP process?
      //printf("found keep process.\n");
      if (memcmp(psp + 0x100, eye_catch, eye_catch_len) == 0) {
        printf("found at %08X.\n", psp + 0x10);
        rc = 1;
        break;
      }
    }
    uint32_t child = *((uint32_t*)(psp + 12));
    if (child == 0) {
      printf("not found.\n");
      break;
    }
    psp = (uint8_t*)child;
  }

exit:
  return rc;
}