//
// Print raw bytes
//
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

//
// Print "ram" variables
//
void printRamVariables(bool lineNumber, bool rawBytes) {
  for(int i=0; ROM_START > symbolTable[i].addr; i++) {
    if(lineNumber){
      printf("0000 ");
    }

    if(rawBytes) {
      printf(RAW_BYTES_PAD);
    }

    char frmt[17] = "\0";
    sprintf(frmt, "%%%is%%-8s$%%04x\n", LABEL_PAD);
//"%LABEL_PADs%-8s$%04x\n"
    printf(frmt, symbolTable[i].label, ".equ", symbolTable[i].addr);
  }
}

//
// Print ops
//
void printOp(word binCurrPos, opUnion oper, byte* buffPtr, bool lineNumbers, bool rawBytes) {
  char* symbol = getSymbol(binCurrPos);

  doOrg(binCurrPos, &symbol, lineNumbers, rawBytes);

  if(rawBytes) {
    printRaw(buffPtr, oper.op.numBytes);
  }

  char frmt[17] = "\0";
  sprintf(frmt, "%%%is%%-8s", LABEL_PAD);
  // Print label and mnemonic
  //"%-17s%-8s"
  printf(frmt, symbol, oper.op.mnemonic);

  // Operation symbols are processed in here
  if(IMPLIED != oper.op.type) { // No bytes or symbols, just skip
    bool      isSymbol    = 0;
    bool      checkSymbol = 0;

    word opWord = getOpSymbol(binCurrPos, oper, buffPtr, &checkSymbol);

    if(checkSymbol) {
      symbol    = getSymbol(opWord);
      isSymbol  = '\0' != symbol[0];
    }

    if(oper.op.isSubOp) {
      subOpPrint(symbol, oper, buffPtr, isSymbol);
    } else {
      opPrint(symbol, oper, buffPtr, isSymbol, binCurrPos);
    }
  }
}

void subOpPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol) {
  subOp currSubOp = oper.subOp;

  switch(currSubOp.numBytes) {
    case 2:
      printf("%s", formats[(int)currSubOp.type]);
    break;
    case 3:
      printf(formats[(int)currSubOp.type], buffPtr[2]);
    break;
    case 4:
      if(IMMEDIATE16 == currSubOp.type && isSymbol) {
        printf("#%s", symbol);
      } else {
        printf(formats[(int)currSubOp.type], buffPtr[2], buffPtr[3]);
      }
    break;
  };
}

void  opPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol, word binCurrPos) {
  operation currOp = oper.op;
  char*     opSymbols[OP_SYMBOLS_MAX]      = {0};
  word      opSymbolsBytes[OP_SYMBOLS_MAX] = {0};

  switch(currOp.numBytes) {
    case 2:
      if(isSymbol){
        printf("%s", symbol);
      } else {
        printf(formats[(int)currOp.type], buffPtr[1]);
      }
    break;
    case 3:
      if(isSymbol){
        if(IMMEDIATE16 == currOp.type) {
          printf("#%s", symbol);
        } else if(DIRECT2 == currOp.type) {
          printf("%s, #$%02x", symbol, buffPtr[2]);
        } else {
          printf("%s", symbol);
        }
      } else {
        printf(formats[(int)currOp.type], buffPtr[1], buffPtr[2]);
      }
    break;
    case 4:
      if(DIRECT3 == currOp.type) { //TODO: Clean this up
        opSymbolsBytes[0] = buffPtr[1];
        opSymbolsBytes[1] = binCurrPos + (char)buffPtr[3] + 0x0004;
        opSymbols[0]      = getSymbol(opSymbolsBytes[0]);
        opSymbols[1]      = getSymbol(opSymbolsBytes[1]); // Relative

        if('\0' != opSymbols[0][0] && '\0' != opSymbols[1][0]) {
          printf("%s, #$%02x, %s", opSymbols[0], buffPtr[2], opSymbols[1]);
        } else if('\0' != opSymbols[1][0]) {
          printf("$%02x, #$%02x, %s", buffPtr[1], buffPtr[2], opSymbols[1]);
        } else {
          printf("$%02x, #$%02x, $%02x", buffPtr[1], buffPtr[2], buffPtr[3]);
        }
      } else if(DIRECT4 == currOp.type){
        opSymbolsBytes[1] = binCurrPos + (char)buffPtr[3] + 0x0004;
        opSymbols[1]      = getSymbol(opSymbolsBytes[1]); // Relative

        if('\0' != opSymbols[1][0]){
          printf(formats[(int)currOp.type], buffPtr[1], buffPtr[2], opSymbols[1]);
        } else {
          printf("$%04x, x, #$%02x, $%04x", buffPtr[1], buffPtr[2], opSymbolsBytes[1]);
        }
      } else {
        printf(formats[(int)currOp.type], buffPtr[1], buffPtr[2], buffPtr[3]);
      }
    break;
  };
}

//
// Print data
//
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
  if(btns >= 4 && btnl >= 4 && binCurrPos + 4 < ROM_END) {
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
    } else if((byte)0x00 == *buffPtr && (byte)0x00 == buffPtr[1]) { //If at least 2-bytes
      uint num00s = 0;

      while((byte)0x00 == *buffPtr && bytesToNextLabel(buffPtr) >= 0) {
        num00s++;
        buffPtr++;
      }

      //printRaw(buffPtr, 4);
      doOrg(binCurrPos, &symbol, lineNumbers, rawBytes);

      if(rawBytes) {
        printf("            ");
      }

      char frmt[18] = "\0";
      sprintf(frmt, "%%%is%%-8s%%d, $00\n", LABEL_PAD);
      //"%-17s%-8s%d, $FF\n"
      printf(frmt, symbol, ".fill", num00s);
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
    //printf("\nBytes to print: %i\n", bytesToPrint);

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
