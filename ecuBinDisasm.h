#define ROM_START 0x8000
#define MNEMONIC_LEN 6
#define OP_TABLE_SZ 0x100
#define FORMATS_NUM 11
#define SUB_OP_NUM 41
#define SYM_BUF_MAX 128
#define OP_SYMBOLS_MAX 3
#define ORG_MAX 4
#define RAS_MAX 50
#define RAW_BYTES_PAD "            "
#define LABEL_PAD -20

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   uint;

//
// Argp stuff
//
const char *argp_program_version = "7675disassem 0.1";
const char *argp_program_bug_address = "<janehacker1@gmail.com>";
static char doc[] = "Disassembler for the MH6111/TMP76C75T(7675)";
static char args_doc[] = "<BINARY FILE> [SYMBOL FILE]";
static struct argp_option options[] = {
    { "linenumbers", 'l', 0, OPTION_ARG_OPTIONAL, "Print line numbers."},
    { "rawbytes",    'r', 0, OPTION_ARG_OPTIONAL, "Print raw bytes."},
    { 0 }
};

struct arguments {
    bool lineNumbers;
    bool rawBytes;
    char *args[2];
    char *option1;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
      case 'l': arguments->lineNumbers = 1; break;
      case 'r': arguments->rawBytes = 1; break;
      case ARGP_KEY_ARG:
        // Too many arguments, if your program expects only one argument.
        if(state->arg_num > 2)
          argp_usage(state);
        arguments->args[state->arg_num] = arg;
      break;
      case ARGP_KEY_END:
        // Not enough arguments. if your program expects exactly one argument.
        if(state->arg_num < 1)
          argp_usage(state);
      break;
      default:
        return ARGP_ERR_UNKNOWN;
      break;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

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
uint numSymbols = 0;

// Buffer we read the entire binary file into
byte* binBuffer;
byte symBuffer[SYM_BUF_MAX];

// Used for generated labels (RELATIVE for now)
uint generatedLabel = 4003;


romAreaStruct ras[RAS_MAX] = {0};
uint rasIndex = 0;
ROMArea ra = DATA;

word orgTable[ORG_MAX];

word skipArray[ORG_MAX];
uint skipArrayLen = 0;


// Function definitions
uint loadEcuBinFile(char *binFilename);

uint    loadSymbolFile(char *symFilename);
bool    addSymbol(word address, ROMArea ra, char* symbol);
void    generateRelativeSymbols(word binCurrPos, opUnion ou, byte* buffPtr, bool isSubOp);
char*   getSymbol(word address);
word    getOpSymbol(word binCurrPos, opUnion op, byte* buffPtr, bool* chkFlg);
void    printSymbols();

uint    addSubOps();
bool    getSubOp(operation currOp, byte* buffPtr, subOp* currSubOp);
uint    freeSubOps();

word    decodeOpAndJump(word binCurrPos, byte * buffPtr);
byte*   decodeOp(word binCurrPos, byte * buffPtr, decodeType dt, bool lineNumbers, bool rawBytes);
void    printOp(word binCurrPos, opUnion oper, byte* buffPtr, bool lineNumbers, bool rawBytes);
void    subOpPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol);
void    opPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol, word binCurrPos);

ROMArea getRomArea(word binCurrPos, ROMArea ra, bool reset);
void    updateRomArea(byte* buffPtr, ROMArea* ra, bool reset);
uint    bytesToNextSection(byte* buffPtr);
byte*   printData(word binCurrPos, byte* buffPtr, bool LineNumbers, bool rawBytes);
uint    bytesToNextLabel(byte* buffPtr);
void    sortSymbols();
word    getBinPos(byte* buffPtr);
void    printRamVariables(bool lineNumber, bool rawBytes);
void    addOrg(word address);
void    doOrg(word binCurrPos, char** symbol, bool lineNumbers, bool rawBytes);
void    printRaw(byte* buffPtr, uint numBytes);
bool    inSkipArray(word opWord);

void buildRomAreaStruct();

uint loadEcuBinFile(char *binFilename) {
  FILE*   ecuBin;
  long    binSize;
  size_t  bytesRead;

  ecuBin = fopen(binFilename, "rb");

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

word getBinPos(byte* buffPtr) {
  return ROM_START + (int)(buffPtr - binBuffer);
}

int cmpSymbols(const void * a, const void * b) {
   return ( ((symbolStruct*)a)->addr - ((symbolStruct*)b)->addr );
}

void sortSymbols() {
  qsort(symbolTable, numSymbols, sizeof(symbolStruct), cmpSymbols);
}

void updateRomArea(byte* buffPtr, ROMArea* ra, bool reset) {
  word binCurrPos = getBinPos(buffPtr);
  *ra = getRomArea(binCurrPos, *ra, reset);
}

uint bytesToNextSection(byte* buffPtr) {
  uint ret = 0;
  word binCurrPos = getBinPos(buffPtr);

  for(int i = 0; i < RAS_MAX; i++) {
    if(binCurrPos > ras[i].address) {
      continue;
    } else {
      ret = ras[i].address - binCurrPos;
      break;
    }
  }

  if(0 == ret) {
    ret = bytesToNextSection(buffPtr + 1);
  }

  return ret;
}

uint bytesToNextLabel(byte* buffPtr) {
  //static uint lastWasZero = 0;
  uint ret = 0;
  word binCurrPos = getBinPos(buffPtr);

  for(int i = 0; i < numSymbols; i++) {
    if(binCurrPos < symbolTable[i].addr) {
      ret = symbolTable[i].addr - binCurrPos;
      break;
    }
  }

  return ret;
}

ROMArea getRomArea(word binCurrPos, ROMArea ra, bool reset) {
  ROMArea ret = ra;

  for(int i = 0; i < rasIndex; i++) {
    if(binCurrPos == ras[i].address) {
      ret = ras[i].area;
      break;
    } else if(binCurrPos < ras[i].address) {
      ret = ras[i-1].area;
      break;
    }
  }

  return ret;
}
