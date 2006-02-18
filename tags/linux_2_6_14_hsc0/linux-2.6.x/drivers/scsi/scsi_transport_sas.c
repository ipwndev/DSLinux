/*
 * Copyright (C) 2005 Dell Inc.
 *	Released under GPL v2.
 *
 * Serial Attached SCSI (SAS) transport class.
 *
 * The SAS transport class contains common code to deal with SAS HBAs,
 * an aproximated representation of SAS topologies in the driver model,
 * and various sysfs attributes to expose these topologies and managment
 * interfaces to userspace.
 *
 * In addition to the basic SCSI core objects this transport class
 * introduces two additional intermediate objects:  The SAS PHY
 * as represented by struct sas_phy defines an "outgoing" PHY on
 * a SAS HBA or Expander, and the SAS remote PHY represented by
 * struct sas_rphy defines an "incoming" PHY on a SAS Expander or
 * end device.  Note that this is purely a software concept, the
 * underlying hardware for a PHY and a remote PHY is the exactly
 * the same.
 *
 * There is no concept of a SAS port in this code, users can see
 * what PHYs form a wide port based on the port_identifier attribute,
 * which is the same for all PHYs in a port.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>

#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport.h>
#include <scsi/scsi_transport_sas.h>


#define SAS_HOST_ATTRS		0
#define SAS_PORT_ATTRS		11
#define SAS_RPORT_ATTRS		5

struct sas_internal {
	struct scsi_transport_template t;
	struct sas_function_template *f;

	struct class_device_attribute private_host_attrs[SAS_HOST_ATTRS];
	struct class_device_attribute private_phy_attrs[SAS_PORT_ATTRS];
	struct class_device_attribute private_rphy_attrs[SAS_RPORT_ATTRS];

	struct transport_container phy_attr_cont;
	struct transport_container rphy_attr_cont;

	/*
	 * The array of null terminated pointers to attributes
	 * needed by scsi_sysfs.c
	 */
	struct class_device_attribute *host_attrs[SAS_HOST_ATTRS + 1];
	struct class_device_attribute *phy_attrs[SAS_PORT_ATTRS + 1];
	struct class_device_attribute *rphy_attrs[SAS_RPORT_ATTRS + 1];
};
#define to_sas_internal(tmpl)	container_of(tmpl, struct sas_internal, t)

struct sas_host_attrs {
	struct list_head rphy_list;
	spinlock_t lock;
	u32 next_target_id;
};
#define to_sas_host_attrs(host)	((struct sas_host_attrs *)(host)->shost_data)


/*
 * Hack to allow attributes of the same name in different objects.
 */
#define SAS_CLASS_DEVICE_ATTR(_prefix,_name,_mode,_show,_store) \
	struct class_device_attribute class_device_attr_##_prefix##_##_name = \
	__ATTR(_name,_mode,_show,_store)


/*
 * Pretty printing helpers
 */

#define sas_bitfield_name_match(title, table)			\
static ssize_t							\
get_sas_##title##_names(u32 table_key, char *buf)		\
{								\
	char *prefix = "";					\
	ssize_t len = 0;					\
	int i;							\
								\
	for (i = 0; i < sizeof(table)/sizeof(table[0]); i++) {	\
		if (table[i].value & table_key) {		\
			len += sprintf(buf + len, "%s%s",	\
				prefix, table[i].name);		\
			prefix = ", ";				\
		}						\
	}							\
	len += sprintf(buf + len, "\n");			\
	return len;						\
}

#define sas_bitfield_name_search(title, table)			\
static ssize_t							\
get_sas_##title##_names(u32 table_key, char *buf)		\
{								\
	ssize_t len = 0;					\
	int i;							\
								\
	for (i = 0; i < sizeof(table)/sizeof(table[0]); i++) {	\
		if (table[i].value == table_key) {		\
			len += sprintf(buf + len, "%s",		\
				table[i].name);			\
			break;					\
		}						\
	}							\
	len += sprintf(buf + len, "\n");			\
	return len;						\
}

static struct {
	u32		value;
	char		*name;
} sas_device_type_names[] = {
	{ SAS_PHY_UNUSED,		"unused" },
	{ SAS_END_DEVICE,		"end device" },
	{ SAS_EDGE_EXPANDER_DEVICE,	"edge expander" },
	{ SAS_FANOUT_EXPANDER_DEVICE,	"fanout expander" },
};
sas_bitfield_name_search(device_type, sas_device_type_names)


static struct {
	u32		value;
	char		*name;
} sas_protocol_names[] = {
	{ SAS_PROTOCOL_SATA,		"sata" },
	{ SAS_PROTOCOL_SMP,		"smp" },
	{ SAS_PROTOCOL_STP,		"stp" },
	{ SAS_PROTOCOL_SSP,		"ssp" },
};
sas_bitfield_name_match(protocol, sas_protocol_names)

static struct {
	u32		value;
	char		*name;
} sas_linkspeed_names[] = {
	{ SAS_LINK_RATE_UNKNOWN,	"Unknown" },
	{ SAS_PHY_DISABLED,		"Phy disabled" },
	{ SAS_LINK_RATE_FAILED,		"Link Rate failed" },
	{ SAS_SATA_SPINUP_HOLD,		"Spin-up hold" },
	{ SAS_LINK_RATE_1_5_GBPS,	"1.5 Gbit" },
	{ SAS_LINK_RATE_3_0_GBPS,	"3.0 Gbit" },
};
sas_bitfield_name_search(linkspeed, sas_linkspeed_names)


/*
 * SAS host attributes
 */

static int sas_host_setup(struct transport_container *tc, struct device *dev,
			  struct class_device *cdev)
{
	struct Scsi_Host *shost = dev_to_shost(dev);
	struct sas_host_attrs *sas_host = to_sas_host_attrs(shost);

	INIT_LIST_HEAD(&sas_host->rphy_list);
	spin_lock_init(&sas_host->lock);
	sas_host->next_target_id = 0;
	return 0;
}

static DECLARE_TRANSPORT_CLASS(sas_host_class,
		"sas_host", sas_host_setup, NULL, NULL);

static int sas_host_match(struct attribute_container *cont,
			    struct device *dev)
{
	struct Scsi_Host *shost;
	struct sas_internal *i;

	if (!scsi_is_host_device(dev))
		return 0;
	shost = dev_to_shost(dev);

	if (!shost->transportt)
		return 0;
	if (shost->transportt->host_attrs.ac.class !=
			&sas_host_class.class)
		return 0;

	i = to_sas_internal(shost->transportt);
	return &i->t.host_attrs.ac == cont;
}

static int do_sas_phy_delete(struct device *dev, void *data)
{
	if (scsi_is_sas_phy(dev))
		sas_phy_delete(dev_to_phy(dev));
	return 0;
}

/**
 * sas_remove_host  --  tear down a Scsi_Host's SAS data structures
 * @shost:	Scsi Host that is torn down
 *
 * Removes all SAS PHYs and remote PHYs for a given Scsi_Host.
 * Must be called just before scsi_remove_host for SAS HBAs.
 */
void sas_remove_host(struct Scsi_Host *shost)
{
	device_for_each_child(&shost->shost_gendev, NULL, do_sas_phy_delete);
}
EXPORT_SYMBOL(sas_remove_host);


/*
 * SAS Port attributes
 */

#define sas_phy_show_simple(field, name, format_string, cast)		\
static ssize_t								\
show_sas_phy_##name(struct class_device *cdev, char *buf)		\
{									\
	struct sas_phy *phy = transport_class_to_phy(cdev);		\
									\
	return snprintf(buf, 20, format_string, cast phy->field);	\
}

#define sas_phy_simple_attr(field, name, format_string, type)		\
	sas_phy_show_simple(field, name, format_string, (type))	\
static CLASS_DEVICE_ATTR(name, S_IRUGO, show_sas_phy_##name, NULL)

#define sas_phy_show_protocol(field, name)				\
static ssize_t								\
show_sas_phy_##name(struct class_device *cdev, char *buf)		\
{									\
	struct sas_phy *phy = transport_class_to_phy(cdev);		\
									\
	if (!phy->field)						\
		return snprintf(buf, 20, "none\n");			\
	return get_sas_protocol_names(phy->field, buf);		\
}

#define sas_phy_protocol_attr(field, name)				\
	sas_phy_show_protocol(field, name)				\
static CLASS_DEVICE_ATTR(name, S_IRUGO, show_sas_phy_##name, NULL)

#define sas_phy_show_linkspeed(field)					\
static ssize_t								\
show_sas_phy_##field(struct class_device *cdev, char *buf)		\
{									\
	struct sas_phy *phy = transport_class_to_phy(cdev);		\
									\
	return get_sas_linkspeed_names(phy->field, buf);		\
}

#define sas_phy_linkspeed_attr(field)					\
	sas_phy_show_linkspeed(field)					\
static CLASS_DEVICE_ATTR(field, S_IRUGO, show_sas_phy_##field, NULL)

static ssize_t
show_sas_device_type(struct class_device *cdev, char *buf)
{
	struct sas_phy *phy = transport_class_to_phy(cdev);

	if (!phy->identify.device_type)
		return snprintf(buf, 20, "none\n");
	return get_sas_device_type_names(phy->identify.device_type, buf);
}

static CLASS_DEVICE_ATTR(device_type, S_IRUGO, show_sas_device_type, NULL);

sas_phy_protocol_attr(identify.initiator_port_protocols,
		initiator_port_protocols);
sas_phy_protocol_attr(identify.target_port_protocols,
		target_port_protocols);
sas_phy_simple_attr(identify.sas_address, sas_address, "0x%016llx\n",
		unsigned long long);
sas_phy_simple_attr(identify.phy_identifier, phy_identifier, "%d\n", u8);
sas_phy_simple_attr(port_identifier, port_identifier, "%d\n", u8);
sas_phy_linkspeed_attr(negotiated_linkrate);
sas_phy_linkspeed_attr(minimum_linkrate_hw);
sas_phy_linkspeed_attr(minimum_linkrate);
sas_phy_linkspeed_attr(maximum_linkrate_hw);
sas_phy_linkspeed_attr(maximum_linkrate);


static DECLARE_TRANSPORT_CLASS(sas_phy_class,
		"sas_phy", NULL, NULL, NULL);

static int sas_phy_match(struct attribute_container *cont, struct device *dev)
{
	struct Scsi_Host *shost;
	struct sas_internal *i;

	if (!scsi_is_sas_phy(dev))
		return 0;
	shost = dev_to_shost(dev->parent);

	if (!shost->transportt)
		return 0;
	if (shost->transportt->host_attrs.ac.class !=
			&sas_host_class.class)
		return 0;

	i = to_sas_internal(shost->transportt);
	return &i->phy_attr_cont.ac == cont;
}

static void sas_phy_release(struct device *dev)
{
	struct sas_phy *phy = dev_to_phy(dev);

	put_device(dev->parent);
	kfree(phy);
}

/**
 * sas_phy_alloc  --  allocates and initialize a SAS PHY structure
 * @parent:	Parent device
 * @number:	Port number
 *
 * Allocates an SAS PHY structure.  It will be added in the device tree
 * below the device specified by @parent, which has to be either a Scsi_Host
 * or sas_rphy.
 *
 * Returns:
 *	SAS PHY allocated or %NULL if the allocation failed.
 */
struct sas_phy *sas_phy_alloc(struct device *parent, int number)
{
	struct Scsi_Host *shost = dev_to_shost(parent);
	struct sas_phy *phy;

	phy = kmalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return NULL;
	memset(phy, 0, sizeof(*phy));

	get_device(parent);

	phy->number = number;

	device_initialize(&phy->dev);
	phy->dev.parent = get_device(parent);
	phy->dev.release = sas_phy_release;
	sprintf(phy->dev.bus_id, "phy-%d:%d", shost->host_no, number);

	transport_setup_device(&phy->dev);

	return phy;
}
EXPORT_SYMBOL(sas_phy_alloc);

/**
 * sas_phy_add  --  add a SAS PHY to the device hierachy
 * @phy:	The PHY to be added
 *
 * Publishes a SAS PHY to the rest of the system.
 */
int sas_phy_add(struct sas_phy *phy)
{
	int error;

	error = device_add(&phy->dev);
	if (!error) {
		transport_add_device(&phy->dev);
		transport_configure_device(&phy->dev);
	}

	return error;
}
EXPORT_SYMBOL(sas_phy_add);

/**
 * sas_phy_free  --  free a SAS PHY
 * @phy:	SAS PHY to free
 *
 * Frees the specified SAS PHY.
 *
 * Note:
 *   This function must only be called on a PHY that has not
 *   sucessfully been added using sas_phy_add().
 */
void sas_phy_free(struct sas_phy *phy)
{
	transport_destroy_device(&phy->dev);
	put_device(phy->dev.parent);
	put_device(phy->dev.parent);
	put_device(phy->dev.parent);
	kfree(phy);
}
EXPORT_SYMBOL(sas_phy_free);

/**
 * sas_phy_delete  --  remove SAS PHY
 * @phy:	SAS PHY to remove
 *
 * Removes the specified SAS PHY.  If the SAS PHY has an
 * associated remote PHY it is removed before.
 */
void
sas_phy_delete(struct sas_phy *phy)
{
	struct device *dev = &phy->dev;

	if (phy->rphy)
		sas_rphy_delete(phy->rphy);

	transport_remove_device(dev);
	device_del(dev);
	transport_destroy_device(dev);
	put_device(dev->parent);
}
EXPORT_SYMBOL(sas_phy_delete);

/**
 * scsi_is_sas_phy  --  check if a struct device represents a SAS PHY
 * @dev:	device to check
 *
 * Returns:
 *	%1 if the device represents a SAS PHY, %0 else
 */
int scsi_is_sas_phy(const struct device *dev)
{
	return dev->release == sas_phy_release;
}
EXPORT_SYMBOL(scsi_is_sas_phy);

/*
 * SAS remote PHY attributes.
 */

#define sas_rphy_show_simple(field, name, format_string, cast)		\
static ssize_t								\
show_sas_rphy_##name(struct class_device *cdev, char *buf)		\
{									\
	struct sas_rphy *rphy = transport_class_to_rphy(cdev);	\
									\
	return snprintf(buf, 20, format_string, cast rphy->field);	\
}

#define sas_rphy_simple_attr(field, name, format_string, type)		\
	sas_rphy_show_simple(field, name, format_string, (type))	\
static SAS_CLASS_DEVICE_ATTR(rphy, name, S_IRUGO, 			\
		show_sas_rphy_##name, NULL)

#define sas_rphy_show_protocol(field, name)				\
static ssize_t								\
show_sas_rphy_##name(struct class_device *cdev, char *buf)		\
{									\
	struct sas_rphy *rphy = transport_class_to_rphy(cdev);	\
									\
	if (!rphy->field)					\
		return snprintf(buf, 20, "none\n");			\
	return get_sas_protocol_names(rphy->field, buf);	\
}

#define sas_rphy_protocol_attr(field, name)				\
	sas_rphy_show_protocol(field, name)				\
static SAS_CLASS_DEVICE_ATTR(rphy, name, S_IRUGO,			\
		show_sas_rphy_##name, NULL)

static ssize_t
show_sas_rphy_device_type(struct class_device *cdev, char *buf)
{
	struct sas_rphy *rphy = transport_class_to_rphy(cdev);

	if (!rphy->identify.device_type)
		return snprintf(buf, 20, "none\n");
	return get_sas_device_type_names(
			rphy->identify.device_type, buf);
}

static SAS_CLASS_DEVICE_ATTR(rphy, device_type, S_IRUGO,
		show_sas_rphy_device_type, NULL);

sas_rphy_protocol_attr(identify.initiator_port_protocols,
		initiator_port_protocols);
sas_rphy_protocol_attr(identify.target_port_protocols, target_port_protocols);
sas_rphy_simple_attr(identify.sas_address, sas_address, "0x%016llx\n",
		unsigned long long);
sas_rphy_simple_attr(identify.phy_identifier, phy_identifier, "%d\n", u8);

static DECLARE_TRANSPORT_CLASS(sas_rphy_class,
		"sas_rphy", NULL, NULL, NULL);

static int sas_rphy_match(struct attribute_container *cont, struct device *dev)
{
	struct Scsi_Host *shost;
	struct sas_internal *i;

	if (!scsi_is_sas_rphy(dev))
		return 0;
	shost = dev_to_shost(dev->parent->parent);

	if (!shost->transportt)
		return 0;
	if (shost->transportt->host_attrs.ac.class !=
			&sas_host_class.class)
		return 0;

	i = to_sas_internal(shost->transportt);
	return &i->rphy_attr_cont.ac == cont;
}

static void sas_rphy_release(struct device *dev)
{
	struct sas_rphy *rphy = dev_to_rphy(dev);

	put_device(dev->parent);
	kfree(rphy);
}

/**
 * sas_rphy_alloc  --  allocates and initialize a SAS remote PHY structure
 * @parent:		SAS PHY this remote PHY is conneted to
 *
 * Allocates an SAS remote PHY structure, connected to @parent.
 *
 * Returns:
 *	SAS PHY allocated or %NULL if the allocation failed.
 */
struct sas_rphy *sas_rphy_alloc(struct sas_phy *parent)
{
	struct Scsi_Host *shost = dev_to_shost(&parent->dev);
	struct sas_rphy *rphy;

	rphy = kmalloc(sizeof(*rphy), GFP_KERNEL);
	if (!rphy) {
		put_device(&parent->dev);
		return NULL;
	}
	memset(rphy, 0, sizeof(*rphy));

	device_initialize(&rphy->dev);
	rphy->dev.parent = get_device(&parent->dev);
	rphy->dev.release = sas_rphy_release;
	sprintf(rphy->dev.bus_id, "rphy-%d:%d",
		shost->host_no, parent->number);
	transport_setup_device(&rphy->dev);

	return rphy;
}
EXPORT_SYMBOL(sas_rphy_alloc);

/**
 * sas_rphy_add  --  add a SAS remote PHY to the device hierachy
 * @rphy:	The remote PHY to be added
 *
 * Publishes a SAS remote PHY to the rest of the system.
 */
int sas_rphy_add(struct sas_rphy *rphy)
{
	struct sas_phy *parent = dev_to_phy(rphy->dev.parent);
	struct Scsi_Host *shost = dev_to_shost(parent->dev.parent);
	struct sas_host_attrs *sas_host = to_sas_host_attrs(shost);
	struct sas_identify *identify = &rphy->identify;
	int error;

	if (parent->rphy)
		return -ENXIO;
	parent->rphy = rphy;

	error = device_add(&rphy->dev);
	if (error)
		return error;
	transport_add_device(&rphy->dev);
	transport_configure_device(&rphy->dev);

	spin_lock(&sas_host->lock);
	list_add_tail(&rphy->list, &sas_host->rphy_list);
	if (identify->device_type == SAS_END_DEVICE &&
	    (identify->target_port_protocols &
	     (SAS_PROTOCOL_SSP|SAS_PROTOCOL_STP|SAS_PROTOCOL_SATA)))
		rphy->scsi_target_id = sas_host->next_target_id++;
	else
		rphy->scsi_target_id = -1;
	spin_unlock(&sas_host->lock);

	if (rphy->scsi_target_id != -1) {
		scsi_scan_target(&rphy->dev, parent->number,
				rphy->scsi_target_id, ~0, 0);
	}

	return 0;
}
EXPORT_SYMBOL(sas_rphy_add);

/**
 * sas_rphy_free  --  free a SAS remote PHY
 * @rphy	SAS remote PHY to free
 *
 * Frees the specified SAS remote PHY.
 *
 * Note:
 *   This function must only be called on a remote
 *   PHY that has not sucessfully been added using
 *   sas_rphy_add().
 */
void sas_rphy_free(struct sas_rphy *rphy)
{
	struct Scsi_Host *shost = dev_to_shost(rphy->dev.parent->parent);
	struct sas_host_attrs *sas_host = to_sas_host_attrs(shost);

	spin_lock(&sas_host->lock);
	list_del(&rphy->list);
	spin_unlock(&sas_host->lock);

	transport_destroy_device(&rphy->dev);
	put_device(rphy->dev.parent);
	put_device(rphy->dev.parent);
	put_device(rphy->dev.parent);
	kfree(rphy);
}
EXPORT_SYMBOL(sas_rphy_free);

/**
 * sas_rphy_delete  --  remove SAS remote PHY
 * @rphy:	SAS remote PHY to remove
 *
 * Removes the specified SAS remote PHY.
 */
void
sas_rphy_delete(struct sas_rphy *rphy)
{
	struct device *dev = &rphy->dev;
	struct sas_phy *parent = dev_to_phy(dev->parent);
	struct Scsi_Host *shost = dev_to_shost(parent->dev.parent);
	struct sas_host_attrs *sas_host = to_sas_host_attrs(shost);

	scsi_remove_target(dev);

	transport_remove_device(dev);
	device_del(dev);
	transport_destroy_device(dev);

	spin_lock(&sas_host->lock);
	list_del(&rphy->list);
	spin_unlock(&sas_host->lock);

	put_device(&parent->dev);
}
EXPORT_SYMBOL(sas_rphy_delete);

/**
 * scsi_is_sas_rphy  --  check if a struct device represents a SAS remote PHY
 * @dev:	device to check
 *
 * Returns:
 *	%1 if the device represents a SAS remote PHY, %0 else
 */
int scsi_is_sas_rphy(const struct device *dev)
{
	return dev->release == sas_rphy_release;
}
EXPORT_SYMBOL(scsi_is_sas_rphy);


/*
 * SCSI scan helper
 */

static struct device *sas_target_parent(struct Scsi_Host *shost,
					int channel, uint id)
{
	struct sas_host_attrs *sas_host = to_sas_host_attrs(shost);
	struct sas_rphy *rphy;
	struct device *dev = NULL;

	spin_lock(&sas_host->lock);
	list_for_each_entry(rphy, &sas_host->rphy_list, list) {
		struct sas_phy *parent = dev_to_phy(rphy->dev.parent);
		if (parent->number == channel &&
		    rphy->scsi_target_id == id)
			dev = &rphy->dev;
	}
	spin_unlock(&sas_host->lock);

	return dev;
}


/*
 * Setup / Teardown code
 */

#define SETUP_RPORT_ATTRIBUTE(field)					\
	i->private_rphy_attrs[count] = class_device_attr_##field;	\
	i->private_rphy_attrs[count].attr.mode = S_IRUGO;		\
	i->private_rphy_attrs[count].store = NULL;			\
	i->rphy_attrs[count] = &i->private_rphy_attrs[count];	\
	count++

#define SETUP_PORT_ATTRIBUTE(field)					\
	i->private_phy_attrs[count] = class_device_attr_##field;	\
        i->private_phy_attrs[count].attr.mode = S_IRUGO;		\
        i->private_phy_attrs[count].store = NULL;			\
        i->phy_attrs[count] = &i->private_phy_attrs[count];		\
	count++


/**
 * sas_attach_transport  --  instantiate SAS transport template
 * @ft:		SAS transport class function template
 */
struct scsi_transport_template *
sas_attach_transport(struct sas_function_template *ft)
{
	struct sas_internal *i;
	int count;

	i = kmalloc(sizeof(struct sas_internal), GFP_KERNEL);
	if (!i)
		return NULL;
	memset(i, 0, sizeof(struct sas_internal));

	i->t.target_parent = sas_target_parent;

	i->t.host_attrs.ac.attrs = &i->host_attrs[0];
	i->t.host_attrs.ac.class = &sas_host_class.class;
	i->t.host_attrs.ac.match = sas_host_match;
	transport_container_register(&i->t.host_attrs);
	i->t.host_size = sizeof(struct sas_host_attrs);

	i->phy_attr_cont.ac.class = &sas_phy_class.class;
	i->phy_attr_cont.ac.attrs = &i->phy_attrs[0];
	i->phy_attr_cont.ac.match = sas_phy_match;
	transport_container_register(&i->phy_attr_cont);

	i->rphy_attr_cont.ac.class = &sas_rphy_class.class;
	i->rphy_attr_cont.ac.attrs = &i->rphy_attrs[0];
	i->rphy_attr_cont.ac.match = sas_rphy_match;
	transport_container_register(&i->rphy_attr_cont);

	i->f = ft;

	count = 0;
	i->host_attrs[count] = NULL;

	count = 0;
	SETUP_PORT_ATTRIBUTE(initiator_port_protocols);
	SETUP_PORT_ATTRIBUTE(target_port_protocols);
	SETUP_PORT_ATTRIBUTE(device_type);
	SETUP_PORT_ATTRIBUTE(sas_address);
	SETUP_PORT_ATTRIBUTE(phy_identifier);
	SETUP_PORT_ATTRIBUTE(port_identifier);
	SETUP_PORT_ATTRIBUTE(negotiated_linkrate);
	SETUP_PORT_ATTRIBUTE(minimum_linkrate_hw);
	SETUP_PORT_ATTRIBUTE(minimum_linkrate);
	SETUP_PORT_ATTRIBUTE(maximum_linkrate_hw);
	SETUP_PORT_ATTRIBUTE(maximum_linkrate);
	i->phy_attrs[count] = NULL;

	count = 0;
	SETUP_RPORT_ATTRIBUTE(rphy_initiator_port_protocols);
	SETUP_RPORT_ATTRIBUTE(rphy_target_port_protocols);
	SETUP_RPORT_ATTRIBUTE(rphy_device_type);
	SETUP_RPORT_ATTRIBUTE(rphy_sas_address);
	SETUP_RPORT_ATTRIBUTE(rphy_phy_identifier);
	i->rphy_attrs[count] = NULL;

	return &i->t;
}
EXPORT_SYMBOL(sas_attach_transport);

/**
 * sas_release_transport  --  release SAS transport template instance
 * @t:		transport template instance
 */
void sas_release_transport(struct scsi_transport_template *t)
{
	struct sas_internal *i = to_sas_internal(t);

	transport_container_unregister(&i->t.host_attrs);
	transport_container_unregister(&i->phy_attr_cont);
	transport_container_unregister(&i->rphy_attr_cont);

	kfree(i);
}
EXPORT_SYMBOL(sas_release_transport);

static __init int sas_transport_init(void)
{
	int error;

	error = transport_class_register(&sas_host_class);
	if (error)
		goto out;
	error = transport_class_register(&sas_phy_class);
	if (error)
		goto out_unregister_transport;
	error = transport_class_register(&sas_rphy_class);
	if (error)
		goto out_unregister_phy;

	return 0;

 out_unregister_phy:
	transport_class_unregister(&sas_phy_class);
 out_unregister_transport:
	transport_class_unregister(&sas_host_class);
 out:
	return error;

}

static void __exit sas_transport_exit(void)
{
	transport_class_unregister(&sas_host_class);
	transport_class_unregister(&sas_phy_class);
	transport_class_unregister(&sas_rphy_class);
}

MODULE_AUTHOR("Christoph Hellwig");
MODULE_DESCRIPTION("SAS Transphy Attributes");
MODULE_LICENSE("GPL");

module_init(sas_transport_init);
module_exit(sas_transport_exit);
