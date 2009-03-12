/*
 *
 *   (C) Copyright IBM Corp. 2001, 2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: evms.h
 */

#ifndef _EVMS_H_
#define _EVMS_H_ 1

#include "token.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int g_sock_fd;

extern int evms_init(int argc, char * argv[], int sock_fd);
extern int evms_deinit();
extern int evms_func(char * cmd, int len);

extern int My_Printf(char * format, ...);
extern int Indent_Printf(int indent_level, boolean indent_first_line, char * format, ...);
extern int Row_Delimiter(char ch, char * header);

#ifdef __cplusplus
}
#endif

#endif
