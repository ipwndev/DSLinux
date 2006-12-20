/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2002 by Erik Andersen <andersen@uclibc.org>
 * Based in part on the files
 *		./sysdeps/unix/sysv/linux/pwrite.c,
 *		./sysdeps/unix/sysv/linux/pread.c, 
 *		sysdeps/posix/pread.c
 *		sysdeps/posix/pwrite.c
 * from GNU libc 2.2.5, but reworked considerably...
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <features.h>
#undef __OPTIMIZE__
/* We absolutely do _NOT_ want interfaces silently
 *  *  * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif


#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>

#ifdef __NR_pread64             /* Newer kernels renamed but it's the same.  */
# ifdef __NR_pread
#  error "__NR_pread and __NR_pread64 both defined???"
# endif
# define __NR_pread __NR_pread64
#endif

#ifdef __NR_pread

#ifdef __mips64
_syscall4(ssize_t, pread, int, fd, void *, buf, size_t, count, off_t, offset);
#else /* !__mips64 */
#define __NR___syscall_pread __NR_pread 
static inline _syscall6(ssize_t, __syscall_pread, int, fd, void *, buf, 
		size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo);

ssize_t __libc_pread(int fd, void *buf, size_t count, off_t offset)
{ 
	return(__syscall_pread(fd,buf,count,0,__LONG_LONG_PAIR((off_t)0,offset)));
}
weak_alias (__libc_pread, pread)

#if defined __UCLIBC_HAS_LFS__ 
ssize_t __libc_pread64(int fd, void *buf, size_t count, off64_t offset)
{ 
    uint32_t low = offset & 0xffffffff;
    uint32_t high = offset >> 32;
	return(__syscall_pread(fd, buf, count, 0, __LONG_LONG_PAIR (high, low)));
}
weak_alias (__libc_pread64, pread64)
#endif /* __UCLIBC_HAS_LFS__  */
#endif /* !__mips64 */

#endif /* __NR_pread */

/**********************************************************************/

#ifdef __NR_pwrite64            /* Newer kernels renamed but it's the same.  */
# ifdef __NR_pwrite
#  error "__NR_pwrite and __NR_pwrite64 both defined???"
# endif
# define __NR_pwrite __NR_pwrite64
#endif

#ifdef __NR_pwrite

#ifdef __mips64
_syscall4(ssize_t, pwrite, int, fd, const void *, buf, size_t, count, off_t, offset);
#else /* !__mips64 */
#define __NR___syscall_pwrite __NR_pwrite 
static inline _syscall6(ssize_t, __syscall_pwrite, int, fd, const void *, buf, 
		size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo);

ssize_t __libc_pwrite(int fd, const void *buf, size_t count, off_t offset)
{ 
	return(__syscall_pwrite(fd,buf,count,0,__LONG_LONG_PAIR((off_t)0,offset)));
}
weak_alias (__libc_pwrite, pwrite)

#if defined __UCLIBC_HAS_LFS__ 
ssize_t __libc_pwrite64(int fd, const void *buf, size_t count, off64_t offset)
{ 
    uint32_t low = offset & 0xffffffff;
    uint32_t high = offset >> 32;
	return(__syscall_pwrite(fd, buf, count, 0, __LONG_LONG_PAIR (high, low)));
}
weak_alias (__libc_pwrite64, pwrite64)
#endif /* __UCLIBC_HAS_LFS__  */
#endif /* !__mips64 */
#endif /* __NR_pwrite */
