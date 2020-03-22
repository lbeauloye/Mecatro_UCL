/* ----------------------------------------------------------------------------------------	*/
/* Stack sizes 																				*/

/* ----------------------------------------------------------------------------------------	*/
/* main() / Adam & Eve stack size.															*/
/* All left over unused memory is assigned to the heap (malloc())							*/

_STACK_SIZE				= DEFINED(_STACK_SIZE)            ? _STACK_SIZE            : 8192;

/* ----------------------------------------------------------------------------------------	*/

_MDRV_SIZE			    = DEFINED(_MDRV_SIZE)             ? _MDRV_SIZE          : 0x10000000;

/* ----------------------------------------------------------------------------------------	*/
/* Total size (in bytes) per stack type. This is shared between all cores					*/
/* Each core is allocated _????_STACK_SIZE / OS_N_CORE bytes								*/
/* Only used when OS_???_STACK_SIZE == -1 (defined in mAbassi assembly support file)		*/
 
_SUPERVISOR_STACK_SIZE	= DEFINED(_SUPERVISOR_STACK_SIZE) ? _SUPERVISOR_STACK_SIZE : 2048;
_ABORT_STACK_SIZE		= DEFINED(_ABORT_STACK_SIZE)      ? _ABORT_STACK_SIZE      : 2048;
_FIQ_STACK_SIZE			= DEFINED(_FIQ_STACK_SIZE)        ? _FIQ_STACK_SIZE        : 2048;
_IRQ_STACK_SIZE			= DEFINED(_IRQ_STACK_SIZE)        ? _IRQ_STACK_SIZE        : 8192;
_UNDEF_STACK_SIZE		= DEFINED(_UNDEF_STACK_SIZE)      ? _UNDEF_STACK_SIZE      : 2048;

/* ----------------------------------------------------------------------------------------	*/
/* ----------------------------------------------------------------------------------------	*/
/* ----------------------------------------------------------------------------------------	*/

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
ENTRY(_vector_table)
SEARCH_DIR(.)

MEMORY
{
  ram  (rwx) : ORIGIN = 0x02000000, LENGTH = 0x1E000000
  uram (rwx) : ORIGIN = 0xFFFF0000, LENGTH = 0x10000		/* uncached is in the SARAM		*/
}


EXTERN(__cs3_reset __cs3_reset_generic)
EXTERN(__cs3_start_asm _start)
EXTERN(__cs3_interrupt_vector_arm)
EXTERN(__cs3_start_c main__cs3_stack __cs3_heap_start __cs3_heap_end)

PROVIDE(__cs3_region_num = (__cs3_regions_end - __cs3_regions) / 20);

__libc_fini = _fini;

SECTIONS
{
	.text :
	{
    	CREATE_OBJECT_SYMBOLS
		__cs3_region_start_ram = .;
		_ftext = .;
		*(.cs3.region-head.ram)
		ASSERT (. == __cs3_region_start_ram, ".cs3.region-head.ram not permitted");
		__cs3_interrupt_vector = __cs3_interrupt_vector_arm;
		KEEP(*(.cs3.interrupt_vector))
 		KEEP(*(.vectors))
		PROVIDE(__cs3_reset = __cs3_reset_generic);
		*(.cs3.reset)
		_start = DEFINED(__cs3_start_asm) ? __cs3_start_asm : _start;
		*(.text.cs3.init)
		*(.text .text.* .gnu.linkonce.t.*)
		*(.plt)
		*(.gnu.warning)
		*(.glue_7t) *(.glue_7) *(.vfp11_veneer)
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		*(.gcc_except_table)
	} > ram


	.eh_frame_hdr : ALIGN (4)
	{
		KEEP (*(.eh_frame_hdr))
		*(.eh_frame_entry .eh_frame_entry.*)
	} > ram


	.eh_frame : ALIGN (4)
	{
		KEEP (*(.eh_frame)) *(.eh_frame.*)
	} > ram


	PROVIDE_HIDDEN (__exidx_start = .);
	.ARM.exidx :
	{
    	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > ram
	PROVIDE_HIDDEN (__exidx_end = .);


	.rodata : ALIGN (4)
	{
		*(.rodata .rodata.* .gnu.linkonce.r.*)

	    . = ALIGN(4);
		KEEP(*(.init))

		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))
		__preinit_array_end = .;

		. = ALIGN(4);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;

		. = ALIGN(4);
		KEEP(*(.fini))

		. = ALIGN(4);
		__fini_array_start = .;
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		__fini_array_end = .;

		. = ALIGN(0x4);
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*crtend.o(.ctors))
		*(.ctors)

 		. = ALIGN(0x4);
		KEEP (*crtbegin.o(.dtors))
 		KEEP (*crtbegin?.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*crtend.o(.dtors))
 		*(.dtors)

		. = ALIGN(4);
		__cs3_regions = .;
		LONG (0)
		LONG (__cs3_region_init_ram)
		LONG (__cs3_region_start_ram)
		LONG (__cs3_region_init_size_ram)
		LONG (__cs3_region_zero_size_ram)
		__cs3_regions_end = .;
		. = ALIGN (8);
		_etext = .;
	} > ram

	ASSERT (!(__cs3_region_init_ram & 7),      "__cs3_region_init_ram not aligned")
	ASSERT (!(__cs3_region_start_ram & 7),     "__cs3_region_start_ram not aligned")
	ASSERT (!(__cs3_region_init_size_ram & 7), "__cs3_region_init_size_ram not aligned")
	ASSERT (!(__cs3_region_zero_size_ram & 7), "__cs3_region_zero_size_ram not aligned")
 

/* To copy multiple ROM to RAM sections,
 * uncomment .copy.table section and,
 * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S 
	.copy.table :
	{
		. = ALIGN(4);
		__copy_table_start__ = .;
		LONG (__etext)
		LONG (__data_start__)
		LONG (__data_end__ - __data_start__)
		LONG (__etext2)
		LONG (__data2_start__)
		LONG (__data2_end__ - __data2_start__)
		__copy_table_end__ = .;
	} > ram
*/

/* To clear multiple BSS sections,
 * uncomment .zero.table section and,
 * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S
	.zero.table :
	{
		. = ALIGN(4);
		__zero_table_start__ = .;
		LONG (__bss_start__)
		LONG (__bss_end__ - __bss_start__)
		LONG (__bss2_start__)
		LONG (__bss2_end__ - __bss2_start__)
		__zero_table_end__ = .;
	} > ram
*/

	__etext = .;
		
	.data : AT (__etext) ALIGN (8)
	{
		_fdata = .;
		__data_start__ = .;
		*(vtable)
		*(.data*)
		KEEP(*(.jcr*))
		. = ALIGN(8);
		__data_end__ = .;
	    _edata = .;
	} > ram


	.bss (NOLOAD) : ALIGN(8)
	{
		_fbss = .;
		__bss_start__ = .;
		*(.shbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(8);
		*(.ram.b .bss.ram)
		. = ALIGN(8);
		__bss_end__ = .;
		_ebss       = .;
	} > ram

	/* Area reserved for the Memory drive															*/
	/* If using a fixed address, instead of } > ram, define a new MEMORY region and use it			*/

	.memdrive (NOLOAD) : {
		. = ALIGN(8);
		G_MemDrvBase = .;
		. += _MDRV_SIZE;
		G_MemDrvEnd = .;
	} > ram

	.stack_dummy (NOLOAD) : ALIGN (8)
	{
		*(.stack*)
	}

	.heap (NOLOAD):
	{
		_end        = .;
		__end       = .;
		__end__     = .;
		PROVIDE(end = .);
		PROVIDE(__cs3_heap_start = .);
		*(.heap*)
		. = (__cs3_region_start_ram + LENGTH(ram))
		  - ((_SUPERVISOR_STACK_SIZE + 7) & 0xFFFFFFF8)
		  - ((_ABORT_STACK_SIZE      + 7) & 0xFFFFFFF8)
		  - ((_FIQ_STACK_SIZE        + 7) & 0xFFFFFFF8)
		  - ((_IRQ_STACK_SIZE        + 7) & 0xFFFFFFF8)
		  - ((_UNDEF_STACK_SIZE      + 7) & 0xFFFFFFF8)
		  - ((_STACK_SIZE            + 7) & 0xFFFFFFF8);
		__HeapLimit = .;
		PROVIDE(__cs3_heap_end = .);
	} > ram

	.stack (NOLOAD) : ALIGN (8)
	{
		. = . + _SUPERVISOR_STACK_SIZE;
		. = ALIGN(8);
		__supervisor_stack = .;
		. = . + _ABORT_STACK_SIZE;
		. = ALIGN(8);
		__abort_stack = .;
		. = . + _FIQ_STACK_SIZE;
		. = ALIGN(8);
		__fiq_stack = .;
		. = . + _IRQ_STACK_SIZE;
		. = ALIGN(8);
		__irq_stack = .;
		. = . + _UNDEF_STACK_SIZE;
		. = ALIGN(8);
		__undef_stack = .;
		. += _STACK_SIZE;
	} > ram

	__cs3_region_end_ram       = __cs3_region_start_ram + LENGTH(ram);
	__cs3_region_size_ram      = LENGTH(ram);
	__cs3_region_init_ram      = LOADADDR (.text);
	__cs3_region_init_size_ram = _edata - ADDR (.text);
	__cs3_region_zero_size_ram = _end - _edata;

	__StackTop          = ORIGIN(ram) + LENGTH(ram);
	__StackLimit        = __StackTop - SIZEOF(.stack);
	PROVIDE(__stack     = __StackTop);
	PROVIDE(__cs3_stack = __StackTop);

	.uncached (NOLOAD) : ALIGN (8)
	{
		*(.uncached)
		*(.uncached.*)
  	} >uram


	.stab 0 (NOLOAD) : { *(.stab) }
	.stabstr 0 (NOLOAD) : { *(.stabstr) }
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* DWARF 2.1 */
	.debug_ranges   0 : { *(.debug_ranges) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }

	.note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
	.ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }
	/DISCARD/ : { *(.note.GNU-stack) }
	/* Check if data + heap + stack exceeds ram limit */
	ASSERT(__StackLimit >= __HeapLimit, "region ram overflowed with stack")
}

/* EOF */
