operation opTable[OP_TABLE_SZ] = {
  {"test",    1, IMPLIED,   0, 0},   //0x00
  {"nop",     1, IMPLIED,   0, 0}, {"andm",    3, DIRECT2,     0, 0},
  {"orm",     3, DIRECT2,   0, 0}, {"lsrd",    1, IMPLIED,     0, 0},
  {"asld",    1, IMPLIED,   0, 0}, {"tap",     1, IMPLIED,     0, 0},
  {"tpa",     1, IMPLIED,   0, 0}, {"inx",     1, IMPLIED,     0, 0},
  {"dex",     1, IMPLIED,   0, 0}, {"clv",     1, IMPLIED,     0, 0},
  {"sev",     1, IMPLIED,   0, 0}, {"clc",     1, IMPLIED,     0, 0},
  {"sec",     1, IMPLIED,   0, 0}, {"cli",     1, IMPLIED,     0, 0},
  {"sei",     1, IMPLIED,   0, 0},   //0x0f

  {"sba",     1, IMPLIED,   0, 0},   //0x10
  {"cba",     1, IMPLIED,   0, 0}, {"0x12",    0, IMPLIED,     0, 0},
  {"brclr2",  4, DIRECT3,   0, 0}, {"div",     2, DIRECT,      0, 0}, //0x13 MH6211
//  {"0x13",    0, IMPLIED,   0, 0}, {"div",     2, DIRECT,      0, 0},
  {"div",     2, DIRECT,    0, 0}, {"tab",     1, IMPLIED,     0, 0},
  {"tba",     1, IMPLIED,   0, 0}, {"xgxy",    1, IMPLIED,     0, 0},
  {"daa",     1, IMPLIED,   0, 0}, {"xgdx",    1, IMPLIED,     0, 0},
  {"aba",     1, IMPLIED,   0, 0}, {"cpd",     3, IMMEDIATE16, 0, 0},
  {"cmpd1",   2, DIRECT,    0, 0}, {"cpd",     2, INDEXED,     0, 0},
  {"cpd",     3, EXTENDED,  0, 0},   //0x1f

  {"bra",     2, RELATIVE,  0, 0},   //0x20
  {"brn",     2, RELATIVE,  0, 0}, {"bhi",     2, RELATIVE,    0, 0},
  {"bls",     2, RELATIVE,  0, 0}, {"bcc",     2, RELATIVE,    0, 0},
  {"bcs",     2, RELATIVE,  0, 0}, {"bne",     2, RELATIVE,    0, 0},
  {"beq",     2, RELATIVE,  0, 0}, {"bvc",     2, RELATIVE,    0, 0},
  {"bvs",     2, RELATIVE,  0, 0}, {"bpl",     2, RELATIVE,    0, 0},
  {"bmi",     2, RELATIVE,  0, 0}, {"bge",     2, RELATIVE,    0, 0},
  {"blt",     2, RELATIVE,  0, 0}, {"bgt",     2, RELATIVE,    0, 0},
  {"ble",     2, RELATIVE,  0, 0},   //0x2f

  {"tsx",     1, IMPLIED,   0, 0},   //0x30
  {"ins",     1, IMPLIED,   0, 0}, {"pula",    1, IMPLIED,     0, 0},
  {"pulb",    1, IMPLIED,   0, 0}, {"des",     1, IMPLIED,     0, 0},
  {"txs",     1, IMPLIED,   0, 0}, {"psha",    1, IMPLIED,     0, 0},
  {"pshb",    1, IMPLIED,   0, 0}, {"pulx",    1, IMPLIED,     0, 0},
  {"rts",     1, IMPLIED,   0, 0}, {"abx",     1, IMPLIED,     0, 0},
  {"rti",     1, IMPLIED,   0, 0}, {"pshx",    1, IMPLIED,     0, 0},
  {"mul",     1, IMPLIED,   0, 0}, {"wai",     1, IMPLIED,     0, 0},
  {"swi",     1, IMPLIED,   0, 0},   //0x3f

  {"nega",    1, IMPLIED,   0, 0},   //0x40
  {"0x41",    0, IMPLIED,   0, 0}, {"0x42",    0, IMPLIED,     0, 0},
  {"coma",    1, IMPLIED,   0, 0}, {"lsra",    1, IMPLIED,     0, 0},
  {"0x45",    0, IMPLIED,   0, 0}, {"rora",    1, IMPLIED,     0, 0},
  {"asra",    1, IMPLIED,   0, 0}, {"asla",    1, IMPLIED,     0, 0},
  {"rola",    1, IMPLIED,   0, 0}, {"deca",    1, IMPLIED,     0, 0},
  {"0x4b",    0, IMPLIED,   0, 0}, {"inca",    1, IMPLIED,     0, 0},
  {"tsta",    1, IMPLIED,   0, 0}, {"0x4e",    0, IMPLIED,     0, 0},
  {"clra",    1, IMPLIED,   0, 0},   //0x4f

  {"negb",    1, IMPLIED,   0, 0},   //0x50
  {"0x51",    0, IMPLIED,   0, 0}, {"0x52",    0, IMPLIED,     0, 0},
  {"comb",    1, IMPLIED,   0, 0}, {"lsrb",    1, IMPLIED,     0, 0},
  {"0x55",    0, IMPLIED,   0, 0}, {"rorb",    1, IMPLIED,     0, 0},
  {"asrb",    1, IMPLIED,   0, 0}, {"aslb",    1, IMPLIED,     0, 0},
  {"rolb",    1, IMPLIED,   0, 0}, {"decb",    1, IMPLIED,     0, 0},
  {"0x5b",    0, IMPLIED,   0, 0}, {"incb",    1, IMPLIED,     0, 0},
  {"tstb",    1, IMPLIED,   0, 0}, {"0x5e",    0, IMPLIED,     0, 0},
  {"clrb",    1, IMPLIED,   0, 0},   //0x5f

  {"neg",     2, INDEXED,   0, 0},   //0x60
  {"0x61",    0, INDEXED,   0, 0}, {"0x62",    0, INDEXED,     0, 0},
  {"com",     2, INDEXED,   0, 0}, {"lsr",     2, INDEXED,     0, 0},
  {"0x65",    0, INDEXED,   0, 0}, {"ror",     2, INDEXED,     0, 0},
  {"asr",     2, INDEXED,   0, 0}, {"asl",     2, INDEXED,     0, 0},
  {"rol",     2, INDEXED,   0, 0}, {"dec",     2, INDEXED,     0, 0},
  {"0x6b",    0, INDEXED,   0, 0}, {"inc",     2, INDEXED,     0, 0},
  {"tst",     2, INDEXED,   0, 0}, {"jmp",     2, INDEXED,     0, 0},
  {"clr",     2, INDEXED,   0, 0},   //0x6f

  {"neg",     3, EXTENDED,  0, 0},   //0x70
  {"0x71",    0, EXTENDED,  0, 0}, {"0x72",    0, EXTENDED,    0, 0},
  {"com",     3, EXTENDED,  0, 0}, {"lsr",     3, EXTENDED,    0, 0},
  {"0x75",    0, EXTENDED,  0, 0}, {"ror",     3, EXTENDED,    0, 0},
  {"asr",     3, EXTENDED,  0, 0}, {"asl",     3, EXTENDED,    0, 0},
  {"rol",     3, EXTENDED,  0, 0}, {"dec",     3, EXTENDED,    0, 0},
  {"0x7b",    0, EXTENDED,  0, 0}, {"inc",     3, EXTENDED,    0, 0},
  {"tst",     3, EXTENDED,  0, 0}, {"jmp",     3, EXTENDED,    0, 0},
  {"clr",     3, EXTENDED,  0, 0},   //0x7f

  {"suba",    2, IMMEDIATE, 0, 0},   //0x80
  {"cmpa",    2, IMMEDIATE, 0, 0}, {"sbca",    2, IMMEDIATE,   0, 0},
  {"subd",    3, IMMEDIATE16, 0, 0}, {"anda",    2, IMMEDIATE,   0, 0},
  {"bita",    2, IMMEDIATE, 0, 0}, {"ldaa",    2, IMMEDIATE,   0, 0},
  {"brset",   4, DIRECT3,   0, 0}, {"eora",    2, IMMEDIATE,   0, 0},
  {"adca",    2, IMMEDIATE, 0, 0}, {"oraa",    2, IMMEDIATE,   0, 0},
  {"adda",    2, IMMEDIATE, 0, 0}, {"cpx",     3, IMMEDIATE16, 0, 0},
  {"bsr",     2, RELATIVE,  0, 0}, {"lds",     3, IMMEDIATE16, 0, 0},
  {"brclr",   4, DIRECT3,   0, 0},   //0x8f

  {"suba",    2, DIRECT,    0, 0},   //0x90
  {"cmpa",    2, DIRECT,    0, 0}, {"sbca",    2, DIRECT,    0, 0},
  {"subd",    2, DIRECT,    0, 0}, {"anda",    2, DIRECT,    0, 0},
  {"bita",    2, DIRECT,    0, 0}, {"ldaa",    2, DIRECT,    0, 0},
  {"staa",    2, DIRECT,    0, 0}, {"eora",    2, DIRECT,    0, 0},
  {"adca",    2, DIRECT,    0, 0}, {"oraa",    2, DIRECT,    0, 0},
  {"adda",    2, DIRECT,    0, 0}, {"cpx",     2, DIRECT,    0, 0},
  {"jsr",     2, DIRECT,    0, 0}, {"lds",     2, DIRECT,    0, 0},
  {"sts",     2, DIRECT,    0, 0},   //0x9f

  {"suba",    2, INDEXED,   0, 0},   //0xa0
  {"cmpa",    2, INDEXED,   0, 0}, {"sbca",    2, INDEXED,   0, 0},
  {"subd",    2, INDEXED,   0, 0}, {"anda",    2, INDEXED,   0, 0},
  {"bita",    2, INDEXED,   0, 0}, {"ldaa",    2, INDEXED,   0, 0},
  {"staa",    2, INDEXED,   0, 0}, {"eora",    2, INDEXED,   0, 0},
  {"adca",    2, INDEXED,   0, 0}, {"oraa",    2, INDEXED,   0, 0},
  {"adda",    2, INDEXED,   0, 0}, {"cpx",     2, INDEXED,   0, 0},
  {"jsr",     2, INDEXED,   0, 0}, {"lds",     2, INDEXED,   0, 0},
  {"sts",     2, INDEXED,   0, 0},   //0xaf

  {"suba",    3, EXTENDED,  0, 0},   //0xb0
  {"cmpa",    3, EXTENDED,  0, 0}, {"sbca",    3, EXTENDED,  0, 0},
  {"subd",    3, EXTENDED,  0, 0}, {"anda",    3, EXTENDED,  0, 0},
  {"bita",    3, EXTENDED,  0, 0}, {"ldaa",    3, EXTENDED,  0, 0},
  {"staa",    3, EXTENDED,  0, 0}, {"eora",    3, EXTENDED,  0, 0},
  {"adca",    3, EXTENDED,  0, 0}, {"oraa",    3, EXTENDED,  0, 0},
  {"adda",    3, EXTENDED,  0, 0}, {"cpx",     3, EXTENDED,  0, 0},
  {"jsr",     3, EXTENDED,  0, 0}, {"lds",     3, EXTENDED,  0, 0},
  {"sts",     3, EXTENDED,  0, 0},   //0xbf

  {"subb",    2, IMMEDIATE,   0, 0}, //0xc0
  {"cmpb",    2, IMMEDIATE,   0, 0}, {"sbcb",  2, IMMEDIATE,   0, 0},
  {"addd",    3, IMMEDIATE16, 0, 0}, {"andb",  2, IMMEDIATE,   0, 0},
  {"bitb",    2, IMMEDIATE,   0, 0}, {"ldab",  2, IMMEDIATE,   0, 0},
  {"brset",   4, INDEXED,     0, 0}, {"eorb",  2, IMMEDIATE,   0, 0}, //0xC7 MH6211
  {"adcb",    2, IMMEDIATE,   0, 0}, {"orab",  2, IMMEDIATE,   0, 0},
  {"addb",    2, IMMEDIATE,   0, 0}, {"ldd",   3, IMMEDIATE16, 0, 0},
  {"0xcd",    0, IMMEDIATE,   0, 0}, {"ldx",   3, IMMEDIATE16, 0, 0},
  {"brclr",   4, DIRECT4,     0, 0}, //0xcf

  {"subb",    2, DIRECT,    0, 0},   //0xd0
  {"cmpb",    2, DIRECT,    0, 0}, {"sbcb",    2, DIRECT,    0, 0},
  {"addd",    2, DIRECT,    0, 0}, {"andb",    2, DIRECT,    0, 0},
  {"bitb",    2, DIRECT,    0, 0}, {"ldab",    2, DIRECT,    0, 0},
  {"stab",    2, DIRECT,    0, 0}, {"eorb",    2, DIRECT,    0, 0},
  {"adcb",    2, DIRECT,    0, 0}, {"orab",    2, DIRECT,    0, 0},
  {"addb",    2, DIRECT,    0, 0}, {"ldd",     2, DIRECT,    0, 0},
  {"std",     2, DIRECT,    0, 0}, {"ldx",     2, DIRECT,    0, 0},
  {"stx",     2, DIRECT,    0, 0},   //0xdf

  {"subb",    2, INDEXED,   0, 0},   //0xe0
  {"cmpb",    2, INDEXED,   0, 0}, {"sbcb",    2, INDEXED,   0, 0},
  {"addd",    2, INDEXED,   0, 0}, {"andb",    2, INDEXED,   0, 0},
  {"bitb",    2, INDEXED,   0, 0}, {"ldab",    2, INDEXED,   0, 0},
  {"stab",    2, INDEXED,   0, 0}, {"eorb",    2, INDEXED,   0, 0},
  {"adcb",    2, INDEXED,   0, 0}, {"orab",    2, INDEXED,   0, 0},
  {"addb",    2, INDEXED,   0, 0}, {"ldd",     2, INDEXED,   0, 0},
  {"std",     2, INDEXED,   0, 0}, {"ldx",     2, INDEXED,   0, 0},
  {"stx",     2, INDEXED,   0, 0},   //0xef

  {"subb",    3, EXTENDED,  0, 0},   //0xf0
  {"cmpb",    3, EXTENDED,  0, 0}, {"sbcb",    3, EXTENDED,  0, 0},
  {"addd",    3, EXTENDED,  0, 0}, {"andb",    3, EXTENDED,  0, 0},
  {"bitb",    3, EXTENDED,  0, 0}, {"ldab",    3, EXTENDED,  0, 0},
  {"stab",    3, EXTENDED,  0, 0}, {"eorb",    3, EXTENDED,  0, 0},
  {"adcb",    3, EXTENDED,  0, 0}, {"orab",    3, EXTENDED,  0, 0},
  {"addb",    3, EXTENDED,  0, 0}, {"ldd",     3, EXTENDED,  0, 0},
  {"std",     3, EXTENDED,  0, 0}, {"ldx",     3, EXTENDED,  0, 0},
  {"stx",     3, EXTENDED,  0}    //0xff
};

subOpDef soDefs[SUB_OP_NUM] = {
    {0xcd, {"iny",  2, IMPLIED,     1, 0x08}},
    {0xcd, {"dey",  2, IMPLIED,     1, 0x09}},
    {0xcd, {"xgdy", 2, IMPLIED,     1, 0x1a}},
    {0xcd, {"aby",  2, IMPLIED,     1, 0x3a}},
    {0xcd, {"cmpy", 4, IMMEDIATE16, 1, 0x8c}},
    {0xcd, {"cpd",  2, INDEXEDY,    1, 0xa3}}, // MH6211
    {0xcd, {"cpx",  2, INDEXEDY,    1, 0xac}}, // MH6211
    {0xcd, {"ldy",  4, IMMEDIATE16, 1, 0xce}},
    {0xcd, {"sty",  3, DIRECT,      1, 0xdf}},
    {0xcd, {"ldy",  3, DIRECT,      1, 0xde}}, // MH6211 - undocumented
    {0xcd, {"ldy",  3, INDEXED,     1, 0xee}},
    {0xcd, {"stx",  2, INDEXEDY,    1, 0xef}}, // MH6211
    {0xcd, {"ldy",  2, EXTENDED,    1, 0xfe}}, // MH6211
    {0xa0, {"suba", 2, INDEXEDY,    1, 0x80}},
    {0xa1, {"cmpa", 2, INDEXEDY,    1, 0x80}},
    {0xa2, {"sbca", 2, INDEXEDY,    1, 0x80}},
    {0xa3, {"subd", 2, INDEXEDY,    1, 0x80}},
    {0xa4, {"anda", 2, INDEXEDY,    1, 0x80}},
    {0xa5, {"bita", 2, INDEXEDY,    1, 0x80}},
    {0xa6, {"ldaa", 2, INDEXEDY,    1, 0x80}},
    {0xa7, {"staa", 2, INDEXEDY,    1, 0x80}},
    {0xa8, {"eora", 2, INDEXEDY,    1, 0x80}},
    {0xa9, {"adca", 2, INDEXEDY,    1, 0x80}},
    {0xaa, {"oraa", 2, INDEXEDY,    1, 0x80}},
    {0xab, {"adda", 2, INDEXEDY,    1, 0x80}},
    {0xac, {"cpx",  2, INDEXEDY,    1, 0x80}},
    {0xad, {"jsr",  2, INDEXEDY,    1, 0x80}},
    {0xae, {"lds",  2, INDEXEDY,    1, 0x80}},
    {0xaf, {"sts",  2, INDEXEDY,    1, 0x80}},
    {0xe0, {"subb", 2, INDEXEDY,    1, 0x80}},
    {0xe1, {"cmpb", 2, INDEXEDY,    1, 0x80}},
    {0xe2, {"sbcb", 2, INDEXEDY,    1, 0x80}},
    {0xe3, {"addd", 2, INDEXEDY,    1, 0x80}},
    {0xe4, {"andb", 2, INDEXEDY,    1, 0x80}},
    {0xe5, {"bitb", 2, INDEXEDY,    1, 0x80}},
    {0xe6, {"ldab", 2, INDEXEDY,    1, 0x80}},
    {0xe7, {"stab", 2, INDEXEDY,    1, 0x80}},
    {0xe8, {"eorb", 2, INDEXEDY,    1, 0x80}},
    {0xe9, {"adcb", 2, INDEXEDY,    1, 0x80}},
    {0xea, {"orab", 2, INDEXEDY,    1, 0x80}},
    {0xeb, {"addb", 2, INDEXEDY,    1, 0x80}},
    {0xec, {"ldd",  2, INDEXEDY,    1, 0x80}},
    {0xed, {"std",  2, INDEXEDY,    1, 0x80}},
    {0xee, {"ldx",  2, INDEXEDY,    1, 0x80}},
    {0xef, {"stx",  2, INDEXEDY,    1, 0x80}}
  };
