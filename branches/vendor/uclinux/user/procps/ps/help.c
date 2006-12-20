/*
 * Copyright 1998 by Albert Cahalan; all rights reserved.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */

/*
 * The help message must not become longer, because it must fit
 * on an 80x24 screen _with_ the error message and command prompt.
 */


const char *usage_message =
"usage: ps -[Unix98 options]\n"
"       ps [BSD-style options]\n"
"       ps --[GNU-style long options]\n"
"       ps --help for a command summary\n"
;

const char *help_message =
"********* simple selection *********  ********* selection by list *********\n"
"-A all processes                      -C by command name\n"
"-N negate selection                   -G by real group ID (supports names)\n"
"-a all w/ tty except session leaders  -U by real user ID (supports names)\n"
"-d all except session leaders         -g by session leader OR by group name\n"
"-e all processes                      -p by process ID\n"
"T  all processes on this terminal     -s processes in the sessions given\n"
"a  all w/ tty, including other users  -t by tty\n"
"g  all, even group leaders!           -u by effective user ID (supports names)\n"
"r  only running processes             U  processes for specified users\n"
"x  processes w/o controlling ttys     t  by tty\n"
"*********** output format **********  *********** long options ***********\n"
"-o,o user-defined  -f full            --Group --User --pid --cols\n"
"-j,j job control   s  signal          --group --user --sid --rows\n"
"-O,O preloaded -o  v  virtual memory  --cumulative --format --deselect\n"
"-l,l long          u  user-oriented   --sort --tty --forest --version\n"
"                   X  registers       --heading --no-heading\n"
"                    ********* misc options *********\n"
"-V,V show version       L  list format codes  f  ASCII art forest\n"
"-m,m show threads       S  children in sum    -y change -l format\n"
"-n,N set namelist file  c  true command name  n  numeric WCHAN,UID\n"
"-w,w wide output        e  show environment   -H process heirarchy\n"
;



/* Missing:
 *
 * -c -L -P --html --nul -M --info
 *
 */
 
