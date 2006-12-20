/* vi: set sw=8 ts=8: */

/*
 * Various assmbly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/* Define this if the system uses RELOCA.  */
#undef ELF_USES_RELOCA

#define ARCH_NUM 3
#define DT_MIPS_GOTSYM_IDX	(DT_NUM + OS_NUM)
#define DT_MIPS_LOCAL_GOTNO_IDX	(DT_NUM + OS_NUM +1)
#define DT_MIPS_SYMTABNO_IDX	(DT_NUM + OS_NUM +2)

#define ARCH_DYNAMIC_INFO(dpnt,  dynamic, debug_addr) \
do { \
if (dpnt->d_tag == DT_MIPS_GOTSYM) \
     dynamic[DT_MIPS_GOTSYM_IDX] = dpnt->d_un.d_val; \
else if(dpnt->d_tag == DT_MIPS_LOCAL_GOTNO) \
     dynamic[DT_MIPS_LOCAL_GOTNO_IDX] = dpnt->d_un.d_val; \
else if(dpnt->d_tag == DT_MIPS_SYMTABNO) \
     dynamic[DT_MIPS_SYMTABNO_IDX] = dpnt->d_un.d_val; \
else if (dpnt->d_tag == DT_MIPS_RLD_MAP) \
     *(Elf32_Addr *)(dpnt->d_un.d_ptr) =  (Elf32_Addr) debug_addr; \
} while (0)

/* Initialization sequence for the application/library GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)						\
do {										\
	unsigned long i;							\
										\
	/* Check if this is the dynamic linker itself */			\
	if (MODULE->libtype == program_interpreter)				\
		continue;							\
										\
	/* Fill in first two GOT entries according to the ABI */		\
	GOT_BASE[0] = (unsigned long) _dl_linux_resolve;			\
	GOT_BASE[1] = (unsigned long) MODULE;					\
										\
	/* Add load address displacement to all local GOT entries */		\
	i = 2;									\
	while (i < MODULE->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX])		\
		GOT_BASE[i++] += (unsigned long) MODULE->loadaddr;		\
										\
} while (0)


/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_MIPS
#define MAGIC2 EM_MIPS_RS3_LE


/* Used for error messages */
#define ELF_TARGET "MIPS"


unsigned long _dl_linux_resolver(unsigned long sym_index,
	unsigned long old_gpreg);

struct elf_resolve;
void _dl_perform_mips_global_got_relocations(struct elf_resolve *tpnt, int lazy);

#define do_rem(result, n, base) ((result) = (n) % (base))

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

#define elf_machine_type_class(type)		ELF_RTYPE_CLASS_PLT
/* MIPS does not have COPY relocs */
#define DL_NO_COPY_RELOCS
