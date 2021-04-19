#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>
#include <utime.h>

#define MAXNAME 20
#define MAXSIZE 1000

struct keys
{
    int h;
    int v;
    int i;
    int f;
    int p;
};


void print_man()
{
  printf("cp - copy files and directories\n");
  printf("use <cp [OPTION] SOURCE DEST>\n");
  printf("OPTIONS:\n");
  printf("-h --help display this help and exit\n");
  printf("-v --verbose explain what is being done\n");
  printf("-i --interactive prompt before overwrite\n");
  printf("-f --force if an existing destination file cannot be opened, remove it and try again\n");
  printf("-p --preserve preserve the specified attributes (default: st_mode, st_atime, st_mtime, uid, gid)\n");
}


int interact()
{
  char ans;
  printf("Continue copying source file to destination file? y/n \n");
  scanf("%c", &ans);
  if (ans == 'y')
    return 1;
  return 0;
}


int cp_meta(char* sourcename, char* destname)
{
  struct stat mystat;
  struct stat mystatd;
  struct utimbuf times;
  char sdate_a[20], sdate_m[20], ddate_a[20], ddate_m[20];

  uid_t newuid;
  gid_t newgid;
  mode_t newmode;

  stat(destname, &mystatd);
  printf("dest:%d %d %d\n", mystatd.st_uid, mystatd.st_gid, mystatd.st_mode);
  strftime(ddate_a, 20, "%b\t%d\t%H:%M\t", localtime(&(mystatd.st_atime)));
  strftime(ddate_m, 20, "%b\t%d\t%H:%M\t", localtime(&(mystatd.st_mtime)));
  printf("%s %s\n", ddate_a, ddate_m);

  stat(sourcename, &mystat);

  newuid = mystat.st_uid;
  newgid = mystat.st_gid;
  newmode = mystat.st_mode;
  times.actime = mystat.st_atime;
  times.modtime = mystat.st_mtime;


  printf("source:%d %d %d\n", newuid, newgid, newmode);
  strftime(sdate_a, 20, "%b\t%d\t%H:%M\t", localtime(&(mystat.st_atime)));
  strftime(sdate_m, 20, "%b\t%d\t%H:%M\t", localtime(&(mystat.st_mtime)));
  printf("%s %s\n", sdate_a, sdate_m);

  chmod(destname, newmode);
  chown(destname, newuid, newgid);
  utime(destname, &times);

  stat(destname, &mystatd);
  printf("dest:%d %d %d\n", mystatd.st_uid, mystatd.st_gid, mystatd.st_mode);
  strftime(ddate_a, 20, "%b\t%d\t%H:%M\t", localtime(&(mystatd.st_atime)));
  strftime(ddate_m, 20, "%b\t%d\t%H:%M\t", localtime(&(mystatd.st_mtime)));
  printf("%s %s\n", ddate_a, ddate_m);
  return 0;
}


int copy(char* sourcename, char* destname, struct keys key)
{
  FILE* source = fopen(sourcename, "r");
  FILE* dest = fopen(destname, "w");
  if (errno == EACCES)
  {
    if (key.f == 0)
    {
      perror("destination access error");
      exit(0);
    }
    remove(destname);
    dest = fopen(destname, "w");
  }
  char* buff = (char*)calloc(MAXSIZE, sizeof(char));
  int n_symb = fread(buff, sizeof(*buff), MAXSIZE, source);
  fwrite(buff, sizeof(*buff), n_symb, dest);

  if (key.p)
    cp_meta(sourcename, destname);

  fclose(source);
  fclose(dest);

  if (key.v)
    printf("%s > %s\n", sourcename, destname);
  return 0;
}


int main(int argc, char* argv[])
{
  struct keys key = {0, 0, 0, 0};
  static struct option long_options[] =
  {
      {"help", no_argument, NULL, 'h'},
      {"verbose", no_argument, NULL, 'v'},
      {"interactive", no_argument, NULL, 'i'},
      {"force", 0, NULL, 'f'},
      {"preserve", 0, NULL, 'p'},
      {NULL, no_argument, NULL, 0}
  };

  int get = getopt_long(argc, argv, "hvifp", long_options, NULL);
  while(get != -1)
  {
      switch(get) {
          case('h'): {key.h = 1; break;}
          case('v'): {key.v = 1; break;}
          case('i'): {key.i = 1; break;}
          case('f'): {key.f = 1; break;}
          case('p'): {key.p = 1; break;}
          default  :  {break;}
      }
      get = getopt_long(argc, argv, "hvifp", long_options, NULL);
  }

  if (key.h)
  {
    print_man();
    exit(0);
  }
  if (key.i)
  {
    int ans = interact();
    if (ans == 0)
      exit(0);
  }

  char* sourcename = (char*)calloc(MAXNAME,sizeof(char));
  char* destname = (char*)calloc(MAXNAME,sizeof(char));
  strcpy(sourcename, argv[optind]);
  strcpy(destname, argv[optind + 1]);
  copy(sourcename, destname, key);
  free(sourcename);
  free(destname);
  return 0;
}
