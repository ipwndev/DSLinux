/*
 * H-TCP congestion control. The algorithm is detailed in:
 * R.N.Shorten, D.J.Leith:
 *   "H-TCP: TCP for high-speed and long-distance networks"
 *   Proc. PFLDnet, Argonne, 2004.
 * http://www.hamilton.ie/net/htcp3.pdf
 */

#include <linux/config.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <net/tcp.h>

#define ALPHA_BASE	(1<<7)  /* 1.0 with shift << 7 */
#define BETA_MIN	(1<<6)  /* 0.5 with shift << 7 */
#define BETA_MAX	102	/* 0.8 with shift << 7 */

static int use_rtt_scaling = 1;
module_param(use_rtt_scaling, int, 0644);
MODULE_PARM_DESC(use_rtt_scaling, "turn on/off RTT scaling");

static int use_bandwidth_switch = 1;
module_param(use_bandwidth_switch, int, 0644);
MODULE_PARM_DESC(use_bandwidth_switch, "turn on/off bandwidth switcher");

struct htcp {
	u16	alpha;		/* Fixed point arith, << 7 */
	u8	beta;           /* Fixed point arith, << 7 */
	u8	modeswitch;     /* Delay modeswitch until we had at least one congestion event */
	u8	ccount;		/* Number of RTTs since last congestion event */
	u8	undo_ccount;
	u16	packetcount;
	u32	minRTT;
	u32	maxRTT;
	u32	snd_cwnd_cnt2;

	u32	undo_maxRTT;
	u32	undo_old_maxB;

	/* Bandwidth estimation */
	u32	minB;
	u32	maxB;
	u32	old_maxB;
	u32	Bi;
	u32	lasttime;
};

static inline void htcp_reset(struct htcp *ca)
{
	ca->undo_ccount = ca->ccount;
	ca->undo_maxRTT = ca->maxRTT;
	ca->undo_old_maxB = ca->old_maxB;

	ca->ccount = 0;
	ca->snd_cwnd_cnt2 = 0;
}

static u32 htcp_cwnd_undo(struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	struct htcp *ca = inet_csk_ca(sk);
	ca->ccount = ca->undo_ccount;
	ca->maxRTT = ca->undo_maxRTT;
	ca->old_maxB = ca->undo_old_maxB;
	return max(tp->snd_cwnd, (tp->snd_ssthresh<<7)/ca->beta);
}

static inline void measure_rtt(struct sock *sk)
{
	const struct inet_connection_sock *icsk = inet_csk(sk);
	const struct tcp_sock *tp = tcp_sk(sk);
	struct htcp *ca = inet_csk_ca(sk);
	u32 srtt = tp->srtt>>3;

	/* keep track of minimum RTT seen so far, minRTT is zero at first */
	if (ca->minRTT > srtt || !ca->minRTT)
		ca->minRTT = srtt;

	/* max RTT */
	if (icsk->icsk_ca_state == TCP_CA_Open && tp->snd_ssthresh < 0xFFFF && ca->ccount > 3) {
		if (ca->maxRTT < ca->minRTT)
			ca->maxRTT = ca->minRTT;
		if (ca->maxRTT < srtt && srtt <= ca->maxRTT+HZ/50)
			ca->maxRTT = srtt;
	}
}

static void measure_achieved_throughput(struct sock *sk, u32 pkts_acked)
{
	const struct inet_connection_sock *icsk = inet_csk(sk);
	const struct tcp_sock *tp = tcp_sk(sk);
	struct htcp *ca = inet_csk_ca(sk);
	u32 now = tcp_time_stamp;

	/* achieved throughput calculations */
	if (icsk->icsk_ca_state != TCP_CA_Open &&
	    icsk->icsk_ca_state != TCP_CA_Disorder) {
		ca->packetcount = 0;
		ca->lasttime = now;
		return;
	}

	ca->packetcount += pkts_acked;

	if (ca->packetcount >= tp->snd_cwnd - (ca->alpha>>7? : 1)
			&& now - ca->lasttime >= ca->minRTT
			&& ca->minRTT > 0) {
		__u32 cur_Bi = ca->packetcount*HZ/(now - ca->lasttime);
		if (ca->ccount <= 3) {
			/* just after backoff */
			ca->minB = ca->maxB = ca->Bi = cur_Bi;
		} else {
			ca->Bi = (3*ca->Bi + cur_Bi)/4;
			if (ca->Bi > ca->maxB)
				ca->maxB = ca->Bi;
			if (ca->minB > ca->maxB)
				ca->minB = ca->maxB;
		}
		ca->packetcount = 0;
		ca->lasttime = now;
	}
}

static inline void htcp_beta_update(struct htcp *ca, u32 minRTT, u32 maxRTT)
{
	if (use_bandwidth_switch) {
		u32 maxB = ca->maxB;
		u32 old_maxB = ca->old_maxB;
		ca->old_maxB = ca->maxB;

		if (!between(5*maxB, 4*old_maxB, 6*old_maxB)) {
			ca->beta = BETA_MIN;
			ca->modeswitch = 0;
			return;
		}
	}

	if (ca->modeswitch && minRTT > max(HZ/100, 1) && maxRTT) {
		ca->beta = (minRTT<<7)/maxRTT;
		if (ca->beta < BETA_MIN)
			ca->beta = BETA_MIN;
		else if (ca->beta > BETA_MAX)
			ca->beta = BETA_MAX;
	} else {
		ca->beta = BETA_MIN;
		ca->modeswitch = 1;
	}
}

static inline void htcp_alpha_update(struct htcp *ca)
{
	u32 minRTT = ca->minRTT;
	u32 factor = 1;
	u32 diff = ca->ccount * minRTT; /* time since last backoff */

	if (diff > HZ) {
		diff -= HZ;
		factor = 1+ ( 10*diff + ((diff/2)*(diff/2)/HZ) )/HZ;
	}

	if (use_rtt_scaling && minRTT) {
		u32 scale = (HZ<<3)/(10*minRTT);
		scale = min(max(scale, 1U<<2), 10U<<3); /* clamping ratio to interval [0.5,10]<<3 */
		factor = (factor<<3)/scale;
		if (!factor)
			factor = 1;
	}

	ca->alpha = 2*factor*((1<<7)-ca->beta);
	if (!ca->alpha)
		ca->alpha = ALPHA_BASE;
}

/* After we have the rtt data to calculate beta, we'd still prefer to wait one
 * rtt before we adjust our beta to ensure we are working from a consistent
 * data.
 *
 * This function should be called when we hit a congestion event since only at
 * that point do we really have a real sense of maxRTT (the queues en route
 * were getting just too full now).
 */
static void htcp_param_update(struct sock *sk)
{
	struct htcp *ca = inet_csk_ca(sk);
	u32 minRTT = ca->minRTT;
	u32 maxRTT = ca->maxRTT;

	htcp_beta_update(ca, minRTT, maxRTT);
	htcp_alpha_update(ca);

	/* add slowly fading memory for maxRTT to accommodate routing changes etc */
	if (minRTT > 0 && maxRTT > minRTT)
		ca->maxRTT = minRTT + ((maxRTT-minRTT)*95)/100;
}

static u32 htcp_recalc_ssthresh(struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	const struct htcp *ca = inet_csk_ca(sk);
	htcp_param_update(sk);
	return max((tp->snd_cwnd * ca->beta) >> 7, 2U);
}

static void htcp_cong_avoid(struct sock *sk, u32 ack, u32 rtt,
			    u32 in_flight, int data_acked)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct htcp *ca = inet_csk_ca(sk);

	if (in_flight < tp->snd_cwnd)
		return;

        if (tp->snd_cwnd <= tp->snd_ssthresh) {
                /* In "safe" area, increase. */
		if (tp->snd_cwnd < tp->snd_cwnd_clamp)
			tp->snd_cwnd++;
	} else {
		measure_rtt(sk);

		/* keep track of number of round-trip times since last backoff event */
		if (ca->snd_cwnd_cnt2++ > tp->snd_cwnd) {
			ca->ccount++;
			ca->snd_cwnd_cnt2 = 0;
			htcp_alpha_update(ca);
		}

                /* In dangerous area, increase slowly.
		 * In theory this is tp->snd_cwnd += alpha / tp->snd_cwnd
		 */
		if ((tp->snd_cwnd_cnt++ * ca->alpha)>>7 >= tp->snd_cwnd) {
			if (tp->snd_cwnd < tp->snd_cwnd_clamp)
				tp->snd_cwnd++;
			tp->snd_cwnd_cnt = 0;
			ca->ccount++;
		}
	}
}

/* Lower bound on congestion window. */
static u32 htcp_min_cwnd(struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	return tp->snd_ssthresh;
}


static void htcp_init(struct sock *sk)
{
	struct htcp *ca = inet_csk_ca(sk);

	memset(ca, 0, sizeof(struct htcp));
	ca->alpha = ALPHA_BASE;
	ca->beta = BETA_MIN;
}

static void htcp_state(struct sock *sk, u8 new_state)
{
	switch (new_state) {
	case TCP_CA_CWR:
	case TCP_CA_Recovery:
	case TCP_CA_Loss:
		htcp_reset(inet_csk_ca(sk));
		break;
	}
}

static struct tcp_congestion_ops htcp = {
	.init		= htcp_init,
	.ssthresh	= htcp_recalc_ssthresh,
	.min_cwnd	= htcp_min_cwnd,
	.cong_avoid	= htcp_cong_avoid,
	.set_state	= htcp_state,
	.undo_cwnd	= htcp_cwnd_undo,
	.pkts_acked	= measure_achieved_throughput,
	.owner		= THIS_MODULE,
	.name		= "htcp",
};

static int __init htcp_register(void)
{
	BUG_ON(sizeof(struct htcp) > ICSK_CA_PRIV_SIZE);
	BUILD_BUG_ON(BETA_MIN >= BETA_MAX);
	if (!use_bandwidth_switch)
		htcp.pkts_acked = NULL;
	return tcp_register_congestion_control(&htcp);
}

static void __exit htcp_unregister(void)
{
	tcp_unregister_congestion_control(&htcp);
}

module_init(htcp_register);
module_exit(htcp_unregister);

MODULE_AUTHOR("Baruch Even");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("H-TCP");
