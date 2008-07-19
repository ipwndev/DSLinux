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

#include "core_misc.h"
#include "config.h"
#include "core.h"

void
MSN_init(session)
     MSN_session *session;
{
    session->sd = 0;
    session->gtc = 0x0;
    session->blp = 0x0;
    session->num_contacts = 0;
    session->csc = 0;
    session->me.addr[0] = 0x0;
    session->me.name[0] = 0x0;
    session->me.custom_name[0] = 0x0;
    session->me.cmedia[0] = 0x0;
    session->me.psm[0] = 0x0;
    session->me.listnum = 0;
    session->me.status = 0;
    strncpy(session->format.color, "000000", 6);
    strncpy(session->format.font, "Arial", 5);
    session->format.effect = ' ';

    /*
     * Initially allocate space for one contact and one group
     */
    session->contact = (MSN_contact **) malloc(1 * sizeof(MSN_contact));
    session->group = (MSN_group **) malloc(1 * sizeof(MSN_group));
}

int
isBigEndian(void)
{
    short int word = 0x0100;
    char *byte = (char *)&word;

    return (byte[0]);
}

unsigned int
swapInt(unsigned int dw)
{
    unsigned int tmp;

    tmp = (dw & 0x000000FF);
    tmp = ((dw & 0x0000FF00) >> 0x08) | (tmp << 0x08);
    tmp = ((dw & 0x00FF0000) >> 0x10) | (tmp << 0x08);
    tmp = ((dw & 0xFF000000) >> 0x18) | (tmp << 0x08);
    return (tmp);
}

unsigned long long
swapLongLong(unsigned long long qw)
{
    unsigned int *a, *b;
    unsigned long long ret;

    a = (unsigned int *)&qw;
    b = (unsigned int *)&ret;

    b[0] = swapInt(a[1]);
    b[1] = swapInt(a[0]);
    return ret;
}

void
MSN_handle_chl(char *input, char *output)
{
    char *productKey = "CFHUR$52U_{VIX5T", *productID = "PROD0101{0RM?UBW",
        *hexChars = "0123456789abcdef", buf[BUFSIZE];
    unsigned char md5Hash[16], *newHash;
    unsigned int *md5Parts, *chlStringParts, newHashParts[5];

    long long nHigh = 0, nLow = 0;

    int i, bigEndian;

    /*
     * Determine our endianess 
     */
    bigEndian = isBigEndian();

    /*
     * Create the MD5 hash 
     */
    snprintf(buf, BUFSIZE - 1, "%s%s", input, productKey);
    MD5((unsigned char *)buf, strlen(buf), md5Hash);

    /*
     * Split it into four integers 
     */
    md5Parts = (unsigned int *)md5Hash;
    for (i = 0; i < 4; i++) {
        /*
         * check for endianess 
         */
        if (bigEndian)
            md5Parts[i] = swapInt(md5Parts[i]);

        /*
         * & each integer with 0x7FFFFFFF          
         */
        /*
         * and save one unmodified array for later 
         */
        newHashParts[i] = md5Parts[i];
        md5Parts[i] &= 0x7FFFFFFF;
    }

    /*
     * make a new string and pad with '0' 
     */
    snprintf(buf, BUFSIZE - 5, "%s%s", input, productID);
    i = strlen(buf);
    memset(&buf[i], '0', 8 - (i % 8));
    buf[i + (8 - (i % 8))] = '\0';

    /*
     * split into integers 
     */
    chlStringParts = (unsigned int *)buf;

    /*
     * this is magic 
     */
    for (i = 0; i < (strlen(buf) / 4) - 1; i += 2) {
        long long temp;

        if (bigEndian) {
            chlStringParts[i] = swapInt(chlStringParts[i]);
            chlStringParts[i + 1] = swapInt(chlStringParts[i + 1]);
        }
        temp =
            (md5Parts[0] *
             (((0x0E79A9C1 * (long long)chlStringParts[i]) % 0x7FFFFFFF) +
              nHigh) + md5Parts[1]) % 0x7FFFFFFF;
        nHigh =
            (md5Parts[2] *
             (((long long)chlStringParts[i + 1] + temp) % 0x7FFFFFFF) +
             md5Parts[3]) % 0x7FFFFFFF;
        nLow = nLow + nHigh + temp;
    }
    nHigh = (nHigh + md5Parts[1]) % 0x7FFFFFFF;
    nLow = (nLow + md5Parts[3]) % 0x7FFFFFFF;

    newHashParts[0] ^= nHigh;
    newHashParts[1] ^= nLow;
    newHashParts[2] ^= nHigh;
    newHashParts[3] ^= nLow;

    /*
     * swap more bytes if big endian 
     */
    for (i = 0; i < 4 && bigEndian; i++)
        newHashParts[i] = swapInt(newHashParts[i]);

    /*
     * make a string of the parts 
     */
    newHash = (unsigned char *)newHashParts;

    /*
     * convert to hexadecimal 
     */
    for (i = 0; i < 16; i++) {
        output[i * 2] = hexChars[(newHash[i] >> 4) & 0xF];
        output[(i * 2) + 1] = hexChars[newHash[i] & 0xF];
    }

    output[32] = '\0';
}

MSN_contact **
MSN_resize_contact_array(session)
     MSN_session *session;
{
    session->contact = (MSN_contact **) realloc(session->contact,
                                                (session->num_contacts +
                                                 1) * sizeof(MSN_contact *));
    return session->contact;
}

MSN_contact *
MSN_allocate_contact(void)
{
    MSN_contact *ret = (MSN_contact *) malloc(sizeof(MSN_contact));

    ret->addr[0] = '\0';
    ret->name[0] = '\0';
    ret->custom_name[0] = '\0';
    ret->guid[0] = '\0';
    ret->listnum = 0;
    ret->cmedia[0] = 0;	// gfhuang
    ret->psm[0] = 0;	// gfhuang
    ret->ingroup = -1;	// gfhuang
    return ret;
}

// group, added by gfhuang
MSN_group **
MSN_resize_group_array(session)
     MSN_session *session;
{
    session->group = (MSN_group **) realloc(session->group,
                                                (session->num_groups +
                                                 1) * sizeof(MSN_group *));
    return session->group;
}

MSN_group *
MSN_allocate_group(void)
{
    MSN_group *ret = (MSN_group *) malloc(sizeof(MSN_group));

    ret->name[0] = 0;
    ret->guid[0] = 0;
    return ret;
}
//added end

void
MSN_free_contact(contact)
     MSN_contact *contact;
{
    free(contact);
}

/*
 * does the same thing as fgets() but does not
 * require a (FILE *) pointer
 */
int
getline(str, size, sd)
     char *str;
     int size;
     int sd;
{
    char c, *ptr = str;
    int r;

    while ((r = read(sd, &c, 1)) != 0) {
        if (r < 0)
            return -1;

        if ((ptr - str) >= size)
            break;

        *ptr = c;
        ptr++;

        if (c == '\n')
            break;
    }

    *ptr = '\0';
    return (ptr - str);
}

char *
split(str, split, ret)
     char *str;
     char split;
     int ret;
{
    char *ptr;
    int a, b, c = 0;

    /*
     * Find the first split-char 
     */
    if (ret != 0) {
        for (a = 0; a <= strlen(str); a++) {
            if (str[a] == split && ++c == ret) {
                a++;
                break;
            }
            if (a == strlen(str))
                return NULL;
        }
    } else {
        a = 0;
    }

    /*
     * Find the second split-char 
     */
    for (b = a; b <= strlen(str); b++)
        if (str[b] == split)
            break;

    ptr = (char *)malloc(sizeof(char) * (b - a + 1));
    strncpy(ptr, &str[a], b - a);
    ptr[b - a] = 0x0;

    return (char *)ptr;
}

/* Check if addr is a valid e-mail address */
int
MSN_is_valid_address(addr)
     char *addr;
{
    int i;
    char *ptr;

    for (i = 0; i < strlen(addr); i++)
        if (isspace(addr[i]))
            return -1;
    if ((ptr = strchr(addr, '@')) == NULL)
        return -1;
    if ((ptr = strchr(addr, '.')) == NULL)
        return -1;
    return 0;
}

char *
MSN_xml_encode(str, ret, ret_len)
     char *str;
     char *ret;
     int ret_len;
{
    int i, a;

    for (i = 0, a = 0; i < strlen(str) && a < ret_len; i++) {
        if (str[i] == '&') {
            snprintf(&ret[a], ret_len - i, "&amp;");
            a += 5;
        } else if (str[i] == '"') {
            snprintf(&ret[a], ret_len - i, "&quot;");
            a += 6;
        } else if (str[i] == '\'') {
            snprintf(&ret[a], ret_len - i, "&apos;");
            a += 6;
        } else if (str[i] == '<') {
            snprintf(&ret[a], ret_len - i, "&lt;");
            a += 4;
        } else if (str[i] == '>') {
            snprintf(&ret[a], ret_len - i, "&gt;");
            a += 4;
        } else {
            ret[a] = str[i];
            a += 1;
        }
    }
    ret[a] = 0x0;
    return ret;
}

char *
MSN_url_encode(str, ret, ret_len)
     char *str;
     char *ret;
     int ret_len;
{
    int i, a;

    for (i = 0, a = 0; i < strlen(str) && a < ret_len; i++, a++) {
        if (!isalnum(str[i])) {
            snprintf(&ret[a], ret_len - i, "%%%x", str[i]);
            a += 2;
        } else {
            ret[a] = str[i];
        }
    }
    ret[a] = 0x0;
    return ret;
}

char *
MSN_url_decode(str, ret, retsize)
     char *str;
     char *ret;
     int retsize;
{
    char buf[retsize + 1];
    char hex[3];
    int x, y;

    for (x = 0, y = 0; x < strlen(str) && y < retsize; x++, y++) {
        if (str[x] == '%' && x < strlen(str) - 2) {
            strncpy(hex, &str[x + 1], 2);
            buf[y] = (char)strtol(hex, NULL, 16);
            if (buf[y] == '\t' || buf[y] == '\n' || buf[y] == '\r')
                buf[y] = ' ';
            x += 2;
        } else {
            buf[y] = str[x];
        }
    }
    buf[y] = '\0';
    strncpy(ret, buf, retsize);

    return ret;
}

void
MSN_error2str(err, message, message_len)
     int err;
     char *message;
     int message_len;
{
    char buf[128];

    switch (err) {
    case 200:
        strcpy(buf, "Invalid syntax");
        break;
    case 201:
        strcpy(buf, "Invalid parameter");
        break;
    case 205:
        strcpy(buf, "Invalid principal");
        break;
    case 206:
        strcpy(buf, "Domain name missing");
        break;
    case 207:
        strcpy(buf, "Already logged in");
        break;
    case 208:
        strcpy(buf, "Invalid principal-name");
        break;
    case 209:
        strcpy(buf, "Nickname change illegal");
        break;
    case 210:
        strcpy(buf, "Principal list full");
        break;
    case 215:
        strcpy(buf, "Principal already on list");
        break;
    case 216:
        strcpy(buf, "Principal not on list");
        break;
    case 217:
        strcpy(buf, "Principal not on-line");
        break;
    case 218:
        strcpy(buf, "Already in mode");
        break;
    case 219:
        strcpy(buf, "Principal is in the opposite list");
        break;
    case 280:
        strcpy(buf, "Switchboard failed");
        break;
    case 281:
        strcpy(buf, "Transfer to switchboard failed");
        break;

    case 300:
        strcpy(buf, "Required field missing");
        break;
    case 302:
        strcpy(buf, "Not logged in");
        break;

    case 500:
        strcpy(buf, "Internal server error");
        break;
    case 501:
        strcpy(buf, "Database server error");
        break;
    case 502:
        strcpy(buf, "Command disabled");
        break;
    case 510:
        strcpy(buf, "File operation failed");
        break;
    case 520:
        strcpy(buf, "Memory allocation failed");
        break;
    case 540:
        strcpy(buf, "Challenge response failed");
        break;

    case 602:
        strcpy(buf, "Peer nameserver is down");
        break;
    case 603:
        strcpy(buf, "Database connection failed");
        break;
    case 604:
        strcpy(buf, "Server is going down");
        break;

    case 707:
        strcpy(buf, "Could not create connection");
        break;
    case 710:
        strcpy(buf, "Bad CVR parameters sent");
        break;
    case 711:
        strcpy(buf, "Write is blocking");
        break;
    case 712:
        strcpy(buf, "Session is overloaded");
        break;
    case 714:
        strcpy(buf, "Too many sessions");
        break;
    case 715:
    case 731:
        strcpy(buf, "Not expected");
        break;
    case 717:
        strcpy(buf, "Bad friend file");
        break;

    case 800:
    case 713:
        strcpy(buf, "Changing too rapidly");
        break;

    case 600:
    case 910:
    case 912:
    case 918:
    case 919:
    case 921:
    case 922:
        strcpy(buf, "Server too busy");
        break;
    case 911:
    case 917:
        strcpy(buf, "Autentication failed");
        break;
    case 913:
        strcpy(buf, "Not allowed when hiding");
        break;
    case 914:
    case 915:
    case 916:
    case 605:
    case 601:
        strcpy(buf, "Server unavailable");
        break;
    case 920:
        strcpy(buf, "Not accepting new principals");
        break;
    case 923:
        strcpy(buf, "Kids Passport without parental consent");
        break;
    case 924:
        strcpy(buf, "Passport account not yet verified");
        break;
    case 928:
        strcpy(buf, "Bad ticket");
        break;
    default:
        strcpy(buf, "Unknown error");
        break;
    }
    strncpy(message, buf, message_len);
}
