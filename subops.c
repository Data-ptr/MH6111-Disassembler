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
