#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "history.h"

//#define ENV_STORAGE
#define STR_LEN 256
#define ENV_PREFIX "VIEWMLH"

History::History()
{
  char buf[256];
  int index;

  // allocate string storage
  index = 0;
  do
  {
    m_list[index] = (char *)malloc(STR_LEN);
    *(m_list[index]) = 0x00;
  }while(++index < MAX_HISTORY_LEN);

  // push an initial URL on the stack
  strcpy(m_list[0],"http://www.viewml.com");

  // load all entries from the environment
#ifdef ENV_STORAGE
  char * ptr;
  index = 0;
  do
  {
    sprintf(buf,"%s%02d",ENV_PREFIX,index);
    ptr = getenv(buf);
    if(ptr == NULL)
      break;
    AddToHistoryList(ptr);
  }while(++index < MAX_HISTORY_LEN);
#else
  FILE *fp;
  strcpy(buf,getenv("HOME"));
  strcat(buf,"/.viewmlh.txt");
  if((fp = fopen(buf,"r")) == NULL)
    return;
  index = 0;
  while(1)
  {
    if(fgets(buf,256,fp) == NULL)
      break;
    buf[strlen(buf) - 1] = 0x00;
    AddToHistoryList(buf);
  }
  fclose(fp);
#endif
}

History::~History(void)
{
  char buf[256];
  int index;

  // save all entries to the environment
#ifdef ENV_STORAGE
  
  index = MAX_HISTORY_LEN - 1;
  do
  {
    if(*(m_list[index]) == 0x00)
      continue;
    sprintf(buf,"%s%02d",ENV_PREFIX,index);
    setenv(buf,m_list[index],1);
  }while(--index >= 0);
#else
  FILE *fp;
  strcpy(buf,getenv("HOME"));
  strcat(buf,"/.viewmlh.txt");
  if((fp = fopen(buf,"w")) == NULL)
    return;
  index = MAX_HISTORY_LEN - 1;
  do
  {
    if(*(m_list[index]) == 0x00)
      continue;
    fprintf(fp,"%s\n",m_list[index]);
  }while(--index >= 0);
  fclose(fp);
#endif

  // free string storage
  index = 0;
  do
  {
    free(m_list[index]);
  }while(++index < MAX_HISTORY_LEN);
}

void History::AddToHistoryList(const char *str)
{
  char *ptr;
  int index;

  // scan the list - see if "str" is already there
  index = 0;
  do
  {
    if(strcmp(str,m_list[index]) == 0)
       break;
  }while(++index < MAX_HISTORY_LEN);

  // entry exists - just bubble it up to top
  if(index != MAX_HISTORY_LEN)
  {
    if(index == 0)
      return;
    ptr = m_list[index--];
    while(index >= 0)
      m_list[index + 1] = m_list[index--];
    m_list[0] = ptr;
  }

  // entry does not exist - push it on the stack
  else
  {
    ptr = m_list[MAX_HISTORY_LEN - 1];
    index = MAX_HISTORY_LEN - 2;
    while(index >= 0)
      m_list[index + 1] = m_list[index--];
    m_list[0] = ptr;
    strcpy(m_list[0],str);
  }
}

const char *History::GetHistoryEntry(int entry)
{
  if(entry >= MAX_HISTORY_LEN)
    return(NULL);
  if(*(m_list[entry]) == 0x00)
    return(NULL);
  return(m_list[entry]);
}
