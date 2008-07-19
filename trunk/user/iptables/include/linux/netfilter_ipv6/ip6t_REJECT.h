#ifndef _IP6T_REJECT_H
#define _IP6T_REJECT_H

enum ip6t_reject_with {
	IP6T_ICMP6_NO_ROUTE,
	IP6T_ICMP6_ADM_PROHIBITED,
	IP6T_ICMP6_ADDR_UNREACH,
	IP6T_ICMP6_PORT_UNREACH,
	IP6T_TCP_RESET
};

struct ip6t_reject_info {
	enum ip6t_reject_with with;      /* reject type */
};

#endif /*_IP6T_REJECT_H*/
