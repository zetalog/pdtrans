#include <stdio.h>	/* May get P_tmpdir.  */
#include <stdlib.h>
#include <string.h>
#include <io.h>

#ifndef R_OK
#define R_OK 4
#define W_OK 2
#define X_OK 1
#endif

#include "libiberty.h"
extern char *choose_tmpdir (void);

/* '/' works just fine on MS-DOS based systems.  */
#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR '/'
#endif

/* Name of temporary file.
   mktemp requires 6 trailing X's.  */
#define TEMP_FILE "ccXXXXXX"
#define TEMP_FILE_LEN (sizeof(TEMP_FILE) - 1)

/*

@deftypefn Extension char* choose_temp_base (void)

Return a prefix for temporary file names or @code{NULL} if unable to
find one.  The current directory is chosen if all else fails so the
program is exited if a temporary directory can't be found (@code{mktemp}
fails).  The buffer for the result is obtained with @code{xmalloc}.

This function is provided for backwards compatability only.  Its use is
not recommended.

@end deftypefn

*/

char *
choose_temp_base ()
{
  const char *base = choose_tmpdir ();
  char *temp_filename;
  int len;

  len = strlen (base);
  temp_filename = malloc (len + TEMP_FILE_LEN + 1);
  strcpy (temp_filename, base);
  strcpy (temp_filename + len, TEMP_FILE);

  mktemp (temp_filename);
  if (strlen (temp_filename) == 0)
    abort ();
  return temp_filename;
}

/* Subroutine of choose_tmpdir.
   If BASE is non-NULL, return it.
   Otherwise it checks if DIR is a usable directory.
   If success, DIR is returned.
   Otherwise NULL is returned.  */

static const char *xtry (const char *, const char *);

static const char *
xtry (dir, base)
     const char *dir, *base;
{
  if (base != 0)
    return base;
  if (dir != 0
      && access (dir, R_OK | W_OK | X_OK) == 0)
    return dir;
  return 0;
}

static const char tmp[] = { DIR_SEPARATOR, 't', 'm', 'p', 0 };
static const char usrtmp[] =
{ DIR_SEPARATOR, 'u', 's', 'r', DIR_SEPARATOR, 't', 'm', 'p', 0 };
static const char vartmp[] =
{ DIR_SEPARATOR, 'v', 'a', 'r', DIR_SEPARATOR, 't', 'm', 'p', 0 };

static char *memoized_tmpdir;

/*

@deftypefn Replacement char* choose_tmpdir ()

Returns a pointer to a directory path suitable for creating temporary
files in.

@end deftypefn

*/

char *
choose_tmpdir ()
{
  const char *base = 0;
  char *tmpdir;
  unsigned int len;

  if (memoized_tmpdir)
    return memoized_tmpdir;

  base = xtry (getenv ("TMPDIR"), base);
  base = xtry (getenv ("TMP"), base);
  base = xtry (getenv ("TEMP"), base);

#ifdef P_tmpdir
  base = xtry (P_tmpdir, base);
#endif

  /* Try /var/tmp, /usr/tmp, then /tmp.  */
  base = xtry (vartmp, base);
  base = xtry (usrtmp, base);
  base = xtry (tmp, base);
 
  /* If all else fails, use the current directory!  */
  if (base == 0)
    base = ".";

  /* Append DIR_SEPARATOR to the directory we've chosen
     and return it.  */
  len = strlen (base);
  tmpdir = malloc (len + 2);
  strcpy (tmpdir, base);
  tmpdir[len] = DIR_SEPARATOR;
  tmpdir[len+1] = '\0';

  memoized_tmpdir = tmpdir;
  return tmpdir;
}
