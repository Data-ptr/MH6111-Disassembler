//
// ROM
//
void buildRomAreaStruct() {
  int i = 0;
  ROMArea lastRa = CODE;
  //uint rasIndex = 0;

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

void updateRomArea(byte* buffPtr, ROMArea* ra, bool reset) {
  word binCurrPos = getBinPos(buffPtr);
  *ra = getRomArea(binCurrPos, *ra, reset);
}
