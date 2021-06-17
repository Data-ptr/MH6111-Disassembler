#define ROM_START 0x8000
#define ROM_END 0xFFFF
#define VALID_ROM_START 0xD000
#define MNEMONIC_LEN 6
#define OP_TABLE_SZ 0x100
#define FORMATS_NUM 11
#define SUB_OP_NUM 46
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
const         char       *argp_program_version = "76XXdisassem 0.1";
const         char       *argp_program_bug_address = "<janehacker1@gmail.com>";
static        char        doc[] = "Disassembler for the MH6X11/TMP76XX(76XX)";
static        char        args_doc[] = "<BINARY FILE> <SYMBOL FILE>";
static struct argp_option options[] = {
    { "linenumbers", 'l', 0,      OPTION_ARG_OPTIONAL,
      "Print line numbers" },
    { "rawbytes",    'r', 0,      OPTION_ARG_OPTIONAL,
      "Print raw bytes" },
    { "rom-start",   's', "ADDR", 0,
      "Address (HEX) of ROM start          Default:0x8000" },
    { "valid-rom",   'v', "ADDR", 0,
      "Address (HEX) of start of valid ROM Default:0xD000" },
    { 0 }
};

struct arguments {
    bool lineNumbers;
    bool rawBytes;
    word romStart;
    word validRomStart;
    char *args[2];
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
      case 'l': arguments->lineNumbers = 1; break;
      case 'r': arguments->rawBytes = 1; break;
      case 's':
        arguments->romStart = strtol(arg, NULL, 16);
      break;
      case 'v':
        arguments->validRomStart = strtol(arg, NULL, 16);
      break;
      case ARGP_KEY_ARG:
        // Too many arguments, if your program expects only one argument.
        if(state->arg_num > 2)
          argp_usage(state);
        arguments->args[state->arg_num] = arg;
      break;
      case ARGP_KEY_END:
        // Not enough arguments. if your program expects exactly two argument.
        if(state->arg_num < 2)
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
  bool landed;      // Was this address used at the start of a "line"
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
uint generatedLabel = 1; //Set to where you want the symbol counting to start


romAreaStruct ras[RAS_MAX] = {0};
uint rasIndex = 0;
ROMArea ra = DATA;

word orgTable[ORG_MAX];

word skipArray[ORG_MAX];
uint skipArrayLen = 0;

word* validSymAddr;
uint validSymAddrSz = 0;

struct arguments storedArgs;


// Function definitions
uint loadEcuBinFile(char *binFilename);

// symbols.c
uint    loadSymbolFile(char *symFilename);
bool    addSymbol(word address, ROMArea ra, char* symbol);
void    generateRelativeSymbols(word binCurrPos, opUnion ou, byte* buffPtr, bool isSubOp);
char*   getSymbol(word address);
word    getOpSymbol(word binCurrPos, opUnion op, byte* buffPtr, bool* chkFlg);
void    sortSymbols();
void    printSymbols();
void    addOrg(word address);
void    doOrg(word binCurrPos, char** symbol, bool lineNumbers, bool rawBytes);
bool    inSkipArray(word opWord);
bool    addValidSymAddr(word address);

// decode.c
word    decodeOpAndJump(word binCurrPos, byte * buffPtr);
byte*   decodeOp(word binCurrPos, byte * buffPtr, decodeType dt, bool lineNumbers, bool rawBytes);

// output.c
void    printRaw(byte* buffPtr, uint numBytes);
void    printRamVariables(bool lineNumber, bool rawBytes);
void    printOp(word binCurrPos, opUnion oper, byte* buffPtr, bool lineNumbers, bool rawBytes);
void    subOpPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol);
void    opPrint(char* symbol, opUnion oper, byte* buffPtr, bool isSymbol, word binCurrPos);
byte*   printData(word binCurrPos, byte* buffPtr, bool LineNumbers, bool rawBytes);

// subops.c
uint    addSubOps();
bool    getSubOp(operation currOp, byte* buffPtr, subOp* currSubOp);
uint    freeSubOps();

// rom.c
void    buildRomAreaStruct();
ROMArea getRomArea(word binCurrPos, ROMArea ra, bool reset);
void    updateRomArea(byte* buffPtr, ROMArea* ra, bool reset);

//helpers.c
uint    bytesToNextLabel(byte* buffPtr);
uint    bytesToNextSection(byte* buffPtr);
word    getBinPos(byte* buffPtr);
word    getRomStart();
word    getValidRomStart();

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
