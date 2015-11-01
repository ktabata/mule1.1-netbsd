/* Buffer insertion/deletion and gap motion for GNU Emacs.
   Copyright (C) 1985, 1986, 1990 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* 87.5.1   modified by K.Handa */
/* 87.12.10 modified for Nemacs Ver.2.0 by K.Handa */
/* 89.11.30 modified for Nemacs Ver.3.3 by K.Handa <handa@etl.go.jp> */
/* 91.11.13 modified for Nemacs Ver.4.0.0 by K.Handa <handa@etl.go.jp> */
/* 92.3.6   modified for Mule Ver.0.9.0 by K.Handa <handa@etl.go.jp> */
/* 92.4.16  modified for Mule Ver.0.9.3 by K.Handa <handa@etl.go.jp>
	Cope with a new function insert_from_string(). */
/* 92.11.16 modified for Mule Ver.0.9.6 by K.Handa <handa@etl.go.jp>
	make_gap() allocates an extra byte for an anchor.
	insert2() and insert_from_string2() put (char *)0 at GPT_ADDR. */

#include "config.h"
#include "lisp.h"
#include "buffer.h"
#include "window.h"

/* Move gap to position `pos'.
   Note that this can quit!  */

void gap_left (int, int);
void gap_right (int);
void adjust_markers (int, int, int);
void adjust_markers2 (int, int, int, int);

void
move_gap (int pos)
{
  if (pos < GPT)
    gap_left (pos, 0);
  else if (pos > GPT)
    gap_right (pos);
}

/* Move the gap to POS, which is less than the current GPT.
   If NEWGAP is nonzero, then don't update beg_unchanged and end_unchanged.  */

void
gap_left (register int pos, int newgap)
{
  register unsigned char *to, *from;
  register int i;
  int new_s1;

  pos--;

  if (!newgap)
    {
      if (unchanged_modified == MODIFF)
	{
	  beg_unchanged = pos;
	  end_unchanged = Z - pos - 1;
	}
      else
	{
	  if (Z - GPT < end_unchanged)
	    end_unchanged = Z - GPT;
	  if (pos < beg_unchanged)
	    beg_unchanged = pos;
	}
    }

  i = GPT;
  to = GAP_END_ADDR;
  from = GPT_ADDR;
  new_s1 = GPT - BEG;

  /* Now copy the characters.  To move the gap down,
     copy characters up.  */

  while (1)
    {
      /* I gets number of characters left to copy.  */
      i = new_s1 - pos;
      if (i == 0)
	break;
      /* If a quit is requested, stop copying now.
	 Change POS to be where we have actually moved the gap to.  */
      if (QUITP)
	{
	  pos = new_s1;
	  break;
	}
      /* Move at most 32000 chars before checking again for a quit.  */
      if (i > 32000)
	i = 32000;
      new_s1 -= i;
      while (--i >= 0)
	*--to = *--from;
    }

  /* Adjust markers, and buffer data structure, to put the gap at POS.
     POS is where the loop above stopped, which may be what was specified
     or may be where a quit was detected.  */
  adjust_markers (pos + 1, GPT, GAP_SIZE);
  GPT = pos + 1;
  QUIT;
}

void
gap_right (register int pos)
{
  register unsigned char *to, *from;
  register int i;
  int new_s1;

  pos--;

  if (unchanged_modified == MODIFF)
    {
      beg_unchanged = pos;
      end_unchanged = Z - pos - 1;
    }
  else
    {
      if (Z - pos - 1 < end_unchanged)
	end_unchanged = Z - pos - 1;
      if (GPT - BEG < beg_unchanged)
	beg_unchanged = GPT - BEG;
    }

  i = GPT;
  from = GAP_END_ADDR;
  to = GPT_ADDR;
  new_s1 = GPT - 1;

  /* Now copy the characters.  To move the gap up,
     copy characters down.  */

  while (1)
    {
      /* I gets number of characters left to copy.  */
      i = pos - new_s1;
      if (i == 0)
	break;
      /* If a quit is requested, stop copying now.
	 Change POS to be where we have actually moved the gap to.  */
      if (QUITP)
	{
	  pos = new_s1;
	  break;
	}
      /* Move at most 32000 chars before checking again for a quit.  */
      if (i > 32000)
	i = 32000;
      new_s1 += i;
      while (--i >= 0)
	*to++ = *from++;
    }

  adjust_markers (GPT + GAP_SIZE, pos + 1 + GAP_SIZE, - GAP_SIZE);
  GPT = pos + 1;
  QUIT;
}

/* Add `amount' to the position of every marker in the current buffer
   whose current position is between `from' (exclusive) and `to' (inclusive).
   Also, any markers past the outside of that interval, in the direction
   of adjustment, are first moved back to the near end of the interval
   and then adjusted by `amount'.  */

/* 89.11.30 by K.Handa */
void
adjust_markers (register int from, register int to, register int amount)
{
  adjust_markers2 (from, to, amount, MARKER_ALL_TYPE);
}

void
adjust_markers2 (int from, int to, register int amount, register int marker_type)
{
  Lisp_Object marker;
  register struct Lisp_Marker *m;
  register int mpos, range1, range2, newpos;

  marker = current_buffer->markers;

  if (amount > 0) range1 = to, range2 = to + amount, newpos = range2;
  else range1 = from + amount, range2 = from + 1, newpos = range1;

  while (!NILP (marker))
    {
      m = XMARKER (marker);
      if (m->type & marker_type) {
	mpos = m->bufpos;
	if (mpos > range1 && mpos < range2)
	  mpos = newpos;
	if (mpos > from && mpos <= to)
	  mpos += amount;
	m->bufpos = mpos;
      }
      marker = m->chain;
    }
}
/* end of patch */

/* Make the gap INCREMENT characters longer.  */

void
make_gap (int increment)
{
  unsigned char *memory;
  Lisp_Object tem;
  int real_gap_loc;
  int old_gap_size;

  /* If we have to get more space, get enough to last a while.  */
  increment += 2000;

  memory = (unsigned char *) realloc (BEG_ADDR,
				      Z - BEG + GAP_SIZE + increment + 1);
  if (memory == 0)
    memory_full ();
  memory[Z - BEG + GAP_SIZE + increment] = 0; /* 92.11.16 by K.Handa */
  BEG_ADDR = memory;

  /* Prevent quitting in move_gap.  */
  tem = Vinhibit_quit;
  Vinhibit_quit = Qt;

  real_gap_loc = GPT;
  old_gap_size = GAP_SIZE;
  /* Call the newly allocated space a gap at the end of the whole space.  */
  GPT = Z + GAP_SIZE;
  GAP_SIZE = increment;
  /* Move the new gap down to be consecutive with the end of the old one.
     This adjusts the markers properly too.  */
  gap_left (real_gap_loc + old_gap_size, 1);
  /* Now combine the two into one large gap.  */
  GAP_SIZE += old_gap_size;
  GPT = real_gap_loc;

  Vinhibit_quit = tem;
}

/* Insert the character c before point */

void
insert_char (const unsigned char c)
{
  insert (&c, 1);
}

/* Insert the null-terminated string s before point */

void
InsStr (char *s)
{
  insert (s, strlen (s));
}

/* Insert a string of specified length before point.
   DO NOT use this for the contents of a Lisp string!
   prepare_to_modify_buffer could relocate the string.  */

void
insert (register const unsigned char *string, register int length)
/* 89.11.30 patch for point_type_marker by K.Handa */
{
  register int opoint = point;
  insert2 (string, length);
  adjust_markers2 (opoint - 1, opoint, length, MARKER_POINT_TYPE);
}

void
insert2 (register const unsigned char *string, register int length)
{
/* end of patch */
  register Lisp_Object temp;

  if (length < 1)
    return;

  /* Make sure point-max won't overflow after this insertion.  */
  XSET (temp, Lisp_Int, length + Z);
  if (length + Z != XINT (temp))
    error ("maximum buffer size exceeded");

  prepare_to_modify_buffer ();

  if (point != GPT)
    move_gap (point);
  if (GAP_SIZE < length + 1)	/* 92.11.18 by K.Handa */
    make_gap (length + 1 - GAP_SIZE);

  record_insert (point, length);
  MODIFF++;

  bcopy (string, GPT_ADDR, length);
  *(char *)(GPT_ADDR + length) = 0; /* 92.11.18 by K.Handa -- Put an anchor */

  GAP_SIZE -= length;
  GPT += length;
  ZV += length;
  Z += length;
  point += length;
}

/* Function to insert part of the text of a string (STRING) consisting
   of LENGTH characters at position POS.
   It does not work to use `insert' for this, becase a GC could happen
   before we bcopy the stuff into the buffer, and relocate the string
   without insert noticing.  */
void
insert_from_string (Lisp_Object string, register int pos, register int length)
{
/* 92.4.16 patch for point_type_marker by K.Handa */
  register int opoint = point;
  insert_from_string2 (string, pos, length);
  adjust_markers2 (opoint - 1, opoint, length, MARKER_POINT_TYPE);
}

void
insert_from_string2 (Lisp_Object string, register int pos, register int length)
{
/* end of patch */
  register Lisp_Object temp;
  struct gcpro gcpro1;

  if (length < 1)
    return;

  /* Make sure point-max won't overflow after this insertion.  */
  XSET (temp, Lisp_Int, length + Z);
  if (length + Z != XINT (temp))
    error ("maximum buffer size exceeded");

  GCPRO1 (string);
  prepare_to_modify_buffer ();

  if (point != GPT)
    move_gap (point);
  if (GAP_SIZE < length + 1)	/* 92.11.18 by K.Handa */
    make_gap (length + 1 - GAP_SIZE);

  record_insert (point, length);
  MODIFF++;
  UNGCPRO;

  bcopy (XSTRING (string)->data, GPT_ADDR, length);
  *(char *)(GPT_ADDR + length) = 0; /* 92.11.18 by K.Handa -- Put an anchor */

  GAP_SIZE -= length;
  GPT += length;
  ZV += length;
  Z += length;
  point += length;
}

/* Like `insert' except that all markers pointing at the place where
   the insertion happens are adjusted to point after it.
   Don't use this function to insert part of a Lisp string,
   since gc could happen and relocate it.  */

void
insert_before_markers (const unsigned char *string, register int length)
{
  register int opoint = point;
  insert2 (string, length);	/* 89.11.30  by K.Handa */
  adjust_markers (opoint - 1, opoint, length);
}

/* Insert part of a Lisp string, relocating markers after.  */

void
insert_from_string_before_markers (Lisp_Object string, register int pos, register int length)
{
  register int opoint = point;
  insert_from_string2 (string, pos, length); /* 92.4.16 by K.Handa */
  adjust_markers (opoint - 1, opoint, length);
}

/* Delete characters in current buffer
  from `from' up to (but not incl) `to' */

void
del_range (register int from, register int to)
{
  register int numdel;

  /* Make args be valid */
  if (from < BEGV)
    from = BEGV;
  if (to > ZV)
    to = ZV;

  if ((numdel = to - from) <= 0)
    return;

  /* Make sure the gap is somewhere in or next to what we are deleting */
  if (from > GPT)
    gap_right (from);
  if (to < GPT)
    gap_left (to, 0);

  prepare_to_modify_buffer ();
  record_delete (from, numdel);
  MODIFF++;

  /* Relocate point as if it were a marker.  */
  if (from < point)
    {
      if (point < to)
	point = from;
      else
	point -= numdel;
    }

  /* Relocate all markers pointing into the new, larger gap
     to point at the end of the text before the gap.  */
  adjust_markers (to + GAP_SIZE, to + GAP_SIZE, - numdel - GAP_SIZE);

  GAP_SIZE += numdel;
  ZV -= numdel;
  Z -= numdel;
  GPT = from;

  if (GPT - BEG < beg_unchanged)
    beg_unchanged = GPT - BEG;
  if (Z - GPT < end_unchanged)
    end_unchanged = Z - GPT;
}

void
modify_region (int start, int end)
{
  prepare_to_modify_buffer ();
  if (start - 1 < beg_unchanged || unchanged_modified == MODIFF)
    beg_unchanged = start - 1;
  if (Z - end < end_unchanged
      || unchanged_modified == MODIFF)
    end_unchanged = Z - end;
  MODIFF++;
}

void
prepare_to_modify_buffer (void)
{
  if (!NILP (current_buffer->read_only))
    Fbarf_if_buffer_read_only();

#ifdef CLASH_DETECTION
  if (!NILP (current_buffer->filename)
      && current_buffer->save_modified >= MODIFF)
    lock_file (current_buffer->filename);
#else
  /* At least warn if this file has changed on disk since it was visited.  */
  if (!NILP (current_buffer->filename)
      && current_buffer->save_modified >= MODIFF
      && NILP (Fverify_visited_file_modtime (Fcurrent_buffer ()))
      && !NILP (Ffile_exists_p (current_buffer->filename)))
    call1 (intern ("ask-user-about-supersession-threat"),
	   current_buffer->filename);
#endif /* not CLASH_DETECTION */
}
