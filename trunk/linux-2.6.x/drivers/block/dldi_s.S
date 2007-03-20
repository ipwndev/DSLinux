@---------------------------------------------------------------------------------
	.align	4
	.arm
	.global _io_dldi
	.global _param_dldi
	.global	_call_dldi
	.global _buf_dldi

@---------------------------------------------------------------------------------
.equ FEATURE_MEDIUM_CANREAD,		0x00000001
.equ FEATURE_MEDIUM_CANWRITE,		0x00000002
.equ FEATURE_SLOT_GBA,			0x00000010
.equ FEATURE_SLOT_NDS,			0x00000020
.equ NDS_IME,				0x04000208

_dldi_start:

@---------------------------------------------------------------------------------
@ Driver patch file standard header -- 16 bytes
	.word	0xBF8DA5ED		@ Magic number to identify this region
	.asciz	" Chishm"		@ Identifying Magic string (8 bytes with null terminator)
	.byte	0x01			@ Version number
	.byte	0x0F	@32KiB		@ Log [base-2] of the size of this driver in bytes.
	.byte	0x00			@ Sections to fix
	.byte 	0x0F	@32KiB		@ Log [base-2] of the allocated space in bytes.
	
@---------------------------------------------------------------------------------
@ Text identifier - can be anything up to 47 chars + terminating null -- 16 bytes
	.align	4
	.asciz "Default (No interface)"

@---------------------------------------------------------------------------------
@ Offsets to important sections within the data	-- 32 bytes
	.align	6
	.word   _dldi_start		@ data start
	.word   _dldi_end		@ data end
	.word	0x00000000		@ Interworking glue start 	-- Needs address fixing
	.word	0x00000000		@ Interworking glue end
	.word   0x00000000		@ GOT start			-- Needs address fixing
	.word   0x00000000		@ GOT end
	.word   0x00000000		@ bss start			-- Needs setting to zero
	.word   0x00000000		@ bss end

@---------------------------------------------------------------------------------
@ IO_INTERFACE data -- 32 bytes
_io_dldi:
	.ascii	"DLDI"					@ ioType
	.word	0x00000000				@ Features
	.word	_DLDI_startup			@ 
	.word	_DLDI_isInserted		@ 
	.word	_DLDI_readSectors		@   Function pointers to standard device driver functions
	.word	_DLDI_writeSectors		@ 
	.word	_DLDI_clearStatus		@ 
	.word	_DLDI_shutdown			@ 
	
@---------------------------------------------------------------------------------

_DLDI_startup:
_DLDI_isInserted:
_DLDI_readSectors:
_DLDI_writeSectors:
_DLDI_clearStatus:
_DLDI_shutdown:
	mov		r0, #0x00				@ Return false for every function
	bx		lr



@---------------------------------------------------------------------------------
	.align
	.pool

.space 32632						@ Fill to 32KiB

_dldi_end:

@---------------------------------------------------------------------------------

	.align
_param_dldi:
	.space	6*4

	.align
_call_dldi:
	@ Disable interrupts (problems with stack switching without)
	ldr	r3, =NDS_IME		@ interrupt mask register
	mov	ip, #0			@ 0 = disable
	strh	ip, [r3]
	mcr	p15, 0, ip, c7, c10, 4	@ drain write buffer for IME to take effect.
	@ Disable Write Buffer for all except main RAM
	@	      gGViWDMI
	ldr	r0,=0b00000010
	mcr	p15, 0, r0, c3, c0, 0
	ldr	ip,=_param_dldi		@ ip = IO parameter block
	str	sp,[ip]			@ store old stack pointer
	str	lr,[ip,#4]		@ store old link register
	ldr	r0,[ip,#8]		@ read parameter 1
	ldr	r1,[ip,#12]		@ read parameter 2
	ldr	r2,[ip,#16]		@ read parameter 3
	ldr	r3,[ip,#20]		@ read pointer to function
	ldr	sp,=_stack_dldi		@ set new stack
	blx	r3			@ execute the function
	ldr	ip,=_param_dldi		@ ip = IO parameter block
	ldr	sp,[ip]			@ restore stack
	ldr	lr,[ip,#4]		@ restore link register
	@ Restore Write Buffer
	@	      gGViWDMI
	ldr	r0,=0b11111010
	mcr	p15, 0, r0, c3, c0, 0
	@ enable interrupts
	ldr	r3, =NDS_IME		@ interrupt mask register
	mov	ip, #1			@ 1 = enable
	strh	ip, [r3]
	bx	lr			@ return

	.pool
	.align
	.space	4096	
_stack_dldi:
	.space	4			@ for safety
_buf_dldi:
	.space	2048			@ allows for 1 physical sector on NAND flash
	.end
@---------------------------------------------------------------------------------
