/* Multithread-safety test for setlocale_null (LC_xxx, ...).
   Copyright (C) 2019 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

/* Specification.  */
#include <locale.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "glthread/thread.h"


/* Some common locale names.  */

#if defined _WIN32 && !defined __CYGWIN__
# define ENGLISH "English_United States"
# define GERMAN  "German_Germany"
# define FRENCH  "French_France"
# define ENCODING ".1252"
#else
# define ENGLISH "en_US"
# define GERMAN  "de_DE"
# define FRENCH  "fr_FR"
# if defined __sgi
#  define ENCODING ".ISO8859-15"
# elif defined __hpux
#  define ENCODING ".utf8"
# else
#  define ENCODING ".UTF-8"
# endif
#endif

static const char LOCALE1[] = ENGLISH ENCODING;
static const char LOCALE2[] = GERMAN ENCODING;
static const char LOCALE3[] = FRENCH ENCODING;

static char *expected;

static void *
thread1_func (void *arg)
{
  for (;;)
    {
      char buf[SETLOCALE_NULL_MAX];

      if (setlocale_null (LC_NUMERIC, buf, sizeof (buf)))
        abort ();
      if (strcmp (expected, buf) != 0)
        {
          fprintf (stderr, "thread1 disturbed by thread2!\n"); fflush (stderr);
          abort ();
        }
    }

  /*NOTREACHED*/
  return NULL;
}

static void *
thread2_func (void *arg)
{
  for (;;)
    {
      char buf[SETLOCALE_NULL_MAX];

      setlocale_null (LC_NUMERIC, buf, sizeof (buf));
      setlocale_null (LC_TIME, buf, sizeof (buf));
    }

  /*NOTREACHED*/
  return NULL;
}

int
main (int argc, char *argv[])
{
  if (setlocale (LC_ALL, LOCALE1) == NULL)
    {
      fprintf (stderr, "Skipping test: LOCALE1 not recognized\n");
      return 77;
    }
  if (setlocale (LC_NUMERIC, LOCALE2) == NULL)
    {
      fprintf (stderr, "Skipping test: LOCALE2 not recognized\n");
      return 77;
    }
  if (setlocale (LC_TIME, LOCALE3) == NULL)
    {
      fprintf (stderr, "Skipping test: LOCALE3 not recognized\n");
      return 77;
    }

  expected = strdup (setlocale (LC_NUMERIC, NULL));

  /* Create the two threads.  */
  gl_thread_create (thread1_func, NULL);
  gl_thread_create (thread2_func, NULL);

  /* Let them run for 2 seconds.  */
  {
    struct timespec duration;
    duration.tv_sec = 2;
    duration.tv_nsec = 0;

    nanosleep (&duration, NULL);
  }

  return 0;
}

/* Without locking, the results of this test would be:
glibc                OK
musl libc            OK
macOS                OK
FreeBSD              OK
NetBSD               OK
OpenBSD              crash < 1 sec
AIX                  crash < 2 sec
HP-UX                OK
IRIX                 OK
Solaris 10           OK
Solaris 11.0         OK
Solaris 11.4         OK
Solaris OpenIndiana  OK
Haiku                OK
Cygwin               OK
mingw                OK
MSVC                 OK (assuming compiler option /MD !)
*/