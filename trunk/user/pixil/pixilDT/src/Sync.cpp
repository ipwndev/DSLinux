#include <pixil_config.h>

#ifdef CONFIG_SYNC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <FL/Fl.H>

#include <coder.h>
#include <sync/msg_defs.h>
#include <sync/syncerr.h>

#include "AddressBookDB.h"

static int read_fd = 0;
static int write_fd = 0;

static void
SendMsg(char *msg, int size)
{
    char buf[10];

    printf("OUTGOING:  %s\n", msg);

    sprintf(buf, "%d;", size);
    write(write_fd, buf, strlen(buf));
    write(write_fd, msg, size);
    //write(write_fd, "\n", 1); 
}

typedef struct
{
    int table_num;

  void (*save) (vector < string, allocator < string > >);
  int (*get) (int &, int &, vector < string, allocator < string > >&);
  void (*schema) (vector < char >&, vector < int >&);
  void (*update) (void);
}
table_t;

typedef struct
{
    int table_count;
    table_t *tables;
}
database_t;

typedef struct
{
    char name[32];
    database_t *db;
}
db_list_t;

void
AddrSaveData(vector < string, allocator < string > >data)
{

    AddressBookDB *addrDB = AddressBookDB::GetAddressBookDB();

    int flags = atoi(data[1].c_str());
    int key = atoi(data[2].c_str());	/* The record number */

    printf("INCOMING:  flags=%d, key=%d\n", flags, key);

    int nRow = addrDB->FindRecno(key);

    printf("INCOMING:  Found row number %d\n", nRow);

    if (flags & NxDb::NEW && nRow >= 0) {
	printf("CONFLICT:  New record id %d already exists.  Skipping\n",
	       key);
	return;
    }

    if ((flags & (NxDb::NEW | NxDb::CHANGED)) && nRow == -1) {
	nRow = addrDB->Insert();
    }

    if ((flags & (NxDb::NEW | NxDb::CHANGED))) {
	for (unsigned int i = 3; i < data.size(); i += 2) {
	    int col = atoi(data[i].c_str());
	    const char *value = data[i + 1].c_str();

	    printf("INCOMING: Setting [%d] to [%s]\n", col, value);
	    addrDB->SetColumn(nRow, col, value);
	}
    } else if (flags & (NxDb::ERASED | NxDb::DELETED)) {
	addrDB->Delete(nRow);
    }
}

static int cur_row = 0;

int
AddrGetData(int &flags, int &key, vector < string > &data)
{
  vector < string > record;

    AddressBookDB *addrDB = AddressBookDB::GetAddressBookDB();

    data.clear();

    while (1) {
	if (cur_row == addrDB->NumRecs())
	    return 0;		/* All done */
	flags = addrDB->GetFlags(cur_row);

	printf("[%d] FLAGS=%x\n", cur_row, flags);

	if (flags != NxDb::NONE) break;
	cur_row++;
    }

    key = addrDB->GetIntValue(cur_row, 0);

    /* Don't use the address book export here, because it will
       append the notes.  Instead, cast to the NxDbAccess item */
    
    ((NxDbAccess *) addrDB)->Export(cur_row, record);
      
    for(unsigned int i = 0; i < record.size() - 1; i++) {
      char str[16];
      sprintf(str, "%d", i);

      data.push_back(str);
      data.push_back(record[i]);
    }

    /* Should this be done here, or in the update? */
    addrDB->SetFlags(cur_row, NxDb::NONE);
    cur_row++;

    return 1;
}

void
AddrGetSchema(vector < char >&col_type, vector < int >&col_size)
{
    AddressBookDB *addrDB = AddressBookDB::GetAddressBookDB();
    addrDB->GetSchema(col_type, col_size);
}

void 
AddrDoUpdate(void) {
  AddressBookDB *addrDB = AddressBookDB::GetAddressBookDB();

  printf("AddrDoUpdate:  UPDATING\n");

  addrDB->Save();        /* This saves the records */
  addrDB->UpdateFlags(); /* Ensure that the flags are set*/
}

table_t address_tables[] = {
  { 0, AddrSaveData, AddrGetData, AddrGetSchema, AddrDoUpdate }
};

database_t address_db = { 1, address_tables };
    
db_list_t databases[] = {
    {"nxaddress", &address_db},
    {"", 0}
};

bool
ValidMsgId(short int msg_id)
{

    bool ret = false;

    switch (msg_id) {
    case ERR:
    case ABORT:
    case OK:
    case INFO:
    case BP:
    case EP:
    case STATUS:
    case BS:
    case ES:
    case TS:
    case RD:
    case ET:
    case EOT:
    case FLIP:
    case COMMIT:
	ret = true;
	break;
    default:
	ret = false;
	break;
    }

    return ret;
}

static db_list_t *curdb = 0;
static int cur_table = 0;
static MsgCoder *coder = 0;
static int syncState = -1;

void
SyncReset(void)
{
    syncState = -1;
    curdb = 0;
    cur_table = 0;
    cur_row = 0;
}

void
SyncApp(const char *incoming)
{

    int flags, key, ret;
    vector < string > data;

    string msg(incoming);
    string next_msg;

    if (!coder)
	coder = new MsgCoder;

    coder->vmessages.clear();
    coder->DecodeMsg(msg);

    int msg_id = atoi((char *) coder->vmessages[0].c_str());
    if (!ValidMsgId(msg_id))
	return;

    /* FIXME: Error here? */
    if (!curdb && msg_id != BP)
	return;

    switch (msg_id) {
    case ABORT:
	printf("ABORT:  [%s]\n", coder->vmessages[2].c_str());
	SyncReset();
	return;

    case ERR:
	printf("ERR:  [%s]\n", coder->vmessages[2].c_str());
	SyncReset();
	return;

    case BP:
	printf("DEBUG:  BP\n");

	if (coder->vmessages[1].c_str()) {
	    int i;
	    for (i = 0; databases[i].db; i++) {
		if (!strcmp(databases[i].name, coder->vmessages[1].c_str())) {
		    curdb = &databases[i];
		    break;
		}
	    }
	}

	if (!curdb) {
	    syncState = ERR;
	    next_msg = coder->Err(NO_DB_REG);
	} else {
	    syncState = TS;
	    next_msg = coder->Ok();
	}

	break;

    case TS:{			/* Table start */
	    printf("DEBUG:  TS\n");
	    string t_num = coder->vmessages[1];
	    int tableid = atoi(t_num.c_str());
	    if (tableid <= curdb->db->table_count) {

		/* We don't check the schema here - we probably should, but
		   we're assuming all is well */

		cur_table = tableid - 1;

		syncState = INFO;
		next_msg = coder->Info(0);
	    } else {
		syncState = ERR;
		next_msg = coder->Err(EXP_INFO);
	    }
	}
	break;

    case RD:			/* Row data */
	printf("DEBUG:  RD\n");
	curdb->db->tables[cur_table].save(coder->vmessages);

	syncState = RD;
	next_msg = coder->Ok();
	break;

    case EOT:
      printf("DEBUG: EOT\n");
      printf("curdb->db->tables[%d].update = %p\n",
	     cur_table, curdb->db->tables[cur_table].update);

      if (curdb->db->tables[cur_table].update)
	curdb->db->tables[cur_table].update();

      syncState = FLIP;
      next_msg = coder->Ok();
      break;

    case FLIP:{
	    vector < char >col_type;
	    vector < int >col_size;

	    printf("DEBUG:  FLIP\n");
	    cur_table = 0;
	    cur_row = 0;

	    syncState = INFO;
	    curdb->db->tables[cur_table].schema(col_type, col_size);
	    next_msg = coder->TableSchema(cur_table + 1, col_type, col_size);
	}
	break;

    case INFO:
	printf("DEBUG:  INFO\n");
	ret = curdb->db->tables[cur_table].get(flags, key, data);

	if (!ret) {
	    syncState = ET;
	    next_msg = coder->EndTable();
	} else {
	    syncState = RD;
	    next_msg = coder->RowData(flags, key, data);
	}
	break;

    case OK:
	if (syncState == RD) {
	    ret = curdb->db->tables[cur_table].get(flags, key, data);
	    if (!ret) {
		syncState = ET;
		next_msg = coder->EndTable();
	    } else {
		syncState = RD;
		next_msg = coder->RowData(flags, key, data);
	    }
	} else if (syncState == ET) {
	  cur_table++;

	  if (cur_table >= curdb->db->table_count) {
	    syncState = EOT;
	    next_msg = coder->EndOfTables();
	  }
	  else {
	    vector < char >col_type;
	    vector < int >col_size;
	    
	    syncState = INFO;
	    curdb->db->tables[cur_table].schema(col_type, col_size);
	    next_msg = coder->TableSchema(cur_table + 1, col_type, col_size);
	  }
	}
	else if (syncState == EOT) {
	  syncState = COMMIT;
	  next_msg = coder->Flip();
	}

	break;
	
    case ET:
	printf("DEBUG:  ET\n");
	next_msg = coder->Ok();   /* Just send an OK */
	printf("CURTABLE:  %d\n", cur_table);

	break;

#ifdef OLDSCHOOL
	if (cur_table++ > curdb->db->table_count) {
	    syncState = ERR;
	    next_msg = coder->Err(EXP_INFO);
	} else {
	    syncState = INFO;
	    next_msg = coder->Info(0);
	}
	break;
#endif

    case COMMIT:
	printf("DEBUG:  COMMIT\n");
	next_msg = coder->Ok();
	syncState = EP;
	break;

    case EP:
      printf("DEBUG:  EP\n");
      SyncReset();
      return;
    }

    SendMsg((char *) next_msg.c_str(), next_msg.length());
}

void
sync_fd_cb(int fb, void *data)
{
    char len[10];
    char *ptr = len;
    int ret;

    do {
	char ch;
	ret = read(read_fd, &ch, 1);
	if (ret <= 0)
	    return;
	if (ch == ';')
	    break;

	*ptr++ = ch;
    } while (1);

    int size = atoi(len);
    if (size == 0)
	return;

    /* Something bad happened - reset the engine */

    if (size == -1) {
	SyncReset();
	return;
    }

    char *buffer = (char *) calloc(size + 1, 1);
    if (!buffer)
	return;

    ret = read(read_fd, buffer, size);

    if (ret < 0) {
	free(buffer);
	return;
    }

    printf("INCOMING:  %s\n", buffer);

    SyncApp((const char *) buffer);
    free(buffer);
}

static int sync_pid = 0;

void
handle_child(int sig)
{
    int status;
    int pid;

    pid = wait(&status);

    if (pid != sync_pid)
	return;

    printf("Oops - the sync app has bailed on us\n");

    /* Gracefully try to get out of this */

    Fl::remove_fd(read_fd);

    close(write_fd);
    close(read_fd);

    write_fd = 0;
    read_fd = 0;
    sync_pid = 0;
}

void
CloseSync(void)
{
    int pid, status;

    if (!sync_pid) return;

    signal(SIGCHLD,SIG_DFL); /* Reset the signal */

    if (sync_pid > 0)
	kill(sync_pid, SIGTERM);

    printf("Waiting for the sync app to exit\n");
    pid = waitpid(sync_pid, &status, 0);

    sync_pid = 0;
    printf("Sync closed down\n");
}

/* The syncapp should be in the current working directory */
#define APP "dtsyncapp"

int
InitSync(void)
{

    int outfd[2];		/* Parent uses outfd[1] */
    int infd[2];		/* Parent uses infd[0]  */

    signal(SIGCHLD, handle_child);

    /* Set up the pipes */

    if (pipe(infd)) {
	printf("Error - unable to create the incoming pipes [%s]\n",
	       strerror(errno));
	return -1;
    }

    if (pipe(outfd)) {
	printf("Error - unable to create the outgoing pipes [%s]\n",
	       strerror(errno));

	close(infd[0]);
	close(infd[1]);
	return -1;
    }

    if (!(sync_pid = vfork())) {	/* Child */
	dup2(outfd[0], 0);	/* Parent outgoing goes into child stdin */
	dup2(infd[1], 1);	/* Parent incoming comes from child stdout */

	close(outfd[1]);	/* Bail on the unused fds */
	close(infd[0]);

	/* Exec away */
	int ret = execlp(APP, "syncapp", 0);
	fprintf(stderr, "SYNCAGENT - the execl returned [%d] [%s]\n", ret,
		strerror(errno));
	exit(-1);
    } else {
	if (sync_pid == -1) {
	    printf("There was a problem while forking\n");
	    close(outfd[0]);
	    close(outfd[1]);
	    close(infd[0]);
	    close(infd[1]);

	    return -1;
	}
    }

    close(outfd[0]);
    close(infd[1]);

    read_fd = infd[0];
    write_fd = outfd[1];

    /* Make sure it doesn't block on read */
    fcntl(read_fd, F_SETFL, O_NONBLOCK);

    Fl::add_fd(read_fd, sync_fd_cb);
    return 0;
}

#else

int
InitSync(void)
{
    return 0;
}

void
CloseSync(void)
{
}

#endif
