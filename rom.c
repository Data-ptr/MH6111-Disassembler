//
// ROM
//
void buildRomAreaStruct() {
  int i = 0;
  ROMArea lastRa = CODE;
  //uint rasIndex = 0;

  while(i < numSymbols) {
    // Skip ORGs
    if(ORG == symbolTable[i].area) {
      i++;
      continue;
    }

    // If the ROM area has changed
    if(lastRa != symbolTable[i].area) {
      // Update the ROM area we are no in
      lastRa = symbolTable[i].area;

      // Add this ROM area to the list
      ras[rasIndex].address = symbolTable[i].addr;
      ras[rasIndex].area = symbolTable[i].area;
      // Increment the list
      rasIndex++;
    }

    i++;
  }
}

ROMArea getRomArea(word binCurrPos, ROMArea ra, bool reset) {
  ROMArea ret = ra;

  // Scan through RAM AreaS
  for(int i = 0; i < rasIndex; i++) {
    // If we match the address, store as return and leave the loop
    if(binCurrPos == ras[i].address) {
      ret = ras[i].area;
      break;
    // Otherwise, if the next address in the array is past the address...
    } else if(binCurrPos < ras[i].address) {
      // ... store the previous address as return and leave loop
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
