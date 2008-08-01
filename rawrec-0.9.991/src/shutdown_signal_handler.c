/* Copyright (C) 2006  Britton Leo Kerin, see copyright.  */

/* Handler for SIGTERM.  This function may eventually be generalized
   to handle other signals which cause the program to shutdown as well
   in the same way (by setting a flag) at which time its name will
   need to be changed.  */

#include <signal.h>
#include <stdio.h>

/* This prototype isn't in rawrec.h, since this somewhat awkward
   method of dealing with these signals will go away someday.  */
void shutdown_signal_handler(int signum);

/* Flag to indicate to the outside world that we caught a shutdown
   signal, since we can't POSIXly do pthread calls from the handler
   itself.  It true, this gets set to the value of the signal we
   got.  */
int got_watched_for_shutdown_signal = 0;

/* This handler is only installed on signals for which we want to do
   clean managed shutdown.  */
void shutdown_signal_handler(int signum) 
{
  got_watched_for_shutdown_signal = signum;
}
