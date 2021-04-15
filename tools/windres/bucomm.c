#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/* Error reporting.  */

char *program_name;

#define VA_START(VA_LIST, VAR)	va_start(VA_LIST, VAR)

/* variadic function helper macros */
/* "struct Qdmy" swallows the semicolon after VA_OPEN/VA_FIXEDARG's
   use without inhibiting further decls and without declaring an
   actual variable.  */
#define VA_OPEN(AP, VAR)	{ va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)		} va_end(AP); }
#define VA_FIXEDARG(AP, T, N)	struct Qdmy

void
report (const char * format, va_list args)
{
  fprintf (stderr, "%s: ", program_name);
  vfprintf (stderr, format, args);
  putc ('\n', stderr);
}

void
fatal (const char *format, ...)
{
  VA_OPEN (args, format);
  VA_FIXEDARG (args, const char *, format);

  report (format, args);
  VA_CLOSE (args);
  exit (1);
}

void
non_fatal (const char *format, ...)
{
  VA_OPEN (args, format);
  VA_FIXEDARG (args, const char *, format);

  report (format, args);
  VA_CLOSE (args);
}
