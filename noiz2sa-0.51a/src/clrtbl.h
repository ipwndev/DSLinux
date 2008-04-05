/*
 * $Id: clrtbl.h,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Color table header file.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "SDL.h"

extern SDL_Color color[256];
extern Uint8 colorDfs[256];
extern Uint8 colorAlp[256][256];
