#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define VERSION "0.3b"

// get time from pts
void show_ts (double f) {
  time_t ts = f / 90000;
  struct tm *tmp = gmtime (&ts);

  printf ("%02i:%02i:%02i", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

// resolve pts type
void get_ptstype (int i, char *txt) {
  switch (i)
    {
    case 0:
      sprintf (txt, "in");
      break;
    case 1:
      sprintf (txt, "out");
      break;
    case 2:
      sprintf (txt, "mark");
      break;
    case 3:
      sprintf (txt, "last play position");
      break;
    default:
      sprintf (txt, "unknown");
      break;
    }
}

int main (int argc, char *argv[]) {
  unsigned char barray[12];
  uint64_t pts;
  uint32_t type;
  char txt[20] = { "0" };
  FILE *fp;
  off_t f_size;

  if (argc < 2)
    {
      printf ("cutsreader Version %s\n", VERSION);
      printf ("Usage: %s <filename>\n", argv[0]);
      return EXIT_FAILURE;
    }

  fp = fopen (argv[1], "rb");
  if (!fp)
    {
      if (errno)
        {
          perror("error opening file");
        }
      else
        {
          fprintf (stderr, "read error\n");
        }
      return EXIT_FAILURE;
    }

  fseeko(fp, 0, SEEK_END);
  f_size = ftello(fp);
  rewind(fp);
  if (f_size > (1 << 12))
    {
      fprintf (stderr, "ERROR: unreasonably large file, size %jd\n", (intmax_t)f_size);
      fclose (fp);
      return EXIT_FAILURE;
    }
  if (f_size%12)
    {
      fprintf (stderr, "ERROR: wrong file format - size not divisible by 12\n");
      fclose (fp);
      return EXIT_FAILURE;
    }

  while (fread (&barray[0], 12, 1, fp) == 1)
    {
      pts = ((uint64_t) barray[0] << 56) |	// cast to get rid of gcc warning
	    ((uint64_t) barray[1] << 48) |
	    ((uint64_t) barray[2] << 40) |
    	    ((uint64_t) barray[3] << 32) |
    	    (barray[4] << 24) |
    	    (barray[5] << 16) | 
	    (barray[6] << 8) | 
	    barray[7];
      type = (barray[8] << 24) |
	      (barray[9] << 16) | (barray[10] << 8) | barray[11];

      // result
      printf ("PTS: ");
      show_ts ((float) pts);
      get_ptstype (type, txt);
      printf ("\tType: %s (%i)\n", txt, type);
    }

  fclose (fp);

  return EXIT_SUCCESS;
}
