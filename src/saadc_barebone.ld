/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx)   : ORIGIN = 0x0000, LENGTH = 0x80000
  RAM (rwx)    : ORIGIN = 0x20004010, LENGTH = 0x9f80
  m_data1 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x2000		
  m_data2 (rwx) : ORIGIN = 0x20002000, LENGTH = 0x2000

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
  .mydata1 0x20000000 :
  {
    KEEP(*(.mydata1)) /* keep my variable even if not referenced */
  } > m_data1
  
  .mydata2 0x20002000 :
  {
    KEEP(*(.mydata2)) /* keep my variable even if not referenced */
  } > m_data2

 
  /* other placements follow here... */
}

INCLUDE "nrf5x_common.ld"
