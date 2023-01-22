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

  uint8_t* eye_catch = NULL;
  for (int16_t i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'd') {
        // dump mode
        uint8_t* exec_file = argv[2];
        FILE* fp = fopen(exec_file,"rb");
        if (fp == NULL) {
          printf("error: cannot open (%s).\n", exec_file);
          goto exit;
        }
        static uint8_t buf[MAX_EYE_CATCH_LEN];
        fread(buf,1,MAX_EYE_CATCH_LEN,fp);
        fclose(fp);
        for (int16_t i = 0; i < MAX_EYE_CATCH_LEN; i++) {
          printf("%02X ", buf[i]);
        }
        for (int16_t i = 0; i < MAX_EYE_CATCH_LEN; i++) {
          printf("'%c'", buf[i]);
        }
        rc = 0;
        goto exit;
      } else {
        printf("error: unknown option.\n");
        goto exit;
      }
    } else {
      eye_catch = argv[i];
    }
  }

  if (eye_catch == NULL) {
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
        printf("found a KEEP process with eye catch (%s) at %08X.\n", eye_catch, psp);
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