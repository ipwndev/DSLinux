/*
 *  linux/drivers/message/fusion/mptsas.c
 *      For use with LSI Logic PCI chip/adapter(s)
 *      running LSI Logic Fusion MPT (Message Passing Technology) firmware.
 *
 *  Copyright (c) 1999-2005 LSI Logic Corporation
 *  (mailto:mpt_linux_developer@lsil.com)
 *  Copyright (c) 2005 Dell
 */
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    NO WARRANTY
    THE PROGRAM IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT
    LIMITATION, ANY WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT,
    MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Each Recipient is
    solely responsible for determining the appropriateness of using and
    distributing the Program and assumes all risks associated with its
    exercise of rights under this Agreement, including but not limited to
    the risks and costs of program errors, damage to or loss of data,
    programs or equipment, and unavailability or interruption of operations.

    DISCLAIMER OF LIABILITY
    NEITHER RECIPIENT NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
    USE OR DISTRIBUTION OF THE PROGRAM OR THE EXERCISE OF ANY RIGHTS GRANTED
    HEREUNDER, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport_sas.h>

#include "mptbase.h"
#include "mptscsih.h"


#define my_NAME		"Fusion MPT SAS Host driver"
#define my_VERSION	MPT_LINUX_VERSION_COMMON
#define MYNAM		"mptsas"

MODULE_AUTHOR(MODULEAUTHOR);
MODULE_DESCRIPTION(my_NAME);
MODULE_LICENSE("GPL");

static int mpt_pq_filter;
module_param(mpt_pq_filter, int, 0);
MODULE_PARM_DESC(mpt_pq_filter,
		"Enable peripheral qualifier filter: enable=1  "
		"(default=0)");

static int mpt_pt_clear;
module_param(mpt_pt_clear, int, 0);
MODULE_PARM_DESC(mpt_pt_clear,
		"Clear persistency table: enable=1  "
		"(default=MPTSCSIH_PT_CLEAR=0)");

static int	mptsasDoneCtx = -1;
static int	mptsasTaskCtx = -1;
static int	mptsasInternalCtx = -1; /* Used only for internal commands */


/*
 * SAS topology structures
 *
 * The MPT Fusion firmware interface spreads information about the
 * SAS topology over many manufacture pages, thus we need some data
 * structure to collect it and process it for the SAS transport class.
 */

struct mptsas_devinfo {
	u16	handle;		/* unique id to address this device */
	u8	phy_id;		/* phy number of parent device */
	u8	port_id;	/* sas physical port this device
				   is assoc'd with */
	u8	target;		/* logical target id of this device */
	u8	bus;		/* logical bus number of this device */
	u64	sas_address;    /* WWN of this device,
				   SATA is assigned by HBA,expander */
	u32	device_info;	/* bitfield detailed info about this device */
};

struct mptsas_phyinfo {
	u8	phy_id; 		/* phy index */
	u8	port_id; 		/* port number this phy is part of */
	u8	negotiated_link_rate;	/* nego'd link rate for this phy */
	u8	hw_link_rate; 		/* hardware max/min phys link rate */
	u8	programmed_link_rate;	/* programmed max/min phy link rate */
	struct mptsas_devinfo identify;	/* point to phy device info */
	struct mptsas_devinfo attached;	/* point to attached device info */
	struct sas_rphy *rphy;
};

struct mptsas_portinfo {
	struct list_head list;
	u16		handle;		/* unique id to address this */
	u8		num_phys;	/* number of phys */
	struct mptsas_phyinfo *phy_info;
};

/*
 * This is pretty ugly.  We will be able to seriously clean it up
 * once the DV code in mptscsih goes away and we can properly
 * implement ->target_alloc.
 */
static int
mptsas_slave_alloc(struct scsi_device *device)
{
	struct Scsi_Host	*host = device->host;
	MPT_SCSI_HOST		*hd = (MPT_SCSI_HOST *)host->hostdata;
	struct sas_rphy		*rphy;
	struct mptsas_portinfo	*p;
	VirtDevice		*vdev;
	uint			target = device->id;
	int i;

	if ((vdev = hd->Targets[target]) != NULL)
		goto out;

	vdev = kmalloc(sizeof(VirtDevice), GFP_KERNEL);
	if (!vdev) {
		printk(MYIOC_s_ERR_FMT "slave_alloc kmalloc(%zd) FAILED!\n",
				hd->ioc->name, sizeof(VirtDevice));
		return -ENOMEM;
	}

	memset(vdev, 0, sizeof(VirtDevice));
	vdev->tflags = MPT_TARGET_FLAGS_Q_YES|MPT_TARGET_FLAGS_VALID_INQUIRY;
	vdev->ioc_id = hd->ioc->id;

	rphy = dev_to_rphy(device->sdev_target->dev.parent);
	list_for_each_entry(p, &hd->ioc->sas_topology, list) {
		for (i = 0; i < p->num_phys; i++) {
			if (p->phy_info[i].attached.sas_address ==
					rphy->identify.sas_address) {
				vdev->target_id =
					p->phy_info[i].attached.target;
				vdev->bus_id = p->phy_info[i].attached.bus;
				hd->Targets[device->id] = vdev;
				goto out;
			}
		}
	}

	printk("No matching SAS device found!!\n");
	kfree(vdev);
	return -ENODEV;

 out:
	vdev->num_luns++;
	device->hostdata = vdev;
	return 0;
}

static struct scsi_host_template mptsas_driver_template = {
	.proc_name			= "mptsas",
	.proc_info			= mptscsih_proc_info,
	.name				= "MPT SPI Host",
	.info				= mptscsih_info,
	.queuecommand			= mptscsih_qcmd,
	.slave_alloc			= mptsas_slave_alloc,
	.slave_configure		= mptscsih_slave_configure,
	.slave_destroy			= mptscsih_slave_destroy,
	.change_queue_depth 		= mptscsih_change_queue_depth,
	.eh_abort_handler		= mptscsih_abort,
	.eh_device_reset_handler	= mptscsih_dev_reset,
	.eh_bus_reset_handler		= mptscsih_bus_reset,
	.eh_host_reset_handler		= mptscsih_host_reset,
	.bios_param			= mptscsih_bios_param,
	.can_queue			= MPT_FC_CAN_QUEUE,
	.this_id			= -1,
	.sg_tablesize			= MPT_SCSI_SG_DEPTH,
	.max_sectors			= 8192,
	.cmd_per_lun			= 7,
	.use_clustering			= ENABLE_CLUSTERING,
};

static struct sas_function_template mptsas_transport_functions = {
};

static struct scsi_transport_template *mptsas_transport_template;

#ifdef SASDEBUG
static void mptsas_print_phy_data(MPI_SAS_IO_UNIT0_PHY_DATA *phy_data)
{
	printk("---- IO UNIT PAGE 0 ------------\n");
	printk("Handle=0x%X\n",
		le16_to_cpu(phy_data->AttachedDeviceHandle));
	printk("Controller Handle=0x%X\n",
		le16_to_cpu(phy_data->ControllerDevHandle));
	printk("Port=0x%X\n", phy_data->Port);
	printk("Port Flags=0x%X\n", phy_data->PortFlags);
	printk("PHY Flags=0x%X\n", phy_data->PhyFlags);
	printk("Negotiated Link Rate=0x%X\n", phy_data->NegotiatedLinkRate);
	printk("Controller PHY Device Info=0x%X\n",
		le32_to_cpu(phy_data->ControllerPhyDeviceInfo));
	printk("DiscoveryStatus=0x%X\n",
		le32_to_cpu(phy_data->DiscoveryStatus));
	printk("\n");
}

static void mptsas_print_phy_pg0(SasPhyPage0_t *pg0)
{
	__le64 sas_address;

	memcpy(&sas_address, &pg0->SASAddress, sizeof(__le64));

	printk("---- SAS PHY PAGE 0 ------------\n");
	printk("Attached Device Handle=0x%X\n",
			le16_to_cpu(pg0->AttachedDevHandle));
	printk("SAS Address=0x%llX\n",
			(unsigned long long)le64_to_cpu(sas_address));
	printk("Attached PHY Identifier=0x%X\n", pg0->AttachedPhyIdentifier);
	printk("Attached Device Info=0x%X\n",
			le32_to_cpu(pg0->AttachedDeviceInfo));
	printk("Programmed Link Rate=0x%X\n", pg0->ProgrammedLinkRate);
	printk("Change Count=0x%X\n", pg0->ChangeCount);
	printk("PHY Info=0x%X\n", le32_to_cpu(pg0->PhyInfo));
	printk("\n");
}

static void mptsas_print_device_pg0(SasDevicePage0_t *pg0)
{
	__le64 sas_address;

	memcpy(&sas_address, &pg0->SASAddress, sizeof(__le64));

	printk("---- SAS DEVICE PAGE 0 ---------\n");
	printk("Handle=0x%X\n" ,le16_to_cpu(pg0->DevHandle));
	printk("Enclosure Handle=0x%X\n", le16_to_cpu(pg0->EnclosureHandle));
	printk("Slot=0x%X\n", le16_to_cpu(pg0->Slot));
	printk("SAS Address=0x%llX\n", le64_to_cpu(sas_address));
	printk("Target ID=0x%X\n", pg0->TargetID);
	printk("Bus=0x%X\n", pg0->Bus);
	printk("Parent Phy Num=0x%X\n", pg0->PhyNum);
	printk("Access Status=0x%X\n", le16_to_cpu(pg0->AccessStatus));
	printk("Device Info=0x%X\n", le32_to_cpu(pg0->DeviceInfo));
	printk("Flags=0x%X\n", le16_to_cpu(pg0->Flags));
	printk("Physical Port=0x%X\n", pg0->PhysicalPort);
	printk("\n");
}

static void mptsas_print_expander_pg1(SasExpanderPage1_t *pg1)
{
	printk("---- SAS EXPANDER PAGE 1 ------------\n");

	printk("Physical Port=0x%X\n", pg1->PhysicalPort);
	printk("PHY Identifier=0x%X\n", pg1->PhyIdentifier);
	printk("Negotiated Link Rate=0x%X\n", pg1->NegotiatedLinkRate);
	printk("Programmed Link Rate=0x%X\n", pg1->ProgrammedLinkRate);
	printk("Hardware Link Rate=0x%X\n", pg1->HwLinkRate);
	printk("Owner Device Handle=0x%X\n",
			le16_to_cpu(pg1->OwnerDevHandle));
	printk("Attached Device Handle=0x%X\n",
			le16_to_cpu(pg1->AttachedDevHandle));
}
#else
#define mptsas_print_phy_data(phy_data)		do { } while (0)
#define mptsas_print_phy_pg0(pg0)		do { } while (0)
#define mptsas_print_device_pg0(pg0)		do { } while (0)
#define mptsas_print_expander_pg1(pg1)		do { } while (0)
#endif

static int
mptsas_sas_io_unit_pg0(MPT_ADAPTER *ioc, struct mptsas_portinfo *port_info)
{
	ConfigExtendedPageHeader_t hdr;
	CONFIGPARMS cfg;
	SasIOUnitPage0_t *buffer;
	dma_addr_t dma_handle;
	int error, i;

	hdr.PageVersion = MPI_SASIOUNITPAGE0_PAGEVERSION;
	hdr.ExtPageLength = 0;
	hdr.PageNumber = 0;
	hdr.Reserved1 = 0;
	hdr.Reserved2 = 0;
	hdr.PageType = MPI_CONFIG_PAGETYPE_EXTENDED;
	hdr.ExtPageType = MPI_CONFIG_EXTPAGETYPE_SAS_IO_UNIT;

	cfg.cfghdr.ehdr = &hdr;
	cfg.physAddr = -1;
	cfg.pageAddr = 0;
	cfg.action = MPI_CONFIG_ACTION_PAGE_HEADER;
	cfg.dir = 0;	/* read */
	cfg.timeout = 10;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out;
	if (!hdr.ExtPageLength) {
		error = -ENXIO;
		goto out;
	}

	buffer = pci_alloc_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
					    &dma_handle);
	if (!buffer) {
		error = -ENOMEM;
		goto out;
	}

	cfg.physAddr = dma_handle;
	cfg.action = MPI_CONFIG_ACTION_PAGE_READ_CURRENT;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out_free_consistent;

	port_info->num_phys = buffer->NumPhys;
	port_info->phy_info = kcalloc(port_info->num_phys,
		sizeof(struct mptsas_phyinfo),GFP_KERNEL);
	if (!port_info->phy_info) {
		error = -ENOMEM;
		goto out_free_consistent;
	}

	for (i = 0; i < port_info->num_phys; i++) {
		mptsas_print_phy_data(&buffer->PhyData[i]);
		port_info->phy_info[i].phy_id = i;
		port_info->phy_info[i].port_id =
		    buffer->PhyData[i].Port;
		port_info->phy_info[i].negotiated_link_rate =
		    buffer->PhyData[i].NegotiatedLinkRate;
	}

 out_free_consistent:
	pci_free_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
			    buffer, dma_handle);
 out:
	return error;
}

static int
mptsas_sas_phy_pg0(MPT_ADAPTER *ioc, struct mptsas_phyinfo *phy_info,
		u32 form, u32 form_specific)
{
	ConfigExtendedPageHeader_t hdr;
	CONFIGPARMS cfg;
	SasPhyPage0_t *buffer;
	dma_addr_t dma_handle;
	int error;

	hdr.PageVersion = MPI_SASPHY0_PAGEVERSION;
	hdr.ExtPageLength = 0;
	hdr.PageNumber = 0;
	hdr.Reserved1 = 0;
	hdr.Reserved2 = 0;
	hdr.PageType = MPI_CONFIG_PAGETYPE_EXTENDED;
	hdr.ExtPageType = MPI_CONFIG_EXTPAGETYPE_SAS_PHY;

	cfg.cfghdr.ehdr = &hdr;
	cfg.dir = 0;	/* read */
	cfg.timeout = 10;

	/* Get Phy Pg 0 for each Phy. */
	cfg.physAddr = -1;
	cfg.pageAddr = form + form_specific;
	cfg.action = MPI_CONFIG_ACTION_PAGE_HEADER;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out;

	if (!hdr.ExtPageLength) {
		error = -ENXIO;
		goto out;
	}

	buffer = pci_alloc_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
				      &dma_handle);
	if (!buffer) {
		error = -ENOMEM;
		goto out;
	}

	cfg.physAddr = dma_handle;
	cfg.action = MPI_CONFIG_ACTION_PAGE_READ_CURRENT;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out_free_consistent;

	mptsas_print_phy_pg0(buffer);

	phy_info->hw_link_rate = buffer->HwLinkRate;
	phy_info->programmed_link_rate = buffer->ProgrammedLinkRate;
	phy_info->identify.handle = le16_to_cpu(buffer->OwnerDevHandle);
	phy_info->attached.handle = le16_to_cpu(buffer->AttachedDevHandle);

 out_free_consistent:
	pci_free_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
			    buffer, dma_handle);
 out:
	return error;
}

static int
mptsas_sas_device_pg0(MPT_ADAPTER *ioc, struct mptsas_devinfo *device_info,
		u32 form, u32 form_specific)
{
	ConfigExtendedPageHeader_t hdr;
	CONFIGPARMS cfg;
	SasDevicePage0_t *buffer;
	dma_addr_t dma_handle;
	__le64 sas_address;
	int error;

	hdr.PageVersion = MPI_SASDEVICE0_PAGEVERSION;
	hdr.ExtPageLength = 0;
	hdr.PageNumber = 0;
	hdr.Reserved1 = 0;
	hdr.Reserved2 = 0;
	hdr.PageType = MPI_CONFIG_PAGETYPE_EXTENDED;
	hdr.ExtPageType = MPI_CONFIG_EXTPAGETYPE_SAS_DEVICE;

	cfg.cfghdr.ehdr = &hdr;
	cfg.pageAddr = form + form_specific;
	cfg.physAddr = -1;
	cfg.action = MPI_CONFIG_ACTION_PAGE_HEADER;
	cfg.dir = 0;	/* read */
	cfg.timeout = 10;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out;
	if (!hdr.ExtPageLength) {
		error = -ENXIO;
		goto out;
	}

	buffer = pci_alloc_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
				      &dma_handle);
	if (!buffer) {
		error = -ENOMEM;
		goto out;
	}

	cfg.physAddr = dma_handle;
	cfg.action = MPI_CONFIG_ACTION_PAGE_READ_CURRENT;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out_free_consistent;

	mptsas_print_device_pg0(buffer);

	device_info->handle = le16_to_cpu(buffer->DevHandle);
	device_info->phy_id = buffer->PhyNum;
	device_info->port_id = buffer->PhysicalPort;
	device_info->target = buffer->TargetID;
	device_info->bus = buffer->Bus;
	memcpy(&sas_address, &buffer->SASAddress, sizeof(__le64));
	device_info->sas_address = le64_to_cpu(sas_address);
	device_info->device_info =
	    le32_to_cpu(buffer->DeviceInfo);

 out_free_consistent:
	pci_free_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
			    buffer, dma_handle);
 out:
	return error;
}

static int
mptsas_sas_expander_pg0(MPT_ADAPTER *ioc, struct mptsas_portinfo *port_info,
		u32 form, u32 form_specific)
{
	ConfigExtendedPageHeader_t hdr;
	CONFIGPARMS cfg;
	SasExpanderPage0_t *buffer;
	dma_addr_t dma_handle;
	int error;

	hdr.PageVersion = MPI_SASEXPANDER0_PAGEVERSION;
	hdr.ExtPageLength = 0;
	hdr.PageNumber = 0;
	hdr.Reserved1 = 0;
	hdr.Reserved2 = 0;
	hdr.PageType = MPI_CONFIG_PAGETYPE_EXTENDED;
	hdr.ExtPageType = MPI_CONFIG_EXTPAGETYPE_SAS_EXPANDER;

	cfg.cfghdr.ehdr = &hdr;
	cfg.physAddr = -1;
	cfg.pageAddr = form + form_specific;
	cfg.action = MPI_CONFIG_ACTION_PAGE_HEADER;
	cfg.dir = 0;	/* read */
	cfg.timeout = 10;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out;

	if (!hdr.ExtPageLength) {
		error = -ENXIO;
		goto out;
	}

	buffer = pci_alloc_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
				      &dma_handle);
	if (!buffer) {
		error = -ENOMEM;
		goto out;
	}

	cfg.physAddr = dma_handle;
	cfg.action = MPI_CONFIG_ACTION_PAGE_READ_CURRENT;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out_free_consistent;

	/* save config data */
	port_info->num_phys = buffer->NumPhys;
	port_info->handle = le16_to_cpu(buffer->DevHandle);
	port_info->phy_info = kcalloc(port_info->num_phys,
		sizeof(struct mptsas_phyinfo),GFP_KERNEL);
	if (!port_info->phy_info) {
		error = -ENOMEM;
		goto out_free_consistent;
	}

 out_free_consistent:
	pci_free_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
			    buffer, dma_handle);
 out:
	return error;
}

static int
mptsas_sas_expander_pg1(MPT_ADAPTER *ioc, struct mptsas_phyinfo *phy_info,
		u32 form, u32 form_specific)
{
	ConfigExtendedPageHeader_t hdr;
	CONFIGPARMS cfg;
	SasExpanderPage1_t *buffer;
	dma_addr_t dma_handle;
	int error;

	hdr.PageVersion = MPI_SASEXPANDER0_PAGEVERSION;
	hdr.ExtPageLength = 0;
	hdr.PageNumber = 1;
	hdr.Reserved1 = 0;
	hdr.Reserved2 = 0;
	hdr.PageType = MPI_CONFIG_PAGETYPE_EXTENDED;
	hdr.ExtPageType = MPI_CONFIG_EXTPAGETYPE_SAS_EXPANDER;

	cfg.cfghdr.ehdr = &hdr;
	cfg.physAddr = -1;
	cfg.pageAddr = form + form_specific;
	cfg.action = MPI_CONFIG_ACTION_PAGE_HEADER;
	cfg.dir = 0;	/* read */
	cfg.timeout = 10;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out;

	if (!hdr.ExtPageLength) {
		error = -ENXIO;
		goto out;
	}

	buffer = pci_alloc_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
				      &dma_handle);
	if (!buffer) {
		error = -ENOMEM;
		goto out;
	}

	cfg.physAddr = dma_handle;
	cfg.action = MPI_CONFIG_ACTION_PAGE_READ_CURRENT;

	error = mpt_config(ioc, &cfg);
	if (error)
		goto out_free_consistent;


	mptsas_print_expander_pg1(buffer);

	/* save config data */
	phy_info->phy_id = buffer->PhyIdentifier;
	phy_info->port_id = buffer->PhysicalPort;
	phy_info->negotiated_link_rate = buffer->NegotiatedLinkRate;
	phy_info->programmed_link_rate = buffer->ProgrammedLinkRate;
	phy_info->hw_link_rate = buffer->HwLinkRate;
	phy_info->identify.handle = le16_to_cpu(buffer->OwnerDevHandle);
	phy_info->attached.handle = le16_to_cpu(buffer->AttachedDevHandle);


 out_free_consistent:
	pci_free_consistent(ioc->pcidev, hdr.ExtPageLength * 4,
			    buffer, dma_handle);
 out:
	return error;
}

static void
mptsas_parse_device_info(struct sas_identify *identify,
		struct mptsas_devinfo *device_info)
{
	u16 protocols;

	identify->sas_address = device_info->sas_address;
	identify->phy_identifier = device_info->phy_id;

	/*
	 * Fill in Phy Initiator Port Protocol.
	 * Bits 6:3, more than one bit can be set, fall through cases.
	 */
	protocols = device_info->device_info & 0x78;
	identify->initiator_port_protocols = 0;
	if (protocols & MPI_SAS_DEVICE_INFO_SSP_INITIATOR)
		identify->initiator_port_protocols |= SAS_PROTOCOL_SSP;
	if (protocols & MPI_SAS_DEVICE_INFO_STP_INITIATOR)
		identify->initiator_port_protocols |= SAS_PROTOCOL_STP;
	if (protocols & MPI_SAS_DEVICE_INFO_SMP_INITIATOR)
		identify->initiator_port_protocols |= SAS_PROTOCOL_SMP;
	if (protocols & MPI_SAS_DEVICE_INFO_SATA_HOST)
		identify->initiator_port_protocols |= SAS_PROTOCOL_SATA;

	/*
	 * Fill in Phy Target Port Protocol.
	 * Bits 10:7, more than one bit can be set, fall through cases.
	 */
	protocols = device_info->device_info & 0x780;
	identify->target_port_protocols = 0;
	if (protocols & MPI_SAS_DEVICE_INFO_SSP_TARGET)
		identify->target_port_protocols |= SAS_PROTOCOL_SSP;
	if (protocols & MPI_SAS_DEVICE_INFO_STP_TARGET)
		identify->target_port_protocols |= SAS_PROTOCOL_STP;
	if (protocols & MPI_SAS_DEVICE_INFO_SMP_TARGET)
		identify->target_port_protocols |= SAS_PROTOCOL_SMP;
	if (protocols & MPI_SAS_DEVICE_INFO_SATA_DEVICE)
		identify->target_port_protocols |= SAS_PROTOCOL_SATA;

	/*
	 * Fill in Attached device type.
	 */
	switch (device_info->device_info &
			MPI_SAS_DEVICE_INFO_MASK_DEVICE_TYPE) {
	case MPI_SAS_DEVICE_INFO_NO_DEVICE:
		identify->device_type = SAS_PHY_UNUSED;
		break;
	case MPI_SAS_DEVICE_INFO_END_DEVICE:
		identify->device_type = SAS_END_DEVICE;
		break;
	case MPI_SAS_DEVICE_INFO_EDGE_EXPANDER:
		identify->device_type = SAS_EDGE_EXPANDER_DEVICE;
		break;
	case MPI_SAS_DEVICE_INFO_FANOUT_EXPANDER:
		identify->device_type = SAS_FANOUT_EXPANDER_DEVICE;
		break;
	}
}

static int mptsas_probe_one_phy(struct device *dev,
		struct mptsas_phyinfo *phy_info, int index)
{
	struct sas_phy *port;
	int error;

	port = sas_phy_alloc(dev, index);
	if (!port)
		return -ENOMEM;

	port->port_identifier = phy_info->port_id;
	mptsas_parse_device_info(&port->identify, &phy_info->identify);

	/*
	 * Set Negotiated link rate.
	 */
	switch (phy_info->negotiated_link_rate) {
	case MPI_SAS_IOUNIT0_RATE_PHY_DISABLED:
		port->negotiated_linkrate = SAS_PHY_DISABLED;
		break;
	case MPI_SAS_IOUNIT0_RATE_FAILED_SPEED_NEGOTIATION:
		port->negotiated_linkrate = SAS_LINK_RATE_FAILED;
		break;
	case MPI_SAS_IOUNIT0_RATE_1_5:
		port->negotiated_linkrate = SAS_LINK_RATE_1_5_GBPS;
		break;
	case MPI_SAS_IOUNIT0_RATE_3_0:
		port->negotiated_linkrate = SAS_LINK_RATE_3_0_GBPS;
		break;
	case MPI_SAS_IOUNIT0_RATE_SATA_OOB_COMPLETE:
	case MPI_SAS_IOUNIT0_RATE_UNKNOWN:
	default:
		port->negotiated_linkrate = SAS_LINK_RATE_UNKNOWN;
		break;
	}

	/*
	 * Set Max hardware link rate.
	 */
	switch (phy_info->hw_link_rate & MPI_SAS_PHY0_PRATE_MAX_RATE_MASK) {
	case MPI_SAS_PHY0_HWRATE_MAX_RATE_1_5:
		port->maximum_linkrate_hw = SAS_LINK_RATE_1_5_GBPS;
		break;
	case MPI_SAS_PHY0_PRATE_MAX_RATE_3_0:
		port->maximum_linkrate_hw = SAS_LINK_RATE_3_0_GBPS;
		break;
	default:
		break;
	}

	/*
	 * Set Max programmed link rate.
	 */
	switch (phy_info->programmed_link_rate &
			MPI_SAS_PHY0_PRATE_MAX_RATE_MASK) {
	case MPI_SAS_PHY0_PRATE_MAX_RATE_1_5:
		port->maximum_linkrate = SAS_LINK_RATE_1_5_GBPS;
		break;
	case MPI_SAS_PHY0_PRATE_MAX_RATE_3_0:
		port->maximum_linkrate = SAS_LINK_RATE_3_0_GBPS;
		break;
	default:
		break;
	}

	/*
	 * Set Min hardware link rate.
	 */
	switch (phy_info->hw_link_rate & MPI_SAS_PHY0_HWRATE_MIN_RATE_MASK) {
	case MPI_SAS_PHY0_HWRATE_MIN_RATE_1_5:
		port->minimum_linkrate_hw = SAS_LINK_RATE_1_5_GBPS;
		break;
	case MPI_SAS_PHY0_PRATE_MIN_RATE_3_0:
		port->minimum_linkrate_hw = SAS_LINK_RATE_3_0_GBPS;
		break;
	default:
		break;
	}

	/*
	 * Set Min programmed link rate.
	 */
	switch (phy_info->programmed_link_rate &
			MPI_SAS_PHY0_PRATE_MIN_RATE_MASK) {
	case MPI_SAS_PHY0_PRATE_MIN_RATE_1_5:
		port->minimum_linkrate = SAS_LINK_RATE_1_5_GBPS;
		break;
	case MPI_SAS_PHY0_PRATE_MIN_RATE_3_0:
		port->minimum_linkrate = SAS_LINK_RATE_3_0_GBPS;
		break;
	default:
		break;
	}

	error = sas_phy_add(port);
	if (error) {
		sas_phy_free(port);
		return error;
	}

	if (phy_info->attached.handle) {
		struct sas_rphy *rphy;

		rphy = sas_rphy_alloc(port);
		if (!rphy)
			return 0; /* non-fatal: an rphy can be added later */

		mptsas_parse_device_info(&rphy->identify, &phy_info->attached);
		error = sas_rphy_add(rphy);
		if (error) {
			sas_rphy_free(rphy);
			return error;
		}

		phy_info->rphy = rphy;
	}

	return 0;
}

static int
mptsas_probe_hba_phys(MPT_ADAPTER *ioc, int *index)
{
	struct mptsas_portinfo *port_info;
	u32 handle = 0xFFFF;
	int error = -ENOMEM, i;

	port_info = kmalloc(sizeof(*port_info), GFP_KERNEL);
	if (!port_info)
		goto out;
	memset(port_info, 0, sizeof(*port_info));

	error = mptsas_sas_io_unit_pg0(ioc, port_info);
	if (error)
		goto out_free_port_info;

	list_add_tail(&port_info->list, &ioc->sas_topology);

	for (i = 0; i < port_info->num_phys; i++) {
		mptsas_sas_phy_pg0(ioc, &port_info->phy_info[i],
			(MPI_SAS_PHY_PGAD_FORM_PHY_NUMBER <<
			 MPI_SAS_PHY_PGAD_FORM_SHIFT), i);

		mptsas_sas_device_pg0(ioc, &port_info->phy_info[i].identify,
			(MPI_SAS_DEVICE_PGAD_FORM_GET_NEXT_HANDLE <<
			 MPI_SAS_DEVICE_PGAD_FORM_SHIFT), handle);
		port_info->phy_info[i].identify.phy_id =
		    port_info->phy_info[i].phy_id;
		handle = port_info->phy_info[i].identify.handle;

		if (port_info->phy_info[i].attached.handle) {
			mptsas_sas_device_pg0(ioc,
				&port_info->phy_info[i].attached,
				(MPI_SAS_DEVICE_PGAD_FORM_HANDLE <<
				 MPI_SAS_DEVICE_PGAD_FORM_SHIFT),
				port_info->phy_info[i].attached.handle);
		}

		mptsas_probe_one_phy(&ioc->sh->shost_gendev,
				     &port_info->phy_info[i], *index);
		(*index)++;
	}

	return 0;

 out_free_port_info:
	kfree(port_info);
 out:
	return error;
}

static int
mptsas_probe_expander_phys(MPT_ADAPTER *ioc, u32 *handle, int *index)
{
	struct mptsas_portinfo *port_info, *p;
	int error = -ENOMEM, i, j;

	port_info = kmalloc(sizeof(*port_info), GFP_KERNEL);
	if (!port_info)
		goto out;
	memset(port_info, 0, sizeof(*port_info));

	error = mptsas_sas_expander_pg0(ioc, port_info,
		(MPI_SAS_EXPAND_PGAD_FORM_GET_NEXT_HANDLE <<
		 MPI_SAS_EXPAND_PGAD_FORM_SHIFT), *handle);
	if (error)
		goto out_free_port_info;

	*handle = port_info->handle;

	list_add_tail(&port_info->list, &ioc->sas_topology);
	for (i = 0; i < port_info->num_phys; i++) {
		struct device *parent;

		mptsas_sas_expander_pg1(ioc, &port_info->phy_info[i],
			(MPI_SAS_EXPAND_PGAD_FORM_HANDLE_PHY_NUM <<
			 MPI_SAS_EXPAND_PGAD_FORM_SHIFT), (i << 16) + *handle);

		if (port_info->phy_info[i].identify.handle) {
			mptsas_sas_device_pg0(ioc,
				&port_info->phy_info[i].identify,
				(MPI_SAS_DEVICE_PGAD_FORM_HANDLE <<
				 MPI_SAS_DEVICE_PGAD_FORM_SHIFT),
				port_info->phy_info[i].identify.handle);
			port_info->phy_info[i].identify.phy_id =
			    port_info->phy_info[i].phy_id;
		}

		if (port_info->phy_info[i].attached.handle) {
			mptsas_sas_device_pg0(ioc,
				&port_info->phy_info[i].attached,
				(MPI_SAS_DEVICE_PGAD_FORM_HANDLE <<
				 MPI_SAS_DEVICE_PGAD_FORM_SHIFT),
				port_info->phy_info[i].attached.handle);
		}

		/*
		 * If we find a parent port handle this expander is
		 * attached to another expander, else it hangs of the
		 * HBA phys.
		 */
		parent = &ioc->sh->shost_gendev;
		list_for_each_entry(p, &ioc->sas_topology, list) {
			for (j = 0; j < p->num_phys; j++) {
				if (port_info->phy_info[i].identify.handle ==
						p->phy_info[j].attached.handle)
					parent = &p->phy_info[j].rphy->dev;
			}
		}

		mptsas_probe_one_phy(parent, &port_info->phy_info[i], *index);
		(*index)++;
	}

	return 0;

 out_free_port_info:
	kfree(port_info);
 out:
	return error;
}

static void
mptsas_scan_sas_topology(MPT_ADAPTER *ioc)
{
	u32 handle = 0xFFFF;
	int index = 0;

	mptsas_probe_hba_phys(ioc, &index);
	while (!mptsas_probe_expander_phys(ioc, &handle, &index))
		;
}

static int
mptsas_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct Scsi_Host	*sh;
	MPT_SCSI_HOST		*hd;
	MPT_ADAPTER 		*ioc;
	unsigned long		 flags;
	int			 sz, ii;
	int			 numSGE = 0;
	int			 scale;
	int			 ioc_cap;
	u8			*mem;
	int			error=0;
	int			r;

	r = mpt_attach(pdev,id);
	if (r)
		return r;

	ioc = pci_get_drvdata(pdev);
	ioc->DoneCtx = mptsasDoneCtx;
	ioc->TaskCtx = mptsasTaskCtx;
	ioc->InternalCtx = mptsasInternalCtx;

	/*  Added sanity check on readiness of the MPT adapter.
	 */
	if (ioc->last_state != MPI_IOC_STATE_OPERATIONAL) {
		printk(MYIOC_s_WARN_FMT
		  "Skipping because it's not operational!\n",
		  ioc->name);
		return -ENODEV;
	}

	if (!ioc->active) {
		printk(MYIOC_s_WARN_FMT "Skipping because it's disabled!\n",
		  ioc->name);
		return -ENODEV;
	}

	/*  Sanity check - ensure at least 1 port is INITIATOR capable
	 */
	ioc_cap = 0;
	for (ii = 0; ii < ioc->facts.NumberOfPorts; ii++) {
		if (ioc->pfacts[ii].ProtocolFlags &
				MPI_PORTFACTS_PROTOCOL_INITIATOR)
			ioc_cap++;
	}

	if (!ioc_cap) {
		printk(MYIOC_s_WARN_FMT
			"Skipping ioc=%p because SCSI Initiator mode "
			"is NOT enabled!\n", ioc->name, ioc);
		return 0;
	}

	sh = scsi_host_alloc(&mptsas_driver_template, sizeof(MPT_SCSI_HOST));
	if (!sh) {
		printk(MYIOC_s_WARN_FMT
			"Unable to register controller with SCSI subsystem\n",
			ioc->name);
                return -1;
        }

	spin_lock_irqsave(&ioc->FreeQlock, flags);

	/* Attach the SCSI Host to the IOC structure
	 */
	ioc->sh = sh;

	sh->io_port = 0;
	sh->n_io_port = 0;
	sh->irq = 0;

	/* set 16 byte cdb's */
	sh->max_cmd_len = 16;

	sh->max_id = ioc->pfacts->MaxDevices + 1;

	sh->transportt = mptsas_transport_template;

	sh->max_lun = MPT_LAST_LUN + 1;
	sh->max_channel = 0;
	sh->this_id = ioc->pfacts[0].PortSCSIID;

	/* Required entry.
	 */
	sh->unique_id = ioc->id;

	INIT_LIST_HEAD(&ioc->sas_topology);

	/* Verify that we won't exceed the maximum
	 * number of chain buffers
	 * We can optimize:  ZZ = req_sz/sizeof(SGE)
	 * For 32bit SGE's:
	 *  numSGE = 1 + (ZZ-1)*(maxChain -1) + ZZ
	 *               + (req_sz - 64)/sizeof(SGE)
	 * A slightly different algorithm is required for
	 * 64bit SGEs.
	 */
	scale = ioc->req_sz/(sizeof(dma_addr_t) + sizeof(u32));
	if (sizeof(dma_addr_t) == sizeof(u64)) {
		numSGE = (scale - 1) *
		  (ioc->facts.MaxChainDepth-1) + scale +
		  (ioc->req_sz - 60) / (sizeof(dma_addr_t) +
		  sizeof(u32));
	} else {
		numSGE = 1 + (scale - 1) *
		  (ioc->facts.MaxChainDepth-1) + scale +
		  (ioc->req_sz - 64) / (sizeof(dma_addr_t) +
		  sizeof(u32));
	}

	if (numSGE < sh->sg_tablesize) {
		/* Reset this value */
		dprintk((MYIOC_s_INFO_FMT
		  "Resetting sg_tablesize to %d from %d\n",
		  ioc->name, numSGE, sh->sg_tablesize));
		sh->sg_tablesize = numSGE;
	}

	spin_unlock_irqrestore(&ioc->FreeQlock, flags);

	hd = (MPT_SCSI_HOST *) sh->hostdata;
	hd->ioc = ioc;

	/* SCSI needs scsi_cmnd lookup table!
	 * (with size equal to req_depth*PtrSz!)
	 */
	sz = ioc->req_depth * sizeof(void *);
	mem = kmalloc(sz, GFP_ATOMIC);
	if (mem == NULL) {
		error = -ENOMEM;
		goto mptsas_probe_failed;
	}

	memset(mem, 0, sz);
	hd->ScsiLookup = (struct scsi_cmnd **) mem;

	dprintk((MYIOC_s_INFO_FMT "ScsiLookup @ %p, sz=%d\n",
		 ioc->name, hd->ScsiLookup, sz));

	/* Allocate memory for the device structures.
	 * A non-Null pointer at an offset
	 * indicates a device exists.
	 * max_id = 1 + maximum id (hosts.h)
	 */
	sz = sh->max_id * sizeof(void *);
	mem = kmalloc(sz, GFP_ATOMIC);
	if (mem == NULL) {
		error = -ENOMEM;
		goto mptsas_probe_failed;
	}

	memset(mem, 0, sz);
	hd->Targets = (VirtDevice **) mem;

	dprintk((KERN_INFO
	  "  Targets @ %p, sz=%d\n", hd->Targets, sz));

	/* Clear the TM flags
	 */
	hd->tmPending = 0;
	hd->tmState = TM_STATE_NONE;
	hd->resetPending = 0;
	hd->abortSCpnt = NULL;

	/* Clear the pointer used to store
	 * single-threaded commands, i.e., those
	 * issued during a bus scan, dv and
	 * configuration pages.
	 */
	hd->cmdPtr = NULL;

	/* Initialize this SCSI Hosts' timers
	 * To use, set the timer expires field
	 * and add_timer
	 */
	init_timer(&hd->timer);
	hd->timer.data = (unsigned long) hd;
	hd->timer.function = mptscsih_timer_expired;

	hd->mpt_pq_filter = mpt_pq_filter;
	ioc->sas_data.ptClear = mpt_pt_clear;

	if (ioc->sas_data.ptClear==1) {
		mptbase_sas_persist_operation(
		    ioc, MPI_SAS_OP_CLEAR_ALL_PERSISTENT);
	}

	ddvprintk((MYIOC_s_INFO_FMT
		"mpt_pq_filter %x mpt_pq_filter %x\n",
		ioc->name,
		mpt_pq_filter,
		mpt_pq_filter));

	init_waitqueue_head(&hd->scandv_waitq);
	hd->scandv_wait_done = 0;
	hd->last_queue_full = 0;

	error = scsi_add_host(sh, &ioc->pcidev->dev);
	if (error) {
		dprintk((KERN_ERR MYNAM
		  "scsi_add_host failed\n"));
		goto mptsas_probe_failed;
	}

	mptsas_scan_sas_topology(ioc);

	return 0;

mptsas_probe_failed:

	mptscsih_remove(pdev);
	return error;
}

static void __devexit mptsas_remove(struct pci_dev *pdev)
{
	MPT_ADAPTER *ioc = pci_get_drvdata(pdev);
	struct mptsas_portinfo *p, *n;

	sas_remove_host(ioc->sh);

	list_for_each_entry_safe(p, n, &ioc->sas_topology, list) {
		list_del(&p->list);
		kfree(p);
	}

	mptscsih_remove(pdev);
}

static struct pci_device_id mptsas_pci_table[] = {
	{ PCI_VENDOR_ID_LSI_LOGIC, PCI_DEVICE_ID_LSI_SAS1064,
		PCI_ANY_ID, PCI_ANY_ID },
	{ PCI_VENDOR_ID_LSI_LOGIC, PCI_DEVICE_ID_LSI_SAS1066,
		PCI_ANY_ID, PCI_ANY_ID },
	{ PCI_VENDOR_ID_LSI_LOGIC, PCI_DEVICE_ID_LSI_SAS1068,
		PCI_ANY_ID, PCI_ANY_ID },
	{ PCI_VENDOR_ID_LSI_LOGIC, PCI_DEVICE_ID_LSI_SAS1064E,
		PCI_ANY_ID, PCI_ANY_ID },
	{ PCI_VENDOR_ID_LSI_LOGIC, PCI_DEVICE_ID_LSI_SAS1066E,
		PCI_ANY_ID, PCI_ANY_ID },
	{ PCI_VENDOR_ID_LSI_LOGIC, PCI_DEVICE_ID_LSI_SAS1068E,
		PCI_ANY_ID, PCI_ANY_ID },
	{0}	/* Terminating entry */
};
MODULE_DEVICE_TABLE(pci, mptsas_pci_table);


static struct pci_driver mptsas_driver = {
	.name		= "mptsas",
	.id_table	= mptsas_pci_table,
	.probe		= mptsas_probe,
	.remove		= __devexit_p(mptsas_remove),
	.shutdown	= mptscsih_shutdown,
#ifdef CONFIG_PM
	.suspend	= mptscsih_suspend,
	.resume		= mptscsih_resume,
#endif
};

static int __init
mptsas_init(void)
{
	show_mptmod_ver(my_NAME, my_VERSION);

	mptsas_transport_template =
	    sas_attach_transport(&mptsas_transport_functions);
	if (!mptsas_transport_template)
		return -ENODEV;

	mptsasDoneCtx = mpt_register(mptscsih_io_done, MPTSAS_DRIVER);
	mptsasTaskCtx = mpt_register(mptscsih_taskmgmt_complete, MPTSAS_DRIVER);
	mptsasInternalCtx =
		mpt_register(mptscsih_scandv_complete, MPTSAS_DRIVER);

	if (mpt_event_register(mptsasDoneCtx, mptscsih_event_process) == 0) {
		devtprintk((KERN_INFO MYNAM
		  ": Registered for IOC event notifications\n"));
	}

	if (mpt_reset_register(mptsasDoneCtx, mptscsih_ioc_reset) == 0) {
		dprintk((KERN_INFO MYNAM
		  ": Registered for IOC reset notifications\n"));
	}

	return pci_register_driver(&mptsas_driver);
}

static void __exit
mptsas_exit(void)
{
	pci_unregister_driver(&mptsas_driver);
	sas_release_transport(mptsas_transport_template);

	mpt_reset_deregister(mptsasDoneCtx);
	mpt_event_deregister(mptsasDoneCtx);

	mpt_deregister(mptsasInternalCtx);
	mpt_deregister(mptsasTaskCtx);
	mpt_deregister(mptsasDoneCtx);
}

module_init(mptsas_init);
module_exit(mptsas_exit);
