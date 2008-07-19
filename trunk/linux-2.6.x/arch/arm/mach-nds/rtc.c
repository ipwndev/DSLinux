/*
 *  linux/arch/armnommu/mach-nds/time.c
 *
 */
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/init.h>

#include <asm/segment.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <linux/timex.h>
#include <asm/hardware.h>

#include <asm/arch/fifo.h>



static u32 ndshwtime = 0;
static u8  timeavail = 0;

static DECLARE_WAIT_QUEUE_HEAD(ndstime_wait);

static u32 getTimeFromArm7(void) {
    timeavail = 0;
    
    nds_fifo_send(FIFO_TIME);

    wait_event_interruptible(ndstime_wait, timeavail != 0);

    return ndshwtime;
}

static void ndsupdate_time(u32 seconds)
{
        if(seconds & FIFO_HIGH_BITS) {
                timeavail = 0;
                ndshwtime = (seconds & 0xffff) << 16;
        } else if(seconds & FIFO_LOW_BITS) {
                ndshwtime = ndshwtime | (seconds & 0xffff);
                timeavail = 1;
                wake_up_interruptible(&ndstime_wait);
        } 
}

extern void setTimeFromRTC (void) {
    xtime.tv_sec = getTimeFromArm7();
}

static struct fifo_cb ndstime_fifocb = {
        .type = FIFO_TIME,
        .handler.time_handler = ndsupdate_time
};

static int __init ndsrtc_init(void)
{
    register_fifocb( &ndstime_fifocb );
	setTimeFromRTC();

    return 0;
}

static void __exit ndsrtc_exit(void)
{
}

module_init(ndsrtc_init);
module_exit(ndsrtc_exit);
