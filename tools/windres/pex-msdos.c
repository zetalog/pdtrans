#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "libiberty.h"
#include "safe-ctype.h"
#include <process.h>

/* MSDOS doesn't multitask, but for the sake of a consistent interface
   the code behaves like it does.  pexecute runs the program, tucks the
   exit code away, and returns a "pid".  pwait must be called to fetch the
   exit code.  */

/* For communicating information from pexecute to pwait.  */
static int last_pid = 0;
static int last_status = 0;
static int last_reaped = 0;

int
pexecute (program, argv, this_pname, temp_base, errmsg_fmt, errmsg_arg, flags)
     const char *program;
     char * const *argv;
     const char *this_pname;
     const char *temp_base;
     char **errmsg_fmt, **errmsg_arg;
     int flags;
{
  int rc;
  char *scmd, *rf;
  FILE *argfile;
  int i, el = flags & PEXECUTE_SEARCH ? 4 : 0;

  last_pid++;
  if (last_pid < 0)
    last_pid = 1;

  if ((flags & PEXECUTE_ONE) != PEXECUTE_ONE)
    abort ();

  if (temp_base == 0)
    temp_base = choose_temp_base ();
  scmd = (char *) malloc (strlen (program) + strlen (temp_base) + 6 + el);
  rf = scmd + strlen(program) + 2 + el;
  sprintf (scmd, "%s%s @%s.gp", program,
	   (flags & PEXECUTE_SEARCH ? ".exe" : ""), temp_base);
  argfile = fopen (rf, "w");
  if (argfile == 0)
    {
      int errno_save = errno;
      free (scmd);
      errno = errno_save;
      *errmsg_fmt = "cannot open `%s.gp'";
      *errmsg_arg = temp_base;
      return -1;
    }

  for (i=1; argv[i]; i++)
    {
      char *cp;
      for (cp = argv[i]; *cp; cp++)
	{
	  if (*cp == '"' || *cp == '\'' || *cp == '\\' || ISSPACE (*cp))
	    fputc ('\\', argfile);
	  fputc (*cp, argfile);
	}
      fputc ('\n', argfile);
    }
  fclose (argfile);

  rc = system (scmd);

  {
    int errno_save = errno;
    remove (rf);
    free (scmd);
    errno = errno_save;
  }

  if (rc == -1)
    {
      *errmsg_fmt = install_error_msg;
      *errmsg_arg = (char *)program;
      return -1;
    }

  /* Tuck the status away for pwait, and return a "pid".  */
  last_status = rc << 8;
  return last_pid;
}

/* Use ECHILD if available, otherwise use EINVAL.  */
#ifdef ECHILD
#define PWAIT_ERROR ECHILD
#else
#define PWAIT_ERROR EINVAL
#endif

int
pwait (pid, status, flags)
     int pid;
     int *status;
     int flags;
{
  /* On MSDOS each pexecute must be followed by its associated pwait.  */
  if (pid != last_pid
      /* Called twice for the same child?  */
      || pid == last_reaped)
    {
      errno = PWAIT_ERROR;
      return -1;
    }
  /* ??? Here's an opportunity to canonicalize the values in STATUS.
     Needed?  */
  *status = last_status;
  last_reaped = last_pid;
  return last_pid;
}
