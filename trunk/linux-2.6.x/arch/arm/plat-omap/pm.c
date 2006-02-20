/*
 * linux/arch/arm/plat-omap/pm.c
 *
 * OMAP Power Management Routines
 *
 * Original code for the SA11x0:
 * Copyright (c) 2001 Cliff Brake <cbrake@accelent.com>
 *
 * Modified for the PXA250 by Nicolas Pitre:
 * Copyright (c) 2002 Monta Vista Software, Inc.
 *
 * Modified for the OMAP1510 by David Singleton:
 * Copyright (c) 2002 Monta Vista Software, Inc.
 *
 * Cleanup 2004 for OMAP1510/1610 by Dirk Behme <dirk.behme@de.bosch.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/pm.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach/time.h>
#include <asm/mach/irq.h>

#include <asm/mach-types.h>
#include <asm/arch/irqs.h>
#include <asm/arch/tc.h>
#include <asm/arch/pm.h>
#include <asm/arch/mux.h>
#include <asm/arch/tps65010.h>
#include <asm/arch/dsp_common.h>

#include "clock.h"
#include "sram.h"

static unsigned int arm_sleep_save[ARM_SLEEP_SAVE_SIZE];
static unsigned short ulpd_sleep_save[ULPD_SLEEP_SAVE_SIZE];
static unsigned int mpui1510_sleep_save[MPUI1510_SLEEP_SAVE_SIZE];
static unsigned int mpui1610_sleep_save[MPUI1610_SLEEP_SAVE_SIZE];

static void (*omap_sram_idle)(void) = NULL;
static void (*omap_sram_suspend)(unsigned long r0, unsigned long r1) = NULL;

/*
 * Let's power down on idle, but only if we are really
 * idle, because once we start down the path of
 * going idle we continue to do idle even if we get
 * a clock tick interrupt . .
 */
void omap_pm_idle(void)
{
	unsigned int mask32 = 0;

	/*
	 * If the DSP is being used let's just idle the CPU, the overhead
	 * to wake up from Big Sleep is big, milliseconds versus micro
	 * seconds for wait for interrupt.
	 */

	local_irq_disable();
	local_fiq_disable();
	if (need_resched()) {
		local_fiq_enable();
		local_irq_enable();
		return;
	}
	mask32 = omap_readl(ARM_SYSST);

	/*
	 * Prevent the ULPD from entering low power state by setting
	 * POWER_CTRL_REG:4 = 0
	 */
	omap_writew(omap_readw(ULPD_POWER_CTRL) &
		    ~ULPD_DEEP_SLEEP_TRANSITION_EN, ULPD_POWER_CTRL);

	/*
	 * Since an interrupt may set up a timer, we don't want to
	 * reprogram the hardware timer with interrupts enabled.
	 * Re-enable interrupts only after returning from idle.
	 */
	timer_dyn_reprogram();

	if ((mask32 & DSP_IDLE) == 0) {
		__asm__ volatile ("mcr	p15, 0, r0, c7, c0, 4");
	} else
		omap_sram_idle();

	local_fiq_enable();
	local_irq_enable();
}

/*
 * Configuration of the wakeup event is board specific. For the
 * moment we put it into this helper function. Later it may move
 * to board specific files.
 */
static void omap_pm_wakeup_setup(void)
{
	u32 level1_wake = OMAP_IRQ_BIT(INT_IH2_IRQ);
	u32 level2_wake = OMAP_IRQ_BIT(INT_UART2) | OMAP_IRQ_BIT(INT_KEYBOARD);

	/*
	 * Turn off all interrupts except GPIO bank 1, L1-2nd level cascade,
	 * and the L2 wakeup interrupts: keypad and UART2. Note that the
	 * drivers must still separately call omap_set_gpio_wakeup() to
	 * wake up to a GPIO interrupt.
	 */
	if (cpu_is_omap1510() || cpu_is_omap16xx())
		level1_wake |= OMAP_IRQ_BIT(INT_GPIO_BANK1);
	else if (cpu_is_omap730())
		level1_wake |= OMAP_IRQ_BIT(INT_730_GPIO_BANK1);

	omap_writel(~level1_wake, OMAP_IH1_MIR);

	if (cpu_is_omap1510())
		omap_writel(~level2_wake,  OMAP_IH2_MIR);

	/* INT_1610_WAKE_UP_REQ is needed for GPIO wakeup... */
	if (cpu_is_omap16xx()) {
		omap_writel(~level2_wake, OMAP_IH2_0_MIR);
		omap_writel(~OMAP_IRQ_BIT(INT_1610_WAKE_UP_REQ), OMAP_IH2_1_MIR);
		omap_writel(~0x0, OMAP_IH2_2_MIR);
		omap_writel(~0x0, OMAP_IH2_3_MIR);
	}

	/*  New IRQ agreement, recalculate in cascade order */
	omap_writel(1, OMAP_IH2_CONTROL);
 	omap_writel(1, OMAP_IH1_CONTROL);
}

void omap_pm_suspend(void)
{
	unsigned long arg0 = 0, arg1 = 0;

	printk("PM: OMAP%x is trying to enter deep sleep...\n", system_rev);

	omap_serial_wake_trigger(1);

	if (machine_is_omap_osk()) {
		/* Stop LED1 (D9) blink */
		tps65010_set_led(LED1, OFF);
	}

	omap_writew(0xffff, ULPD_SOFT_DISABLE_REQ_REG);

	/*
	 * Step 1: turn off interrupts (FIXME: NOTE: already disabled)
	 */

	local_irq_disable();
	local_fiq_disable();

	/*
	 * Step 2: save registers
	 *
	 * The omap is a strange/beautiful device. The caches, memory
	 * and register state are preserved across power saves.
	 * We have to save and restore very little register state to
	 * idle the omap.
         *
 	 * Save interrupt, MPUI, ARM and UPLD control registers.
	 */

	if (cpu_is_omap1510()) {
		MPUI1510_SAVE(OMAP_IH1_MIR);
		MPUI1510_SAVE(OMAP_IH2_MIR);
		MPUI1510_SAVE(MPUI_CTRL);
		MPUI1510_SAVE(MPUI_DSP_BOOT_CONFIG);
		MPUI1510_SAVE(MPUI_DSP_API_CONFIG);
		MPUI1510_SAVE(EMIFS_CONFIG);
		MPUI1510_SAVE(EMIFF_SDRAM_CONFIG);
	} else if (cpu_is_omap16xx()) {
		MPUI1610_SAVE(OMAP_IH1_MIR);
		MPUI1610_SAVE(OMAP_IH2_0_MIR);
		MPUI1610_SAVE(OMAP_IH2_1_MIR);
		MPUI1610_SAVE(OMAP_IH2_2_MIR);
		MPUI1610_SAVE(OMAP_IH2_3_MIR);
		MPUI1610_SAVE(MPUI_CTRL);
		MPUI1610_SAVE(MPUI_DSP_BOOT_CONFIG);
		MPUI1610_SAVE(MPUI_DSP_API_CONFIG);
		MPUI1610_SAVE(EMIFS_CONFIG);
		MPUI1610_SAVE(EMIFF_SDRAM_CONFIG);
	}

	ARM_SAVE(ARM_CKCTL);
	ARM_SAVE(ARM_IDLECT1);
	ARM_SAVE(ARM_IDLECT2);
	if (!(cpu_is_omap1510()))
		ARM_SAVE(ARM_IDLECT3);
	ARM_SAVE(ARM_EWUPCT);
	ARM_SAVE(ARM_RSTCT1);
	ARM_SAVE(ARM_RSTCT2);
	ARM_SAVE(ARM_SYSST);
	ULPD_SAVE(ULPD_CLOCK_CTRL);
	ULPD_SAVE(ULPD_STATUS_REQ);

	/* (Step 3 removed - we now allow deep sleep by default) */

	/*
	 * Step 4: OMAP DSP Shutdown
	 */


	/*
	 * Step 5: Wakeup Event Setup
	 */

	omap_pm_wakeup_setup();

	/*
	 * Step 6: ARM and Traffic controller shutdown
	 */

	/* disable ARM watchdog */
	omap_writel(0x00F5, OMAP_WDT_TIMER_MODE);
	omap_writel(0x00A0, OMAP_WDT_TIMER_MODE);

	/*
	 * Step 6b: ARM and Traffic controller shutdown
	 *
	 * Step 6 continues here. Prepare jump to power management
	 * assembly code in internal SRAM.
	 *
	 * Since the omap_cpu_suspend routine has been copied to
	 * SRAM, we'll do an indirect procedure call to it and pass the
	 * contents of arm_idlect1 and arm_idlect2 so it can restore
	 * them when it wakes up and it will return.
	 */

	arg0 = arm_sleep_save[ARM_SLEEP_SAVE_ARM_IDLECT1];
	arg1 = arm_sleep_save[ARM_SLEEP_SAVE_ARM_IDLECT2];

	/*
	 * Step 6c: ARM and Traffic controller shutdown
	 *
	 * Jump to assembly code. The processor will stay there
 	 * until wake up.
	 */
        omap_sram_suspend(arg0, arg1);

	/*
	 * If we are here, processor is woken up!
	 */

	/*
	 * Restore ARM state, except ARM_IDLECT1/2 which omap_cpu_suspend did
	 */

	if (!(cpu_is_omap1510()))
		ARM_RESTORE(ARM_IDLECT3);
	ARM_RESTORE(ARM_CKCTL);
	ARM_RESTORE(ARM_EWUPCT);
	ARM_RESTORE(ARM_RSTCT1);
	ARM_RESTORE(ARM_RSTCT2);
	ARM_RESTORE(ARM_SYSST);
	ULPD_RESTORE(ULPD_CLOCK_CTRL);
	ULPD_RESTORE(ULPD_STATUS_REQ);

	if (cpu_is_omap1510()) {
		MPUI1510_RESTORE(MPUI_CTRL);
		MPUI1510_RESTORE(MPUI_DSP_BOOT_CONFIG);
		MPUI1510_RESTORE(MPUI_DSP_API_CONFIG);
		MPUI1510_RESTORE(EMIFS_CONFIG);
		MPUI1510_RESTORE(EMIFF_SDRAM_CONFIG);
		MPUI1510_RESTORE(OMAP_IH1_MIR);
		MPUI1510_RESTORE(OMAP_IH2_MIR);
	} else if (cpu_is_omap16xx()) {
		MPUI1610_RESTORE(MPUI_CTRL);
		MPUI1610_RESTORE(MPUI_DSP_BOOT_CONFIG);
		MPUI1610_RESTORE(MPUI_DSP_API_CONFIG);
		MPUI1610_RESTORE(EMIFS_CONFIG);
		MPUI1610_RESTORE(EMIFF_SDRAM_CONFIG);

		MPUI1610_RESTORE(OMAP_IH1_MIR);
		MPUI1610_RESTORE(OMAP_IH2_0_MIR);
		MPUI1610_RESTORE(OMAP_IH2_1_MIR);
		MPUI1610_RESTORE(OMAP_IH2_2_MIR);
		MPUI1610_RESTORE(OMAP_IH2_3_MIR);
	}

	omap_writew(0, ULPD_SOFT_DISABLE_REQ_REG);

	/*
	 * Reenable interrupts
	 */

	local_irq_enable();
	local_fiq_enable();

	omap_serial_wake_trigger(0);

	printk("PM: OMAP%x is re-starting from deep sleep...\n", system_rev);

	if (machine_is_omap_osk()) {
		/* Let LED1 (D9) blink again */
		tps65010_set_led(LED1, BLINK);
	}
}

#if defined(DEBUG) && defined(CONFIG_PROC_FS)
static int g_read_completed;

/*
 * Read system PM registers for debugging
 */
static int omap_pm_read_proc(
	char *page_buffer,
	char **my_first_byte,
	off_t virtual_start,
	int length,
	int *eof,
	void *data)
{
	int my_buffer_offset = 0;
	char * const my_base = page_buffer;

	ARM_SAVE(ARM_CKCTL);
	ARM_SAVE(ARM_IDLECT1);
	ARM_SAVE(ARM_IDLECT2);
	if (!(cpu_is_omap1510()))
		ARM_SAVE(ARM_IDLECT3);
	ARM_SAVE(ARM_EWUPCT);
	ARM_SAVE(ARM_RSTCT1);
	ARM_SAVE(ARM_RSTCT2);
	ARM_SAVE(ARM_SYSST);

	ULPD_SAVE(ULPD_IT_STATUS);
	ULPD_SAVE(ULPD_CLOCK_CTRL);
	ULPD_SAVE(ULPD_SOFT_REQ);
	ULPD_SAVE(ULPD_STATUS_REQ);
	ULPD_SAVE(ULPD_DPLL_CTRL);
	ULPD_SAVE(ULPD_POWER_CTRL);

	if (cpu_is_omap1510()) {
		MPUI1510_SAVE(MPUI_CTRL);
		MPUI1510_SAVE(MPUI_DSP_STATUS);
		MPUI1510_SAVE(MPUI_DSP_BOOT_CONFIG);
		MPUI1510_SAVE(MPUI_DSP_API_CONFIG);
		MPUI1510_SAVE(EMIFF_SDRAM_CONFIG);
		MPUI1510_SAVE(EMIFS_CONFIG);
	} else if (cpu_is_omap16xx()) {
		MPUI1610_SAVE(MPUI_CTRL);
		MPUI1610_SAVE(MPUI_DSP_STATUS);
		MPUI1610_SAVE(MPUI_DSP_BOOT_CONFIG);
		MPUI1610_SAVE(MPUI_DSP_API_CONFIG);
		MPUI1610_SAVE(EMIFF_SDRAM_CONFIG);
		MPUI1610_SAVE(EMIFS_CONFIG);
	}

	if (virtual_start == 0) {
		g_read_completed = 0;

		my_buffer_offset += sprintf(my_base + my_buffer_offset,
		   "ARM_CKCTL_REG:            0x%-8x     \n"
		   "ARM_IDLECT1_REG:          0x%-8x     \n"
		   "ARM_IDLECT2_REG:          0x%-8x     \n"
		   "ARM_IDLECT3_REG:	      0x%-8x     \n"
		   "ARM_EWUPCT_REG:           0x%-8x     \n"
		   "ARM_RSTCT1_REG:           0x%-8x     \n"
		   "ARM_RSTCT2_REG:           0x%-8x     \n"
		   "ARM_SYSST_REG:            0x%-8x     \n"
		   "ULPD_IT_STATUS_REG:       0x%-4x     \n"
		   "ULPD_CLOCK_CTRL_REG:      0x%-4x     \n"
		   "ULPD_SOFT_REQ_REG:        0x%-4x     \n"
		   "ULPD_DPLL_CTRL_REG:       0x%-4x     \n"
		   "ULPD_STATUS_REQ_REG:      0x%-4x     \n"
		   "ULPD_POWER_CTRL_REG:      0x%-4x     \n",
		   ARM_SHOW(ARM_CKCTL),
		   ARM_SHOW(ARM_IDLECT1),
		   ARM_SHOW(ARM_IDLECT2),
		   ARM_SHOW(ARM_IDLECT3),
		   ARM_SHOW(ARM_EWUPCT),
		   ARM_SHOW(ARM_RSTCT1),
		   ARM_SHOW(ARM_RSTCT2),
		   ARM_SHOW(ARM_SYSST),
		   ULPD_SHOW(ULPD_IT_STATUS),
		   ULPD_SHOW(ULPD_CLOCK_CTRL),
		   ULPD_SHOW(ULPD_SOFT_REQ),
		   ULPD_SHOW(ULPD_DPLL_CTRL),
		   ULPD_SHOW(ULPD_STATUS_REQ),
		   ULPD_SHOW(ULPD_POWER_CTRL));

		if (cpu_is_omap1510()) {
			my_buffer_offset += sprintf(my_base + my_buffer_offset,
			   "MPUI1510_CTRL_REG             0x%-8x \n"
			   "MPUI1510_DSP_STATUS_REG:      0x%-8x \n"
			   "MPUI1510_DSP_BOOT_CONFIG_REG: 0x%-8x \n"
		   	   "MPUI1510_DSP_API_CONFIG_REG:  0x%-8x \n"
		   	   "MPUI1510_SDRAM_CONFIG_REG:    0x%-8x \n"
		   	   "MPUI1510_EMIFS_CONFIG_REG:    0x%-8x \n",
		   	   MPUI1510_SHOW(MPUI_CTRL),
		   	   MPUI1510_SHOW(MPUI_DSP_STATUS),
		   	   MPUI1510_SHOW(MPUI_DSP_BOOT_CONFIG),
		   	   MPUI1510_SHOW(MPUI_DSP_API_CONFIG),
		   	   MPUI1510_SHOW(EMIFF_SDRAM_CONFIG),
		   	   MPUI1510_SHOW(EMIFS_CONFIG));
		} else if (cpu_is_omap16xx()) {
			my_buffer_offset += sprintf(my_base + my_buffer_offset,
			   "MPUI1610_CTRL_REG             0x%-8x \n"
			   "MPUI1610_DSP_STATUS_REG:      0x%-8x \n"
			   "MPUI1610_DSP_BOOT_CONFIG_REG: 0x%-8x \n"
		   	   "MPUI1610_DSP_API_CONFIG_REG:  0x%-8x \n"
		   	   "MPUI1610_SDRAM_CONFIG_REG:    0x%-8x \n"
		   	   "MPUI1610_EMIFS_CONFIG_REG:    0x%-8x \n",
		   	   MPUI1610_SHOW(MPUI_CTRL),
		   	   MPUI1610_SHOW(MPUI_DSP_STATUS),
		   	   MPUI1610_SHOW(MPUI_DSP_BOOT_CONFIG),
		   	   MPUI1610_SHOW(MPUI_DSP_API_CONFIG),
		   	   MPUI1610_SHOW(EMIFF_SDRAM_CONFIG),
		   	   MPUI1610_SHOW(EMIFS_CONFIG));
		}

		g_read_completed++;
	} else if (g_read_completed >= 1) {
		 *eof = 1;
		 return 0;
	}
	g_read_completed++;

	*my_first_byte = page_buffer;
	return  my_buffer_offset;
}

static void omap_pm_init_proc(void)
{
	struct proc_dir_entry *entry;

	entry = create_proc_read_entry("driver/omap_pm",
				       S_IWUSR | S_IRUGO, NULL,
	   omap_pm_read_proc, NULL);
}

#endif /* DEBUG && CONFIG_PROC_FS */

/*
 *	omap_pm_prepare - Do preliminary suspend work.
 *	@state:		suspend state we're entering.
 *
 */
//#include <asm/arch/hardware.h>

static int omap_pm_prepare(suspend_state_t state)
{
	int error = 0;

	switch (state)
	{
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		break;

	case PM_SUSPEND_DISK:
		return -ENOTSUPP;

	default:
		return -EINVAL;
	}

	return error;
}


/*
 *	omap_pm_enter - Actually enter a sleep state.
 *	@state:		State we're entering.
 *
 */

static int omap_pm_enter(suspend_state_t state)
{
	switch (state)
	{
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		omap_pm_suspend();
		break;

	case PM_SUSPEND_DISK:
		return -ENOTSUPP;

	default:
		return -EINVAL;
	}

	return 0;
}


/**
 *	omap_pm_finish - Finish up suspend sequence.
 *	@state:		State we're coming out of.
 *
 *	This is called after we wake back up (or if entering the sleep state
 *	failed).
 */

static int omap_pm_finish(suspend_state_t state)
{
	return 0;
}


static irqreturn_t  omap_wakeup_interrupt(int  irq, void *  dev,
				     struct pt_regs *  regs)
{
	return IRQ_HANDLED;
}

static struct irqaction omap_wakeup_irq = {
	.name		= "peripheral wakeup",
	.flags		= SA_INTERRUPT,
	.handler	= omap_wakeup_interrupt
};



static struct pm_ops omap_pm_ops ={
	.pm_disk_mode = 0,
        .prepare        = omap_pm_prepare,
        .enter          = omap_pm_enter,
        .finish         = omap_pm_finish,
};

static int __init omap_pm_init(void)
{
	printk("Power Management for TI OMAP.\n");
	/*
	 * We copy the assembler sleep/wakeup routines to SRAM.
	 * These routines need to be in SRAM as that's the only
	 * memory the MPU can see when it wakes up.
	 */
	if (cpu_is_omap1510()) {
		omap_sram_idle = omap_sram_push(omap1510_idle_loop_suspend,
						omap1510_idle_loop_suspend_sz);
		omap_sram_suspend = omap_sram_push(omap1510_cpu_suspend,
						   omap1510_cpu_suspend_sz);
	} else if (cpu_is_omap16xx()) {
		omap_sram_idle = omap_sram_push(omap1610_idle_loop_suspend,
						omap1610_idle_loop_suspend_sz);
		omap_sram_suspend = omap_sram_push(omap1610_cpu_suspend,
						   omap1610_cpu_suspend_sz);
	}

	if (omap_sram_idle == NULL || omap_sram_suspend == NULL) {
		printk(KERN_ERR "PM not initialized: Missing SRAM support\n");
		return -ENODEV;
	}

	pm_idle = omap_pm_idle;

	setup_irq(INT_1610_WAKE_UP_REQ, &omap_wakeup_irq);
#if 0
	/* --- BEGIN BOARD-DEPENDENT CODE --- */
	/* Sleepx mask direction */
	omap_writew((omap_readw(0xfffb5008) & ~2), 0xfffb5008);
	/* Unmask sleepx signal */
	omap_writew((omap_readw(0xfffb5004) & ~2), 0xfffb5004);
	/* --- END BOARD-DEPENDENT CODE --- */
#endif

	/* Program new power ramp-up time
	 * (0 for most boards since we don't lower voltage when in deep sleep)
	 */
	omap_writew(ULPD_SETUP_ANALOG_CELL_3_VAL, ULPD_SETUP_ANALOG_CELL_3);

	/* Setup ULPD POWER_CTRL_REG - enter deep sleep whenever possible */
	omap_writew(ULPD_POWER_CTRL_REG_VAL, ULPD_POWER_CTRL);

	/* Configure IDLECT3 */
	if (cpu_is_omap16xx())
		omap_writel(OMAP1610_IDLECT3_VAL, OMAP1610_IDLECT3);

	pm_set_ops(&omap_pm_ops);

#if defined(DEBUG) && defined(CONFIG_PROC_FS)
	omap_pm_init_proc();
#endif

	/* configure LOW_PWR pin */
	omap_cfg_reg(T20_1610_LOW_PWR);

	return 0;
}
__initcall(omap_pm_init);

