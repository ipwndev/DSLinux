
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

#include "filters.h"
#include "common.h"

int
UI_set_filter(filter)
     char *filter;
{
    config *cf = UI_get_config();

    if (strcasecmp(filter, "austro") == 0)
        cf->filter = AUSTRO;
    else if (strcasecmp(filter, "b1ff") == 0)
        cf->filter = B1FF;
    else if (strcasecmp(filter, "brooklyn") == 0)
        cf->filter = BROOKLYN;
    else if (strcasecmp(filter, "chef") == 0)
        cf->filter = CHEF;
    else if (strcasecmp(filter, "cockney") == 0)
        cf->filter = COCKNEY;
    else if (strcasecmp(filter, "drawl") == 0)
        cf->filter = DRAWL;
    else if (strcasecmp(filter, "dubya") == 0)
        cf->filter = DUBYA;
    else if (strcasecmp(filter, "fudd") == 0)
        cf->filter = FUDD;
    else if (strcasecmp(filter, "funetak") == 0)
        cf->filter = FUNETAK;
    else if (strcasecmp(filter, "jethro") == 0)
        cf->filter = JETHRO;
    else if (strcasecmp(filter, "jive") == 0)
        cf->filter = JIVE;
    else if (strcasecmp(filter, "kraut") == 0)
        cf->filter = KRAUT;
    else if (strcasecmp(filter, "pansy") == 0)
        cf->filter = PANSY;
    else if (strcasecmp(filter, "pirate") == 0)
        cf->filter = PIRATE;
    else if (strcasecmp(filter, "postmodern") == 0)
        cf->filter = POSTMODERN;
    else if (strcasecmp(filter, "redneck") == 0)
        cf->filter = REDNECK;
    else if (strcasecmp(filter, "valspeak") == 0)
        cf->filter = VALSPEAK;
    else if (strcasecmp(filter, "warez") == 0)
        cf->filter = WAREZ;
    else if (strcasecmp(filter, "none") == 0)
        cf->filter = 0;
    else
        return -1;
    return 0;
}

void
UI_filter_translate(buffer, size)
     char *buffer;
     int size;
{
#ifdef HAVE_TALKFILTERS
    config *cf = UI_get_config();

    switch (cf->filter) {
    case AUSTRO:
        gtf_filter_austro(buffer, buffer, size);
        break;
    case B1FF:
        gtf_filter_b1ff(buffer, buffer, size);
        break;
    case BROOKLYN:
        gtf_filter_brooklyn(buffer, buffer, size);
        break;
    case CHEF:
        gtf_filter_chef(buffer, buffer, size);
        break;
    case COCKNEY:
        gtf_filter_cockney(buffer, buffer, size);
        break;
    case DRAWL:
        gtf_filter_drawl(buffer, buffer, size);
        break;
    case DUBYA:
        gtf_filter_dubya(buffer, buffer, size);
        break;
    case FUDD:
        gtf_filter_fudd(buffer, buffer, size);
        break;
    case FUNETAK:
        gtf_filter_funetak(buffer, buffer, size);
        break;
    case JETHRO:
        gtf_filter_jethro(buffer, buffer, size);
        break;
    case JIVE:
        gtf_filter_jive(buffer, buffer, size);
        break;
    case KRAUT:
        gtf_filter_kraut(buffer, buffer, size);
        break;
    case PANSY:
        gtf_filter_pansy(buffer, buffer, size);
        break;
    case PIRATE:
        gtf_filter_pirate(buffer, buffer, size);
        break;
    case POSTMODERN:
        gtf_filter_postmodern(buffer, buffer, size);
        break;
    case REDNECK:
        gtf_filter_redneck(buffer, buffer, size);
        break;
    case VALSPEAK:
        gtf_filter_valspeak(buffer, buffer, size);
        break;
    case WAREZ:
        gtf_filter_warez(buffer, buffer, size);
        break;
    default:
        break;
    }
#endif
}
