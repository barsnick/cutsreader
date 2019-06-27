#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define VERSION "0.2b"

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
      sprintf (txt, "in\0");
      break;
    case 1:
      sprintf (txt, "out\0");
      break;
    case 2:
      sprintf (txt, "mark\0");
      break;
    case 3:
      sprintf (txt, "last play position\0");
      break;
    default:
      sprintf (txt, "unknown\0");
      break;
    }
}

// read file into array
int read_file (char *argv, unsigned char *barray) {
  int i = 0;
  FILE *fp;

  fp = fopen (argv, "rb");
  if (!fp)
    {
      printf ("read error\n");
      return -1;
    }

  while (fread (&barray[i++], 1, 1, fp) == 1)
    {
    if (i == 119) 
      {
        printf ("error in file\n");
        fclose (fp);
        return -1;
      }
  }
  fclose (fp);

  return i;
}

int main (int argc, char *argv[]) {
  unsigned char barray[120] = { "0" };
  int i = 0, j = 0;
  uint64_t pts;
  uint32_t type;
  char txt[20] = { "0" };

  if (argc < 2)
    {
      printf ("cutsreader Version %s\n", VERSION);
      printf ("Usage: %s <filename>\n", argv[0]);
      return -1;
    }

  i = read_file (argv[1], barray);
  if (i < 0)
    return i;			//file error

  // transform big endian into readable values
  // .cuts file structure: pts(64bit)type(32bit)[...] 
  // TODO: check endianess at startup
  i--;

  while (j < i) {
      pts = ((uint64_t) barray[j] << 56) |	// cast to get rid of gcc warning
	      ((uint64_t) barray[j + 1] << 48) |
	      ((uint64_t) barray[j + 2] << 40) |
        ((uint64_t) barray[j + 3] << 32) |
        (barray[j + 4] << 24) |
        (barray[j + 5] << 16) | (barray[j + 6] << 8) | barray[j + 7];
      type = (barray[j + 8] << 24) |
	      (barray[j + 9] << 16) | (barray[j + 10] << 8) | barray[j + 11];

      // result
      printf ("PTS: ");
      show_ts ((float) pts);
      get_ptstype (type, txt);
      printf ("\tType: %s (%i)\n", txt, type);
      j = j + 12;
  }

  return 0;
}
