/* FR-V FDPIC ELF shared library loader suppport
   Copyright (C) 2003, 2004 Red Hat, Inc.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   Lots of code copied from ../i386/elfinterp.c, so:
   Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
  				David Engel, Hongjiu Lu and Mitch D'Souza
   Copyright (C) 2001-2002, Erik Andersen
   All rights reserved.

This file is part of uClibc.

uClibc is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

uClibc is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with uClibc; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
USA.  */

#include <sys/cdefs.h>	    /* __attribute_used__ */

#if defined (__SUPPORT_LD_DEBUG__)
static const char *_dl_reltypes_tab[] =
{
  [0]	"R_FRV_NONE",		"R_FRV_32",
  [2]	"R_FRV_LABEL16",	"R_FRV_LABEL24",
  [4]	"R_FRV_LO16",		"R_FRV_HI16",
  [6]	"R_FRV_GPREL12",	"R_FRV_GPRELU12",
  [8]	"R_FRV_GPREL32",	"R_FRV_GPRELHI",	"R_FRV_GPRELLO",
  [11]	"R_FRV_GOT12",		"R_FRV_GOTHI",		"R_FRV_GOTLO",
  [14]	"R_FRV_FUNCDESC",
  [15]	"R_FRV_FUNCDESC_GOT12",	"R_FRV_FUNCDESC_GOTHI",	"R_FRV_FUNCDESC_GOTLO",
  [18]	"R_FRV_FUNCDESC_VALUE", "R_FRV_FUNCDESC_GOTOFF12",
  [20]	"R_FRV_FUNCDESC_GOTOFFHI", "R_FRV_FUNCDESC_GOTOFFLO",
  [22]	"R_FRV_GOTOFF12",	"R_FRV_GOTOFFHI",	"R_FRV_GOTOFFLO",
#if 0
  [200]	"R_FRV_GNU_VTINHERIT",	"R_FRV_GNU_VTENTRY"
#endif
};

static const char *
_dl_reltypes(int type)
{
  static char buf[22];  
  const char *str;
  
  if (type >= (int)(sizeof (_dl_reltypes_tab)/sizeof(_dl_reltypes_tab[0])) ||
      NULL == (str = _dl_reltypes_tab[type]))
  {
    str =_dl_simple_ltoa( buf, (unsigned long)(type));
  }
  return str;
}

static 
void debug_sym(Elf32_Sym *symtab,char *strtab,int symtab_index)
{
  if(_dl_debug_symbols)
  {
    if(symtab_index){
      _dl_dprintf(_dl_debug_file, "\n%s\n\tvalue=%x\tsize=%x\tinfo=%x\tother=%x\tshndx=%x",
		  strtab + symtab[symtab_index].st_name,
		  symtab[symtab_index].st_value,
		  symtab[symtab_index].st_size,
		  symtab[symtab_index].st_info,
		  symtab[symtab_index].st_other,
		  symtab[symtab_index].st_shndx);
    }
  }
}

static void debug_reloc(Elf32_Sym *symtab,char *strtab, ELF_RELOC *rpnt)
{
  if(_dl_debug_reloc)
  {
    int symtab_index;
    const char *sym;
    symtab_index = ELF32_R_SYM(rpnt->r_info);
    sym = symtab_index ? strtab + symtab[symtab_index].st_name : "sym=0x0";
    
  if(_dl_debug_symbols)
	  _dl_dprintf(_dl_debug_file, "\n\t");
  else
	  _dl_dprintf(_dl_debug_file, "\n%s\n\t", sym);
#ifdef ELF_USES_RELOCA
    _dl_dprintf(_dl_debug_file, "%s\toffset=%x\taddend=%x",
		_dl_reltypes(ELF32_R_TYPE(rpnt->r_info)),
		rpnt->r_offset,
		rpnt->r_addend);
#else
    _dl_dprintf(_dl_debug_file, "%s\toffset=%x\n",
		_dl_reltypes(ELF32_R_TYPE(rpnt->r_info)),
		rpnt->r_offset);
#endif
  }
}
#endif

/* Program to load an ELF binary on a linux system, and run it.
   References to symbols in sharable libraries can be resolved by either
   an ELF sharable library or a linux style of shared library. */

/* Disclaimer:  I have never seen any AT&T source code for SVr4, nor have
   I ever taken any courses on internals.  This program was developed using
   information available through the book "UNIX SYSTEM V RELEASE 4,
   Programmers guide: Ansi C and Programming Support Tools", which did
   a more than adequate job of explaining everything required to get this
   working. */

struct funcdesc_value volatile *__attribute__((__visibility__("hidden")))
_dl_linux_resolver (struct elf_resolve *tpnt, int reloc_entry)
{
	int reloc_type;
	ELF_RELOC *this_reloc;
	char *strtab;
	Elf32_Sym *symtab;
	int symtab_index;
	char *rel_addr;
	struct elf_resolve *new_tpnt;
	char *new_addr;
	struct funcdesc_value funcval;
	struct funcdesc_value volatile *got_entry;
	char *symname;

	rel_addr = DL_RELOC_ADDR (tpnt->dynamic_info[DT_JMPREL],
				  tpnt->loadaddr);

	this_reloc = (ELF_RELOC *)(intptr_t)(rel_addr + reloc_entry);
	reloc_type = ELF32_R_TYPE(this_reloc->r_info);
	symtab_index = ELF32_R_SYM(this_reloc->r_info);

	symtab = (Elf32_Sym *)(intptr_t)
				  DL_RELOC_ADDR (tpnt->dynamic_info[DT_SYMTAB],
						 tpnt->loadaddr);
	strtab = DL_RELOC_ADDR (tpnt->dynamic_info[DT_STRTAB], tpnt->loadaddr);
	symname= strtab + symtab[symtab_index].st_name;

	if (reloc_type != R_FRV_FUNCDESC_VALUE) {
		_dl_dprintf(2, "%s: Incorrect relocation type in jump relocations\n", 
				_dl_progname);
		_dl_exit(1);
	}

	/* Address of GOT entry fix up */
	got_entry = (struct funcdesc_value *)
	  DL_RELOC_ADDR (this_reloc->r_offset, tpnt->loadaddr);

	/* Get the address to be used to fill in the GOT entry.  */
	new_addr = _dl_find_hash_mod(symname, tpnt->symbol_scope, NULL, 0,
				     &new_tpnt);
	if (!new_addr) {
		new_addr = _dl_find_hash_mod(symname, NULL, NULL, 0,
					     &new_tpnt);
		if (!new_addr) {
			_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
				    _dl_progname, symname);
			_dl_exit(1);
		}
	}

	funcval.entry_point = new_addr;
	funcval.got_value = new_tpnt->loadaddr.got_value;

#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_bindings)
		{
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
			if(_dl_debug_detail)
				_dl_dprintf(_dl_debug_file, 
					    "\n\tpatched (%x,%x) ==> (%x,%x) @ %x\n",
					    got_entry->entry_point, got_entry->got_value,
					    funcval.entry_point, funcval.got_value,
					    got_entry);
		}
	if (!_dl_debug_nofixups) {
		*got_entry = funcval;
	}
#else
	*got_entry = funcval;
#endif

	return got_entry;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct dyn_elf *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc) (struct elf_resolve *tpnt, struct dyn_elf *scope,
			    ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab))
{
	unsigned int i;
	char *strtab;
	Elf32_Sym *symtab;
	ELF_RELOC *rpnt;
	int symtab_index;

	/* Now parse the relocation information */
	rpnt = (ELF_RELOC *)(intptr_t) DL_RELOC_ADDR (rel_addr, tpnt->loadaddr);
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (Elf32_Sym *)(intptr_t)
	  DL_RELOC_ADDR (tpnt->dynamic_info[DT_SYMTAB], tpnt->loadaddr);
	strtab = DL_RELOC_ADDR (tpnt->dynamic_info[DT_STRTAB], tpnt->loadaddr);

	  for (i = 0; i < rel_size; i++, rpnt++) {
	        int res;
	    
		symtab_index = ELF32_R_SYM(rpnt->r_info);
#if defined (__SUPPORT_LD_DEBUG__)
		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);
#endif

		res = reloc_fnc (tpnt, scope, rpnt, symtab, strtab);

		if (res==0) continue;

		_dl_dprintf(2, "\n%s: ",_dl_progname);
		
		if (symtab_index)
		  _dl_dprintf(2, "symbol '%s': ", strtab + symtab[symtab_index].st_name);
		  
		if (res <0)
		{
		        int reloc_type = ELF32_R_TYPE(rpnt->r_info);
#if defined (__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "can't handle reloc type %s\n ", _dl_reltypes(reloc_type));
#else
			_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
#endif			
			_dl_exit(-res);
		}
		else if (res >0)
		{
			_dl_dprintf(2, "can't resolve symbol\n");
			return res;
		}
	  }
	  return 0;
}

static int
_dl_do_reloc (struct elf_resolve *tpnt,struct dyn_elf *scope,
	      ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long reloc_value = 0, *reloc_addr;
	struct { unsigned long v; } __attribute__((__packed__))
					    *reloc_addr_packed;
	unsigned long symbol_addr;
	struct elf_resolve *symbol_tpnt;
	struct funcdesc_value funcval;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	reloc_addr   = (unsigned long *)(intptr_t)
	  DL_RELOC_ADDR (rpnt->r_offset, tpnt->loadaddr);
	asm ("" : "=r" (reloc_addr_packed) : "0" (reloc_addr));
	reloc_type   = ELF32_R_TYPE(rpnt->r_info);
	symtab_index = ELF32_R_SYM(rpnt->r_info);
	symbol_addr  = 0;
	symname      = strtab + symtab[symtab_index].st_name;

	if (ELF32_ST_BIND (symtab[symtab_index].st_info) == STB_LOCAL) {
		symbol_addr = (unsigned long)
		  DL_RELOC_ADDR (symtab[symtab_index].st_value,
				 tpnt->loadaddr);
		symbol_tpnt = tpnt;
	} else {

		symbol_addr = (unsigned long)
		  _dl_find_hash_mod(symname, scope, NULL, 0, &symbol_tpnt);

		/*
		 * We want to allow undefined references to weak symbols - this might
		 * have been intentional.  We should not be linking local symbols
		 * here, so all bases should be covered.
		 */

		if (!symbol_addr && ELF32_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK) {
			_dl_dprintf (2, "%s: can't resolve symbol '%s'\n",
				     _dl_progname, strtab + symtab[symtab_index].st_name);
			_dl_exit (1);
		}
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
	  {
	    if ((long)reloc_addr_packed & 3)
	      old_val = reloc_addr_packed->v;
	    else
	      old_val = *reloc_addr;
	  }
	else
	  old_val = 0;
#endif
	switch (reloc_type) {
	case R_FRV_NONE:
		break;
	case R_FRV_32:
		if ((long)reloc_addr_packed & 3)
			reloc_value = reloc_addr_packed->v += symbol_addr;
		else
			reloc_value = *reloc_addr += symbol_addr;
		break;
	case R_FRV_FUNCDESC_VALUE:
		funcval.entry_point = (void*)symbol_addr;
		/* The addend of FUNCDESC_VALUE
		   relocations referencing global
		   symbols must be ignored, because it
		   may hold the address of a lazy PLT
		   entry.  */
		if (ELF32_ST_BIND
		    (symtab[symtab_index].st_info)
		    == STB_LOCAL)
			funcval.entry_point += *reloc_addr;
		reloc_value = (unsigned long)funcval.entry_point;
		if (symbol_addr)
			funcval.got_value
				= symbol_tpnt->loadaddr.got_value;
		else
			funcval.got_value = 0;
		asm ("std%I0\t%1, %M0"
		     : "=m" (*(struct funcdesc_value *)reloc_addr)
		     : "e" (funcval));
		break;
	case R_FRV_FUNCDESC:
		if ((long)reloc_addr_packed & 3)
			reloc_value = reloc_addr_packed->v;
		else
			reloc_value = *reloc_addr;
		if (symbol_addr)
			reloc_value = (unsigned long)_dl_funcdesc_for
				((char *)symbol_addr + reloc_value,
				 symbol_tpnt->loadaddr.got_value);
		else
			reloc_value = 0;
		if ((long)reloc_addr_packed & 3)
			reloc_addr_packed->v = reloc_value;
		else
			*reloc_addr = reloc_value;
		break;
	default:
		return -1; /*call _dl_exit(1) */
	}
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail) {
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x", old_val, reloc_value, reloc_addr);
		switch (reloc_type) {
		case R_FRV_FUNCDESC_VALUE:
			_dl_dprintf(_dl_debug_file, " got %x", ((struct funcdesc_value *)reloc_value)->got_value);
			break;
		case R_FRV_FUNCDESC:
			if (! reloc_value)
				break;
			_dl_dprintf(_dl_debug_file, " funcdesc (%x,%x)",
				    ((struct funcdesc_value *)reloc_value)->entry_point,
				    ((struct funcdesc_value *)reloc_value)->got_value);
			break;
		}
	}
#endif

	return 0;
}

static int
_dl_do_lazy_reloc (struct elf_resolve *tpnt,
		   struct dyn_elf *scope __attribute_used__,
		   ELF_RELOC *rpnt, Elf32_Sym *symtab __attribute_used__,
		   char *strtab __attribute_used__)
{
	int reloc_type;
	struct funcdesc_value volatile *reloc_addr;
	struct funcdesc_value funcval;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	reloc_addr = (struct funcdesc_value *)(intptr_t)
	  DL_RELOC_ADDR (rpnt->r_offset, tpnt->loadaddr);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = (unsigned long)reloc_addr->entry_point;
#endif
		switch (reloc_type) {
			case R_FRV_NONE:
				break;
			case R_FRV_FUNCDESC_VALUE:
				funcval = *reloc_addr;
				funcval.entry_point =
				  DL_RELOC_ADDR (funcval.entry_point,
						 tpnt->loadaddr);
				funcval.got_value = tpnt->loadaddr.got_value;
				*reloc_addr = funcval;
				break;
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x", old_val, reloc_addr->entry_point, reloc_addr);
#endif
	return 0;

}

void
_dl_parse_lazy_relocation_information
(struct dyn_elf *rpnt, unsigned long rel_addr, unsigned long rel_size)
{
  _dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int
_dl_parse_relocation_information
(struct dyn_elf *rpnt, unsigned long rel_addr, unsigned long rel_size)
{
  return _dl_parse(rpnt->dyn, rpnt->dyn->symbol_scope, rel_addr, rel_size, _dl_do_reloc);
}

/* We don't have copy relocs.  */

int
_dl_parse_copy_information
(struct dyn_elf *rpnt __attribute_used__,
 unsigned long rel_addr __attribute_used__,
 unsigned long rel_size __attribute_used__)
{
  return 0;
}

#ifndef LIBDL
# include "../../libc/sysdeps/linux/frv/crtreloc.c"
#endif

