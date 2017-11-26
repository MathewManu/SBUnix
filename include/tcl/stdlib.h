/*
 * stdlib.h --
 *
 *	Declares facilities exported by the "stdlib" portion of
 *	the C library.  This file isn't complete in the ANSI-C
 *	sense;  it only declares things that are needed by Tcl.
 *	This file is needed even on many systems with their own
 *	stdlib.h (e.g. SunOS) because not all stdlib.h files
 *	declare all the procedures needed here (such as strtod).
 *
 * Copyright 1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * $Header: /user6/ouster/tcl/compat/RCS/stdlib.h,v 1.7 93/01/22 15:16:32 ouster Exp $ SPRITE (Berkeley)
 */

#ifndef _STDLIB
#define _STDLIB

#include <tcl/tcl.h>
#include <sys/defs.h>

typedef int uid_t;
typedef int ino_t;

void *Tcl_Malloc(int size);
void Tcl_Free(void *ptr);

#endif /* _STDLIB */
