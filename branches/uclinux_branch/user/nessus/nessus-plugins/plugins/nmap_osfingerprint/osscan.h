
/***********************************************************************
 * osscan.h -- Routines used for OS detection via TCP/IP               *
 * fingerprinting.  For more information on how this works in Nmap,    *
 * see my paper at                                                     *
 * http://www.insecure.org/nmap/nmap-fingerprinting-article.html       *
 *                                                                     *
 ***********************************************************************
 *  The Nmap Security Scanner is (C) 1995-2001 Insecure.Com LLC. This  *
 *  program is free software; you can redistribute it and/or modify    *
 *  it under the terms of the GNU General Public License as published  *
 *  by the Free Software Foundation; Version 2.  This guarantees your  *
 *  right to use, modify, and redistribute this software under certain *
 *  conditions.  If this license is unacceptable to you, we may be     *
 *  willing to sell alternative licenses (contact sales@insecure.com). *
 *                                                                     *
 *  If you received these files with a written license agreement       *
 *  stating terms other than the (GPL) terms above, then that          *
 *  alternative license agreement takes precendence over this comment. *
 *                                                                     *
 *  Source is provided to this software because we believe users have  *
 *  a right to know exactly what a program is going to do before they  *
 *  run it.  This also allows you to audit the software for security   *
 *  holes (none have been found so far).                               *
 *                                                                     *
 *  Source code also allows you to port Nmap to new platforms, fix     *
 *  bugs, and add new features.  You are highly encouraged to send     *
 *  your changes to fyodor@insecure.org for possible incorporation     *
 *  into the main distribution.  By sending these changes to Fyodor or *
 *  one the insecure.org development mailing lists, it is assumed that *
 *  you are offering Fyodor the unlimited, non-exclusive right to      *
 *  reuse, modify, and relicense the code.  This is important because  *
 *  the inability to relicense code has caused devastating problems    *
 *  for other Free Software projects (such as KDE and NASM).  Nmap     *
 *  will always be available Open Source.  If you wish to specify      *
 *  special license conditions of your contributions, just say so      *
 *  when you send them.                                                *
 *                                                                     *
 *  This program is distributed in the hope that it will be useful,    *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details (                          *
 *  http://www.gnu.org/copyleft/gpl.html ).                            *
 *                                                                     *
 ***********************************************************************/

/* $Id$ */

#ifndef OSSCAN_H
#define OSSCAN_H

#include <includes.h>
#include "nmap.h"
#include "tcpip.h"
#include "global_structures.h"

#define OSSCAN_SUCCESS 0
#define OSSCAN_NOMATCHES -1
#define OSSCAN_TOOMANYMATCHES -2

/* We won't even consider matches with a lower accuracy than this */
#define OSSCAN_GUESS_THRESHOLD 0.85
/**********************  STRUCTURES  ***********************************/

/* moved to global_structures.h */

/**********************  PROTOTYPES  ***********************************/
FILE* fetch_fingerprint_file(void);
int os_scan(struct hoststruct *target);
FingerPrint *get_fingerprint(struct hoststruct *target, struct seq_info *si);
struct AVal *fingerprint_iptcppacket(struct ip *ip, int mss, unsigned int syn);
struct AVal *fingerprint_portunreach(struct ip *ip, struct udpprobeinfo *upi);
struct udpprobeinfo *send_closedudp_probe(int rawsd, struct in_addr *dest, u16 sport, u16 dport);
char *fp2ascii(FingerPrint * FP);

/* Compares 2 fingerprints -- a referenceFP (can have expression
   attributes) with an observed fingerprint (no expressions).  If
   verbose is nonzero, differences will be printed.  The comparison
   accuracy (between 0 and 1) is returned) */
double compare_fingerprints(FingerPrint *referenceFP, FingerPrint *observedFP,
			    int verbose);

/* Takes a fingerprint and looks for matches inside reference_FPs[].
   The results are stored in in FPR (which must point to an allocated
   FingerPrintResults structure) -- results will be reverse-sorted by
   accuracy.  No results below accuracy_threshhold will be included.
   The max matches returned is the maximum that fits in a
   FingerPrintResults structure.  The allocated FingerPrintResults
   does not have to be initialized -- that will be done in this
   function.  */
void match_fingerprint(FingerPrint *FP, struct FingerPrintResults *FPR, 
		       double accuracy_threshold);
struct AVal *str2AVal(char *p);
struct AVal *gettestbyname(FingerPrint *FP, const char *name);


/* Returns true if perfect match -- if num_subtests & num_subtests_succeeded are non_null it updates them.  if shortcircuit is zero, it does all the tests, otherwise it returns when the first one fails */

/* Returns true if perfect match -- if num_subtests &
   num_subtests_succeeded are non_null it ADDS THE NEW VALUES to what
   is already there.  So initialize them to zero first if you only
   want to see the results from this match.  if shortcircuit is zero,
   it does all the tests, otherwise it returns when the first one
   fails */
int AVal_match(struct AVal *reference, struct AVal *fprint, unsigned long *num_subtests, unsigned long *num_subtests_succeeded, int shortcut);

void freeFingerPrint(FingerPrint * FP);
char *mergeFPs(FingerPrint * FPs[], int numFPs, int openport, int closedport);

/* Writes an informational "Test" result suitable for including at the
   top of a fingerprint.  Gives info which might be useful when the
   FPrint is submitted (eg Nmap version, etc).  Result is written (up
   to ostrlen) to the ostr var passed in */
void WriteSInfo(char *ostr, int ostrlen, int openport, int closedport);

/* This function takes an array of "numSamples" IP IDs and analyzes
 them to determine their sequenceability classification.  It returns
 one of the IPID_SEQ_* classifications defined in nmap.h .  If the
 function cannot determine the sequence, IPID_SEQ_UNKNOWN is returned.
 This islocalhost argument is a boolean specifying whether these
 numbers were generated by scanning localhost.  NOTE: the "ipids" argument
 may be modified if localhost is set to true. */
int ipid_sequence(int numSamples, u16 * ipids, int islocalhost);

#endif				/*OSSCAN_H */
