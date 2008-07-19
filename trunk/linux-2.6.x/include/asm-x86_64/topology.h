#ifndef _ASM_X86_64_TOPOLOGY_H
#define _ASM_X86_64_TOPOLOGY_H

#include <linux/config.h>

#ifdef CONFIG_NUMA

#include <asm/mpspec.h>
#include <asm/bitops.h>

/* Map the K8 CPU local memory controllers to a simple 1:1 CPU:NODE topology */

extern cpumask_t cpu_online_map;

extern unsigned char cpu_to_node[];
extern cpumask_t     node_to_cpumask[];

#ifdef CONFIG_ACPI_NUMA
extern int __node_distance(int, int);
#define node_distance(a,b) __node_distance(a,b)
/* #else fallback version */
#endif

#define cpu_to_node(cpu)		(cpu_to_node[cpu])
#define parent_node(node)		(node)
#define node_to_first_cpu(node) 	(__ffs(node_to_cpumask[node]))
#define node_to_cpumask(node)		(node_to_cpumask[node])
#define pcibus_to_node(bus)		((long)(bus->sysdata))	
#define pcibus_to_cpumask(bus)		node_to_cpumask(pcibus_to_node(bus));

/* sched_domains SD_NODE_INIT for x86_64 machines */
#define SD_NODE_INIT (struct sched_domain) {		\
	.span			= CPU_MASK_NONE,	\
	.parent			= NULL,			\
	.groups			= NULL,			\
	.min_interval		= 8,			\
	.max_interval		= 32,			\
	.busy_factor		= 32,			\
	.imbalance_pct		= 125,			\
	.cache_hot_time		= (10*1000000),		\
	.cache_nice_tries	= 2,			\
	.busy_idx		= 3,			\
	.idle_idx		= 2,			\
	.newidle_idx		= 0, 			\
	.wake_idx		= 1,			\
	.forkexec_idx		= 1,			\
	.per_cpu_gain		= 100,			\
	.flags			= SD_LOAD_BALANCE	\
				| SD_BALANCE_FORK	\
				| SD_BALANCE_EXEC	\
				| SD_WAKE_BALANCE,	\
	.last_balance		= jiffies,		\
	.balance_interval	= 1,			\
	.nr_balance_failed	= 0,			\
}

#endif

#include <asm-generic/topology.h>

#endif
