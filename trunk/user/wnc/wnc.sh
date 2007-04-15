#!/bin/msh
# vim: noexpandtab
# wnc.sh - wireless net configuration script for DSLinux
# Licence: GNU GPL v2 or later at your option
# Author:  John S. Skogtvedt (jss2k2@chello.no)
# Year:    2006-2007
# Version: 0.9
#
# wnc works around the following msh bugs/limitations (list may be incomplete):
#  No support for functions
#  break in nested loop causes msh to segfault or hang
#  . (source) leaks "io contexts", causing msh to exit when it runs out of them
#  msh doesn't restore stdin after "cat file | while read; ... ; done"
#
# wnc depends on the following implementation specific behaviour:
#  echo foo | read bar; echo $bar # should output foo
#
# If you make changes, see the README for information on how to test
# without having to copy wnc.sh over to your DS.

VARDIR=/var/wncsh
PROFILEDIR="$VARDIR/profiles"
NEWLINE="
"
OLDIFS="$IFS"
DLGSIZE="27 57"
# XXX these are hardcoded several places, and have to be kept up to sync with
# DSLinux' /etc/rc.d/network script
PROFILEVARS="essid channel wepkey ip netmask broadcast gateway dns1 dns2 use_dhcp PROFILE"

if [ ! -d "$VARDIR" ]
then
	mkdir "$VARDIR" || exit 1
fi
if [ ! -d "$PROFILEDIR" ]
then
	mkdir "$PROFILEDIR" || exit 1
fi

a=`/bin/false`
if [ $? != 1 ]
then
	dialog --msgbox "Unpatched msh detected, install a recent DSLinux snapshot" $DLGSIZE
	exit 1
fi

func=start
while :
do
#echo $func # FIXME comment out testing only
#sleep 1 # FIXME comment out testing only
case $func in
start)
	dlginput=`dialog --stdout --title "Wifi configuration" \
	--menu "Press Esc or choose Cancel to exit.
This is a wizard-style interface, use OK and Cancel to go back and forth." $DLGSIZE 8 \
	"WFC" "Use WFC settings" \
	"Load" "Load existing profile" \
	"Auto" "Load existing profile matching scan" \
	"Create" "Create new profile" \
	"Scan" "Create new profile from scan" \
	"Edit" "Edit existing profile" \
	"Delete" "Delete profile" \
	"Off" "Bring down wifi"`
	case $? in
		1|255) exit ;;
	esac
	case $dlginput in
		WFC) func=wfc ;;
		Load) choose_profile_call=load2; func=choose_profile ;;
		Auto) func=auto ;;
		Create) create1_prev=bstart; func=create1 ;;
		Scan) func=scan ;;
		Edit) choose_profile_call=edit2; func=choose_profile ;;
		Delete) choose_profile_call=delete2; func=choose_profile ;;
		Off) exec /etc/rc.d/network stop ;;
	esac
	dlginput=
	;;
wfc)
	args=
	for x in 1 2 3
	do
		IFS="$NEWLINE"
		wfcdump -c$x 2>/dev/null | while read line
		do
			case "$line" in
			essid=*) eval "$line"; args="$args $x \"essid: $essid\"" ;;
			esac
		done
		IFS="$OLDIFS"
		[ -z "$essid" ] && args="$args $x \"not configured\""
		essid=
	done
	exec 0</dev/tty
	eval dlginput=\`dialog --stdout --title WFC --menu \"Choose WFC slot\" $DLGSIZE 3 "$args"\`
	case $? in
		1|255) func=start ;;
		*)
		if wfcdump -c $dlginput > "$VARDIR/wfc" 2>/dev/null
		then
			export NETWORK_CONFIG="$VARDIR/wfc"
			exec /etc/rc.d/network start
		else
			dialog --msgbox "WFC slot $dlginput is not configured." $DLGSIZE
		fi
	esac
	;;

bstart) # blank all vars and go to start
	for var in $PROFILEVARS 
	do
		eval $var=
	done
	func=start
	;;

create1) # create new profile - also called from auto
	dlginput=`dialog --stdout --title Profile --inputbox \
	"Profile name
Please use only alphanumeric characters" $DLGSIZE "$PROFILE"`
	case $? in
		0)
			if [ -e "$PROFILEDIR/$dlginput" ]
			then
				dialog --msgbox "Profile \"$dlginput\" already exists." $DLGSIZE
			else
				PROFILE="$dlginput"
				edit2_prev=create1
				func=edit2
			fi
			;;
		1|255)
			if [ -n "$edit2_prev" ] && [ "$create1_prev" != scan ]
			then
				func=bstart
			else
				func=$create1_prev
			fi
			;;
	esac
	dlginput=
	;;

edit2) # start to do the actual work of asking for config info
	dlginput=`dialog --stdout --form \
	"Wireless configuration
WEP key: For ASCII, use the s: prefix, e.g. s:mykey" $DLGSIZE 3 \
	"ESSID" 1 1 "$essid" 1 12 33 32 \
	"Channel" 2 1 "$channel" 2 12 3 2 \
	"WEP key" 3 1 "$wepkey" 3 12 27 26`

	if [ $? = 0 ]
	then
		IFS="$NEWLINE"
		set -- $dlginput
		IFS="$OLDIFS"
		dlginput=
		essid="$1"
		channel="$2"
		wepkey="$3"
		set ""
		func=edit3
	else
		func=$edit2_prev
	fi
	;;

edit3) # prompt for DHCP or manual configuration
	dlginput=`dialog --stdout --title "Configuration type" --menu "" $DLGSIZE 2 \
	"DHCP" "Automatic network configuration using DHCP" \
	"Manual" "Manual network configuration"`
	if [ $? = 0 ]
	then
		case $dlginput in
			DHCP) ip=; use_dhcp=YES; func=edit_done ;;
			Manual) use_dhcp=NO; func=edit_manual ;;
		esac
	else
		func=edit2
	fi
	dlginput=
	;;

edit_manual) # ask for manual configuration info
	dlginput=`dialog --stdout --form \
	"Manual configuration
1st DNS: First DNS nameserver
2nd DNS: Second DNS nameserver" $DLGSIZE 6 \
	"IP address" 1 1 "$ip" 1 12 16 15 \
	"Netmask" 2 1 "$netmask" 2 12 16 15 \
	"Broadcast" 3 1 "$broadcast" 3 12 16 15 \
	"Gateway" 4 1 "$gateway" 4 12 255 35 \
	"1st DNS" 5 1 "$dns1" 5 12 16 15 \
	"2nd DNS" 6 1 "$dns2" 6 12 16 15`
	IFS="$OLDIFS"

	if [ $? = 0 ]
	then
		IFS="$NEWLINE"
		set -- $dlginput
		IFS="$OLDIFS"
		dlginput=
		ip="$1"
		netmask="$2"
		broadcast="$3"
		gateway="$4"
		dns1="$5"
		dns2="$6"
		set ""
		func=edit_done
	else
		func=edit3
	fi
	;;

edit_done) # edit done, save profile
	> "$PROFILEDIR/$PROFILE" # clear file
	for var in $PROFILEVARS 
	do
		eval echo "$var=\"\\\"\$$var\\\"\"" \>\> "\"$PROFILEDIR/$PROFILE\""
	done
	var=
	func=bstart
	;;

choose_profile)
	prof=
	cd "$PROFILEDIR"
	for f in *
	do
		prof="$prof \"$f\" \"\""
	done
	if [ "$f" = \* ]
	then
		func=start
	else
		eval PROFILE=\`dialog --stdout --title Profile --menu \"Choose profile\" $DLGSIZE 15 "$prof"\`
		if [ $? = 0 ]
		then
			# avoid . "$f", msh leaks io contexts
			cat "$PROFILEDIR/$PROFILE" | while read line
			do
				eval "$line"
			done
			exec 0</dev/tty
			line=
			eval ${choose_profile_call}_prev=choose_profile
			func=$choose_profile_call
		else
			func=start
		fi
	fi
	f=
	prof=
	;;

load2)
	export NETWORK_CONFIG="$PROFILEDIR/$PROFILE"
	exec /etc/rc.d/network start
	;;

doscan) # fill up scan_results essid|channel|wep / essid|channel|wep ...
	scan_results=
	echo "Scanning..."
	ifconfig nds up
	iwlist nds scan | while read line
	do
		case "$line" in
			*ESSID:*)
				IFS=:
				set -- $line
				IFS="$OLDIFS"
				# essid is passed through eval, check that it
				# doesn't contain " or ` (injection attack)
				case "$2" in
					\"*\"*\"|*\`*) essid=ignored ;;
					*) eval essid="$2" ;; # eval because $2 contains ""
				esac
				scan_results="$scan_results$essid"
				essid=
				;;
			*Channel:*)
				IFS=:
				set -- $line
				IFS="$OLDIFS"
				# same as with the essid
				case "$2" in
					*\"*|*\`*) scan_results="$scan_results|0" ;;
					*) scan_results="$scan_results|$2" ;;
				esac
				;;
			*key:on)
				scan_results="$scan_results|on/"
				;;
			*key:off)
				scan_results="$scan_results|off/"
				;;
		esac
	done
	exec 0</dev/tty
	line=
	func=$doscan_prev
	doscan=1
	;;
	
scan)
	if [ "$doscan" != 1 ]
	then
		func=doscan
		doscan_prev=scan
		continue
	else
		doscan=0
	fi
	cmd=

	i=1
	IFS=/
	for net in $scan_results
	do
		IFS=\|
		set -- $net
		IFS="$OLDIFS"
		#[ $# = 3 ] && [ -z "$1" ] && shift 1 # XXX remove - for ksh testing
		if [ $# = 3 ] # FIXME support hidden ESSID?
		then	
			cmd="$cmd $i \"ESSID: $1 CHANNEL: $2 ENC: $3\""
		fi
                i=`expr $i + 1`
	done
	IFS="$OLDIFS"
	eval ii=\`dialog --stdout --title \"Choose wireless network\" --menu \"Hidden ESSIDs are not shown.\" $DLGSIZE $i "$cmd"\` || { func=bstart; continue; }

	i=1
	IFS=/
	for net in $scan_results
	do
		IFS=\|
		set -- $net
		IFS="$OLDIFS"
		if [ $i = $ii ]
		then
			essid="$1"
			channel="$2"
			wep="$3"
			PROFILE="$1"
		fi
                i=`expr $i + 1`
	done
	IFS="$OLDIFS"

	net=
	i=
	ii=
	cmd=
	scan_results=
	[ $wep = on ] && wep_required=1
	create1_prev=scan
	func=create1
	;;

auto)
	if [ "$doscan" != 1 ]
	then
		func=doscan
		doscan_prev=auto
		continue
	else
		doscan=0
	fi

	profiles=
	cd "$PROFILEDIR"
	for f in *
	do
		if [ "$f" != "*" ]
		then
			# avoid . "$f", msh leaks io contexts
			cat "$f" | while read line
			do
				eval "$line"
			done
			exec 0</dev/tty
			line=
			[ -n "$wepkey" ] && profiles="$profiles$f|$essid|$channel|on/" || profiles="$profiles$f|$essid|$channel|off/"
		fi
	done
	f=

	cmd=
	IFS=/
	for profile in $profiles
	do
		IFS=\|
		set -- $profile

		IFS=/
		for net in $scan_results
		do
			if [ "$2|$3|$4" = "$net" ]
			then
				cmd="$cmd \"$1\" \"ESSID: $2 CHANNEL: $3 ENC: $4\""
			fi
		done
	done
	IFS="$OLDIFS"
	profile=
	net=
	profiles=
	scan_results=

	eval PROFILE=\`dialog --stdout --title Profile --menu \"Choose profile\" $DLGSIZE 15 "$cmd"\` || { func=bstart; continue; }
	cat "$PROFILEDIR/$PROFILE" | while read line
	do
		eval "$line"
	done
	exec 0</dev/tty
	func=load2
	cmd=
	;;

delete2)
	rm -f "$PROFILEDIR/$PROFILE"
	func=bstart
	;;
esac
done
