#!/bin/sh

IP=$1

IPTABLES=/opt/iptables-1.2.6/sbin/iptables
UNIVERSE=0.0.0.0/0

#OUTER SERVER  IP
#SRV=192.168.1.1
IFIN=eth1
IFOUT=ppp0
SRV="`ifconfig $IFOUT | grep 'inet addr' | awk '{print $2}' | sed -e 's/.*://'`"

echo "1" > /proc/sys/net/ipv4/ip_forward
echo "1" > /proc/sys/net/ipv4/ip_dynaddr

$IPTABLES -F INPUT
$IPTABLES -F OUTPUT
$IPTABLES -F FORWARD
$IPTABLES -F -t nat

$IPTABLES -F
$IPTABLES -X
$IPTABLES -Z

$IPTABLES -P INPUT DROP
$IPTABLES -P OUTPUT DROP
$IPTABLES -P FORWARD DROP


# loopback interfaces are valid.
$IPTABLES -A INPUT -i lo -s $UNIVERSE -d $UNIVERSE -j ACCEPT
$IPTABLES -A OUTPUT -o lo -s $UNIVERSE -d $UNIVERSE -j ACCEPT

#if [ -n "`$IPTABLES -L | grep rule_$IP`" ]; then
   $IPTABLES -F rule_$IP
#fi

# external forwarding counter rule
$IPTABLES -N rule_$IP
$IPTABLES -A rule_$IP -c 1 1 -s $IP -o $IFOUT 
$IPTABLES -A rule_$IP -c 1 1 -d $IP -i $IFOUT
$IPTABLES -A rule_$IP -j ACCEPT
# forwarding ==>> accounting
$IPTABLES -A FORWARD -d $IP -i $IFOUT -o $IFIN -m state --state ESTABLISHED,RELATED -j rule_$IP
$IPTABLES -A FORWARD -s $IP -i $IFIN -o $IFOUT -j rule_$IP
# machine <-> server ==>> accounting
$IPTABLES -A INPUT -i $IFIN -s $IP -d $UNIVERSE -j rule_$IP
$IPTABLES -A OUTPUT -o $IFIN -s $UNIVERSE -d $IP -j rule_$IP
# block internal network adressess, comes from external interface
$IPTABLES -A OUTPUT  -o $IFOUT -s $UNIVERSE -d $IP/24 -j DROP
$IPTABLES -A INPUT   -i $IFOUT -s $IP/24 -d $UNIVERSE -j DROP
# machine NAT
$IPTABLES -t nat -A POSTROUTING -s $IP -o $IFOUT -j MASQUERADE
#$IPTABLES -t nat -A POSTROUTING -o $IFOUT -j SNAT --to $SRV

# opens server
$IPTABLES -A OUTPUT  -o $IFOUT -s $SRV -d $UNIVERSE -j ACCEPT
$IPTABLES -A INPUT   -i $IFOUT -s $UNIVERSE -d $SRV -m state --state ESTABLISHED,RELATED -j ACCEPT
# ends firewall - close forward
#$IPTABLES -D FORWARD -j DROP
$IPTABLES -A FORWARD -j DROP

