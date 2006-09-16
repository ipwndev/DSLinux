#
# Makefile for the kernel mmc device drivers.
#

#
# Core
#
obj-$(CONFIG_MMC)		+= mmc_core.o

#
# Media drivers
#
obj-$(CONFIG_MMC_BLOCK)		+= mmc_block.o

#
# Host drivers
#
obj-$(CONFIG_MMC_ARMMMCI)	+= mmci.o
obj-$(CONFIG_MMC_PXA)		+= pxamci.o
obj-$(CONFIG_MMC_WBSD)		+= wbsd.o
obj-$(CONFIG_MMC_M3SD)		+= m3sd.o
obj-$(CONFIG_MMC_DSMEMSD)	+= dsmemsd.o
obj-$(CONFIG_MMC_SCSD)		+= scsd.o

mmc_core-y := mmc.o mmc_queue.o mmc_sysfs.o
scsd-y := scsd_c.o scsd_s.o
m3sd-y := m3sd_c.o m3sd_s.o
