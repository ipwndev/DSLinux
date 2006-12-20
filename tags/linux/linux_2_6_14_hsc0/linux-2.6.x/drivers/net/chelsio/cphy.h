/*****************************************************************************
 *                                                                           *
 * File: cphy.h                                                              *
 * $Revision$                                                          *
 * $Date$                                              *
 * Description:                                                              *
 *  part of the Chelsio 10Gb Ethernet Driver.                                *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License, version 2, as       *
 * published by the Free Software Foundation.                                *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED    *
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF      *
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.                     *
 *                                                                           *
 * http://www.chelsio.com                                                    *
 *                                                                           *
 * Copyright (c) 2003 - 2005 Chelsio Communications, Inc.                    *
 * All rights reserved.                                                      *
 *                                                                           *
 * Maintainers: maintainers@chelsio.com                                      *
 *                                                                           *
 * Authors: Dimitrios Michailidis   <dm@chelsio.com>                         *
 *          Tina Yang               <tainay@chelsio.com>                     *
 *          Felix Marti             <felix@chelsio.com>                      *
 *          Scott Bardone           <sbardone@chelsio.com>                   *
 *          Kurt Ottaway            <kottaway@chelsio.com>                   *
 *          Frank DiMambro          <frank@chelsio.com>                      *
 *                                                                           *
 * History:                                                                  *
 *                                                                           *
 ****************************************************************************/

#ifndef _CXGB_CPHY_H_
#define _CXGB_CPHY_H_

#include "common.h"

struct mdio_ops {
	void (*init)(adapter_t *adapter, const struct board_info *bi);
	int  (*read)(adapter_t *adapter, int phy_addr, int mmd_addr,
		     int reg_addr, unsigned int *val);
	int  (*write)(adapter_t *adapter, int phy_addr, int mmd_addr,
		      int reg_addr, unsigned int val);
};

/* PHY interrupt types */
enum {
	cphy_cause_link_change = 0x1,
	cphy_cause_error = 0x2
};

struct cphy;

/* PHY operations */
struct cphy_ops {
	void (*destroy)(struct cphy *);
	int (*reset)(struct cphy *, int wait);

	int (*interrupt_enable)(struct cphy *);
	int (*interrupt_disable)(struct cphy *);
	int (*interrupt_clear)(struct cphy *);
	int (*interrupt_handler)(struct cphy *);

	int (*autoneg_enable)(struct cphy *);
	int (*autoneg_disable)(struct cphy *);
	int (*autoneg_restart)(struct cphy *);

	int (*advertise)(struct cphy *phy, unsigned int advertise_map);
	int (*set_loopback)(struct cphy *, int on);
	int (*set_speed_duplex)(struct cphy *phy, int speed, int duplex);
	int (*get_link_status)(struct cphy *phy, int *link_ok, int *speed,
			       int *duplex, int *fc);
};

/* A PHY instance */
struct cphy {
	int addr;                            /* PHY address */
	adapter_t *adapter;                  /* associated adapter */
	struct cphy_ops *ops;                /* PHY operations */
	int (*mdio_read)(adapter_t *adapter, int phy_addr, int mmd_addr,
			 int reg_addr, unsigned int *val);
	int (*mdio_write)(adapter_t *adapter, int phy_addr, int mmd_addr,
			  int reg_addr, unsigned int val);
	struct cphy_instance *instance;
};

/* Convenience MDIO read/write wrappers */
static inline int mdio_read(struct cphy *cphy, int mmd, int reg,
			    unsigned int *valp)
{
	return cphy->mdio_read(cphy->adapter, cphy->addr, mmd, reg, valp);
}

static inline int mdio_write(struct cphy *cphy, int mmd, int reg,
			     unsigned int val)
{
	return cphy->mdio_write(cphy->adapter, cphy->addr, mmd, reg, val);
}

static inline int simple_mdio_read(struct cphy *cphy, int reg,
				   unsigned int *valp)
{
	return mdio_read(cphy, 0, reg, valp);
}

static inline int simple_mdio_write(struct cphy *cphy, int reg,
				    unsigned int val)
{
	return mdio_write(cphy, 0, reg, val);
}

/* Convenience initializer */
static inline void cphy_init(struct cphy *phy, adapter_t *adapter,
			     int phy_addr, struct cphy_ops *phy_ops,
			     struct mdio_ops *mdio_ops)
{
	phy->adapter = adapter;
	phy->addr    = phy_addr;
	phy->ops     = phy_ops;
	if (mdio_ops) {
		phy->mdio_read  = mdio_ops->read;
		phy->mdio_write = mdio_ops->write;
	}
}

/* Operations of the PHY-instance factory */
struct gphy {
	/* Construct a PHY instance with the given PHY address */
	struct cphy *(*create)(adapter_t *adapter, int phy_addr,
			       struct mdio_ops *mdio_ops);

	/*
	 * Reset the PHY chip.  This resets the whole PHY chip, not individual
	 * ports.
	 */
	int (*reset)(adapter_t *adapter);
};

extern struct gphy t1_mv88x201x_ops;
extern struct gphy t1_dummy_phy_ops;

#endif /* _CXGB_CPHY_H_ */
