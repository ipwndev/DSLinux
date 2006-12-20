/*
 * IBM PowerPC iSeries Virtual I/O Infrastructure Support.
 *
 *    Copyright (c) 2005 Stephen Rothwell, IBM Corp.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */
#include <linux/types.h>
#include <linux/device.h>
#include <linux/init.h>

#include <asm/vio.h>
#include <asm/iommu.h>
#include <asm/abs_addr.h>
#include <asm/page.h>
#include <asm/iSeries/vio.h>
#include <asm/iSeries/HvTypes.h>
#include <asm/iSeries/HvLpConfig.h>
#include <asm/iSeries/HvCallXm.h>

struct device *iSeries_vio_dev = &vio_bus_device.dev;
EXPORT_SYMBOL(iSeries_vio_dev);

static struct iommu_table veth_iommu_table;
static struct iommu_table vio_iommu_table;

static void __init iommu_vio_init(void)
{
	struct iommu_table *t;
	struct iommu_table_cb cb;
	unsigned long cbp;
	unsigned long itc_entries;

	cb.itc_busno = 255;    /* Bus 255 is the virtual bus */
	cb.itc_virtbus = 0xff; /* Ask for virtual bus */

	cbp = virt_to_abs(&cb);
	HvCallXm_getTceTableParms(cbp);

	itc_entries = cb.itc_size * PAGE_SIZE / sizeof(union tce_entry);
	veth_iommu_table.it_size        = itc_entries / 2;
	veth_iommu_table.it_busno       = cb.itc_busno;
	veth_iommu_table.it_offset      = cb.itc_offset;
	veth_iommu_table.it_index       = cb.itc_index;
	veth_iommu_table.it_type        = TCE_VB;
	veth_iommu_table.it_blocksize	= 1;

	t = iommu_init_table(&veth_iommu_table);

	if (!t)
		printk("Virtual Bus VETH TCE table failed.\n");

	vio_iommu_table.it_size         = itc_entries - veth_iommu_table.it_size;
	vio_iommu_table.it_busno        = cb.itc_busno;
	vio_iommu_table.it_offset       = cb.itc_offset +
					  veth_iommu_table.it_size;
	vio_iommu_table.it_index        = cb.itc_index;
	vio_iommu_table.it_type         = TCE_VB;
	vio_iommu_table.it_blocksize	= 1;

	t = iommu_init_table(&vio_iommu_table);

	if (!t)
		printk("Virtual Bus VIO TCE table failed.\n");
}

/**
 * vio_register_device_iseries: - Register a new iSeries vio device.
 * @voidev:	The device to register.
 */
static struct vio_dev *__init vio_register_device_iseries(char *type,
		uint32_t unit_num)
{
	struct vio_dev *viodev;

	/* allocate a vio_dev for this device */
	viodev = kmalloc(sizeof(struct vio_dev), GFP_KERNEL);
	if (!viodev)
		return NULL;
	memset(viodev, 0, sizeof(struct vio_dev));

	snprintf(viodev->dev.bus_id, BUS_ID_SIZE, "%s%d", type, unit_num);

	viodev->name = viodev->dev.bus_id;
	viodev->type = type;
	viodev->unit_address = unit_num;
	viodev->iommu_table = &vio_iommu_table;
	if (vio_register_device(viodev) == NULL) {
		kfree(viodev);
		return NULL;
	}
	return viodev;
}

void __init probe_bus_iseries(void)
{
	HvLpIndexMap vlan_map;
	struct vio_dev *viodev;
	int i;

	/* there is only one of each of these */
	vio_register_device_iseries("viocons", 0);
	vio_register_device_iseries("vscsi", 0);

	vlan_map = HvLpConfig_getVirtualLanIndexMap();
	for (i = 0; i < HVMAXARCHITECTEDVIRTUALLANS; i++) {
		if ((vlan_map & (0x8000 >> i)) == 0)
			continue;
		viodev = vio_register_device_iseries("vlan", i);
		/* veth is special and has it own iommu_table */
		viodev->iommu_table = &veth_iommu_table;
	}
	for (i = 0; i < HVMAXARCHITECTEDVIRTUALDISKS; i++)
		vio_register_device_iseries("viodasd", i);
	for (i = 0; i < HVMAXARCHITECTEDVIRTUALCDROMS; i++)
		vio_register_device_iseries("viocd", i);
	for (i = 0; i < HVMAXARCHITECTEDVIRTUALTAPES; i++)
		vio_register_device_iseries("viotape", i);
}

/**
 * vio_match_device_iseries: - Tell if a iSeries VIO device matches a
 *	vio_device_id
 */
static int vio_match_device_iseries(const struct vio_device_id *id,
		const struct vio_dev *dev)
{
	return strncmp(dev->type, id->type, strlen(id->type)) == 0;
}

static struct vio_bus_ops vio_bus_ops_iseries = {
	.match = vio_match_device_iseries,
};

/**
 * vio_bus_init_iseries: - Initialize the iSeries virtual IO bus
 */
static int __init vio_bus_init_iseries(void)
{
	int err;

	err = vio_bus_init(&vio_bus_ops_iseries);
	if (err == 0) {
		iommu_vio_init();
		vio_bus_device.iommu_table = &vio_iommu_table;
		iSeries_vio_dev = &vio_bus_device.dev;
		probe_bus_iseries();
	}
	return err;
}

__initcall(vio_bus_init_iseries);
