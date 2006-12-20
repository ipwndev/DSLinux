/*
 *	x86 SMP booting functions
 *
 *	(c) 1995 Alan Cox, Building #3 <alan@redhat.com>
 *	(c) 1998, 1999, 2000 Ingo Molnar <mingo@redhat.com>
 *	Copyright 2001 Andi Kleen, SuSE Labs.
 *
 *	Much of the core SMP work is based on previous work by Thomas Radke, to
 *	whom a great many thanks are extended.
 *
 *	Thanks to Intel for making available several different Pentium,
 *	Pentium Pro and Pentium-II/Xeon MP machines.
 *	Original development of Linux SMP code supported by Caldera.
 *
 *	This code is released under the GNU General Public License version 2
 *
 *	Fixes
 *		Felix Koop	:	NR_CPUS used properly
 *		Jose Renau	:	Handle single CPU case.
 *		Alan Cox	:	By repeated request 8) - Total BogoMIP report.
 *		Greg Wright	:	Fix for kernel stacks panic.
 *		Erich Boleyn	:	MP v1.4 and additional changes.
 *	Matthias Sattler	:	Changes for 2.1 kernel map.
 *	Michel Lespinasse	:	Changes for 2.1 kernel map.
 *	Michael Chastain	:	Change trampoline.S to gnu as.
 *		Alan Cox	:	Dumb bug: 'B' step PPro's are fine
 *		Ingo Molnar	:	Added APIC timers, based on code
 *					from Jose Renau
 *		Ingo Molnar	:	various cleanups and rewrites
 *		Tigran Aivazian	:	fixed "0.00 in /proc/uptime on SMP" bug.
 *	Maciej W. Rozycki	:	Bits for genuine 82489DX APICs
 *	Andi Kleen		:	Changed for SMP boot into long mode.
 *		Rusty Russell	:	Hacked into shape for new "hotplug" boot process.
 *      Andi Kleen              :       Converted to new state machine.
 *					Various cleanups.
 *					Probably mostly hotplug CPU ready now.
 *	Ashok Raj			: CPU hotplug support
 */


#include <linux/config.h>
#include <linux/init.h>

#include <linux/mm.h>
#include <linux/kernel_stat.h>
#include <linux/smp_lock.h>
#include <linux/bootmem.h>
#include <linux/thread_info.h>
#include <linux/module.h>

#include <linux/delay.h>
#include <linux/mc146818rtc.h>
#include <asm/mtrr.h>
#include <asm/pgalloc.h>
#include <asm/desc.h>
#include <asm/kdebug.h>
#include <asm/tlbflush.h>
#include <asm/proto.h>
#include <asm/nmi.h>
#include <asm/irq.h>
#include <asm/hw_irq.h>

/* Number of siblings per CPU package */
int smp_num_siblings = 1;
/* Package ID of each logical CPU */
u8 phys_proc_id[NR_CPUS] __read_mostly = { [0 ... NR_CPUS-1] = BAD_APICID };
u8 cpu_core_id[NR_CPUS] __read_mostly = { [0 ... NR_CPUS-1] = BAD_APICID };
EXPORT_SYMBOL(phys_proc_id);
EXPORT_SYMBOL(cpu_core_id);

/* Bitmask of currently online CPUs */
cpumask_t cpu_online_map __read_mostly;

EXPORT_SYMBOL(cpu_online_map);

/*
 * Private maps to synchronize booting between AP and BP.
 * Probably not needed anymore, but it makes for easier debugging. -AK
 */
cpumask_t cpu_callin_map;
cpumask_t cpu_callout_map;

cpumask_t cpu_possible_map;
EXPORT_SYMBOL(cpu_possible_map);

/* Per CPU bogomips and other parameters */
struct cpuinfo_x86 cpu_data[NR_CPUS] __cacheline_aligned;

/* Set when the idlers are all forked */
int smp_threads_ready;

cpumask_t cpu_sibling_map[NR_CPUS] __read_mostly;
cpumask_t cpu_core_map[NR_CPUS] __read_mostly;
EXPORT_SYMBOL(cpu_core_map);

/*
 * Trampoline 80x86 program as an array.
 */

extern unsigned char trampoline_data[];
extern unsigned char trampoline_end[];

/* State of each CPU */
DEFINE_PER_CPU(int, cpu_state) = { 0 };

/*
 * Store all idle threads, this can be reused instead of creating
 * a new thread. Also avoids complicated thread destroy functionality
 * for idle threads.
 */
struct task_struct *idle_thread_array[NR_CPUS] __cpuinitdata ;

#define get_idle_for_cpu(x)     (idle_thread_array[(x)])
#define set_idle_for_cpu(x,p)   (idle_thread_array[(x)] = (p))

/*
 * Currently trivial. Write the real->protected mode
 * bootstrap into the page concerned. The caller
 * has made sure it's suitably aligned.
 */

static unsigned long __cpuinit setup_trampoline(void)
{
	void *tramp = __va(SMP_TRAMPOLINE_BASE); 
	memcpy(tramp, trampoline_data, trampoline_end - trampoline_data);
	return virt_to_phys(tramp);
}

/*
 * The bootstrap kernel entry code has set these up. Save them for
 * a given CPU
 */

static void __cpuinit smp_store_cpu_info(int id)
{
	struct cpuinfo_x86 *c = cpu_data + id;

	*c = boot_cpu_data;
	identify_cpu(c);
	print_cpu_info(c);
}

/*
 * New Funky TSC sync algorithm borrowed from IA64.
 * Main advantage is that it doesn't reset the TSCs fully and
 * in general looks more robust and it works better than my earlier
 * attempts. I believe it was written by David Mosberger. Some minor
 * adjustments for x86-64 by me -AK
 *
 * Original comment reproduced below.
 *
 * Synchronize TSC of the current (slave) CPU with the TSC of the
 * MASTER CPU (normally the time-keeper CPU).  We use a closed loop to
 * eliminate the possibility of unaccounted-for errors (such as
 * getting a machine check in the middle of a calibration step).  The
 * basic idea is for the slave to ask the master what itc value it has
 * and to read its own itc before and after the master responds.  Each
 * iteration gives us three timestamps:
 *
 *	slave		master
 *
 *	t0 ---\
 *             ---\
 *		   --->
 *			tm
 *		   /---
 *	       /---
 *	t1 <---
 *
 *
 * The goal is to adjust the slave's TSC such that tm falls exactly
 * half-way between t0 and t1.  If we achieve this, the clocks are
 * synchronized provided the interconnect between the slave and the
 * master is symmetric.  Even if the interconnect were asymmetric, we
 * would still know that the synchronization error is smaller than the
 * roundtrip latency (t0 - t1).
 *
 * When the interconnect is quiet and symmetric, this lets us
 * synchronize the TSC to within one or two cycles.  However, we can
 * only *guarantee* that the synchronization is accurate to within a
 * round-trip time, which is typically in the range of several hundred
 * cycles (e.g., ~500 cycles).  In practice, this means that the TSCs
 * are usually almost perfectly synchronized, but we shouldn't assume
 * that the accuracy is much better than half a micro second or so.
 *
 * [there are other errors like the latency of RDTSC and of the
 * WRMSR. These can also account to hundreds of cycles. So it's
 * probably worse. It claims 153 cycles error on a dual Opteron,
 * but I suspect the numbers are actually somewhat worse -AK]
 */

#define MASTER	0
#define SLAVE	(SMP_CACHE_BYTES/8)

/* Intentionally don't use cpu_relax() while TSC synchronization
   because we don't want to go into funky power save modi or cause
   hypervisors to schedule us away.  Going to sleep would likely affect
   latency and low latency is the primary objective here. -AK */
#define no_cpu_relax() barrier()

static __cpuinitdata DEFINE_SPINLOCK(tsc_sync_lock);
static volatile __cpuinitdata unsigned long go[SLAVE + 1];
static int notscsync __cpuinitdata;

#undef DEBUG_TSC_SYNC

#define NUM_ROUNDS	64	/* magic value */
#define NUM_ITERS	5	/* likewise */

/* Callback on boot CPU */
static __cpuinit void sync_master(void *arg)
{
	unsigned long flags, i;

	go[MASTER] = 0;

	local_irq_save(flags);
	{
		for (i = 0; i < NUM_ROUNDS*NUM_ITERS; ++i) {
			while (!go[MASTER])
				no_cpu_relax();
			go[MASTER] = 0;
			rdtscll(go[SLAVE]);
		}
	}
	local_irq_restore(flags);
}

/*
 * Return the number of cycles by which our tsc differs from the tsc
 * on the master (time-keeper) CPU.  A positive number indicates our
 * tsc is ahead of the master, negative that it is behind.
 */
static inline long
get_delta(long *rt, long *master)
{
	unsigned long best_t0 = 0, best_t1 = ~0UL, best_tm = 0;
	unsigned long tcenter, t0, t1, tm;
	int i;

	for (i = 0; i < NUM_ITERS; ++i) {
		rdtscll(t0);
		go[MASTER] = 1;
		while (!(tm = go[SLAVE]))
			no_cpu_relax();
		go[SLAVE] = 0;
		rdtscll(t1);

		if (t1 - t0 < best_t1 - best_t0)
			best_t0 = t0, best_t1 = t1, best_tm = tm;
	}

	*rt = best_t1 - best_t0;
	*master = best_tm - best_t0;

	/* average best_t0 and best_t1 without overflow: */
	tcenter = (best_t0/2 + best_t1/2);
	if (best_t0 % 2 + best_t1 % 2 == 2)
		++tcenter;
	return tcenter - best_tm;
}

static __cpuinit void sync_tsc(unsigned int master)
{
	int i, done = 0;
	long delta, adj, adjust_latency = 0;
	unsigned long flags, rt, master_time_stamp, bound;
#ifdef DEBUG_TSC_SYNC
	static struct syncdebug {
		long rt;	/* roundtrip time */
		long master;	/* master's timestamp */
		long diff;	/* difference between midpoint and master's timestamp */
		long lat;	/* estimate of tsc adjustment latency */
	} t[NUM_ROUNDS] __cpuinitdata;
#endif

	printk(KERN_INFO "CPU %d: Syncing TSC to CPU %u.\n",
		smp_processor_id(), master);

	go[MASTER] = 1;

	/* It is dangerous to broadcast IPI as cpus are coming up,
	 * as they may not be ready to accept them.  So since
	 * we only need to send the ipi to the boot cpu direct
	 * the message, and avoid the race.
	 */
	smp_call_function_single(master, sync_master, NULL, 1, 0);

	while (go[MASTER])	/* wait for master to be ready */
		no_cpu_relax();

	spin_lock_irqsave(&tsc_sync_lock, flags);
	{
		for (i = 0; i < NUM_ROUNDS; ++i) {
			delta = get_delta(&rt, &master_time_stamp);
			if (delta == 0) {
				done = 1;	/* let's lock on to this... */
				bound = rt;
			}

			if (!done) {
				unsigned long t;
				if (i > 0) {
					adjust_latency += -delta;
					adj = -delta + adjust_latency/4;
				} else
					adj = -delta;

				rdtscll(t);
				wrmsrl(MSR_IA32_TSC, t + adj);
			}
#ifdef DEBUG_TSC_SYNC
			t[i].rt = rt;
			t[i].master = master_time_stamp;
			t[i].diff = delta;
			t[i].lat = adjust_latency/4;
#endif
		}
	}
	spin_unlock_irqrestore(&tsc_sync_lock, flags);

#ifdef DEBUG_TSC_SYNC
	for (i = 0; i < NUM_ROUNDS; ++i)
		printk("rt=%5ld master=%5ld diff=%5ld adjlat=%5ld\n",
		       t[i].rt, t[i].master, t[i].diff, t[i].lat);
#endif

	printk(KERN_INFO
	       "CPU %d: synchronized TSC with CPU %u (last diff %ld cycles, "
	       "maxerr %lu cycles)\n",
	       smp_processor_id(), master, delta, rt);
}

static void __cpuinit tsc_sync_wait(void)
{
	if (notscsync || !cpu_has_tsc)
		return;
	sync_tsc(0);
}

static __init int notscsync_setup(char *s)
{
	notscsync = 1;
	return 0;
}
__setup("notscsync", notscsync_setup);

static atomic_t init_deasserted __cpuinitdata;

/*
 * Report back to the Boot Processor.
 * Running on AP.
 */
void __cpuinit smp_callin(void)
{
	int cpuid, phys_id;
	unsigned long timeout;

	/*
	 * If waken up by an INIT in an 82489DX configuration
	 * we may get here before an INIT-deassert IPI reaches
	 * our local APIC.  We have to wait for the IPI or we'll
	 * lock up on an APIC access.
	 */
	while (!atomic_read(&init_deasserted))
		cpu_relax();

	/*
	 * (This works even if the APIC is not enabled.)
	 */
	phys_id = GET_APIC_ID(apic_read(APIC_ID));
	cpuid = smp_processor_id();
	if (cpu_isset(cpuid, cpu_callin_map)) {
		panic("smp_callin: phys CPU#%d, CPU#%d already present??\n",
					phys_id, cpuid);
	}
	Dprintk("CPU#%d (phys ID: %d) waiting for CALLOUT\n", cpuid, phys_id);

	/*
	 * STARTUP IPIs are fragile beasts as they might sometimes
	 * trigger some glue motherboard logic. Complete APIC bus
	 * silence for 1 second, this overestimates the time the
	 * boot CPU is spending to send the up to 2 STARTUP IPIs
	 * by a factor of two. This should be enough.
	 */

	/*
	 * Waiting 2s total for startup (udelay is not yet working)
	 */
	timeout = jiffies + 2*HZ;
	while (time_before(jiffies, timeout)) {
		/*
		 * Has the boot CPU finished it's STARTUP sequence?
		 */
		if (cpu_isset(cpuid, cpu_callout_map))
			break;
		cpu_relax();
	}

	if (!time_before(jiffies, timeout)) {
		panic("smp_callin: CPU%d started up but did not get a callout!\n",
			cpuid);
	}

	/*
	 * the boot CPU has finished the init stage and is spinning
	 * on callin_map until we finish. We are free to set up this
	 * CPU, first the APIC. (this is probably redundant on most
	 * boards)
	 */

	Dprintk("CALLIN, before setup_local_APIC().\n");
	setup_local_APIC();

	/*
	 * Get our bogomips.
 	 *
 	 * Need to enable IRQs because it can take longer and then
	 * the NMI watchdog might kill us.
	 */
	local_irq_enable();
	calibrate_delay();
	local_irq_disable();
	Dprintk("Stack at about %p\n",&cpuid);

	disable_APIC_timer();

	/*
	 * Save our processor parameters
	 */
 	smp_store_cpu_info(cpuid);

	/*
	 * Allow the master to continue.
	 */
	cpu_set(cpuid, cpu_callin_map);
}

static inline void set_cpu_sibling_map(int cpu)
{
	int i;

	if (smp_num_siblings > 1) {
		for_each_cpu(i) {
			if (cpu_core_id[cpu] == cpu_core_id[i]) {
				cpu_set(i, cpu_sibling_map[cpu]);
				cpu_set(cpu, cpu_sibling_map[i]);
			}
		}
	} else {
		cpu_set(cpu, cpu_sibling_map[cpu]);
	}

	if (current_cpu_data.x86_num_cores > 1) {
		for_each_cpu(i) {
			if (phys_proc_id[cpu] == phys_proc_id[i]) {
				cpu_set(i, cpu_core_map[cpu]);
				cpu_set(cpu, cpu_core_map[i]);
			}
		}
	} else {
		cpu_core_map[cpu] = cpu_sibling_map[cpu];
	}
}

/*
 * Setup code on secondary processor (after comming out of the trampoline)
 */
void __cpuinit start_secondary(void)
{
	/*
	 * Dont put anything before smp_callin(), SMP
	 * booting is too fragile that we want to limit the
	 * things done here to the most necessary things.
	 */
	cpu_init();
	smp_callin();

	/* otherwise gcc will move up the smp_processor_id before the cpu_init */
	barrier();

	Dprintk("cpu %d: setting up apic clock\n", smp_processor_id()); 	
	setup_secondary_APIC_clock();

	Dprintk("cpu %d: enabling apic timer\n", smp_processor_id());

	if (nmi_watchdog == NMI_IO_APIC) {
		disable_8259A_irq(0);
		enable_NMI_through_LVT0(NULL);
		enable_8259A_irq(0);
	}

	enable_APIC_timer();

	/*
	 * The sibling maps must be set before turing the online map on for
	 * this cpu
	 */
	set_cpu_sibling_map(smp_processor_id());

	/* 
  	 * Wait for TSC sync to not schedule things before.
	 * We still process interrupts, which could see an inconsistent
	 * time in that window unfortunately. 
	 * Do this here because TSC sync has global unprotected state.
 	 */
	tsc_sync_wait();

	/*
	 * We need to hold call_lock, so there is no inconsistency
	 * between the time smp_call_function() determines number of
	 * IPI receipients, and the time when the determination is made
	 * for which cpus receive the IPI in genapic_flat.c. Holding this
	 * lock helps us to not include this cpu in a currently in progress
	 * smp_call_function().
	 */
	lock_ipi_call_lock();

	/*
	 * Allow the master to continue.
	 */
	cpu_set(smp_processor_id(), cpu_online_map);
	per_cpu(cpu_state, smp_processor_id()) = CPU_ONLINE;
	unlock_ipi_call_lock();

	cpu_idle();
}

extern volatile unsigned long init_rsp;
extern void (*initial_code)(void);

#ifdef APIC_DEBUG
static void inquire_remote_apic(int apicid)
{
	unsigned i, regs[] = { APIC_ID >> 4, APIC_LVR >> 4, APIC_SPIV >> 4 };
	char *names[] = { "ID", "VERSION", "SPIV" };
	int timeout, status;

	printk(KERN_INFO "Inquiring remote APIC #%d...\n", apicid);

	for (i = 0; i < sizeof(regs) / sizeof(*regs); i++) {
		printk("... APIC #%d %s: ", apicid, names[i]);

		/*
		 * Wait for idle.
		 */
		apic_wait_icr_idle();

		apic_write(APIC_ICR2, SET_APIC_DEST_FIELD(apicid));
		apic_write(APIC_ICR, APIC_DM_REMRD | regs[i]);

		timeout = 0;
		do {
			udelay(100);
			status = apic_read(APIC_ICR) & APIC_ICR_RR_MASK;
		} while (status == APIC_ICR_RR_INPROG && timeout++ < 1000);

		switch (status) {
		case APIC_ICR_RR_VALID:
			status = apic_read(APIC_RRR);
			printk("%08x\n", status);
			break;
		default:
			printk("failed\n");
		}
	}
}
#endif

/*
 * Kick the secondary to wake up.
 */
static int __cpuinit wakeup_secondary_via_INIT(int phys_apicid, unsigned int start_rip)
{
	unsigned long send_status = 0, accept_status = 0;
	int maxlvt, timeout, num_starts, j;

	Dprintk("Asserting INIT.\n");

	/*
	 * Turn INIT on target chip
	 */
	apic_write(APIC_ICR2, SET_APIC_DEST_FIELD(phys_apicid));

	/*
	 * Send IPI
	 */
	apic_write(APIC_ICR, APIC_INT_LEVELTRIG | APIC_INT_ASSERT
				| APIC_DM_INIT);

	Dprintk("Waiting for send to finish...\n");
	timeout = 0;
	do {
		Dprintk("+");
		udelay(100);
		send_status = apic_read(APIC_ICR) & APIC_ICR_BUSY;
	} while (send_status && (timeout++ < 1000));

	mdelay(10);

	Dprintk("Deasserting INIT.\n");

	/* Target chip */
	apic_write(APIC_ICR2, SET_APIC_DEST_FIELD(phys_apicid));

	/* Send IPI */
	apic_write(APIC_ICR, APIC_INT_LEVELTRIG | APIC_DM_INIT);

	Dprintk("Waiting for send to finish...\n");
	timeout = 0;
	do {
		Dprintk("+");
		udelay(100);
		send_status = apic_read(APIC_ICR) & APIC_ICR_BUSY;
	} while (send_status && (timeout++ < 1000));

	atomic_set(&init_deasserted, 1);

	num_starts = 2;

	/*
	 * Run STARTUP IPI loop.
	 */
	Dprintk("#startup loops: %d.\n", num_starts);

	maxlvt = get_maxlvt();

	for (j = 1; j <= num_starts; j++) {
		Dprintk("Sending STARTUP #%d.\n",j);
		apic_read_around(APIC_SPIV);
		apic_write(APIC_ESR, 0);
		apic_read(APIC_ESR);
		Dprintk("After apic_write.\n");

		/*
		 * STARTUP IPI
		 */

		/* Target chip */
		apic_write(APIC_ICR2, SET_APIC_DEST_FIELD(phys_apicid));

		/* Boot on the stack */
		/* Kick the second */
		apic_write(APIC_ICR, APIC_DM_STARTUP | (start_rip >> 12));

		/*
		 * Give the other CPU some time to accept the IPI.
		 */
		udelay(300);

		Dprintk("Startup point 1.\n");

		Dprintk("Waiting for send to finish...\n");
		timeout = 0;
		do {
			Dprintk("+");
			udelay(100);
			send_status = apic_read(APIC_ICR) & APIC_ICR_BUSY;
		} while (send_status && (timeout++ < 1000));

		/*
		 * Give the other CPU some time to accept the IPI.
		 */
		udelay(200);
		/*
		 * Due to the Pentium erratum 3AP.
		 */
		if (maxlvt > 3) {
			apic_read_around(APIC_SPIV);
			apic_write(APIC_ESR, 0);
		}
		accept_status = (apic_read(APIC_ESR) & 0xEF);
		if (send_status || accept_status)
			break;
	}
	Dprintk("After Startup.\n");

	if (send_status)
		printk(KERN_ERR "APIC never delivered???\n");
	if (accept_status)
		printk(KERN_ERR "APIC delivery error (%lx).\n", accept_status);

	return (send_status | accept_status);
}

struct create_idle {
	struct task_struct *idle;
	struct completion done;
	int cpu;
};

void do_fork_idle(void *_c_idle)
{
	struct create_idle *c_idle = _c_idle;

	c_idle->idle = fork_idle(c_idle->cpu);
	complete(&c_idle->done);
}

/*
 * Boot one CPU.
 */
static int __cpuinit do_boot_cpu(int cpu, int apicid)
{
	unsigned long boot_error;
	int timeout;
	unsigned long start_rip;
	struct create_idle c_idle = {
		.cpu = cpu,
		.done = COMPLETION_INITIALIZER(c_idle.done),
	};
	DECLARE_WORK(work, do_fork_idle, &c_idle);

	c_idle.idle = get_idle_for_cpu(cpu);

	if (c_idle.idle) {
		c_idle.idle->thread.rsp = (unsigned long) (((struct pt_regs *)
			(THREAD_SIZE + (unsigned long) c_idle.idle->thread_info)) - 1);
		init_idle(c_idle.idle, cpu);
		goto do_rest;
	}

	/*
	 * During cold boot process, keventd thread is not spun up yet.
	 * When we do cpu hot-add, we create idle threads on the fly, we should
	 * not acquire any attributes from the calling context. Hence the clean
	 * way to create kernel_threads() is to do that from keventd().
	 * We do the current_is_keventd() due to the fact that ACPI notifier
	 * was also queuing to keventd() and when the caller is already running
	 * in context of keventd(), we would end up with locking up the keventd
	 * thread.
	 */
	if (!keventd_up() || current_is_keventd())
		work.func(work.data);
	else {
		schedule_work(&work);
		wait_for_completion(&c_idle.done);
	}

	if (IS_ERR(c_idle.idle)) {
		printk("failed fork for CPU %d\n", cpu);
		return PTR_ERR(c_idle.idle);
	}

	set_idle_for_cpu(cpu, c_idle.idle);

do_rest:

	cpu_pda[cpu].pcurrent = c_idle.idle;

	start_rip = setup_trampoline();

	init_rsp = c_idle.idle->thread.rsp;
	per_cpu(init_tss,cpu).rsp0 = init_rsp;
	initial_code = start_secondary;
	clear_ti_thread_flag(c_idle.idle->thread_info, TIF_FORK);

	printk(KERN_INFO "Booting processor %d/%d APIC 0x%x\n", cpu,
		cpus_weight(cpu_present_map),
		apicid);

	/*
	 * This grunge runs the startup process for
	 * the targeted processor.
	 */

	atomic_set(&init_deasserted, 0);

	Dprintk("Setting warm reset code and vector.\n");

	CMOS_WRITE(0xa, 0xf);
	local_flush_tlb();
	Dprintk("1.\n");
	*((volatile unsigned short *) phys_to_virt(0x469)) = start_rip >> 4;
	Dprintk("2.\n");
	*((volatile unsigned short *) phys_to_virt(0x467)) = start_rip & 0xf;
	Dprintk("3.\n");

	/*
	 * Be paranoid about clearing APIC errors.
	 */
	if (APIC_INTEGRATED(apic_version[apicid])) {
		apic_read_around(APIC_SPIV);
		apic_write(APIC_ESR, 0);
		apic_read(APIC_ESR);
	}

	/*
	 * Status is now clean
	 */
	boot_error = 0;

	/*
	 * Starting actual IPI sequence...
	 */
	boot_error = wakeup_secondary_via_INIT(apicid, start_rip);

	if (!boot_error) {
		/*
		 * allow APs to start initializing.
		 */
		Dprintk("Before Callout %d.\n", cpu);
		cpu_set(cpu, cpu_callout_map);
		Dprintk("After Callout %d.\n", cpu);

		/*
		 * Wait 5s total for a response
		 */
		for (timeout = 0; timeout < 50000; timeout++) {
			if (cpu_isset(cpu, cpu_callin_map))
				break;	/* It has booted */
			udelay(100);
		}

		if (cpu_isset(cpu, cpu_callin_map)) {
			/* number CPUs logically, starting from 1 (BSP is 0) */
			Dprintk("CPU has booted.\n");
		} else {
			boot_error = 1;
			if (*((volatile unsigned char *)phys_to_virt(SMP_TRAMPOLINE_BASE))
					== 0xA5)
				/* trampoline started but...? */
				printk("Stuck ??\n");
			else
				/* trampoline code not run */
				printk("Not responding.\n");
#ifdef APIC_DEBUG
			inquire_remote_apic(apicid);
#endif
		}
	}
	if (boot_error) {
		cpu_clear(cpu, cpu_callout_map); /* was set here (do_boot_cpu()) */
		clear_bit(cpu, &cpu_initialized); /* was set by cpu_init() */
		cpu_clear(cpu, cpu_present_map);
		cpu_clear(cpu, cpu_possible_map);
		x86_cpu_to_apicid[cpu] = BAD_APICID;
		x86_cpu_to_log_apicid[cpu] = BAD_APICID;
		return -EIO;
	}

	return 0;
}

cycles_t cacheflush_time;
unsigned long cache_decay_ticks;

/*
 * Cleanup possible dangling ends...
 */
static __cpuinit void smp_cleanup_boot(void)
{
	/*
	 * Paranoid:  Set warm reset code and vector here back
	 * to default values.
	 */
	CMOS_WRITE(0, 0xf);

	/*
	 * Reset trampoline flag
	 */
	*((volatile int *) phys_to_virt(0x467)) = 0;
}

/*
 * Fall back to non SMP mode after errors.
 *
 * RED-PEN audit/test this more. I bet there is more state messed up here.
 */
static __init void disable_smp(void)
{
	cpu_present_map = cpumask_of_cpu(0);
	cpu_possible_map = cpumask_of_cpu(0);
	if (smp_found_config)
		phys_cpu_present_map = physid_mask_of_physid(boot_cpu_id);
	else
		phys_cpu_present_map = physid_mask_of_physid(0);
	cpu_set(0, cpu_sibling_map[0]);
	cpu_set(0, cpu_core_map[0]);
}

#ifdef CONFIG_HOTPLUG_CPU
/*
 * cpu_possible_map should be static, it cannot change as cpu's
 * are onlined, or offlined. The reason is per-cpu data-structures
 * are allocated by some modules at init time, and dont expect to
 * do this dynamically on cpu arrival/departure.
 * cpu_present_map on the other hand can change dynamically.
 * In case when cpu_hotplug is not compiled, then we resort to current
 * behaviour, which is cpu_possible == cpu_present.
 * If cpu-hotplug is supported, then we need to preallocate for all
 * those NR_CPUS, hence cpu_possible_map represents entire NR_CPUS range.
 * - Ashok Raj
 */
__init void prefill_possible_map(void)
{
	int i;
	for (i = 0; i < NR_CPUS; i++)
		cpu_set(i, cpu_possible_map);
}
#endif

/*
 * Various sanity checks.
 */
static int __init smp_sanity_check(unsigned max_cpus)
{
	if (!physid_isset(hard_smp_processor_id(), phys_cpu_present_map)) {
		printk("weird, boot CPU (#%d) not listed by the BIOS.\n",
		       hard_smp_processor_id());
		physid_set(hard_smp_processor_id(), phys_cpu_present_map);
	}

	/*
	 * If we couldn't find an SMP configuration at boot time,
	 * get out of here now!
	 */
	if (!smp_found_config) {
		printk(KERN_NOTICE "SMP motherboard not detected.\n");
		disable_smp();
		if (APIC_init_uniprocessor())
			printk(KERN_NOTICE "Local APIC not detected."
					   " Using dummy APIC emulation.\n");
		return -1;
	}

	/*
	 * Should not be necessary because the MP table should list the boot
	 * CPU too, but we do it for the sake of robustness anyway.
	 */
	if (!physid_isset(boot_cpu_id, phys_cpu_present_map)) {
		printk(KERN_NOTICE "weird, boot CPU (#%d) not listed by the BIOS.\n",
								 boot_cpu_id);
		physid_set(hard_smp_processor_id(), phys_cpu_present_map);
	}

	/*
	 * If we couldn't find a local APIC, then get out of here now!
	 */
	if (APIC_INTEGRATED(apic_version[boot_cpu_id]) && !cpu_has_apic) {
		printk(KERN_ERR "BIOS bug, local APIC #%d not detected!...\n",
			boot_cpu_id);
		printk(KERN_ERR "... forcing use of dummy APIC emulation. (tell your hw vendor)\n");
		nr_ioapics = 0;
		return -1;
	}

	/*
	 * If SMP should be disabled, then really disable it!
	 */
	if (!max_cpus) {
		printk(KERN_INFO "SMP mode deactivated, forcing use of dummy APIC emulation.\n");
		nr_ioapics = 0;
		return -1;
	}

	return 0;
}

/*
 * Prepare for SMP bootup.  The MP table or ACPI has been read
 * earlier.  Just do some sanity checking here and enable APIC mode.
 */
void __init smp_prepare_cpus(unsigned int max_cpus)
{
	nmi_watchdog_default();
	current_cpu_data = boot_cpu_data;
	current_thread_info()->cpu = 0;  /* needed? */

	if (smp_sanity_check(max_cpus) < 0) {
		printk(KERN_INFO "SMP disabled\n");
		disable_smp();
		return;
	}


	/*
	 * Switch from PIC to APIC mode.
	 */
	connect_bsp_APIC();
	setup_local_APIC();

	if (GET_APIC_ID(apic_read(APIC_ID)) != boot_cpu_id) {
		panic("Boot APIC ID in local APIC unexpected (%d vs %d)",
		      GET_APIC_ID(apic_read(APIC_ID)), boot_cpu_id);
		/* Or can we switch back to PIC here? */
	}

	/*
	 * Now start the IO-APICs
	 */
	if (!skip_ioapic_setup && nr_ioapics)
		setup_IO_APIC();
	else
		nr_ioapics = 0;

	/*
	 * Set up local APIC timer on boot CPU.
	 */

	setup_boot_APIC_clock();
}

/*
 * Early setup to make printk work.
 */
void __init smp_prepare_boot_cpu(void)
{
	int me = smp_processor_id();
	cpu_set(me, cpu_online_map);
	cpu_set(me, cpu_callout_map);
	cpu_set(0, cpu_sibling_map[0]);
	cpu_set(0, cpu_core_map[0]);
	per_cpu(cpu_state, me) = CPU_ONLINE;
}

/*
 * Entry point to boot a CPU.
 */
int __cpuinit __cpu_up(unsigned int cpu)
{
	int err;
	int apicid = cpu_present_to_apicid(cpu);

	WARN_ON(irqs_disabled());

	Dprintk("++++++++++++++++++++=_---CPU UP  %u\n", cpu);

	if (apicid == BAD_APICID || apicid == boot_cpu_id ||
	    !physid_isset(apicid, phys_cpu_present_map)) {
		printk("__cpu_up: bad cpu %d\n", cpu);
		return -EINVAL;
	}

	/*
	 * Already booted CPU?
	 */
 	if (cpu_isset(cpu, cpu_callin_map)) {
		Dprintk("do_boot_cpu %d Already started\n", cpu);
 		return -ENOSYS;
	}

	per_cpu(cpu_state, cpu) = CPU_UP_PREPARE;
	/* Boot it! */
	err = do_boot_cpu(cpu, apicid);
	if (err < 0) {
		Dprintk("do_boot_cpu failed %d\n", err);
		return err;
	}

	/* Unleash the CPU! */
	Dprintk("waiting for cpu %d\n", cpu);

	while (!cpu_isset(cpu, cpu_online_map))
		cpu_relax();
	err = 0;

	return err;
}

/*
 * Finish the SMP boot.
 */
void __init smp_cpus_done(unsigned int max_cpus)
{
#ifndef CONFIG_HOTPLUG_CPU
	zap_low_mappings();
#endif
	smp_cleanup_boot();

#ifdef CONFIG_X86_IO_APIC
	setup_ioapic_dest();
#endif

	time_init_gtod();

	check_nmi_watchdog();
}

#ifdef CONFIG_HOTPLUG_CPU

static void remove_siblinginfo(int cpu)
{
	int sibling;

	for_each_cpu_mask(sibling, cpu_sibling_map[cpu])
		cpu_clear(cpu, cpu_sibling_map[sibling]);
	for_each_cpu_mask(sibling, cpu_core_map[cpu])
		cpu_clear(cpu, cpu_core_map[sibling]);
	cpus_clear(cpu_sibling_map[cpu]);
	cpus_clear(cpu_core_map[cpu]);
	phys_proc_id[cpu] = BAD_APICID;
	cpu_core_id[cpu] = BAD_APICID;
}

void remove_cpu_from_maps(void)
{
	int cpu = smp_processor_id();

	cpu_clear(cpu, cpu_callout_map);
	cpu_clear(cpu, cpu_callin_map);
	clear_bit(cpu, &cpu_initialized); /* was set by cpu_init() */
}

int __cpu_disable(void)
{
	int cpu = smp_processor_id();

	/*
	 * Perhaps use cpufreq to drop frequency, but that could go
	 * into generic code.
 	 *
	 * We won't take down the boot processor on i386 due to some
	 * interrupts only being able to be serviced by the BSP.
	 * Especially so if we're not using an IOAPIC	-zwane
	 */
	if (cpu == 0)
		return -EBUSY;

	disable_APIC_timer();

	/*
	 * HACK:
	 * Allow any queued timer interrupts to get serviced
	 * This is only a temporary solution until we cleanup
	 * fixup_irqs as we do for IA64.
	 */
	local_irq_enable();
	mdelay(1);

	local_irq_disable();
	remove_siblinginfo(cpu);

	/* It's now safe to remove this processor from the online map */
	cpu_clear(cpu, cpu_online_map);
	remove_cpu_from_maps();
	fixup_irqs(cpu_online_map);
	return 0;
}

void __cpu_die(unsigned int cpu)
{
	/* We don't do anything here: idle task is faking death itself. */
	unsigned int i;

	for (i = 0; i < 10; i++) {
		/* They ack this in play_dead by setting CPU_DEAD */
		if (per_cpu(cpu_state, cpu) == CPU_DEAD) {
			printk ("CPU %d is now offline\n", cpu);
			return;
		}
		msleep(100);
	}
 	printk(KERN_ERR "CPU %u didn't die...\n", cpu);
}

#else /* ... !CONFIG_HOTPLUG_CPU */

int __cpu_disable(void)
{
	return -ENOSYS;
}

void __cpu_die(unsigned int cpu)
{
	/* We said "no" in __cpu_disable */
	BUG();
}
#endif /* CONFIG_HOTPLUG_CPU */
