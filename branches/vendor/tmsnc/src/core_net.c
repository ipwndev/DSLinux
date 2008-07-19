
/*
 * TMSNC - Textbased MSN Client Copyright (C) 2004 The IR Developer Group
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the IR Public Domain License as published by the IR Group;
 * either version 1.6 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the IR Public Domain License along with
 * this program; if not, write to sanoix@gmail.com.
 */

#include "core_net.h"
#include "config.h"
#include "core.h"

time_t timestamp;


//added by gfhuang
void MSN_sort_group(session)
	MSN_session *session;
{
	int i, j;
	MSN_group *temp;
	int n;

	n = session->num_groups;
	for(i = 0; i < n; ++i) { 
		for(j = i + 1; j < n; ++j) 
			if(strcmp(session->group[j]->name, session->group[i]->name) < 0) {
				temp = session->group[j];
				session->group[j] = session->group[i];
				session->group[i] = temp;
			}
#ifdef DEBUG
		debug_log("group %d : %s %s\n", i, session->group[i]->name, session->group[i]->guid);
#endif
	}
}
int MSN_group_guid2index(session, guid) 
	MSN_session *session;
	char *guid;
{		//added by gfhuang
	int i;
	for(i = 0; i < session->num_groups; ++i) {
		if(0 != session->group[i]->guid[0] &&
		  strstr(guid, session->group[i]->guid) != NULL) {
			return i;
		}
	}
	return -1;
}
int MSN_addr2index(session, addr)
	MSN_session *session;
	char *addr;
{
	int i;	
        for (i = 0; i < session->num_contacts; i++)
            if (strcmp(session->contact[i]->addr, addr) == 0)
                break;
	return i;
}
//added end


void
nonblock(sd, ret_sd)
     int sd;
     int ret_sd;
{
    fcntl(ret_sd, F_SETFL, O_NONBLOCK);
    fcntl(sd, F_SETFL, O_NONBLOCK);
}

int
MSN_conversation_initiate(session, addr, callback)
     MSN_session *session;
     char *addr;
     void (*callback) (int, void *);
{
    char buf[1024], *ptr[4];
    int xfr_port, ret_sd = -1;

    /*
     * Make sd blocking 
     */
    fcntl(session->sd, F_SETFL, 0);
    lseek(session->sd, 0, SEEK_END);

    if (callback != NULL)
        callback(12, "Requesting the creation of a switchboard");
    snprintf(buf, sizeof(buf) - 1, "XFR %d SB\r\n", ++session->csc);
    if (send(session->sd, buf, strlen(buf), 0) < 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    if (recv(session->sd, &buf, sizeof(buf) - 1, 0) < 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    if (strncmp(buf, "XFR", 3) != 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    /*
     * remove trailing '\r\n' from buf 
     */
    if ((ptr[0] = strstr(buf, "\r\n")) != NULL)
        *ptr[0] = 0x0;

    if ((ptr[0] = (char *)split(buf, ' ', 3)) == NULL ||
        (ptr[1] = (char *)split(ptr[0], ':', 0)) == NULL ||
        (ptr[2] = (char *)split(ptr[0], ':', 1)) == NULL ||
        (ptr[3] = (char *)split(buf, ' ', 5)) == NULL) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    free(ptr[0]);

    if (callback != NULL)
        callback(13, "Connecting to switchboard");
    xfr_port = atoi(ptr[2]);
    free(ptr[2]);

    if ((ret_sd = tcp_connect(ptr[1], xfr_port)) < 0) {
        free(ptr[1]);
        free(ptr[3]);
        nonblock(session->sd, ret_sd);
        return -1;
    }
    free(ptr[1]);

    snprintf(buf, sizeof(buf) - 1, "USR 1 %s %s\r\n", session->me.addr, ptr[3]);
    free(ptr[3]);

    if (send(ret_sd, buf, strlen(buf), 0) < 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    if (recv(ret_sd, &buf, sizeof(buf) - 1, 0) < 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    if (strncmp(buf, "USR", 3) != 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    if (callback != NULL)
        callback(13, "Inviting principal");
    snprintf(buf, sizeof(buf) - 1, "CAL 2 %s\r\n", addr);

    if (send(ret_sd, buf, strlen(buf), 0) < 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    if (recv(ret_sd, &buf, sizeof(buf) - 1, 0) < 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    if (strncmp(buf, "CAL", 3) != 0) {
        nonblock(session->sd, ret_sd);
        return -1;
    }
    /*
     * Make ret_sd and sd non-blocking 
     */
    nonblock(session->sd, ret_sd);
    return ret_sd;
}

int
MSN_conversation_sendmsg(sd, csc, str, session)
     int sd;
     int *csc;
     char *str;
     MSN_session *session;
{
    char buf[2048], format[128], *conv_str;
    int conv_size = strlen(str) * 2;

    snprintf(format, sizeof(format) - 1,
             "X-MMS-IM-Format: FN=%s; EF=%c; CO=%s; CS=0; PF=22",
             session->format.font, session->format.effect,
             session->format.color);

    if ((conv_str = (char *)malloc(conv_size)) == NULL)
        return -1;

#ifdef HAVE_ICONV
    convert_to_utf8(str, strlen(str), conv_str, conv_size);
#else
    strncpy(conv_str, str, strlen(str));
#endif

    *csc++;
    snprintf(buf, sizeof(buf) - 1, "MSG %d N %d\r\n"
             "MIME-Version: 1.0\r\n"
             "Content-Type: text/plain; charset=UTF-8\r\n"
             "%s\r\n"
             "\r\n%s",
             *csc,
             (int)strlen(format) + (int)strlen(conv_str) + 64, format,
             conv_str);

    free(conv_str);

    /*
     * Do not send SIGPIPE on 'connection closed' 
     */

#ifdef MSG_NOSIGNAL
    if (send(sd, buf, strlen(buf), MSG_NOSIGNAL) < 0)
        return -1;
#else
    if (send(sd, buf, strlen(buf), 0) < 0)
        return -1;
#endif

    return 0;
}

int
MSN_conversation_handle(sd, num_ppl, message, message_len, session)
     int sd;
     int *num_ppl;
     char *message;
     int message_len;
     MSN_session *session;		//added by gfhuang
{
    char buf[1024], *ptr[4];
    int i, msg_len, header_len;

#ifdef HAVE_ICONV
    char tr_buf[2048];
#endif

    memset(buf, 0x0, sizeof(buf));
    if (getline(buf, sizeof(buf) - 1, sd) > 0) {
#ifdef DEBUG
        debug_log("sb %d: %s", sd, buf);
#endif
        if (strncmp(buf, "IRO", 3) == 0 || strncmp(buf, "JOI", 3) == 0) {
            if (strncmp(buf, "IRO", 3) == 0)
                i = 4;
            else
                i = 1;

            if ((ptr[0] = (char *)split(buf, ' ', i)) == NULL) {
                strncpy(message, "Couldn't parse IRO or JOI",
                        message_len - 1);
                return -1;
            }
            *num_ppl += 1;
            snprintf(message, message_len, "%s joins the conversation!", ptr[0]);
            free(ptr[0]);

            if (strncmp(buf, "IRO", 3) == 0)
                return 1;
            else
                return 5;
        } else if (strncmp(buf, "MSG", 3) == 0) {
            /*
             * Extract the address of the person and also the length of the
             * MSG
             */
            if ((ptr[0] = (char *)split(buf, ' ', 1)) == NULL ||
                (ptr[1] = (char *)split(buf, ' ', 3)) == NULL) {
                strncpy(message, "Couldn't parse MSG", message_len - 1);
                return -1;
            }
            msg_len = atoi(ptr[1]);
            free(ptr[1]);

            header_len = 0;

            while ((i = getline(buf, sizeof(buf) - 1, sd)) > 0) {
                header_len += i;
#ifdef DEBUG
                debug_log("sb %d: %s", sd, buf);
#endif
                /*
                 * We do not handle anything else than text/plain 
                 */
                if (strncmp(buf, "Content-Type", 12) == 0) {
                    if (strncmp(buf, "Content-Type: application/x-msnmsgrp2p", 38) == 0) {
                        MSN_handle_p2p_msg(sd);
                        snprintf(message, message_len,
                                 "Received file-transfer invitation. Not supported yet.");
                        return -1;
                    } else if (strncmp(buf, "Content-Type: text/plain", 24) != 0) {
                        free(ptr[0]);
                        return 0;
                    }
                }
                /*
                 * Break when we've got \r\n on a line by themselves 
                 */
                if (strncmp(buf, "\r\n", 2) == 0)
                    break;
            }

            /*
             * Now read the actual text 
             */
            msg_len -= header_len;
            if ((ptr[1] = (char *)malloc((msg_len + 1) * sizeof(char))) == NULL) {
                strncpy(message, "Cannot allocate memory", message_len - 1);
                return -1;
            }
            if (read(sd, ptr[1], msg_len) == -1) {
                free(ptr[0]);
                free(ptr[1]);
                return -1;
            }
            ptr[1][msg_len] = 0x0;

            /*
             * Replace \r with space, else they'll fsck up multilines 
             */
            for (i = 0; i < strlen(ptr[1]); i++) {
                if (ptr[1][i] == '\r')
                    ptr[1][i] = ' ';
            }

#ifdef HAVE_ICONV
            strncpy(tr_buf, ptr[1], sizeof(tr_buf) - 1);
            convert_from_utf8(tr_buf, sizeof(tr_buf), ptr[1], msg_len);
#endif

	    //added by gfhuang
	    i = MSN_addr2index(session, ptr[0]);
	    if(i >= session->num_contacts) 
	            snprintf(message, message_len, "%s says:\n     %s", ptr[0], ptr[1]);
	    else
	            snprintf(message, message_len, "%s says:\n     %s", session->contact[i]->name, ptr[1]);
		

            free(ptr[0]);
            free(ptr[1]);
            return 2;
        } else if (strncmp(buf, "BYE", 3) == 0) {
            *num_ppl -= 1;

            if ((ptr[0] = (char *)split(buf, ' ', 1)) == NULL) {
                strncpy(message, "Couldn't parse BYE", message_len - 1);
                return -1;
            }
            if ((strlen(ptr[0]) + 4) != strlen(buf)) {
                snprintf(message, message_len,
                         "%s left the conversation due to inactivity",
                         ptr[0]);
                free(ptr[0]);
                return 3;
            } else {            /* remove the trailing \r\n */
                ptr[0][strlen(ptr[0]) - 2] = 0x0;
                snprintf(message, message_len,
                         "%s left the conversation", ptr[0]);
                free(ptr[0]);
                return 4;
            }
        } else if (strncmp(buf, "NAK", 3) == 0) {
            snprintf(message, message_len, "Couldn't deliver message!");
            return -1;
        } else if (isdigit(buf[0])) {
            buf[3] = 0x0;
            i = atoi(buf);
            MSN_error2str(i, buf, sizeof(buf) - 1);
            snprintf(message, message_len - 1, "Error: %s (code %d)", buf,
                     i);
            return -1;
        }
    }
    if (errno != 0) {           /* The server has disconnected us!! */
        if (errno == ENOTCONN) {
            return -2;
        }
        errno = 0;
    }
    return 0;
}

int
MSN_conversation_call(sd, csc, addr)
     int sd;
     int *csc;
     char *addr;
{
    char buf[512];

    if (MSN_is_valid_address(addr) != 0)
        return -1;

    *csc += 1;
    snprintf(buf, sizeof(buf) - 1, "CAL %d %s\r\n", *csc, addr);
    if (send(sd, buf, strlen(buf), 0) < 0)
        return -1;

    return 0;
}

int
MSN_list2int(list)
     char *list;
{
    int ret;

    if (strcmp(list, "FL") == 0)
        ret = 1;
    else if (strcmp(list, "AL") == 0)
        ret = 2;
    else if (strcmp(list, "BL") == 0)
        ret = 4;
    else if (strcmp(list, "RL") == 0)
        ret = 8;
    else if (strcmp(list, "PL") == 0)
        ret = 16;
    else
        ret = 0;

    return ret;
}

int
MSN_block_contact(addr, session, errbuf, err_len)
     char *addr;
     MSN_session *session;
     char *errbuf;
     int err_len;
{
    int i;
    char buf[256];

    if (strcasecmp(addr, session->me.addr) == 0) {
        snprintf(errbuf, err_len, "you cannot block yourself");
        return -1;
    } else if (MSN_is_valid_address(addr) != 0) {
        snprintf(errbuf, err_len, "'%s' is not a valid address", addr);
        return -1;
    } else {
        /*
         * See if the address is on the contact list 
         */
        for (i = 0; i < session->num_contacts; i++)
            if (strcmp(session->contact[i]->addr, addr) == 0)
                break;

        /*
         * If not, exit 
         */
        if (i == session->num_contacts) {
            snprintf(errbuf, err_len, "'%s' is not on your contact-list",
                     addr);
            return -1;
        }
        /*
         * If already on BL, exit 
         */
        if ((BL & session->contact[i]->listnum) > 0) {
            snprintf(errbuf, err_len, "'%s' is already blocked", addr);
            return -1;
        }
        /*
         * If on AL, then remove 
         */
        if ((AL & session->contact[i]->listnum) > 0) {
            snprintf(buf, sizeof(buf) - 1, "REM %d AL %s\r\n", session->csc++,
                     addr);
            send(session->sd, buf, strlen(buf), 0);
        }
        /*
         * Add to BL 
         */
        snprintf(buf, sizeof(buf) - 1, "ADC %d BL N=%s\r\n", session->csc++,
                 addr);
        send(session->sd, buf, strlen(buf), 0);
    }
    return 0;
}

int
MSN_unblock_contact(addr, session, errbuf, err_len)
     char *addr;
     MSN_session *session;
     char *errbuf;
     int err_len;
{
    int i;
    char buf[256];

    if (strcasecmp(addr, session->me.addr) == 0) {
        snprintf(errbuf, err_len, "you cannot unblock yourself");
        return -1;
    } else if (MSN_is_valid_address(addr) != 0) {
        snprintf(errbuf, err_len, "'%s' is not a valid address", addr);
        return -1;
    } else {
        /*
         * See if the address is on the contact list 
         */
        for (i = 0; i < session->num_contacts; i++)
            if (strcmp(session->contact[i]->addr, addr) == 0)
                break;

        /*
         * If not, exit 
         */
        if (i == session->num_contacts) {
            snprintf(errbuf, err_len, "'%s' is not on your contact-list",
                     addr);
            return -1;
        }
        /*
         * If already on AL, exit 
         */
        if ((AL & session->contact[i]->listnum) > 0) {
            snprintf(errbuf, err_len, "'%s' is not blocked", addr);
            return -1;
        }
        /*
         * If on BL, then remove 
         */
        if ((BL & session->contact[i]->listnum) > 0) {
            snprintf(buf, sizeof(buf) - 1, "REM %d BL %s\r\n", session->csc++,
                     addr);
            send(session->sd, buf, strlen(buf), 0);
        }
        /*
         * Add to AL 
         */
        snprintf(buf, sizeof(buf) - 1, "ADC %d AL N=%s\r\n", session->csc++,
                 addr);
        send(session->sd, buf, strlen(buf), 0);
    }
    return 0;
}

int
MSN_add_contact(addr, session, allow_or_block, errbuf, err_len)
     char *addr;
     MSN_session *session;
     char allow_or_block;
     char *errbuf;
     int err_len;
{
    int i;
    char buf[256];

    if (strcasecmp(addr, session->me.addr) == 0) {
        snprintf(errbuf, err_len, "you cannot add yourself");
        return -1;
    } else if (MSN_is_valid_address(addr) != 0) {
        snprintf(errbuf, err_len, "'%s' is not a valid address", addr);
        return -1;
    } else {
        /*
         * See if the address is on the contact list already 
         */
        for (i = 0; i < session->num_contacts; i++)
            if (strcmp(session->contact[i]->addr, addr) == 0)
                break;

        /*
         * Nope it wasn't, let's add it 
         */
        if (i == session->num_contacts) {
            snprintf(buf, sizeof(buf) - 1,
                     "ADC %d FL N=%s F=%s\r\n", session->csc++, addr, addr);
            send(session->sd, buf, strlen(buf), 0);

            snprintf(buf, sizeof(buf) - 1, "ADC %d AL N=%s\r\n",
                     session->csc++, addr);
            send(session->sd, buf, strlen(buf), 0);
        } else {
            /*
             * If on PL, then remove 
             */
            if ((PL & session->contact[i]->listnum) > 0) {
                snprintf(buf, sizeof(buf) - 1, "REM %d PL %s\r\n",
                         session->csc++, addr);
                send(session->sd, buf, strlen(buf), 0);
            }

            /*
             * If not on FL, then add 
             */
            if ((FL & session->contact[i]->listnum) == 0) {
                snprintf(buf, sizeof(buf) - 1,
                         "ADC %d FL N=%s F=%s\r\n", session->csc++, addr, addr);
                send(session->sd, buf, strlen(buf), 0);
            }
            /*
             * If not on RL, then add 
             */
            if ((RL & session->contact[i]->listnum) == 0) {
                snprintf(buf, sizeof(buf) - 1,
                         "ADC %d RL N=%s\r\n", session->csc++, addr);
                send(session->sd, buf, strlen(buf), 0);
            }

            /*
             *  Add to AL or BL
             */
            if (allow_or_block == 'b') {
                if ((BL & session->contact[i]->listnum) == 0) {
                    snprintf(buf, sizeof(buf) - 1, "ADC %d BL N=%s\r\n",
                             session->csc++, addr);
                    send(session->sd, buf, strlen(buf), 0);
                }
            } else {
                if ((AL & session->contact[i]->listnum) == 0) {
                    snprintf(buf, sizeof(buf) - 1, "ADC %d AL N=%s\r\n",
                             session->csc++, addr);
                    send(session->sd, buf, strlen(buf), 0);
                }
            }
        }
    }
    return 0;
}

int
MSN_remove_contact(co, session, errbuf, err_len)
     MSN_contact *co;
     MSN_session *session;
     char *errbuf;
     int err_len;
{
    char buf[256];
    
    if ((AL & co->listnum) > 0) {
        snprintf(buf, sizeof(buf) - 1, "REM %d AL %s\r\n", session->csc++,
                 co->addr);
        send(session->sd, buf, strlen(buf), 0);
    }
    if ((BL & co->listnum) > 0) {
        snprintf(buf, sizeof(buf) - 1, "REM %d BL %s\r\n", session->csc++,
                 co->addr);
        send(session->sd, buf, strlen(buf), 0);
    }
    if ((PL & co->listnum) > 0) {
        snprintf(buf, sizeof(buf) - 1, "REM %d PL %s\r\n", session->csc++,
                 co->addr);
        send(session->sd, buf, strlen(buf), 0);
    }
    if ((FL & co->listnum) > 0) {
        snprintf(buf, sizeof(buf) - 1, "REM %d FL %s\r\n", session->csc++,
                 co->guid);
        send(session->sd, buf, strlen(buf), 0);
    }
    return 0;
}

void
MSN_change_nick(nick, session)
     char *nick;
     MSN_session *session;
{
    char buf[256], r_buf[256];

#ifdef HAVE_ICONV
    convert_to_utf8(nick, strlen(nick), buf, sizeof(buf));
#else
    strncpy(buf, nick, sizeof(buf) - 1);
#endif

    MSN_url_encode(buf, r_buf, sizeof(r_buf) - 1);

    snprintf(buf, sizeof(buf) - 1, "PRP %d MFN %s\r\n", session->csc++, r_buf);
    send(session->sd, buf, strlen(buf), 0);
}

void
MSN_send_uux(session)
     MSN_session *session;
{
    char buf[512];
    
    snprintf(buf, sizeof(buf) - 1, "UUX %d %d\r\n"
                                   "<Data><PSM>%s</PSM><CurrentMedia>%s</CurrentMedia></Data>",
                                   session->csc++, (int)strlen(session->me.psm) +
                                   (int)strlen(session->me.cmedia) + 53,
                                   session->me.psm, session->me.cmedia);
    send(session->sd, buf, strlen(buf), 0);
}

int MSN_set_current_media(str, session)
     char *str;
     MSN_session *session;
{
    char buf[512];

    MSN_xml_encode(str, buf, sizeof(buf) - 1);

#ifdef HAVE_ICONV
    convert_to_utf8(buf, strlen(buf), session->me.cmedia, CMEDIA_LEN);
#else
    strncpy(session->me.cmedia, buf, CMEDIA_LEN - 1);
#endif

    MSN_send_uux(session);
    return 0;
}

int MSN_set_personal_message(str, session)
     char *str;
     MSN_session *session;
{
    char buf[512];

    MSN_xml_encode(str, buf, sizeof(buf) - 1);

#ifdef HAVE_ICONV
    convert_to_utf8(buf, strlen(buf), session->me.psm, PSM_LEN);
#else
    strncpy(session->me.psm, buf, PSM_LEN - 1);
#endif

    MSN_send_uux(session);
    return 0;
}

int
MSN_server_handle(session, message, message_len)
     MSN_session *session;
     char *message;
     int message_len;
{
    time_t tm;
    char buf[512], md_hex[48];
    char *ptr[6];
    int i, j, ret_sd;

#ifdef HAVE_ICONV
    char tr_buf[1024];
#endif

    time(&tm);
    if ((long)tm > (long)timestamp + 30) {
        if (send(session->sd, "PNG\r\n", 5, 0) == -1)
            return -1;
        timestamp = tm;
    }
    while (getline(buf, sizeof(buf) - 1, session->sd) > 0) {
#ifdef HAVE_ICONV
        strncpy(tr_buf, buf, sizeof(tr_buf) - 1);
        convert_from_utf8(tr_buf, sizeof(tr_buf), buf, sizeof(buf));
#endif


        if ((ptr[0] = strstr(buf, "\r\n")) != NULL)
            *ptr[0] = '\0';

#ifdef DEBUG
        debug_log("ns: %s\n", buf);
#endif

        if (strncmp(buf, "CHL", 3) == 0) {
            MSN_handle_chl(&buf[6], md_hex);
            snprintf(buf, sizeof(buf) - 1,
                     "QRY %d %s %d\r\n%s",
                     ++session->csc, PRODUCT_ID, (int)strlen(md_hex), md_hex);

            if (send(session->sd, buf, strlen(buf), 0) == -1)
                return -1;
            strncpy(message,"Received and answered challenge (M$ can't control me!! :P)",
                    message_len - 1);
            return 1;
        } else if (strncmp(buf, "ILN", 3) == 0) {
            if ((ptr[0] = (char *)split(buf, ' ', 2)) == NULL ||
                ((ptr[1] = (char *)split(buf, ' ', 3)) == NULL ||
                 ((ptr[2] = (char *)split(buf, ' ', 4)) == NULL))) {
                strncpy(message, "Couldn't parse ILN", message_len - 1);
                return -1;
            }
            for (i = 0; i < session->num_contacts; i++)
                if (strcmp(session->contact[i]->addr, ptr[1]) == 0)
                    break;

            MSN_url_decode(ptr[2], session->contact[i]->name, NAME_LEN - 1);

            session->contact[i]->status = MSN_status2int(ptr[0]);
            snprintf(message, message_len - 1,
                     "%s (%s) is %s",
                     session->contact[i]->name,
                     ptr[1], MSN_status2str(session->contact[i]->status));
            for (i = 0; i < 3; i++)
                free(ptr[i]);
            return 2;
        }
        if (strncmp(buf, "NLN", 3) == 0) {
            if ((ptr[0] = (char *)split(buf, ' ', 1)) == NULL ||
                ((ptr[1] = (char *)split(buf, ' ', 2)) == NULL ||
                 ((ptr[2] = (char *)split(buf, ' ', 3)) == NULL))) {
                strncpy(message, "Couldn't parse NLN", message_len - 1);
                return -1;
            }
            for (i = 0; i < session->num_contacts; i++)
                if (strcmp(session->contact[i]->addr, ptr[1]) == 0)
                    break;

            MSN_url_decode(ptr[2], session->contact[i]->name, NAME_LEN - 1);

            /*
             * someone just logged in, set the return code 
             */
            if (session->contact[i]->status == 7 && MSN_status2int(ptr[0]) != 7)
                j = 14;
            else
                j = 13;

            session->contact[i]->status = MSN_status2int(ptr[0]);
	    if(0 == session->contact[i]->psm[0]) 
                snprintf(message, message_len - 1,
                     "%s (%s) is %s",
                     session->contact[i]->name,
                     ptr[1], 
		     MSN_status2str(session->contact[i]->status));
	    else //added by gfhuang
                snprintf(message, message_len - 1,
                     "%s (%s) {%s} is %s",
                     session->contact[i]->name,
                     ptr[1], 
		     session->contact[i]->psm,
		     MSN_status2str(session->contact[i]->status));
            for (i = 0; i < 3; i++)
                free(ptr[i]);
            return j;           /* 13 or 14 */
        } else if (strncmp(buf, "UBX", 3) == 0) {
            /*
             * we read the payload of this command 
             */
            /*
             * but do not do anything with it      
             */
            if ((ptr[1] = (char *)split(buf, ' ', 1)) == NULL ||	//by gfhuang
		(ptr[0] = (char *)split(buf, ' ', 2)) == NULL) {
                strncpy(message, "Couldn't parse UBX", message_len - 1);
                return -1;
            }
            i = atoi(ptr[0]);
            free(ptr[0]);

	    if (read(session->sd, buf, i) != i) {
                strncpy(message, "Couldn't read UBX payload",
                        message_len - 1);
                return -1;
            }
	    // parsing PSM, by gfhuang
	    if(0 == i) buf[0] = 0;	//important, by gfhuang, when i=0, buf is untouched!

	    i = MSN_addr2index(session, ptr[1]);
	    free(ptr[1]);
	    if(i >= session->num_contacts) {
                strncpy(message, "Couldn't find UBX contact in your list",
                        message_len - 1);
		return -1;
	    }
	    session->contact[i]->psm[0] = 0;
	    if(0 == buf[0]) return 0; 

	    if((ptr[0] = strstr(buf, "<PSM>")) != NULL && NULL != (ptr[1] = strstr(ptr[0], "</PSM>"))) {
		ptr[0] += 5;
		*ptr[1] = 0;
	    }
	    else {
                strncpy(message, "error in parsing PSM",
                        message_len - 1);
#ifdef DEBUG
		debug_log("unparsed psm: %s\n", buf);
#endif
		return -1;
	    }
	    if(0 != *ptr[0]) {	//convert_from_utf8 will fail when len==0 ????
#ifdef HAVE_ICONV
	    	    convert_from_utf8(ptr[0], strlen(ptr[0]), session->contact[i]->psm, PSM_LEN - 1);
#else
		    strncpy(session->contact[i]->psm, ptr[0], PSM_LEN - 1);
#endif
            	    snprintf(message, message_len-1, "%s's psm : %s", session->contact[i]->addr,  
    	    						session->contact[i]->psm);
		    return -1;   // has message
	    }
//	    else {
//		    strncpy(session->contact[i]->psm, "<empty>", PSM_LEN - 1);
//	    }
            return 0;
        } else if (strncmp(buf, "QNG", 3) == 0)
            return 0;
        else if (strncmp(buf, "FLN", 3) == 0) {
            for (i = 0; i < session->num_contacts; i++)
                if (strcmp(session->contact[i]->addr, &buf[4]) == 0)
                    break;
            session->contact[i]->status = 7;
            snprintf(message, message_len - 1, "%s logged out :(", &buf[4]);
            return 6;
        } else if (strncmp(buf, "RNG", 3) == 0) {
            if ((ptr[0] = (char *)split(buf, ' ', 1)) == NULL ||
                (ptr[1] = (char *)split(buf, ' ', 2)) == NULL ||
                (ptr[2] = (char *)split(buf, ' ', 4)) == NULL ||
                (ptr[3] = (char *)split(buf, ' ', 5)) == NULL ||
                (ptr[4] = (char *)split(ptr[1], ':', 0)) == NULL ||
                (ptr[5] = (char *)split(ptr[1], ':', 1)) == NULL) {
                strncpy(message, "Couldn't parse RNG", message_len - 1);
                return -1;
            }
            ret_sd = tcp_connect(ptr[4], atoi(ptr[5]));
            if (ret_sd < 0) {
                strncpy(message,
                        "Couldn't join chat session (probably timeout)",
                        message_len - 1);
                for (i = 0; i < 6; i++)
                    free(ptr[i]);
                return -1;
            }
            snprintf(buf, sizeof(buf) - 1,
                     "ANS 1 %s %s %s\r\n", session->me.addr, ptr[2], ptr[0]);
            send(ret_sd, buf, strlen(buf), 0);

            /*
             * Make ret_sd non-blocking 
             */
            fcntl(ret_sd, F_SETFL, O_NONBLOCK);

            /*
             * Return the open socket descriptor 
             */
            snprintf(message, message_len - 1, "%d %s", ret_sd, ptr[3]);
            for (i = 0; i < 6; i++)
                free(ptr[i]);
            return 8;
        } else if (strncmp(buf, "PRP", 3) == 0) {
            if ((ptr[0] = (char *)split(buf, ' ', 2)) == NULL ||
                (ptr[1] = (char *)split(buf, ' ', 3)) == NULL) {
                strncpy(message, "Couldn't parse PRP", message_len - 1);
                return -1;
            }
            if (strcmp("MFN", ptr[0]) != 0) {
                for (i = 0; i < 2; i++)
                    free(ptr[i]);
                return 0;
            }
            MSN_url_decode(ptr[1], session->me.name, NAME_LEN - 1);
            snprintf(message, message_len - 1,
                     "Changed nick to '%s'", session->me.name);
            for (i = 0; i < 2; i++)
                free(ptr[i]);
            return 9;
        } else if (strncmp(buf, "ADC", 3) == 0) {
            /*
             * ptr[0] contains FL AL RL or BL, ptr[1] contains principal
             * address prefixed with 'N='
             */
            if ((ptr[0] = split(buf, ' ', 2)) == NULL ||
                (ptr[1] = split(buf, ' ', 3)) == NULL) {
                strncpy(message, "Couldn't parse ADC", message_len - 1);
                return -1;
            }
            /*
             * ptr[2] will contain the actual address 
             */
            ptr[2] = &ptr[1][2];

            for (i = 0; i < session->num_contacts; i++)
                if (strcmp(session->contact[i]->addr, ptr[2]) == 0)
                    break;

            if (i == session->num_contacts) {   /* We have a new contact */
                i = session->num_contacts++;
                MSN_resize_contact_array(session);
                session->contact[i] = MSN_allocate_contact();

                strncpy(session->contact[i]->addr, ptr[2], ADDR_LEN - 1);
                strncpy(session->contact[i]->name, ptr[2], NAME_LEN - 1);
                session->contact[i]->status = 7;        /* Offline */
                session->contact[i]->listnum = MSN_list2int(ptr[0]);

                if ((ptr[3] = split(buf, ' ', 5)) != NULL) {
                    strncpy(session->contact[i]->guid, &ptr[3][2],
                            GUID_LEN - 1);
                    free(ptr[3]);
                }
            } else {
                /*
                 * The principal is on contact list already 
                 */
                session->contact[i]->listnum += MSN_list2int(ptr[0]);
            }
            snprintf(message, message_len - 1,
                     "Added '%s' to the %s list", ptr[2], ptr[0]);
            for (i = 0; i < 2; i++)
                free(ptr[i]);
            return 11;
        } else if (strncmp(buf, "REM", 3) == 0) {
            /*
             * ptr[0] contains FL AL RL or BL, ptr[1] contains principal
             * address
             */
            if ((ptr[0] = (char *)split(buf, ' ', 2)) == NULL ||
                (ptr[1] = (char *)split(buf, ' ', 3)) == NULL) {
                strncpy(message, "Couldn't parse REM", message_len - 1);
                return -1;
            }
            /*
             * Principal was removed from FL 
             */
            if (strcmp(ptr[0], "FL") == 0) {
                for (i = 0; i < session->num_contacts; i++)
                    if (strcmp(session->contact[i]->guid, ptr[1]) == 0)
                        break;

                /*
                 * Shift all contacts one step 
                 */
                MSN_free_contact(session->contact[i]);
                for (j = i;
                     j < session->num_contacts && session->num_contacts > 1;
                     j++) {
                    session->contact[j] = session->contact[j + 1];
                }
                session->num_contacts--;
                MSN_resize_contact_array(session);
            } else {
                for (i = 0; i < session->num_contacts; i++)
                    if (strcmp(session->contact[i]->addr, ptr[1]) == 0)
                        break;
            }
            /*
             * Subtract the list removed from 
             */
            session->contact[i]->listnum -= MSN_list2int(ptr[0]);

            snprintf(message, message_len - 1,
                     "Removed '%s' from the %s list", ptr[1], ptr[0]);

            for (i = 0; i < 2; i++)
                free(ptr[i]);
            return 12;
        } else if (strncmp(buf, "OUT OTH", 7) == 0) {
            snprintf(message, message_len - 1,
                     "Logged in from other location");
            return -2;
        } else if (isdigit(buf[0])) {
            /*
             * An error has occurred 
             */
            buf[3] = 0x0;
            i = atoi(buf);
            MSN_error2str(i, buf, sizeof(buf) - 1);
            snprintf(message, message_len - 1, "Error: %s (code %d)", buf,
                     i);
            return -1;
        }
    }

    if (errno != 0) {           /* Error */
        if (errno != EAGAIN) {
            snprintf(message, message_len - 1, "%s", strerror(errno));
            return -2;
        }
        errno = 0;
    }
    return 0;
}

char *
MSN_status2str(status)
     int status;
{
    if (status == 1)
        return "busy";
    else if (status == 2)
        return "idle";
    else if (status == 3)
        return "away";
    else if (status == 4)
        return "on phone";
    else if (status == 5)
        return "on lunch";
    else if (status == 6)
        return "\"brb\"";
    else if (status == 7)
        return "offline";
    else
        return "online";
}

int
MSN_status2int(status)
     char *status;
{
    int ret;

    if (strncmp(status, "BSY", 3) == 0)
        ret = 1;
    else if (strncmp(status, "IDL", 3) == 0)
        ret = 2;
    else if (strncmp(status, "AWY", 3) == 0)
        ret = 3;
    else if (strncmp(status, "PHN", 3) == 0)
        ret = 4;
    else if (strncmp(status, "LUN", 3) == 0)
        ret = 5;
    else if (strncmp(status, "BRB", 3) == 0)
        ret = 6;
    else if (strncmp(status, "HDN", 3) == 0)
        ret = 7;
    else
        ret = 0;

    return ret;
}

int
wait_for_input(int fd, unsigned int seconds)
{
    fd_set set;
    struct timeval timeout;

    /*
     * Initialize the file descriptor set. 
     */
    FD_ZERO(&set);
    FD_SET(fd, &set);

    /*
     * Initialize the timeout data structure. 
     */
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    /*
     * select returns 0 if timeout, 1 if input available, -1 if error. 
     */
    return select(FD_SETSIZE, &set, NULL, NULL, &timeout);
}



int
MSN_load_userlist(session, cb)
     MSN_session *session;
     void (*cb) (int, void *);
{
    char buf[512], tr_buf[256];
    char *ptr;
    int i, a, curGroup;

    /*
     * clear the buffer 
     */
    while (wait_for_input(session->sd, 1) > 0)
        recv(session->sd, buf, sizeof(buf) - 1, 0);

    snprintf(buf, sizeof(buf) - 1, "SYN %d 0 0\r\n", session->csc++);

    if (send(session->sd, buf, strlen(buf), 0) == 0)
        return -1;

    if (getline(buf, sizeof(buf) - 1, session->sd) < 0)
        return -1;

    if ((ptr = split(buf, ' ', 4)) == NULL)
        return -1;

    session->num_contacts = atoi(ptr);
    free(ptr);

    // groups, added by gfhuang
    if ((ptr = split(buf, ' ', 5)) == NULL)
        return -1;

    session->num_groups = atoi(ptr);
    free(ptr);
    MSN_resize_group_array(session);
    for(i = 0; i <= session->num_groups; ++i)
	session->group[i] = MSN_allocate_group();
    curGroup = 0;
    //added end

    /*
     * allocate space for the contacts 
     */
    MSN_resize_contact_array(session);
    for (i = 0; i <= session->num_contacts; i++)
        session->contact[i] = MSN_allocate_contact();

    i = 0;
    while (i < session->num_contacts) {
        if (getline(buf, sizeof(buf) - 1, session->sd) < 0)
            return -1;

        /*
         * remove trailing '\r\n' from buf 
         */
        if ((ptr = strstr(buf, "\r\n")) != NULL)
            *ptr = 0x0;

#ifdef DEBUG
	debug_log("ld : %s\n", buf);
#endif

        if (strncmp(buf, "GTC", 3) == 0) {
            /*
             * Store the char 'A' or 'B' in gtc 
             */
            session->gtc = buf[strlen(buf)];
        } else if (strncmp(buf, "BLP", 3) == 0) {
            /*
             * Store the char 'A' or 'B' in blp 
             */
            session->blp = buf[strlen(buf) - 3];
        } else if (strncmp(buf, "PRP MFN", 7) == 0) {
            MSN_url_decode(&buf[8], session->me.name, NAME_LEN - 1);
            #ifdef HAVE_ICONV
                strncpy(tr_buf, session->me.name, sizeof(tr_buf)-1);
                convert_from_utf8(tr_buf, sizeof(tr_buf) - 1,
                                  session->me.name, NAME_LEN - 1);
            #endif
        } else if (strncmp(buf, "LSG", 3) == 0) {		//group, added by gfhuang
		ptr = split(buf, ' ', 1);
		if(NULL != ptr) {
			strncpy(session->group[curGroup]->name, ptr, NAME_LEN - 1);
			free(ptr);

			ptr = split(buf, ' ', 2);
			if(NULL != ptr) {
				strncpy(session->group[curGroup]->guid, ptr, GUID_LEN - 1);
				++curGroup;
				free(ptr);
			}
		}

	} else if (strncmp(buf, "LST", 3) == 0) {
	    if(0 == i) MSN_sort_group(session);	//added by gfhuang, first time, sort the group

            for (a = 1; (ptr = split(buf, ' ', a)) != NULL; a++) {
                if (strncmp(ptr, "N=", 2) == 0)
                    strncpy(session->contact[i]->addr, &ptr[2], ADDR_LEN - 1);
                else if (strncmp(ptr, "F=", 2) == 0) {
                    MSN_url_decode(&ptr[2], session->contact[i]->name,
                                   NAME_LEN - 1);
                    #ifdef HAVE_ICONV
                        strncpy(tr_buf, session->contact[i]->name, sizeof(tr_buf)-1);
                        convert_from_utf8(tr_buf, sizeof(tr_buf) - 1,
                                          session->contact[i]->name, NAME_LEN - 1);
                    #endif
                } else if (strncmp(ptr, "C=", 2) == 0)
                    strncpy(session->contact[i]->guid, &ptr[2], GUID_LEN - 1);
                else if (isdigit(ptr[0]) && NULL == strstr(ptr, "-")) //thanks poppyer
                    session->contact[i]->listnum = atoi(ptr);
		else		//group_guids, added by gfhuang 
		    session->contact[i]->ingroup = MSN_group_guid2index(session, ptr);

                free(ptr);
            }

            session->contact[i]->status = 7;
            //Initially offline

            i++;
        }
        snprintf(buf, sizeof(buf) - 1,
                 "Downloading contact-list (%d/%d)", i,
                 session->num_contacts);
        cb(12, buf);
    }

    return 0;
}

void
MSN_logout(sd)
     int sd;
{
    send(sd, "OUT\r\n", 5, 0);
}

int
MSN_set_status(session, state)
     MSN_session *session;
     char *state;
{
    char buf[64], status[4];

    if (strcasecmp(state, "online") == 0)
        strcpy(status, "NLN");
    else if (strcasecmp(state, "busy") == 0)
        strcpy(status, "BSY");
    else if (strcasecmp(state, "idle") == 0)
        strcpy(status, "IDL");
    else if (strcasecmp(state, "brb") == 0)
        strcpy(status, "BRB");
    else if (strcasecmp(state, "away") == 0)
        strcpy(status, "AWY");
    else if (strcasecmp(state, "phone") == 0)
        strcpy(status, "PHN");
    else if (strcasecmp(state, "lunch") == 0)
        strcpy(status, "LUN");
    else if (strcasecmp(state, "hidden") == 0)
        strcpy(status, "HDN");
    else
        return -1;

    snprintf(buf, sizeof(buf) - 1, "CHG %d %s 0\r\n", session->csc++, status);

    if (send(session->sd, buf, strlen(buf), 0) < 0)
        return -1;

    session->me.status = MSN_status2int(status);

    return 0;
}

int
tcp_connect(server, port)
     char *server;
     int port;
{
    struct sockaddr_in localAddr, servAddr;     /* Used in connect and bind */
    struct hostent *h;          /* Used for address resolving */
    int rc;                     /* Function return code */
    int sd;                     /* The socket descriptor */

    /*
     * Resolve the address 
     */
    h = gethostbyname(server);
    if (h == NULL)
        return -1;

    servAddr.sin_family = h->h_addrtype;
    memcpy((char *)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr.sin_port = htons(port);

    /*
     * Initialize the socket 
     */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
        return -1;

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    /*
     * Bind a local port for receiving data 
     */
    rc = bind(sd, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (rc < 0)
        return -1;

    /*
     * Connect to the remote server 
     */
    rc = connect(sd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (rc < 0)
        return -1;

    return sd;
}

/*
 * MSN Init Session Function
 *
 * server      = The address which we first connect to (messenger.hotmail.com)
 * port        = The port we first connect to session     = The MSN_session
 * structure msn_passwd  = The password for the passport-account
 */
int
MSN_init_session(server, port, session, msn_passwd, callback)
     char *server;
     int port;
     MSN_session *session;
     char *msn_passwd;
     void (*callback) (int, void *);
{
    char buf[BUFSIZE];          /* The buffer used in recv() and send() */
    int sd;                     /* Socket descriptor */
    char *ptr[2];
    char ticket[512];
    ptr[1] = (char *)malloc(sizeof(char) * (strlen(server) + 1));
    strncpy(ptr[1], server, strlen(server) + 1);

  msn_reconnect:

    /*** Connect to server and check for errors ***/
    if (callback != NULL)
        callback(1, "Connecting to dispatch server");
    sd = tcp_connect(ptr[1], port);
    if (sd < 0) {
        if (callback != NULL)
            callback(-1, "Cannot connect to dispatch server");
        return -1;
    }
    free(ptr[1]);

    /*** Send our version string and check for errors ***/
    if (callback != NULL)
        callback(2, "Sending version string");
    if (send(sd, PROTOCOL_VER, sizeof(PROTOCOL_VER) - 1, 0) < 0)
        if (sd < 0) {
            if (callback != NULL)
                callback(-1, "Error when sending version string");
            return -1;
        }

    /*** Receive the echo and check if it matches what was sent ***/
    if (callback != NULL)
        callback(3, "Receiving echo from dispatch server");
    memset(buf, 0x0, BUFSIZE);
    if (recv(sd, &buf, BUFSIZE - 1, 0) < 0) {
        if (callback != NULL)
            callback(-1, "Error when receiving echo");
        return -1;
    }
    if (strncmp(buf, PROTOCOL_VER, strlen(PROTOCOL_VER)) != 0) {
        if (callback != NULL)
            callback(-1, "The protocols doesn't match\n");
        return -1;
    }

    /*** Send information about the client ***/
    if (callback != NULL)
        callback(4, "Sending client info");
    snprintf(buf, BUFSIZE - 1, "%s %s\r\n", CLIENT_INFO, session->me.addr);
    if (send(sd, buf, strlen(buf), 0) < 0) {
        if (callback != NULL)
            callback(-1, "Error when sending client info");
        return -1;
    }

    /*** Receive the reply from the server ***/
    if (callback != NULL)
        callback(5, "Receiving client info reply");
    memset(buf, 0x0, BUFSIZE);
    if (recv(sd, &buf, BUFSIZE - 1, 0) < 0) {
        if (callback != NULL)
            callback(-1, "Error when receiving client info reply");
        return -1;
    }
    if (strncmp(buf, "CVR", 3) != 0) {
        if (callback != NULL)
            callback(-1, "The server did not accept our client info string");
        return -1;
    }

    /*** Send initial USR ***/
    if (callback != NULL)
        callback(6, "Sending user information");
    snprintf(buf, BUFSIZE, "USR 3 TWN I %s\r\n", session->me.addr);
    if (send(sd, buf, strlen(buf), 0) < 0) {
        if (callback != NULL)
            callback(-1, "Error when sending user information");
        return -1;
    }

    /*** Receive the reply check wether we are transferred or not  ***/
    if (recv(sd, &buf, BUFSIZE - 1, 0) < 0) {
        if (callback != NULL)
            callback(-1, "Error receiving reply");
        return -1;
    }
    if (strncmp(buf, "XFR", 3) == 0) {
        if (callback != NULL)
            callback(7, "Connection was redirected to another server");
        if ((ptr[0] = (char *)split(buf, ' ', 3)) == NULL) {
            if (callback != NULL)
                callback(-1, "Couldn't extract address from string");
            return -1;
        }
        if ((ptr[1] = (char *)split(ptr[0], ':', 1)) == NULL) {
            if (callback != NULL)
                callback(-1, "Couldn't extract port from address");
            return -1;
        }
        port = atoi(ptr[1]);
        free(ptr[1]);

        if ((ptr[1] = (char *)split(ptr[0], ':', 0)) == NULL) {
            if (callback != NULL)
                callback(-1, "Couldn't extract IP from address");
            return -1;
        }
        free(ptr[0]);

        close(sd);
        goto msn_reconnect;
    }

    /*** Get the challenge string from the USR reply ***/
    ptr[0] = (char *)split(buf, ' ', 4);
    if (ptr[0] == NULL) {
        if (callback != NULL)
            callback(-1, "Not a valid address!");
        return -1;
    }
    ptr[0][strlen(ptr[0]) - 2] = '\0';

    /*** Use https to obtain the passport key ***/
    if (callback != NULL)
        callback(8, "Authenticating using SSL with Nexus");
    if (https_auth("loginnet.passport.com",
                   443,
                   "/login2.srf?lc=1033",
                   session->me.addr,
                   msn_passwd, ptr[0], ticket, sizeof(ticket), callback) < 0) {
        free(ptr[0]);
        return -1;
    }
    free(ptr[0]);

    /*** Send final USR ***/
    if (callback != NULL)
        callback(9, "Sending authentication ticket");
    snprintf(buf, BUFSIZE, "USR 4 TWN S %s\r\n", ticket);
    if (send(sd, buf, strlen(buf), 0) < 0) {
        if (callback != NULL)
            callback(-1, "Couldn't send final USR request");
        return -1;
    }

    /*** Receive the reply from the server ***/
    if (callback != NULL)
        callback(10, "Receiving authentication ticket reply");
    memset(buf, 0x0, BUFSIZE);
    if (recv(sd, buf, BUFSIZE - 1, 0) < 0) {
        if (callback != NULL)
            callback(-1, "Couldn't receive server's reply");
        return -1;
    }
    if (strncmp(buf, "USR", 3) != 0) {
        if (callback != NULL)
            callback(-1, "The server replied with an error");
        return -1;
    }
    session->sd = sd;
    return sd;
}
