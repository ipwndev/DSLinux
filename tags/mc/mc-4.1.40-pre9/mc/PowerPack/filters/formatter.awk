#! /usr/bin/gawk -f
# Formater of the text from stdin, Petr Koloros 2003 <silk@sinus.cz>
# Change len parameter to the max line size as you want it.

BEGIN {
  len=800  # required max line size
  fc=0    # field counter, ptr to the last field+1
  fl=0    # length of all fileds in buffer with spaces
  spt=0   # start pointer, ptr to the first valid field
  debug=0 # want to see debug messages?
  emptyl=0# if empty line occured in input stream... flush
}

{
 if (debug) print "new line read:"$0
 if ($1 != "") {
    for (cnt=1; cnt<=NF; cnt++) {
     wl=length($cnt);
     buf[fc]=$cnt; fc++; fl+=wl+1;
    }
  } else {emptyl=1; if (debug) print "emptyl=1"}
  if (debug) print "after new line read, fc="fc", spt="spt
  while ((fl>len) || (emptyl == 1)) {
    out=""
    wl=length(buf[spt])
    llen=wl
    while ((llen <= len) && (spt < fc) ) {
      if (out) out=out" "buf[spt]
      else out=buf[spt]
      spt++
      llen+=length(buf[spt])+1
    }
    if (out) fl-=length(out)+1
    print out
    if (emptyl) { emptyl=0; if (debug) print "emptyl=0"; print}
    if (debug) print "line printed printed fc="fc", spt="spt", emptyl="emptyl", fl="fl
  }
  if (spt == fc) {fc=0; spt=0;if (debug) print "fc=spt, vynulovano: fc="fc", spt="spt} else
  if (spt != 0)
  {
    for (cnt=spt; cnt<fc; cnt++) buf[cnt-spt]=buf[cnt]
    fc-=spt; spt=0
    if (debug) print "shifted na fc="fc", spt="spt
  }
}

END {
  out=""
  for (cnt=0; cnt<fc; cnt++) {
    if (out) out=out" "buf[spt]
    else out=buf[spt]
   }
  print out
}
