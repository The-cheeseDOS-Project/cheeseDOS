ENTRY(start)

MEMORY
{
  CODE (rwx)  : ORIGIN =  0x7c00, LENGTH = 512
}

SECTIONS
{
  . = ORIGIN(CODE);
  .text : {
    *(.text)
  } > CODE

  /DISCARD/ : {
    *(.note.*)
  }
}
