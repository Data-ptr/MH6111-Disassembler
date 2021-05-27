# MH6111 Disassembler
Disassembler for the Mitsubishi MH6111/Toshiba TMS76C75T.

##Objectives
- [x] Disassemble DSM ECU E931 binary that was assembled from the commented source
- [x] Make a bit perfect binary from the disassembly
- [x] Disasseble N/A ECU binary using symbol file by working through step by step against E931 commented source
- [x] Make a bit perfect binary from the disassembly
<s>
 -[ ] Disassemble MH2111 code
</s>

##Symbol files
* Format: TYPE\tBYTES\t\t[LABEL]
  - eg.: `code  E4A1    ecuInit`

###Types
* reg - for registers, but only for the sym file, syn: `code`
* `code`
  - process as code, line starting at address BYTES, with LABEL
* `data`
  - process as data, line starting at address BYTES, with LABEL
* `vector`
  - vector table, line starting at address BYTES, with LABEL
* `org`
  - origin (location counter)
* `skip`
  - do not generate labels for address BYTES

###Comments
* Lines starting with a semi-colon (;) are ignored

##About
This code was, let's be honest, hacked together simply to rip apart a binary and compare it to commented code (E931) and even produce a "usable" binary from it. Beyond that who knows how it may act.

Oh, and "usable" means bit-perfect to the original and NOT useable in an ECU in a running car. Especially if it is used to modify the code!

This is just a toy to learn.
