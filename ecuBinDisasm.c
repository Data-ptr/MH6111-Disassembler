#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecuBinDisasm.h"

// Main function
int main () {
  size_t  bytesRead;
  byte*   buffPtr;
  byte*   lastBuffPtr;

  byte*   entryPoint;
  byte*   codeStart;

  word binCurrPos = 0;
  bool lineNumbers = 0;
  bool rawBytes = 0;

  // Load symbols from file
  loadSymbolFile();
  sortSymbols();
  //printSymbols();

  buildRomAreaStruct();

  char frmt[16] = "\0";
  sprintf(frmt, "%%%is.msfirst\n", LABEL_PAD);
  //"%-16s .msfirst\n"
  printf(frmt, "");

  // Since symbols in the RAM area aren't generated, they were loaded in file
  printRamVariables(lineNumbers, rawBytes);

  // Use soDefs to add multi-byte operations into the operations table (opTable)
  addSubOps();

  // Load bin file
  bytesRead = loadEcuBinFile();

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

    if(lineNumbers) {
      printf("%04X ", binCurrPos);
    }

    if(DATA == ra || VECTOR == ra) {
      buffPtr = printData(binCurrPos, buffPtr, lineNumbers, rawBytes);
    } else if(CODE == ra) {
      buffPtr = decodeOp(binCurrPos, buffPtr, PRINT, lineNumbers, rawBytes);
    }
  }

  // End (.end)
  if(lineNumbers) {
    printf("%05X ",  0x10000);
  }

  if(rawBytes) {
    printf("            ");
  }

  sprintf(frmt, "%%%is .end\n", LABEL_PAD);
  //"%-16s .end\n"
  printf(frmt, "");

  // Clean up
  free(binBuffer);
  free(symbolTable);
  freeSubOps();

  // Success
  return 0;
}

void buildRomAreaStruct() {
  int i = 0;
  ROMArea lastRa = CODE;
  uint rasIndex = 0;

  while(i < numSymbols) {
    if(ORG == symbolTable[i].area) {
      i++;
      continue;
    }

    if(lastRa != symbolTable[i].area) {
      lastRa = symbolTable[i].area;
      ras[rasIndex].address = symbolTable[i].addr;
      ras[rasIndex].area = symbolTable[i].area;
      rasIndex++;
    }

    i++;
  }
}

void printRaw(byte* buffPtr, uint numBytes) {
  // Print raw bytes
  for(int i = 0; i < 4; i++) {
    if(i < numBytes) {
      printf("%02X ", *(buffPtr + i));
    } else {
      printf("   ");
    }
  }
}

byte* printData(word binCurrPos, byte* buffPtr, bool lineNumbers, bool rawBytes) {
  char* symbol = getSymbol(binCurrPos);
  uint btns = bytesToNextSection(buffPtr);
  uint btnl = bytesToNextLabel(buffPtr);

  if(VECTOR == ra) {
    word      vecBytes   = (buffPtr[0] << 8) + buffPtr[1];
    char*     vecSymbol  = getSymbol(vecBytes);

    doOrg(binCurrPos, &symbol, lineNumbers, rawBytes);

    if(rawBytes) {
      printRaw(buffPtr, 2);
    }

    //if((btns >= 2 || 0 == btns) && btnl >= 2 && binCurrPos + 2 < 0x10000) {
    if('\0' != vecSymbol[0]) {
      char frmt[13] = "\0";
      sprintf(frmt, "%%%is%%-8s%%s\n", LABEL_PAD);
      //"%-17s%-8s%s\n"
      printf(frmt, symbol, ".word", vecSymbol);
    } else {
      char frmt[16] = "\0";
      sprintf(frmt, "%%%is%%-8s$%%04x\n", LABEL_PAD);
      printf(frmt, symbol, ".word", vecBytes);
      //"%-17s%-8s$%04x\n"
    }

    buffPtr += 2;

    return buffPtr;
    // return printVectorTable();
  }

  // If there are at least 4 bytes to print, and maybe we are on a label
  if(btns >= 4 && btnl >= 4 && binCurrPos + 4 < 0xFFFF) {
    // Code for .fill directive
    if((byte)0xFF == *buffPtr && (byte)0xFF == buffPtr[1]) { //If at least 2-bytes
      uint numFFs = 0;

      while((byte)0xFF == *buffPtr && bytesToNextLabel(buffPtr) >= 0) {
        numFFs++;
        buffPtr++;
      }

      //printRaw(buffPtr, 4);
      doOrg(binCurrPos, &symbol, lineNumbers, rawBytes);

      if(rawBytes) {
        printf("            ");
      }

      char frmt[18] = "\0";
      sprintf(frmt, "%%%is%%-8s%%d, $FF\n", LABEL_PAD);
      //"%-17s%-8s%d, $FF\n"
      printf(frmt, symbol, ".fill", numFFs);
    } else {
      if(rawBytes) {
        printRaw(buffPtr, 4);
      }

      doOrg(binCurrPos, &symbol, lineNumbers, rawBytes);

      // Code for four .byte directive
      char frmt[37] = "\0";
      sprintf(frmt, "%%%is%%-8s$%%02x, $%%02x, $%%02x, $%%02x\n", LABEL_PAD);
      //"%-17s%-8s$%02x, $%02x, $%02x, $%02x\n"
      printf(
        frmt,
        symbol,
        ".byte",
        buffPtr[0],
        buffPtr[1],
        buffPtr[2],
        buffPtr[3]
      );

      buffPtr += 4;
    }
  } else { //Brint less than a full 4 bytes
    uint bytesToPrint = btnl;

    if(rawBytes) {
      printRaw(buffPtr, bytesToPrint);
    }

    doOrg(binCurrPos, &symbol, lineNumbers, rawBytes);

    char frmt[15] = "\0";
    sprintf(frmt, "%%%is%%-8s$%%02x", LABEL_PAD);
    //"%-17s%-8s$%02x"
    printf(frmt, symbol, ".byte", buffPtr[0]);

    // Start at 1 for next byte
    for(int i = 1; i < bytesToPrint; i++) {
      printf(", $%02x", buffPtr[i]);
    }

    printf("\n");

    buffPtr += bytesToPrint;
  }

  return buffPtr;
}

byte* decodeOp(word binCurrPos, byte* buffPtr, decodeType dt, bool lineNumbers, bool rawBytes) {
  operation currOp     = opTable[*buffPtr];
  subOp     currSubOp;
  uint      isSubOp    = getSubOp(currOp, buffPtr, &currSubOp);
  opUnion   ou;

  if(isSubOp) {
    ou.subOp = currSubOp;
  } else {
    ou.op = currOp;
  }

  if(PRINT == dt){
    printOp(binCurrPos, ou, buffPtr, lineNumbers, rawBytes);

    printf("\n");
  } else if(SCAN == dt) {
    generateRelativeSymbols(binCurrPos, ou, buffPtr, isSubOp);
  }

  return buffPtr + (isSubOp ? currSubOp.numBytes : currOp.numBytes);
}

word decodeOpAndJump(word binCurrPos, byte * buffPtr) {
  word      opBytes    = (buffPtr[1] << 8) + buffPtr[2];
  operation currOp     = opTable[*buffPtr];
  char*     symbol     = getSymbol(opBytes);

  if('\0' != symbol[0]) {
    //printf("%X\t\t\t%s\t%s\n", binCurrPos, currOp.mnemonic, symbol);
  } else {
    //printf("%X\t\t\t%s\t%04x\n", binCurrPos, currOp.mnemonic, opBytes);
  }

  return opBytes;
}

void generateRelativeSymbols(word binCurrPos, opUnion ou, byte* buffPtr, bool isSubOp) {
  operation currOp;
  subOp     currSubOp;

  if(isSubOp) {
    currSubOp = ou.subOp;
  } else {
    currOp = ou.op;
  }

  uint btnl = bytesToNextLabel(buffPtr);

  if(isSubOp) {
    if(IMMEDIATE16 == currSubOp.type) {
      word  opWord    = (buffPtr[2] << 8) + buffPtr[3];


      if(0x8000 < opWord) {
        char* symbol    = getSymbol(opWord);

        // Make up a label
        if('\0' == symbol[0]) {
          symbol = (char*)malloc(sizeof(char) * 6);
          // Make up a label for the symbol
          sprintf(symbol, "T%i", generatedLabel++);
          // Add the made up symbol to the symbol list
          addSymbol(opWord, DATA, symbol);
        }
      }
    }
  } else {
    if(RELATIVE == currOp.type) { // Calculate RELATIVE address
      // Signed char for two's compliment
      word  opWord    = binCurrPos + (char)buffPtr[1] + 0x0002;
      char* symbol    = getSymbol(opWord);

      // Make up a label
      if('\0' == symbol[0]) {
        symbol = (char*)malloc(sizeof(char) * 6);
        // Make up a label for the symbol
        sprintf(symbol, "L%i", generatedLabel++);
        // Add the made up symbol to the symbol list
        addSymbol(opWord, CODE, symbol);
      }
    } else if(DIRECT3 == currOp.type) {
      // Signed char for two's compliment
      word  opWord    = binCurrPos + (char)buffPtr[3] + 0x0004;
      char* symbol    = getSymbol(opWord);

      // Make up a label
      if('\0' == symbol[0]) {
        symbol = (char*)malloc(sizeof(char) * 6);
        // Make up a label for the symbol
        sprintf(symbol, "L%i", generatedLabel++);
        // Add the made up symbol to the symbol list
        addSymbol(opWord, CODE, symbol);
      }
    } else if(IMMEDIATE16 == currOp.type) {
      word  opWord    = (buffPtr[1] << 8) + buffPtr[2];

      if(
          // 0 != strcmp(currOp.mnemonic, "ldd") &&
          //0 != strcmp(currOp.mnemonic, "ldx") &&
          0xD000 < opWord &&
          0xFFFF > opWord
        ) {
        char* symbol    = getSymbol(opWord);

        // Make up a label
        if('\0' == symbol[0]) {
          symbol = (char*)malloc(sizeof(char) * 6);
          // Make up a label for the symbol
          sprintf(symbol, "T%i", generatedLabel++);
          // Add the made up symbol to the symbol list
          addSymbol(opWord, DATA, symbol);
        }
      }
    }
  }
}
