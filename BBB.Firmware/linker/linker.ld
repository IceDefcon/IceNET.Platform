/*                         */
/* Author: Ice.Marek       */
/* IceNET Technology 2023  */
/*                         */
SECTIONS
{
  . = 0x2000000;  .text : { *(.text) }
  . = 0x4000000;  .data : { *(.data) }
  . = 0x6000000;  .bss  : { *(.bss)  }
}