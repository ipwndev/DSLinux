#
# Automatically generated make config: don't edit
# Linux kernel version: 2.6.11.8-hsc0
# Fri Jun 17 16:51:35 2005
#
CONFIG_ARM=y
# CONFIG_MMU is not set
# CONFIG_MPU is not set
CONFIG_NO_MU=y
CONFIG_UID16=y
CONFIG_RWSEM_GENERIC_SPINLOCK=y
CONFIG_GENERIC_CALIBRATE_DELAY=y
CONFIG_GENERIC_IOMAP=y

#
# Code maturity level options
#
CONFIG_EXPERIMENTAL=y
CONFIG_CLEAN_COMPILE=y
CONFIG_BROKEN_ON_SMP=y
CONFIG_LOCK_KERNEL=y

#
# General setup
#
CONFIG_LOCALVERSION=""
# CONFIG_BSD_PROCESS_ACCT is not set
# CONFIG_SYSCTL is not set
# CONFIG_AUDIT is not set
CONFIG_LOG_BUF_SHIFT=14
# CONFIG_HOTPLUG is not set
# CONFIG_IKCONFIG is not set
CONFIG_EMBEDDED=y
# CONFIG_KALLSYMS is not set
CONFIG_FUTEX=y
# CONFIG_EPOLL is not set
# CONFIG_CC_OPTIMIZE_FOR_SIZE is not set
CONFIG_CC_ALIGN_FUNCTIONS=0
CONFIG_CC_ALIGN_LABELS=0
CONFIG_CC_ALIGN_LOOPS=0
CONFIG_CC_ALIGN_JUMPS=0

#
# Loadable module support
#
# CONFIG_MODULES is not set

#
# System Type
#
# CONFIG_ARCH_CLPS7500 is not set
# CONFIG_ARCH_CLPS711X is not set
# CONFIG_ARCH_CO285 is not set
# CONFIG_ARCH_EBSA110 is not set
# CONFIG_ARCH_CAMELOT is not set
# CONFIG_ARCH_FOOTBRIDGE is not set
# CONFIG_ARCH_INTEGRATOR is not set
# CONFIG_ARCH_IOP3XX is not set
# CONFIG_ARCH_IXP4XX is not set
# CONFIG_ARCH_IXP2000 is not set
# CONFIG_ARCH_L7200 is not set
# CONFIG_ARCH_PXA is not set
# CONFIG_ARCH_RPC is not set
# CONFIG_ARCH_SA1100 is not set
# CONFIG_ARCH_S3C2410 is not set
# CONFIG_ARCH_SHARK is not set
# CONFIG_ARCH_LH7A40X is not set
# CONFIG_ARCH_OMAP is not set
# CONFIG_ARCH_VERSATILE is not set
# CONFIG_ARCH_IMX is not set
# CONFIG_ARCH_H720X is not set
# CONFIG_ARCH_S5C7375 is not set
# CONFIG_ARCH_S3C24A0 is not set
# CONFIG_ARCH_S3C2500 is not set
# CONFIG_ARCH_ATMEL is not set
# CONFIG_ARCH_S3C3410 is not set
# CONFIG_ARCH_ESPD_4510B is not set
CONFIG_ARCH_LPC22xx=y
# CONFIG_ARCH_S3C44B0 is not set
# CONFIG_ARCH_P2001 is not set
CONFIG_SET_MEM_PARAM=y
CONFIG_DRAM_BASE=0x81000000
CONFIG_DRAM_SIZE=0x00800000
CONFIG_FLASH_MEM_BASE=0x80000000
CONFIG_FLASH_SIZE=0x00400000
# CONFIG_ARCH_SUPPORTS_BIG_ENDIAN is not set
CONFIG_ARM_CLK=40000000
CONFIG_SKIP_DUMP_CPU_INFO=y
CONFIG_REMAP_VECTORS_TO_RAM=y

#
# LPC22xx Options
#
CONFIG_LPC22xx_Fosc=10000000

#
# Processor Type
#
CONFIG_CPU_32=y
CONFIG_CPU_LPC22xx=y
CONFIG_CPU_32v4=y
CONFIG_CPU_HAS_NO_CP15=y

#
# Processor Features
#

#
# General setup
#
CONFIG_ZBOOT_ROM_TEXT=0x0
CONFIG_ZBOOT_ROM_BSS=0x0
# CONFIG_XIP_KERNEL is not set

#
# PCCARD (PCMCIA/CardBus) support
#
# CONFIG_PCCARD is not set

#
# PC-card bridges
#

#
# At least one math emulation must be selected
#
# CONFIG_FPE_NWFPE is not set
# CONFIG_FPE_FASTFPE is not set
CONFIG_BINFMT_FLAT=y
# CONFIG_BINFMT_ZFLAT is not set
# CONFIG_BINFMT_SHARED_FLAT is not set
# CONFIG_BINFMT_AOUT is not set
# CONFIG_BINFMT_MISC is not set

#
# Generic Driver Options
#
CONFIG_STANDALONE=y
CONFIG_PREVENT_FIRMWARE_BUILD=y
# CONFIG_FW_LOADER is not set
# CONFIG_PM is not set
CONFIG_PREEMPT=y
# CONFIG_ARTHUR is not set
CONFIG_CMDLINE="root=/dev/ram0 initrd=0x81200000,1000K console=ttyS0"
# CONFIG_ALIGNMENT_TRAP is not set

#
# Parallel port support
#
# CONFIG_PARPORT is not set

#
# Plug and Play support
#

#
# Block devices
#
# CONFIG_BLK_DEV_FD is not set
# CONFIG_BLK_DEV_COW_COMMON is not set
CONFIG_BLK_DEV_LOOP=y
# CONFIG_BLK_DEV_CRYPTOLOOP is not set
CONFIG_BLK_DEV_RAM=y
CONFIG_BLK_DEV_RAM_COUNT=16
CONFIG_BLK_DEV_RAM_SIZE=4096
CONFIG_BLK_DEV_INITRD=y
CONFIG_INITRAMFS_SOURCE=""
# CONFIG_CDROM_PKTCDVD is not set

#
# IO Schedulers
#
CONFIG_IOSCHED_NOOP=y
# CONFIG_IOSCHED_AS is not set
# CONFIG_IOSCHED_DEADLINE is not set
# CONFIG_IOSCHED_CFQ is not set

#
# Multi-device support (RAID and LVM)
#
# CONFIG_MD is not set

#
# Networking support
#
# CONFIG_NET is not set
# CONFIG_NETPOLL is not set
# CONFIG_NET_POLL_CONTROLLER is not set

#
# SCSI device support
#
# CONFIG_SCSI is not set

#
# Fusion MPT device support
#

#
# IEEE 1394 (FireWire) support
#
# CONFIG_IEEE1394 is not set

#
# I2O device support
#

#
# ISDN subsystem
#

#
# Input device support
#
# CONFIG_INPUT is not set

#
# Userland interfaces
#

#
# Input I/O drivers
#
# CONFIG_GAMEPORT is not set
CONFIG_SOUND_GAMEPORT=y
# CONFIG_SERIO is not set

#
# Input Device Drivers
#

#
# Character devices
#
# CONFIG_VT is not set
# CONFIG_SERIAL_NONSTANDARD is not set

#
# Serial drivers
#
CONFIG_SERIAL_8250=y
CONFIG_SERIAL_8250_CONSOLE=y
CONFIG_SERIAL_8250_NR_UARTS=0
# CONFIG_SERIAL_8250_EXTENDED is not set

#
# Non-8250 serial port support
#
# CONFIG_SERIAL_DCC is not set
CONFIG_SERIAL_CORE=y
CONFIG_SERIAL_CORE_CONSOLE=y
# CONFIG_UNIX98_PTYS is not set
# CONFIG_LEGACY_PTYS is not set

#
# IPMI
#
# CONFIG_IPMI_HANDLER is not set

#
# Watchdog Cards
#
# CONFIG_WATCHDOG is not set
# CONFIG_NVRAM is not set
# CONFIG_RTC is not set
# CONFIG_DTLK is not set
# CONFIG_R3964 is not set

#
# Ftape, the floppy tape device driver
#
# CONFIG_DRM is not set
# CONFIG_RAW_DRIVER is not set

#
# I2C support
#
# CONFIG_I2C is not set

#
# Multimedia devices
#
# CONFIG_VIDEO_DEV is not set

#
# Digital Video Broadcasting Devices
#

#
# File systems
#
CONFIG_EXT2_FS=y
# CONFIG_EXT2_FS_XATTR is not set
# CONFIG_EXT3_FS is not set
# CONFIG_JBD is not set
# CONFIG_REISERFS_FS is not set
# CONFIG_JFS_FS is not set

#
# XFS support
#
# CONFIG_XFS_FS is not set
# CONFIG_MINIX_FS is not set
CONFIG_ROMFS_FS=y
# CONFIG_MAGIC_ROM_PTR is not set
# CONFIG_QUOTA is not set
# CONFIG_DNOTIFY is not set
# CONFIG_AUTOFS_FS is not set
# CONFIG_AUTOFS4_FS is not set

#
# CD-ROM/DVD Filesystems
#
# CONFIG_ISO9660_FS is not set
# CONFIG_UDF_FS is not set

#
# DOS/FAT/NT Filesystems
#
# CONFIG_MSDOS_FS is not set
# CONFIG_VFAT_FS is not set
# CONFIG_NTFS_FS is not set

#
# Pseudo filesystems
#
CONFIG_PROC_FS=y
CONFIG_SYSFS=y
# CONFIG_DEVFS_FS is not set
# CONFIG_TMPFS is not set
# CONFIG_HUGETLB_PAGE is not set
CONFIG_RAMFS=y

#
# Miscellaneous filesystems
#
# CONFIG_ADFS_FS is not set
# CONFIG_AFFS_FS is not set
# CONFIG_HFS_FS is not set
# CONFIG_HFSPLUS_FS is not set
# CONFIG_BEFS_FS is not set
# CONFIG_BFS_FS is not set
# CONFIG_EFS_FS is not set
# CONFIG_CRAMFS is not set
# CONFIG_VXFS_FS is not set
# CONFIG_HPFS_FS is not set
# CONFIG_QNX4FS_FS is not set
# CONFIG_SYSV_FS is not set
# CONFIG_UFS_FS is not set

#
# Partition Types
#
# CONFIG_PARTITION_ADVANCED is not set
CONFIG_MSDOS_PARTITION=y

#
# Native Language Support
#
CONFIG_NLS=y
CONFIG_NLS_DEFAULT="iso8859-1"
# CONFIG_NLS_CODEPAGE_437 is not set
# CONFIG_NLS_CODEPAGE_737 is not set
# CONFIG_NLS_CODEPAGE_775 is not set
# CONFIG_NLS_CODEPAGE_850 is not set
# CONFIG_NLS_CODEPAGE_852 is not set
# CONFIG_NLS_CODEPAGE_855 is not set
# CONFIG_NLS_CODEPAGE_857 is not set
# CONFIG_NLS_CODEPAGE_860 is not set
# CONFIG_NLS_CODEPAGE_861 is not set
# CONFIG_NLS_CODEPAGE_862 is not set
# CONFIG_NLS_CODEPAGE_863 is not set
# CONFIG_NLS_CODEPAGE_864 is not set
# CONFIG_NLS_CODEPAGE_865 is not set
# CONFIG_NLS_CODEPAGE_866 is not set
# CONFIG_NLS_CODEPAGE_869 is not set
# CONFIG_NLS_CODEPAGE_936 is not set
# CONFIG_NLS_CODEPAGE_950 is not set
# CONFIG_NLS_CODEPAGE_932 is not set
# CONFIG_NLS_CODEPAGE_949 is not set
# CONFIG_NLS_CODEPAGE_874 is not set
# CONFIG_NLS_ISO8859_8 is not set
# CONFIG_NLS_CODEPAGE_1250 is not set
# CONFIG_NLS_CODEPAGE_1251 is not set
# CONFIG_NLS_ASCII is not set
# CONFIG_NLS_ISO8859_1 is not set
# CONFIG_NLS_ISO8859_2 is not set
# CONFIG_NLS_ISO8859_3 is not set
# CONFIG_NLS_ISO8859_4 is not set
# CONFIG_NLS_ISO8859_5 is not set
# CONFIG_NLS_ISO8859_6 is not set
# CONFIG_NLS_ISO8859_7 is not set
# CONFIG_NLS_ISO8859_9 is not set
# CONFIG_NLS_ISO8859_13 is not set
# CONFIG_NLS_ISO8859_14 is not set
# CONFIG_NLS_ISO8859_15 is not set
# CONFIG_NLS_KOI8_R is not set
# CONFIG_NLS_KOI8_U is not set
# CONFIG_NLS_UTF8 is not set

#
# Profiling support
#
# CONFIG_PROFILING is not set

#
# Graphics support
#
# CONFIG_FB is not set

#
# Sound
#
# CONFIG_SOUND is not set

#
# Misc devices
#

#
# USB support
#
# CONFIG_USB is not set
CONFIG_USB_ARCH_HAS_HCD=y
# CONFIG_USB_ARCH_HAS_OHCI is not set

#
# NOTE: USB_STORAGE enables SCSI, and 'SCSI disk support' may also be needed; see USB_STORAGE Help for more information
#

#
# USB Gadget Support
#
# CONFIG_USB_GADGET is not set

#
# MMC/SD Card support
#
# CONFIG_MMC is not set

#
# Kernel hacking
#
# CONFIG_DEBUG_KERNEL is not set
CONFIG_DEBUG_PREEMPT=y
# CONFIG_DEBUG_BUGVERBOSE is not set
# CONFIG_DEBUG_INFO is not set
CONFIG_FRAME_POINTER=y
# CONFIG_DEBUG_USER is not set

#
# Security options
#
# CONFIG_KEYS is not set
# CONFIG_SECURITY is not set

#
# Cryptographic options
#
# CONFIG_CRYPTO is not set

#
# Hardware crypto devices
#

#
# Library routines
#
# CONFIG_CRC_CCITT is not set
# CONFIG_CRC32 is not set
# CONFIG_LIBCRC32C is not set
