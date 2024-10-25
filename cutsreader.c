#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define VERSION "0.3b"

#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) \
          ( \
          ((x) >> 56) | \
          (((x) << 40) & 0x00FF000000000000) | \
          (((x) << 24) & 0x0000FF0000000000) | \
          (((x) << 8) & 0x000000FF00000000) | \
          (((x) >> 8) & 0x00000000FF000000) | \
          (((x) >> 24) & 0x0000000000FF0000) | \
          (((x) >> 40) & 0x000000000000FF00) | \
          ((x) << 56) \
          )
# define ntohll(x) htonll(x)
#endif

// get time from pts
const int MPEGTS_TICKS_PER_SECOND = 90000;
const int MPEGTS_TICKS_PER_MILLISECOND = 90;

void show_ts (uint64_t f) {
  time_t ts = f / MPEGTS_TICKS_PER_SECOND;
  int millis = (f - MPEGTS_TICKS_PER_SECOND * ts) / MPEGTS_TICKS_PER_MILLISECOND;
  struct tm *tmp = gmtime (&ts);

  printf ("%02i:%02i:%02i.%03i", tmp->tm_hour, tmp->tm_min, tmp->tm_sec, millis);
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
      uint64_t pts_bytes;
      memcpy((uint8_t *)&pts_bytes, barray, sizeof(pts));
      const uint64_t pts = ntohll(pts_bytes);
      type = (barray[8] << 24) |
	      (barray[9] << 16) | (barray[10] << 8) | barray[11];

      // result
      printf ("PTS: ");
      show_ts (pts);
      get_ptstype (type, txt);
      printf ("\tType: %s (%i)\n", txt, type);
    }

  fclose (fp);

  return EXIT_SUCCESS;
}
