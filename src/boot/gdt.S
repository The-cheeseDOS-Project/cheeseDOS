.equ gdt_fl_pglimit, 0x80
.equ gdt_fl_32b, 0x40

.equ gdt_a_present, 0x80
.equ gdt_a_dpl0, 0x00
.equ gdt_a_nosys, 0x10
.equ gdt_a_exec, 0x08
.equ gdt_a_conforming, 0x04
.equ gdt_a_grow_down, 0x04
.equ gdt_a_rw, 0x2
.equ gdt_a_accessed, 0x1
