//
// Symbols
//
bool addSymbol(word address, ROMArea ra, char* symbol) {
  symbolStruct* tempSymbolTable = (symbolStruct*)malloc(sizeof(symbolStruct) * (numSymbols + 1)); // End of main

  if(0 != numSymbols) {
    memcpy(tempSymbolTable, symbolTable, sizeof(symbolStruct) * numSymbols);
    free(symbolTable);
  }

  //printf("\nAdding symbol: %-10s, at line: %04x\n", symbol, address);

  tempSymbolTable[numSymbols].addr  = address;
  tempSymbolTable[numSymbols].area = ra;
  strcpy(tempSymbolTable[numSymbols].label, symbol);

  symbolTable = tempSymbolTable;
  numSymbols++;

  return 1;
}

bool addValidSymAddr(word address) {
  word* tempVSA = (word*)malloc(sizeof(word) * (validSymAddrSz + 1)); // End of main

  if(0 != validSymAddrSz) {
    memcpy(tempVSA, validSymAddr, sizeof(word) * validSymAddrSz);
    free(validSymAddr);
  }

  tempVSA[validSymAddrSz] = address;

  validSymAddr = tempVSA;
  validSymAddrSz++;

  return 1;
}

char* getSymbol(word address) {
  char* ret = "\0";

  for(int i=0; i < numSymbols; i++) {
    symbolStruct* ss = &symbolTable[i];

    if(address == ss->addr) {
      ret = ss->label;
      break;
    }
  }

  return ret;
}

uint loadSymbolFile(char *symFilename) {
  FILE*   symFile;
  long    symSize;
  size_t  bytesRead;

  symFile = fopen(symFilename, "r");

  if(symFile == NULL) {
    fputs("\nFile error\n", stderr);
    exit(1);
  }

  // obtain file size:
  fseek(symFile, 0, SEEK_END);
  symSize = ftell(symFile);
  rewind(symFile);

  // read a line into the buffer
  while(fgets((char*)symBuffer, SYM_BUF_MAX, symFile)) {
    word symAddress;
    char symTypeStr[8]    = {0};
    char symLabelStr[32]  = {0};

    // If this line isn't a comment
    if(';' != symBuffer[0]) {
      // sSCANf it
      sscanf((const char*)symBuffer, "%s\t%04x\t\t%s", symTypeStr,
        (uint*)&symAddress, symLabelStr);

      if(0 == strcmp("code", symTypeStr) || 0 == strcmp("reg", symTypeStr)) {
        addSymbol(symAddress, CODE, symLabelStr);
      } else if(0 == strcmp("data", symTypeStr)) {
        addSymbol(symAddress, DATA, symLabelStr);
      } else if(0 == strcmp("org", symTypeStr)) {
        addOrg(symAddress);
      } else if(0 == strcmp("vector", symTypeStr)) {
        addSymbol(symAddress, VECTOR, symLabelStr);
      } else if(0 == strcmp("skip", symTypeStr)) {
        skipArray[skipArrayLen++] = symAddress;
      }
    }
  }

  fclose(symFile);

  return bytesRead;
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

      if(
        VALID_ROM_START < opWord
        && !inSkipArray(binCurrPos)
        && CODE != getRomArea(opWord, CODE, 0)
      ) {
        char* symbol    = getSymbol(opWord);

        // Make up a label
        if('\0' == symbol[0]) {
          symbol = (char*)malloc(sizeof(char) * MNEMONIC_LEN);
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
      if(
        '\0' == symbol[0]
        && !inSkipArray(binCurrPos)
        //&& DATA != getRomArea(opWord, DATA, 0)
      ) {
        symbol = (char*)malloc(sizeof(char) * MNEMONIC_LEN);
        // Make up a label for the symbol
        sprintf(symbol, "L%04X", opWord); //"L%i", generatedLabel++);
        // Add the made up symbol to the symbol list
        addSymbol(opWord, CODE, symbol);
      }
    } else if(DIRECT3 == currOp.type) {
      // Signed char for two's compliment
      word  opWord    = binCurrPos + (char)buffPtr[3] + 0x0004;
      char* symbol    = getSymbol(opWord);

      // Make up a label
      if('\0' == symbol[0] && !inSkipArray(binCurrPos)) {
        symbol = (char*)malloc(sizeof(char) * MNEMONIC_LEN);
        // Make up a label for the symbol
        sprintf(symbol, "L%04X", opWord); //"L%i", generatedLabel++);
        // Add the made up symbol to the symbol list
        addSymbol(opWord, CODE, symbol);
      }
    } else if(IMMEDIATE16 == currOp.type) {
      word  opWord    = (buffPtr[1] << 8) + buffPtr[2];

      if(
        VALID_ROM_START < opWord
        && ROM_END > opWord
        && !inSkipArray(binCurrPos)
        && CODE != getRomArea(opWord, CODE, 0)
      ) {
        char* symbol    = getSymbol(opWord);

        // Make up a label
        if('\0' == symbol[0]) {
          symbol = (char*)malloc(sizeof(char) * MNEMONIC_LEN);
          // Make up a label for the symbol
          sprintf(symbol, "T%i", generatedLabel++);
          // Add the made up symbol to the symbol list
          addSymbol(opWord, DATA, symbol);
        }
      }
    } else if(0xBD == buffPtr[0]) {
      word  opWord    = (buffPtr[1] << 8) + buffPtr[2];

      if(
        VALID_ROM_START < opWord
        && ROM_END > opWord
        && !inSkipArray(binCurrPos)
        && DATA != getRomArea(opWord, DATA, 0)
      ) {
        char* symbol    = getSymbol(opWord);

        // Make up a label
        if('\0' == symbol[0]) {
          symbol = (char*)malloc(sizeof(char) * MNEMONIC_LEN);
          // Make up a label for the symbol
          sprintf(symbol, "SUBROUTINE%i", generatedLabel++);
          // Add the made up symbol to the symbol list
          addSymbol(opWord, CODE, symbol);
        }
      }
    }
  }
}

void printSymbols() {
  for(int i=0; i < numSymbols; i++) {
    printf("%04x:\t%i\t%-8s\n", symbolTable[i].addr, (int)symbolTable[i].area, symbolTable[i].label);
  }
}

word getOpSymbol(word binCurrPos, opUnion op, byte* buffPtr, bool* chkFlg) {
  word opWord = 0;
  operation currOp = op.op;

  // Direct is 1-byte address, EXTENDED is 2-byte address
  switch(currOp.type) {
    case RELATIVE:
      // Calculate RELATIVE address
      opWord = binCurrPos + (char)buffPtr[1] + 0x0002; // Signed char for two's compliment
      *chkFlg = 1;
    break;
    case DIRECT:
    case DIRECT2:
      opWord = buffPtr[currOp.isSubOp ? 2 : 1];
      *chkFlg = 1;
    break;
    case EXTENDED:
      opWord = (buffPtr[currOp.isSubOp ? 2 : 1] << 8) + buffPtr[currOp.isSubOp ? 3 : 2];
      *chkFlg = 1;
    break;
    case IMMEDIATE16:
      opWord = (buffPtr[currOp.isSubOp ? 2 : 1] << 8) + buffPtr[currOp.isSubOp ? 3 : 2];
      *chkFlg = 1;
    break;
    default:
    ;
    break;
  };

  return opWord;
}

//
// org
//
void addOrg(word address) {
  static int i = 0;

  orgTable[i++] = address;
}

void doOrg(word binCurrPos, char** symbol, bool lineNumbers, bool rawBytes) {
  // Origin (.org)
  for(int i = 0; i < ORG_MAX; i++) {
    if(orgTable[i] == binCurrPos) {
      if(rawBytes) {
        printf("            ");
      }
      char frmt[16] = "\0";
      sprintf(frmt, "%%%is%%-8s$%%04x\n", LABEL_PAD);
      // `.org`s should come first with a label
      //"%-17s%-8s$%04x\n"
      printf(frmt, *symbol, ".org", binCurrPos);

      if(lineNumbers) {
        printf("%04X ", binCurrPos);
      }

      // Clear the label for the rest of this binCurrPos
      *symbol = "\0";
    }
  }
}

//
// Sorting
//
int cmpSymbols(const void * a, const void * b) {
   return ( ((symbolStruct*)a)->addr - ((symbolStruct*)b)->addr );
}

void sortSymbols() {
  qsort(symbolTable, numSymbols, sizeof(symbolStruct), cmpSymbols);
}

//
// Helper
//
bool inSkipArray(word opWord) {
  bool ret = 0;

  for(int i=0; i < skipArrayLen; i++) {
    if(skipArray[i] == opWord) {
      ret = 1;
      break;
    }
  }

  return ret;
}
