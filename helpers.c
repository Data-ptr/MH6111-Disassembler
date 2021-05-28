//
// Helpers
//
word getBinPos(byte* buffPtr) {
  return ROM_START + (int)(buffPtr - binBuffer);
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
