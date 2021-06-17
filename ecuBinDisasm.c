#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <stdbool.h>

#include "ecuBinDisasm.h"
#include "symbols.c"
#include "decode.c"
#include "output.c"
#include "subops.c"
#include "rom.c"
#include "helpers.c"

// Main function
int main (int argc, char *argv[]) {
  size_t  bytesRead;
  byte*   buffPtr;
  byte*   lastBuffPtr;

  byte*   entryPoint;
  byte*   codeStart;

  word binCurrPos = 0;

  //
  // Argument stuff
  //
  // Defaults
  storedArgs.lineNumbers   = 0;
  storedArgs.rawBytes      = 0;
  storedArgs.romStart      = ROM_START;
  storedArgs.validRomStart = VALID_ROM_START;

  argp_parse(&argp, argc, argv, 0, 0, &storedArgs);

  // Load symbols from file
  loadSymbolFile(storedArgs.args[1]);
  sortSymbols();
  //printSymbols();

  buildRomAreaStruct();

  char frmt[16] = "\0";
  sprintf(frmt, "%%%is.msfirst\n", LABEL_PAD);
  //"%-16s .msfirst\n"
  printf(frmt, "");

  // Since symbols in the RAM area aren't generated, they were loaded in file
  printRamVariables(storedArgs.lineNumbers, storedArgs.rawBytes);

  // Use soDefs to add multi-byte operations into the operations table (opTable)
  addSubOps();

  // Load bin file
  bytesRead = loadEcuBinFile(storedArgs.args[0]);

  /* the whole file is now loaded in the memory buffer. */

  // Point to start of buffer
  buffPtr = binBuffer;

  // Scan for RELATIVE ops to build symbol table
  while(
    buffPtr < binBuffer + bytesRead &&
    buffPtr != lastBuffPtr
  ) {
    binCurrPos = getBinPos(buffPtr);

    lastBuffPtr = buffPtr;
    updateRomArea(buffPtr, &ra, 0);

    // If we aren't in a CODE region, find the next code region
    if(CODE != ra) {
       buffPtr += bytesToNextSection(buffPtr);
       continue;
    }

    buffPtr = decodeOp(binCurrPos, buffPtr, SCAN, 0, 0);
  }

  // Sort things again
  sortSymbols();
  //printSymbols();

  // Reset pointer
  buffPtr = binBuffer;

  // Reset ROM Area started
  updateRomArea(buffPtr, &ra, 1);

  // Main loop
  while(
    buffPtr < binBuffer + bytesRead &&
    buffPtr != lastBuffPtr
  ) {
    binCurrPos = getBinPos(buffPtr);

    lastBuffPtr = buffPtr;
    updateRomArea(buffPtr, &ra, 0);

    //printf("\nROM area: %s\n", ra == DATA ? "data" : "code");

    if(storedArgs.lineNumbers) {
      printf("%04X ", binCurrPos);
    }

    if(DATA == ra || VECTOR == ra) {
      buffPtr = printData(binCurrPos, buffPtr, storedArgs.lineNumbers, storedArgs.rawBytes);
    } else if(CODE == ra) {
      buffPtr = decodeOp(binCurrPos, buffPtr, PRINT, storedArgs.lineNumbers, storedArgs.rawBytes);
    }
  }

  // End (.end)
  if(storedArgs.lineNumbers) {
    printf("%05X ",  0x10000);
  }

  if(storedArgs.rawBytes) {
    printf("            ");
  }

  sprintf(frmt, "%%%is .end\n", LABEL_PAD);
  //"%-16s .end\n"
  printf(frmt, "");

  // Clean up
  free(validSymAddr);
  free(binBuffer);
  free(symbolTable);
  freeSubOps();

  // Success
  return 0;
}
