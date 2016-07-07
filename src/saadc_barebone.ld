/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx)   : ORIGIN = 0x1c000, LENGTH = 0x64000
  RAM (rwx)    : ORIGIN = 0x20006080, LENGTH = 0x9f80
  m_data (rwx) : ORIGIN = 0x20002080, LENGTH = 0x4000	
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

SECTIONS
{
  /* placing my named section at given address: */
  .myBufBlock 0x20002080 :
  {
    KEEP(*(.myBufSection)) /* keep my variable even if not referenced */
  } > m_data
 
  /* other placements follow here... */
}

INCLUDE "nrf5x_common.ld"
