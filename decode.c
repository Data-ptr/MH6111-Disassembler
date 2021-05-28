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
