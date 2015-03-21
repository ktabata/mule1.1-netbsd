/* Declarations having to do with GNU Emacs syntax tables.
   Copyright (C) 1985 Free Software Foundation, Inc.

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

/* 92.1.10  modified for Mule Ver.0.9.0 by K.Handa <handa@etl.go.jp>
	Function entries char_syntax() and char_syntax_at() are declared. */
/* 92.7.16  modified for Mule Ver.0.9.5 by K.Handa <handa@etl.go.jp>
	Sextword is appended in syntaxcode. */
/* 93.7.12  modified for Mule Ver.0.9.8 by K.Handa <handa@etl.go.jp>
	syntax_table_version introduced. */


extern Lisp_Object Qsyntax_table_p;
extern Lisp_Object Fsyntax_table_p (Lisp_Object);
extern Lisp_Object Fsyntax_table (void);
extern Lisp_Object Fset_syntax_table (Lisp_Object);

/* The standard syntax table is stored where it will automatically
   be used in all new buffers.  */
#define Vstandard_syntax_table buffer_defaults.syntax_table

/* A syntax table is a Lisp vector of length 0400, whose elements are integers.

The low 8 bits of the integer is a code, as follows:
*/

enum syntaxcode
  {
    Swhitespace, /* for a whitespace character */
    Spunct,	 /* for random punctuation characters */
    Sword,	 /* for a word constituent */
    Ssymbol,	 /* symbol constituent but not word constituent */
    Sopen,	 /* for a beginning delimiter */
    Sclose,      /* for an ending delimiter */
    Squote,	 /* for a prefix character like Lisp ' */
    Sstring,	 /* for a string-grouping character like Lisp " */
    Smath,	 /* for delimiters like $ in Tex. */
    Sescape,	 /* for a character that begins a C-style escape */
    Scharquote,  /* for a character that quotes the following character */
    Scomment,    /* for a comment-starting character */
    Sendcomment, /* for a comment-ending character */
    Sextword,	 /* for an extended word constituent. 92.7.16 by K.Handa */
    Smax	 /* Upper bound on codes that are meaningful */
  };

/* 92.1.10 by K.Handa */
extern Lisp_Object char_syntax(unsigned int), char_syntax_at(unsigned char *, int);

#define SYNTAX(c) ((enum syntaxcode) (XFASTINT (char_syntax(c)) & 0xFF))

#define SYNTAX_AT(p,len) \
  ((enum syntaxcode) (XFASTINT (char_syntax_at(p, len)) & 0xFF))

/* The next 8 bits of the number is a character,
 the matching delimiter in the case of Sopen or Sclose. */

#define SYNTAX_MATCH(c) \
  ((XFASTINT (char_syntax(c)) >> 8) & 0377)

/* Then there are four single-bit flags that have the following meanings:
  1. This character is the first of a two-character comment-start sequence.
  2. This character is the second of a two-character comment-start sequence.
  3. This character is the first of a two-character comment-end sequence.
  4. This character is the second of a two-character comment-end sequence.
 Note that any two-character sequence whose first character has flag 1
  and whose second character has flag 2 will be interpreted as a comment start. */

#define SYNTAX_COMSTART_FIRST(c) \
  ((XFASTINT (char_syntax(c)) >> 16) & 1)

#define SYNTAX_COMSTART_SECOND(c) \
  ((XFASTINT (char_syntax(c)) >> 17) & 1)

#define SYNTAX_COMEND_FIRST(c) \
  ((XFASTINT (char_syntax(c)) >> 18) & 1)

#define SYNTAX_COMEND_SECOND(c) \
  ((XFASTINT (char_syntax(c)) >> 19) & 1)

/* This array, indexed by a character, contains the syntax code which that
 character signifies (as a char).  For example,
 (enum syntaxcode) syntax_spec_code['w'] is Sword. */

extern unsigned char syntax_spec_code[0400];
extern char syntax_code_spec[14]; /* 93.7.13 by K.Handa */
extern unsigned int syntax_table_version; /* 93.7.12 by K.Handa */
