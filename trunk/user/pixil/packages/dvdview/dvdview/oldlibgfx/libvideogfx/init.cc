/*
 *  init.cc
 */

#include "init.hh"


char** glob_argv;
int    glob_argc;

void InitULib(int argc,char** argv)
{
  glob_argc = argc;
  glob_argv = argv;
}
