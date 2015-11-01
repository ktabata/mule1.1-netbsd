/* Examine the result of  stat  and make a string describing file modes.
   Copyright (C) 1985 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

In other words, you are welcome to use, share and improve this program.
You are forbidden to forbid anyone else to use, share and improve
what you give them.   Help stamp out software-hoarding!  */

/* 92.10.21 modified for Mule Ver.0.9.6
		by M.Higashida <manabu@sigmath.osaka-u.ac.jp>,
		   S.Hirano <hirano@etl.go.jp>
	DOS-Extender GO32 and EMX supported. */

/* 91.10.16 by S.Hirano */
#include "config.h"
/* end of patch */
#include <sys/types.h>
#include <sys/stat.h>
#include "filetypes.h"

/* filemodestring - set file attribute data 

*** WARNING!  FILE STRUCTURE DEPENDENT ***

   Filemodestring converts the data in the st_mode field of file status
block `s' to a 10 character attribute string, which it stores in
the block that `a' points to.
This attribute string is modelled after the string produced by the Berkeley ls.

As usual under Unix, the elements of the string are numbered
from 0.  Their meanings are:

   0	File type.  'd' for directory, 'c' for character
	special, 'b' for block special, 'm' for multiplex,
	'l' for symbolic link, 's' for socket, 'p' for fifo,
	'-' for any other file type

   1	'r' if the owner may read, '-' otherwise.

   2	'w' if the owner may write, '-' otherwise.

   3	'x' if the owner may execute, 's' if the file is
	set-user-id, '-' otherwise.
	'S' if the file is set-user-id, but the execute
	bit isn't set.  (sys v `feature' which helps to
	catch screw case.)

   4	'r' if group members may read, '-' otherwise.

   5	'w' if group members may write, '-' otherwise.

   6	'x' if group members may execute, 's' if the file is
	set-group-id, '-' otherwise.
	'S' if it is set-group-id but not executable.

   7	'r' if any user may read, '-' otherwise.

   8	'w' if any user may write, '-' otherwise.

   9	'x' if any user may execute, 't' if the file is "sticky"
	(will be retained in swap space after execution), '-'
	otherwise.

 */

#define VOID void

static char ftypelet (struct stat *);
static VOID rwx (unsigned short, char []);
static VOID setst (unsigned short, char []);

VOID
filemodestring (struct stat *s, char *a)
{
   a[0] = ftypelet (s);
   /* Aren't there symbolic names for these byte-fields? */
   rwx ((s->st_mode & 0700) << 0, &(a[1]));
   rwx ((s->st_mode & 0070) << 3, &(a[4]));
   rwx ((s->st_mode & 0007) << 6, &(a[7]));
   setst (s->st_mode, a);
/* 91.10.16 by S.Hirano, 93.2.17 by M.Higashida */
#if defined(MSDOS) || defined(WIN32)
   if (a[0] == 'd' && a[3] == 'x')
       a[2] = 'w';		/* emulate directory write permission */
#endif
/* end of patch */
}

/* ftypelet - file type letter

*** WARNING!  FILE STRUCTURE DEPENDENT ***

   Ftypelet accepts a file status block and returns a character
code describing the type of the file.  'd' is returned for
directories, 'b' for block special files, 'c' for character
special files, 'm' for multiplexor files, 'l' for symbolic link,
's' for socket, 'p' for fifo, '-' for any other file type
 */

static char
ftypelet(struct stat *s)
{
  switch (s->st_mode & S_IFMT)
    {
    default:
      return '-';
    case S_IFDIR:
      return 'd';
#ifdef S_IFLNK
    case S_IFLNK:
      return 'l';
#endif
#ifdef S_IFCHR
    case S_IFCHR:
      return 'c';
#endif
#ifdef S_IFBLK
    case S_IFBLK:
      return 'b';
#endif
#ifdef S_IFMPC
/* These do not seem to exist */
    case S_IFMPC:
    case S_IFMPB:
      return 'm';
#endif
#ifdef S_IFSOCK
    case S_IFSOCK:
      return 's';
#endif
#ifdef S_IFIFO
#if S_IFIFO != S_IFSOCK
    case S_IFIFO:
      return 'p';
#endif
#endif
#ifdef S_IFNWK /* hp-ux hack */
    case S_IFNWK:
      return 'n';
#endif
    }
}


/* rwx - look at read, write, and execute bits and set character
flags accordingly

*** WARNING!  FILE STRUCTURE DEPENDENT ***

 */

static VOID
rwx (unsigned short bits, char chars[])
{
  chars[0] = (bits & S_IREAD)  ? 'r' : '-';
  chars[1] = (bits & S_IWRITE) ? 'w' : '-';
  chars[2] = (bits & S_IEXEC)  ? 'x' : '-';
}


/* setst - set s & t flags in a file attributes string */
/* *** WARNING!  FILE STRUCTURE DEPENDENT *** */
static VOID
setst (unsigned short bits, char chars[])
{
#ifdef S_ISUID
   if (bits & S_ISUID)
     {
       if (chars[3] != 'x')
	 /* Screw case: set-uid, but not executable. */
	 chars[3] = 'S';
       else
	 chars[3] = 's';
     }
#endif
#ifdef S_ISGID
   if (bits & S_ISGID)
     {
       if (chars[6] != 'x')
	 /* Screw case: set-gid, but not executable. */
	 chars[6] = 'S';
       else
	 chars[6] = 's';
     }
#endif
#ifdef S_ISVTX
   if (bits & S_ISVTX)
      chars[9] = 't';
#endif
}

