/*****************************************************************************\
 *  xstring.h - "safe" string processing functions with automatic memory
 *	        management
 ******************************************************************************
 *  Copyright (C) 2002 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Jim Garlick <garlick@llnl.gov>, et. al.
 *  CODE-OCEC-09-009. All rights reserved.
 *
 *  This file is part of Slurm, a resource management program.
 *  For details, see <https://slurm.schedmd.com/>.
 *  Please also read the included file: DISCLAIMER.
 *
 *  Slurm is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of portions of this program with the OpenSSL library under
 *  certain conditions as described in each individual source file, and
 *  distribute linked combinations including the two. You must obey the GNU
 *  General Public License in all respects for all of the code used other than
 *  OpenSSL. If you modify file(s) with this exception, you may extend this
 *  exception to your version of the file(s), but you are not obligated to do
 *  so. If you do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source files in
 *  the program, then also delete it here.
 *
 *  Slurm is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Slurm; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#ifndef _XSTRING_H
#define _XSTRING_H	1

#include "src/common/macros.h"

#define xstrcat(__p, __q)		_xstrcat(&(__p), __q)
#define xstrcatat(__p, __q, __s) _xstrncatat(&(__p), __q, __s, -1)
#define xstrncat(__p, __q, __l)		_xstrncat(&(__p), __q, __l)
#define xstrncatat(__p, __q, __s, __l) _xstrncatat(&(__p), __q, __s, __l)
#define xstrcatchar(__p, __c)		_xstrcatchar(&(__p), __c)
#define xstrftimecat(__p, __fmt)	_xstrftimecat(&(__p), __fmt)
#define xiso8601timecat(__p, __msec)            _xiso8601timecat(&(__p), __msec)
#define xrfc5424timecat(__p, __msec)            _xrfc5424timecat(&(__p), __msec)
#define xrfc3339timecat(__p) _xrfc3339timecat(&(__p))
#define xstrfmtcat(__p, __fmt, args...)	_xstrfmtcat(&(__p), __fmt, ## args)
#define xstrfmtcatat(__p, __q, __fmt, args...) \
	_xstrfmtcatat(&(__p), __q, __fmt, ## args)
#define xmemcat(__p, __s, __e)          _xmemcat(&(__p), __s, __e)
#define xstrsubstitute(__p, __pat, __rep) \
	_xstrsubstitute(&(__p), __pat, __rep, 0)
#define xstrsubstituteall(__p, __pat, __rep)			\
	_xstrsubstitute(&(__p), __pat, __rep, 1)

/*
** The following functions take a ptr to a string and expand the
** size of that string as needed to append the requested data.
** the macros above are provided to automatically take the
** address of the first argument, thus simplifying the interface
**
** space is allocated with xmalloc/xrealloc, so caller must use
** xfree to free.
**
*/

/*
** cat str2 onto str1, expanding str1 as necessary
*/
void _xstrcat(char **str1, const char *str2);

/*
 * Append len bytes of str2 onto str at pos, expanding buf as needed.
 * pos is updated to the end of the appended string.
 *
 * Meant to be used in loops constructing longer strings that are performance
 * sensitive, as xstrcat() needs to re-seek to the end of str making the string
 * construction worse by another O(log(strlen)) factor.
 *
 * Set len = -1 to copy all of str2. This is done by the xstrcatat() macro.
 */
void _xstrncatat(char **str1, char **pos, const char *str2, ssize_t len);

/*
** cat len of str2 onto str1, expanding str1 as necessary
*/
void _xstrncat(char **str1, const char *str2, size_t len);

/*
** concatenate one char, `c', onto str1, expanding str1 as needed
*/
void _xstrcatchar(char **str1, char c);

/*
** concatenate current time onto str, using fmt if it is non-NUL
** see strftime(3) for the usage of the format string
*/
void _xstrftimecat(char **str, const char *fmt);

/*
** Concatenate a ISO 8601 timestamp onto str.
*/
void _xiso8601timecat(char **str, bool);

/*
** Concatenate a RFC 5424 timestamp onto str.
*/
void _xrfc5424timecat(char **str, bool);

/*
 * Concatenate a RFC 3339 timestamp onto str.
 */
void _xrfc3339timecat(char **str);

/*
 * Concatenate printf-style formatted string onto str
 */
void _xstrfmtcat(char **str, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

/*
 * Concatenate printf-style formatted string onto str at position pos.
 */
void _xstrfmtcatat(char **str, char **pos, const char *fmt, ...)
	__attribute__((format(printf, 3, 4)));

/*
** concatenate range of memory from start to end (not including end)
** onto str.
*/
void _xmemcat(char **str, char *start, char *end);

/*
** strdup which uses xmalloc routines
*/
char *xstrdup(const char *str);

/*
** strdup formatted which uses xmalloc routines
*/
char *xstrdup_printf(const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

size_t _xstrdup_vprintf(char **str, const char *_fmt, va_list _ap);

/*
** strndup which uses xmalloc routines
*/
char *xstrndup(const char *str, size_t n);

/*
** strtol which only reads 'n' number of chars in the str to get the number
*/
long int xstrntol(const char *str, char **endptr, size_t n, int base);

/*
** replacement for libc basename
*/
char *xbasename(char *path);

/*
 * Specialized dirname implementation
 */
char *xdirname(const char *path);

/*
** Find the first instance of a sub-string "pattern" in the string "str",
** and replace it with the string "replacement".
** If it wasn't found returns 0, otherwise 1
*/
void _xstrsubstitute(char **str, const char *pattern,
		     const char *replacement, const bool all);

/* xshort_hostname
 *   Returns an xmalloc'd string containing the hostname
 *   of the local machine.  The hostname contains only
 *   the short version of the hostname (e.g. "linux123.foo.bar"
 *   becomes "linux123")
 *
 *   Returns NULL on error.
 */
char *xshort_hostname(void);

/*
 * Return true if all characters in a string are whitespace characters,
 * otherwise return false.  ("str" must be terminated by a null character)
 */
bool xstring_is_whitespace(const char *str);

/*
 * If str make everything lowercase.
 * Should not be called on static char *'s.
 * Returns true if any changes have been made.
 */
extern bool xstrtolower(char *str);

/*
 * safe strchr (handles NULL values)
 */
char *xstrchr(const char *s1, int c);

/*
 * safe strrchr (handles NULL values)
 */
char *xstrrchr(const char *s1, int c);

/*
 * safe strcmp (handles NULL values)
 */
int xstrcmp(const char *s1, const char *s2);

/*
 * safe strncmp (handles NULL values)
 */
int xstrncmp(const char *s1, const char *s2, size_t n);

/*
 * safe strcasecmp (handles NULL values)
 */
int xstrcasecmp(const char *s1, const char *s2);

/*
 * safe strncasecmp (handles NULL values)
 */
int xstrncasecmp(const char *s1, const char *s2, size_t n);

/*
 * safe strstr (handles NULL values)
 */
char *xstrstr(const char *haystack, const char *needle);

/* safe case insensitive version of strstr(). */
char *xstrcasestr(const char *haystack, const char *needle);

/*
 * Trims white space before and after string in place.
 * Will left shift the string values to remove white space.
 */
void xstrtrim(char *string);

/*
 * Dump byte string to hex
 * IN string - byte string to dump as hex
 * IN len - number of bytes to dump
 * IN delimiter - delimiter between each hex byte (may be NULL)
 * RET hex string (must xfree()) or NULL on error
 */
extern char *xstring_bytes2hex(const unsigned char *string, int len,
			       const char *delimiter);

/*
 * Dump byte string printable format
 * IN string - byte string to dump as printable format
 * IN len - number of bytes to dump
 * IN replace - character to replace printable characters
 * RET loggable string (must xfree()) or NULL on error
 */
extern char *xstring_bytes2printable(const unsigned char *string, int len,
				     const char replace);

/*
 * Return an xmalloc'd string in base64 format given a base64url string.
 */
extern char *xbase64_from_base64url(const char *in);

#endif /* !_XSTRING_H */
