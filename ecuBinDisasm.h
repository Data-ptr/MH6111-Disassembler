#define ROM_START 0x8000
#define MNEMONIC_LEN 6
#define OP_TABLE_SZ 0x100
#define FORMATS_NUM 11
#define SUB_OP_NUM 41
#define SYM_BUF_MAX 128
#define OP_SYMBOLS_MAX 3
#define ORG_MAX 4

#define BIN_FILE "standard_E932_E931_source.obj" //"DSM_NA_EB20.bin"
#define SYM_FILE "e931.sym" //"eb20.sym"

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   uint;
typedef unsigned int   bool;

typedef enum {
  ORG,
  CODE,
  DATA,
  VECTOR
} ROMArea;

typedef enum {
  PRINT,
  SCAN
} decodeType;

// Primarily used to format the output properly (see: formats)
typedef enum {
  IMPLIED,
  IMMEDIATE,  IMMEDIATE16,
  DIRECT,     DIRECT2,    DIRECT3, DIRECT4,
  INDEXED,    INDEXEDY,
  EXTENDED,
  RELATIVE
} operatonType;

// Formats correspond with operatonTypes
char* formats[FORMATS_NUM] = {
  "",
  "#$%02x",   "#$%02x%02x",                             // IMMEDIATE
  "$%02x",    "$%02x, #$%02x",  "$%02x, #$%02x, $%02x", "$%04x, x, #$%02x, %s",// DIRECT
  "$%02x,x",  "$00,y",                                  // INDEXED
  "$%02x%02x",
  "$%02x"
};

// Multi-byte operations
typedef struct {
  char   mnemonic[MNEMONIC_LEN];
  uint   numBytes; // 1-4
  operatonType type;
  bool   isSubOp;
  byte   byte;
} subOp;

typedef struct {
  byte   parentByte;
  subOp  so;
} subOpDef;

typedef struct {
  char   mnemonic[MNEMONIC_LEN];
  uint  numBytes; // 1-4
  operatonType type;
  bool   isSubOp;
  uint  numSubOps;
  subOp* so;
} operation;

// Symbols
typedef struct {
  word addr;
  ROMArea area;
  char label[32];
} symbolStruct;

typedef union {
  operation op;
  subOp     subOp;
} opUnion;

typedef struct {
  word address;
  ROMArea area;
} romAreaStruct;

#include "operationTables.h"

symbolStruct* symbolTable;

// Buffer we read the entire binary file into
byte* binBuffer;
byte symBuffer[SYM_BUF_MAX];

// Used for generated labels (RELATIVE for now)
uint generatedLabel = 4001;

uint numSymbols = 0;

romAreaStruct ras[50] = {
  // {0xceff, CODE},
  // {0xcf02, DATA},
  // {0xd03e, CODE},
  // {0xe523, DATA},
  // {0xE56D, CODE},
  // {0xE852, DATA},
  // {0xE861, CODE},
  // {0xF60F, DATA},
  // {0xF613, CODE},
  // {0xFB0F, DATA}
};

word orgTable[ORG_MAX];

ROMArea ra = DATA;
uint rasIndex = 0;

// Function definitions
uint    loadEcuBinFile();

uint    loadSymbolFile();
bool    addSymbol(word address, ROMArea ra, char* symbol);
void    generateRelativeSymbols(word binCurrPos, operation currOp, byte* buffPtr);
char*   getSymbol(word address);
word    getOpSymbol(word binCurrPos, opUnion op, byte* buffPtr, bool* chkFlg);
void    printSymbols();

uint    addSubOps();
bool    getSubOp(operation currOp, byte* buffPtr, subOp* currSubOp);
uint    freeSubOps();

word    decodeOpAndJump(word binCurrPos, byte * buffPtr);
byte*   decodeOp(word binCurrPos, byte * buffPtr, decodeType dt, bool lineNumber);
void    printOp(word binCurrPos, opUnion oper, byte* buffPtr, bool lineNumber);
void    subOpPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol);
void    opPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol, word binCurrPos);

ROMArea getRomArea(word binCurrPos, ROMArea ra);
void    updateRomArea(byte* buffPtr, ROMArea* ra);
uint    bytesToNextSection(byte* buffPtr);
byte*   printData(word binCurrPos, byte* buffPtr, bool lineNumber);
uint    bytesToNextLabel(byte* buffPtr);
void    sortSymbols();
word    getBinPos(byte* buffPtr);
void    printRamVariables(bool lineNumber);
void    addOrg(word address);
void    doOrg(word binCurrPos, char** symbol, bool lineNumber);

void buildRomAreaStruct();

uint loadEcuBinFile() {
  FILE*   ecuBin;
  long    binSize;
  size_t  bytesRead;

  ecuBin = fopen(BIN_FILE, "rb");

  if(ecuBin == NULL) {
    fputs("\nFile error\n", stderr);
    exit(1);
  }

  // obtain file size:
  fseek(ecuBin, 0, SEEK_END);
  binSize = ftell(ecuBin);
  rewind(ecuBin);

  // allocate memory to contain the whole file:
  binBuffer = (byte*)malloc(sizeof(byte) * binSize);

  if(binBuffer == NULL) {
    fputs("\nMemory error\n", stderr);
    fclose(ecuBin);
    exit(2);
  }

  // copy the file into the buffer:
  bytesRead = fread(binBuffer, 1, binSize, ecuBin);

  if(bytesRead != binSize) {
    fputs("\nReading error\n", stderr);
    fclose(ecuBin);
    exit(3);
  }

  fclose(ecuBin);

  return bytesRead;
}


uint loadSymbolFile() {
  FILE*   symFile;
  long    symSize;
  size_t  bytesRead;

  symFile = fopen(SYM_FILE, "r");

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

    // sSCANf it
    sscanf((const char*)symBuffer, "%s\t%04x\t\t%s", symTypeStr,
      (uint*)&symAddress, symLabelStr);

    if(0 == strcmp("code", symTypeStr)) {
      addSymbol(symAddress, CODE, symLabelStr);
    } else if(0 == strcmp("data", symTypeStr)) {
      addSymbol(symAddress, DATA, symLabelStr);
    } else if(0 == strcmp("org", symTypeStr)) {
      addOrg(symAddress);
    } else if(0 == strcmp("vector", symTypeStr)) {
      addSymbol(symAddress, VECTOR, symLabelStr);
    }
  }

  fclose(symFile);

  return bytesRead;
}

//
// org
//
void addOrg(word address) {
  static int i = 0;

  orgTable[i++] = address;
}

void doOrg(word binCurrPos, char** symbol, bool lineNumber) {
  // Origin (.org)
  for(int i = 0; i < ORG_MAX; i++) {
    if(orgTable[i] == binCurrPos) {
      if(lineNumber) {
        printf("%04X ", binCurrPos);
      }
      // `.org`s should come firt with a label
      printf("%-17s .org\t$%04x\n", *symbol, binCurrPos);
      // Clear the label for the rest of this binCurrPos
      *symbol = "\0";
    }
  }
}

//
// Subops
//
uint addSubOps() {
  int i;

  for(i=0; i < SUB_OP_NUM; i++) {
    subOpDef    currDef       = soDefs[i];
    operation*  currOp        = &opTable[currDef.parentByte];
    uint        currNumSubOps = currOp->numSubOps++;

    // Cheap expansion of array
    if(0 == currNumSubOps) {
      currOp->so = (subOp*)malloc(sizeof(subOp)); // freeSubOp
    } else {
      subOp* tempSo = (subOp*)malloc(sizeof(subOp) * (currNumSubOps + 1)); // freeSubOp
      memcpy(tempSo, currOp->so, sizeof(subOp) * currNumSubOps);
      free(currOp->so);
      currOp->so = tempSo;
    }

    currOp->so[currNumSubOps] = currDef.so;
  }

  return i; // SubOps added
}

bool getSubOp(operation currOp, byte* buffPtr, subOp* currSubOp) {
  bool ret = 0;

  if(currOp.numSubOps > 0) {
    // Grab the next byte
    byte soByte = buffPtr[1];

    // See if it matches any of the sub ops
    for(int i=0; i < currOp.numSubOps; i++) {
      if(soByte == currOp.so[i].byte) {
        ret = 1;
        *currSubOp = currOp.so[i];
        break;
      }
    }
  }

  return ret;
}

uint freeSubOps() {
  int i;

  for(i=0; i < OP_TABLE_SZ; i++) {
    if(opTable[i].numSubOps > 0) {
      free(opTable[i].so);
    }
  }

  return i;
}

//
// Symbols
//

bool addSymbol(word address, ROMArea ra, char* symbol) {
  symbolStruct* tempSymbolTable = (symbolStruct*)malloc(sizeof(symbolStruct) * (numSymbols + 1)); // End of main

  if(0 != numSymbols) {
    memcpy(tempSymbolTable, symbolTable, sizeof(symbolStruct) * numSymbols);
    free(symbolTable);
  }

  tempSymbolTable[numSymbols].addr  = address;
  tempSymbolTable[numSymbols].area = ra;
  strcpy(tempSymbolTable[numSymbols].label, symbol);

  symbolTable = tempSymbolTable;
  numSymbols++;

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

void printSymbols() {
  for(int i=0; i < numSymbols; i++){
    printf("%04x:\t%i\t%-8s\n", symbolTable[i].addr, (int)symbolTable[i].area, symbolTable[i].label);
  }
}

void printRamVariables(bool lineNumber) {
  for(int i=0; ROM_START > symbolTable[i].addr; i++) {
    if(lineNumber){
      printf("0000 ");
    }

    printf("%-17s\t.EQU\t\t$%04x\n", symbolTable[i].label, symbolTable[i].addr);
  }
}

//
// Print ops
//
void printOp(word binCurrPos, opUnion oper, byte* buffPtr, bool lineNumber) {
  char* symbol = getSymbol(binCurrPos);

  doOrg(binCurrPos, &symbol, lineNumber);

  if(lineNumber){
    printf("%X %-17s %s\t", binCurrPos, symbol, oper.op.mnemonic);
  } else {
    printf("%-17s %s\t", symbol, oper.op.mnemonic);
  }

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

        if('\0' != opSymbols[0][0]) {
          printf("%s, #$%02x, %s", opSymbols[0], buffPtr[2], opSymbols[1]);
        } else {
          printf("$%02x, #$%02x, %s", buffPtr[1], buffPtr[2], opSymbols[1]);
        }
      } else if(DIRECT4 == currOp.type){
        opSymbolsBytes[1] = binCurrPos + (char)buffPtr[3] + 0x0004;
        opSymbols[1]      = getSymbol(opSymbolsBytes[1]); // Relative

        printf(formats[(int)currOp.type], buffPtr[1], buffPtr[2], opSymbols[1]);

      } else {
        printf(formats[(int)currOp.type], buffPtr[1], buffPtr[2], buffPtr[3]);
      }
    break;
  };
}

word getBinPos(byte* buffPtr) {
  return ROM_START + (int)(buffPtr - binBuffer);
}

int cmpSymbols(const void * a, const void * b) {
   return ( ((symbolStruct*)a)->addr - ((symbolStruct*)b)->addr );
}

void sortSymbols() {
  qsort(symbolTable, numSymbols, sizeof(symbolStruct), cmpSymbols);
}

void updateRomArea(byte* buffPtr, ROMArea* ra) {
  word binCurrPos = ROM_START + (int)(buffPtr - binBuffer);
  *ra = getRomArea(binCurrPos, *ra);
}

uint bytesToNextSection(byte* buffPtr) {
  uint ret = 0;
  word binCurrPos = ROM_START + (int)(buffPtr - binBuffer);

  for(int i = 0; i < 11; i++) {
    if(binCurrPos > ras[i].address) {
      continue;
    } else {
      ret = ras[i].address - binCurrPos;
      break;
    }
  }

  return ret;
}

uint bytesToNextLabel(byte* buffPtr) {
  uint ret = 0;
  word binCurrPos = ROM_START + (int)(buffPtr - binBuffer);

  for(int i = 0; i < numSymbols; i++) {
    if(binCurrPos > symbolTable[i].addr) {
      continue;
    } else {
      ret = symbolTable[i].addr - binCurrPos;
      break;
    }
  }

  if(0 == ret) {
    ret = bytesToNextLabel(buffPtr + 1) + 1;
  }

  //printf("Bytes to next label: %i\n", ret);

  return ret;
}

ROMArea getRomArea(word binCurrPos, ROMArea ra) {
  static int i = 0;

  if(binCurrPos == ras[i].address) {
    //printf("ROM Area: %s\n", CODE == ras[i].area ? "CODE" : "DATA");
    return ras[i++].area;
  }

  return ra;
}
