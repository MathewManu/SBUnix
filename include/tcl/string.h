/*
 * string.h --
 *
 *	Declarations of ANSI C library procedures for string handling.
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
 * $Header: /user6/ouster/tcl/compat/RCS/string.h,v 1.7 93/01/29 14:19:30 ouster Exp $ SPRITE (Berkeley)
 */

#ifndef _STRING
#define _STRING

#include <tcl/tcl.h>

extern int  my_strcmp(const char *f_str, const char *s_str);
extern char *my_strcpy(char *dest, char *src);
extern int  my_strlen(char *str);
extern int  my_strncmp(const char *f_str, const char *s_str, int n);
extern long	int strtol (char *string, char **endPtr, int base);
extern unsigned long int strtoul (char *string, char **endPtr, int base);

#endif /* _STRING */
