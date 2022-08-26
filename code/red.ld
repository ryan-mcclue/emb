/*  SPDX-License-Identifier: zlib-acknowledgement */

/* https://blog.thea.codes/the-most-thoroughly-commented-linker-script/ */

MEMORY
{
  flash (rx) : ORIGIN = 0x00000000, LENGTH = 256K 
  sram (rwx) : ORIGIN = 0x20000000, LENGTH = 32K 
}

__stack_size = 8K;

SECTIONS
{
  .text :
  {
    __stext = .;
    *(.text*)
    __etext = .;
  } > flash
  .bss (NOLOAD) :
  {
    __sbss = .;
    *(.bss*)
    __ebss = .;
  } > sram
  .data :
  {
    __sdata = .;
    *(.data*)
    __edata = .;
  } > sram AT > flash
  .stack (NOLOAD) :
  {
    . = ALIGN(8);
    __sstack = .;
    . = . + __stack_size;
    . = ALIGN(8);
    __estack = .;
  } > sram

  _end = .;

  /DISCARD/ : { *(*) }
}
