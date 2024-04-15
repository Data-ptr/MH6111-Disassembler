# MH6X11 Disassembler
Disassembler for the Mitsubishi MH6X11/Toshiba TMP76XX.

## Objectives
- [x] Disassemble DSM ECU E931 binary that was assembled from the commented source
- [x] Make a bit perfect binary from the disassembly
- [x] Disasseble N/A ECU binary using symbol file by working through step by step against E931 commented source
- [x] Make a bit perfect binary from the disassembly
- [x] Disassemble MH6211 code
- [x] Disassemble MH6311 code


## Compile
```
cc -largp -o 7675Disassm ecuBinDisasm.c
```


## Usage
```
Usage: 7675Disassm [OPTION...] <BINARY FILE> <SYMBOL FILE>
Disassembler for the MH6X11/TMP76XX(76XX)

  -l, --linenumbers          Print line numbers
  -r, --rawbytes             Print raw bytes
  -s, --rom-start=ADDR       Address (HEX) of ROM start
                             Default:0x8000
  -v, --valid-rom=ADDR       Address (HEX) of start of valid ROM
                             Default:0xD000
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <janehacker1@gmail.com>.

```


## Execute
32k (ROM start 0x8000)
```
./7675Disassm standard_E932_E931_source.obj e931.sym
```

64k (ROM start 0x0000)
```
./7675Disassm -l -r -s0000 -v4000 binaries/EB23C.bin symbols/eb23c.sym > out/eb23c.lst
```


## Docs
* https://app.gitbook.com/@janehacker1/s/dsm-ecu/disassembly-from-scratch/things-you-need


## Symbol files
* Format: TYPE\tBYTES\t\t[LABEL]
  - eg.: `code  E4A1    ecuInit`


### Types
* `reg`
  - for registers, but only for the sym file, syn: `code`
* `code`
  - process as code, line starting at address BYTES, with LABEL
* `data`
  - process as data, line starting at address BYTES, with LABEL
* `vector`
  - vector table, line starting at address BYTES, with LABEL
* `org`
  - origin (location counter)
* `skip`
  - do not generate labels for operands at address BYTES


### Comments
* Lines starting with a semi-colon (;) are ignored


## About
This code was, let's be honest, hacked together simply to rip apart a binary and compare it to commented code (E931) and even produce a "usable" binary from it. Beyond that who knows how it may act.

Oh, and "usable" means bit-perfect to the original and NOT useable in an ECU in a running car. Especially if it is used to modify the code!

This is just a toy to learn.
