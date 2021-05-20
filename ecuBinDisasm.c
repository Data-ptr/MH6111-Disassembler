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

  // Load symbols from file
  loadSymbolFile();
  sortSymbols();
  //printSymbols();

  buildRomAreaStruct();

  printf("%-17s .msfirst\n", "");

  printRamVariables(lineNumbers);

  // Use soDefs to add multi-byte operations into the operations table (opTable)
  addSubOps();

  // Load bin file
  bytesRead = loadEcuBinFile();

  /* the whole file is now loaded in the memory buffer. */

  // Point to start of buffer
  buffPtr = binBuffer;

  // Count 0xFF and find entryPoint
  while((byte)0xFF == *buffPtr) {
    buffPtr++;
  }

  // BuffPtr should point to the reset entryPoint
  entryPoint = buffPtr;

  binCurrPos = getBinPos(buffPtr);

  // Follow the jump for fun
  codeStart  = buffPtr = binBuffer + (decodeOpAndJump(binCurrPos, buffPtr) - ROM_START);

  // Scan for RELATIVE ops to build symbol table
  while(
    buffPtr < binBuffer + bytesRead &&
    buffPtr != lastBuffPtr
  ) {
    binCurrPos = getBinPos(buffPtr);

    lastBuffPtr = buffPtr;
    buffPtr = decodeOp(binCurrPos, buffPtr, SCAN, 0);
  }

  // Reset pointer
  buffPtr = binBuffer;

  // Main loop
  while(
    buffPtr < binBuffer + bytesRead &&
    buffPtr != lastBuffPtr
  ) {
    binCurrPos = getBinPos(buffPtr);

    lastBuffPtr = buffPtr;
    updateRomArea(buffPtr, &ra);

    if(DATA == ra || VECTOR == ra) {
      buffPtr = printData(binCurrPos, buffPtr, lineNumbers);
    } else if(CODE == ra) {
      buffPtr = decodeOp(binCurrPos, buffPtr, PRINT, lineNumbers);
    }
  }

  // End (.end)
  if(lineNumbers) {
    printf("%05X ",  0x10000);
  }

  printf("%-17s .end\n", "");

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

byte* printData(word binCurrPos, byte* buffPtr, bool lineNumber) {
  char* symbol = getSymbol(binCurrPos);
  uint btns = bytesToNextSection(buffPtr);
  uint btnl = bytesToNextLabel(buffPtr);

  doOrg(binCurrPos, &symbol, lineNumber);

  if(lineNumber) {
    printf("%04X ", binCurrPos);
  }

  if(VECTOR == ra) {
    word      vecBytes   = (buffPtr[0] << 8) + buffPtr[1];
    char*     vecSymbol  = getSymbol(vecBytes);

    //if((btns >= 2 || 0 == btns) && btnl >= 2 && binCurrPos + 2 < 0x10000) {
    if('\0' != vecSymbol[0]) {
      printf("%-17s .word %s\n", symbol, vecSymbol);
    } else {
      printf("%-17s .word $%04x\n", symbol, vecBytes);
    }

    buffPtr += 2;

    return buffPtr;
    // return printVectorTable();
  }

  // If there are at least 4 bytes to print, and maybe we are on a label
  if((btns >= 4 || 0 == btns) && btnl >= 4 && binCurrPos + 4 < 0xFFFF) {
    // Code for .fill directive
    if((byte)0xFF == *buffPtr && (byte)0xFF == buffPtr[1]) { //If at least 2-bytes
      uint numFFs = 0;

      while((byte)0xFF == *buffPtr && bytesToNextLabel(buffPtr) >= 0) {
        numFFs++;
        buffPtr++;
      }

      printf("%-17s .fill\t%d, $FF\n", symbol, numFFs);
    } else {
      // Code for four .byte directive
      printf(
        "%-17s .byte\t$%02x, $%02x, $%02x, $%02x\n",
        symbol,
        buffPtr[0],
        buffPtr[1],
        buffPtr[2],
        buffPtr[3]
      );

      buffPtr += 4;
    }
  } else {
    uint bytesToPrint = btnl;

    if(0 == bytesToPrint) {
      bytesToPrint = 1;
    }

    printf("%-17s .byte $%02x", symbol, buffPtr[0]);

    for(int i = 1; i < bytesToPrint; i++) {
      printf(", $%02x", buffPtr[i]);
    }

    printf("\n");

    buffPtr += bytesToPrint;
  }

  return buffPtr;
}

// byte* printVectorTable(word binCurrPos, byte* buffPtr, bool lineNumber) {
//
// }

byte* decodeOp(word binCurrPos, byte* buffPtr, decodeType dt, bool lineNumber) {
  operation currOp     = opTable[*buffPtr];
  subOp     currSubOp;
  uint      isSubOp    = getSubOp(currOp, buffPtr, &currSubOp);

  if(PRINT == dt){
    opUnion ou;

    if(isSubOp) {
      ou.subOp = currSubOp;
    } else {
      ou.op = currOp;
    }

    printOp(binCurrPos, ou, buffPtr, lineNumber);

    printf("\n");
  } else if(SCAN == dt && !isSubOp) {
    generateRelativeSymbols(binCurrPos, currOp, buffPtr);
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

void generateRelativeSymbols(word binCurrPos, operation currOp, byte* buffPtr) {
  if(RELATIVE == currOp.type) { // Calculate RELATIVE address
    // Signed char for two's compliment
    word  opWord    = binCurrPos + (char)buffPtr[1] + 0x0002;
    char* symbol    = getSymbol(opWord);

    // Make up a label
    if('\0' == symbol[0]) {
      symbol = (char*)malloc(sizeof(char) * 5);
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
      symbol = (char*)malloc(sizeof(char) * 5);
      // Make up a label for the symbol
      sprintf(symbol, "L%i", generatedLabel++);
      // Add the made up symbol to the symbol list
      addSymbol(opWord, CODE, symbol);
    }
  }
}
