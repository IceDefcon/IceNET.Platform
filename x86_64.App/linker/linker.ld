/*                         */
/* Author: Ice.Marek       */
/* 2023 IceNET Technology  */
/*                         */
SECTIONS
{
  . = 0x08000000;  .text : { *(.text) }
  . = 0x08001000;  .data : { *(.data) }
  . = 0x09000000;  .bss  : { *(.bss)  }
}