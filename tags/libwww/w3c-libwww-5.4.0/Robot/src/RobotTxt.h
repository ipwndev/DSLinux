/*

  
  					Robots Exclusion Protocol File Parser


!
  Robots Exclusion Protocol File Parser
!
*/

/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This module parses a
robots.txt
exclusion file which nice robots are expected to honor. Together with the
robot
META tags, the webbot should now behave itself on the Internet.
*/

#ifndef ROBOTTXT_H
#define ROBOTTXT_H

typedef struct _user_agent_ {
  char * name;
  HTList * disallow;
} UserAgent;

extern char * skip_comments(char *ptr);

extern UserAgent * new_user_agent(void);

extern BOOL set_name_user_agent(UserAgent *ua,char *name);

extern char * get_name_user_agent(UserAgent *ua);

extern BOOL add_disallow_user_agent(UserAgent *ua, char *disallow);

extern HTList * get_disallow_user_agent(UserAgent *ua);

extern BOOL delete_user_agent(UserAgent *ua);

extern BOOL delete_all_user_agents(HTList *user_agents);

extern void print_user_agent(UserAgent *ua);

extern void print_all_user_agents(HTList *user_agents);

extern HTList * get_all_user_agents(char *rob_str);

extern BOOL get_user_agents(char * ptr, HTList *user_agents);

extern BOOL put_string_disallow(HTChunk *ch, UserAgent *ua);

extern char * get_regular_expression(HTList* user_agents, char *name_robot);

extern char * scan_robots_txt(char *rob_str, char *name_robot);

#endif

/*

  

  @(#) $Id: RobotTxt.html,v 1.2 1998/10/27 17:42:53 frystyk Exp $

*/
