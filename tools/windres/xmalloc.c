#include <stdlib.h>
#include <stdio.h>

/* The program name if set.  */
static const char *name = "";

#ifdef HAVE_SBRK
/* The initial sbrk, set when the program name is set. Not used for win32
   ports other than cygwin32.  */
static char *first_break = NULL;
#endif /* HAVE_SBRK */

void
xmalloc_set_program_name (s)
     const char *s;
{
  name = s;
#ifdef HAVE_SBRK
  /* Win32 ports other than cygwin32 don't have brk() */
  if (first_break == NULL)
    first_break = (char *) sbrk (0);
#endif /* HAVE_SBRK */
}
