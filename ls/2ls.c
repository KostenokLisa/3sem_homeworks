#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <getopt.h>

#define MAXNAME 20


struct keys
{
    int l;
    int a;
    int R;
};

struct owner
{
  char* name;
  char* group;
};

int num_of_file(char* dir_name)
{
    int file_num = 0;
    DIR* dir;
    struct dirent* file;

    dir = opendir(dir_name);
    if(dir == NULL)
        perror("Opendir in num_of_file\n");


    while (((file = readdir(dir)) != NULL))
        file_num++;

    closedir(dir);

    return file_num;
}


char* next_dir(char* name1, char* name2)
{
    char* buf = (char*) calloc (strlen(name1) + strlen(name2) + 2, sizeof(char));
    strcat(buf, name1);
    strcat(buf, "/");
    strcat(buf, name2);
    return buf;
}

void ln_print(struct stat* mystat , char* myfile, struct keys key, struct owner* person)
{

    if(S_ISREG(mystat->st_mode))
        printf("-");
    else
    {
        if(S_ISDIR(mystat->st_mode))
            printf("d");

        if(S_ISLNK(mystat->st_mode))
            printf("l");
    }
    printf((mystat->st_mode & S_IRUSR) ? "r" : "-");
    printf((mystat->st_mode & S_IWUSR) ? "w" : "-");
    printf((mystat->st_mode & S_IXUSR) ? "x" : "-");
    printf((mystat->st_mode & S_IRGRP) ? "r" : "-");
    printf((mystat->st_mode & S_IWGRP) ? "w" : "-");
    printf((mystat->st_mode & S_IXGRP) ? "x" : "-");
    printf((mystat->st_mode & S_IROTH) ? "r" : "-");
    printf((mystat->st_mode & S_IWOTH) ? "w" : "-");
    printf((mystat->st_mode & S_IXOTH) ? "x" : "-");
    printf("\t");


    printf("%ld\t",mystat->st_nlink);

    //printf("%d\t", mystat->st_uid);
    //printf("%d\t", mystat->st_gid);
    printf("%s\t", person->name);
    printf("%s\t", person->group);


    printf("%ld\t",mystat->st_size);

    char date[50];
    time_t now = time(0);

    if((localtime(&(mystat->st_mtime)))->tm_year != localtime(&now)->tm_year)
    {
        strftime(date, 50, "%b\t%d\t%Y\t", localtime(&(mystat->st_mtime)));
        printf("%s\t", date);
    }
    else
    {
        strftime(date, 50, "%b\t%d\t%H:%M\t", localtime(&(mystat->st_mtime)));
        printf("%s\t", date);
    }

    printf("%s", myfile);


    /*if(S_ISLNK(mystat->st_mode))
    {
        int charnumber = readlink(myfile, buf, MAXNAME);
        buf[charnumber] = '\0';
        printf("%s -> %s\n", myfile, buf);
    }*/
    printf("\n");
}

void print(char* dir_name, char* myfile, DIR* mydir, struct keys key, struct owner* person)
{
    struct stat mystat;
    char buf[1024];

    sprintf(buf, "%s/%s", dir_name, myfile);
    lstat(buf, &mystat);

    if(key.l)
        ln_print(&mystat , myfile, key, person);
    else
        printf("%s\n", myfile);
}

void R_next_dir(char* dir_name , struct dirent* myfile , char** dirlist , int* i)
{
  struct stat mystat;
  char buf[1024];
  sprintf(buf, "%s/%s", dir_name, myfile->d_name);
  stat(buf, &mystat);
  if(S_ISDIR(mystat.st_mode) && strcmp(myfile->d_name, ".")
                               && strcmp(myfile->d_name, ".."))
  {
    strcpy(dirlist[*i], myfile->d_name);
    (*i)++;
  }
}


void print_dir(char* dir_name, struct keys key, struct owner* person)
{
    DIR* mydir;
    struct dirent* myfile;

    int i = 0;
    int dn = num_of_file(dir_name);
    char** dirlist = (char**) calloc (dn, sizeof(char*));
    while(i < dn)
    {
      dirlist[i] = (char*) calloc(MAXNAME, sizeof(char));
      i++;
    }
    i = 0;

    mydir = opendir(dir_name);
    if(mydir == NULL)
      perror("Opendir in prin_dir");


      while(myfile = readdir(mydir))
      {
        if(key.a)
        {
          print(dir_name, myfile->d_name, mydir, key, person);
          if(key.R)
            R_next_dir(dir_name , myfile , dirlist , &i);
        }
        else if((myfile->d_name)[0] != '.')
        {
          print(dir_name, myfile->d_name, mydir, key, person);
          if(key.R)
            R_next_dir(dir_name , myfile , dirlist , &i);
        }
      }
    closedir(mydir);
    printf("\n");

    if(key.R)
    {
      int k = 0;
      while(k < i)
      {
        printf("%s:\n",dirlist[k]);
        print_dir(next_dir(dir_name, dirlist[k]), key, person);
        k++;
      }
    }

    i = 0;
    while(i < dn)
    {
      free(dirlist[i]);
      i++;
    }
    free(dirlist);
    free(dir_name);

    printf("\n");
}


int main(int argc, char* argv[])
{
    struct keys key = {0, 0, 0};
    static struct option long_options[] =
    {
        {"all", no_argument, NULL, 'a'},
        {"long", no_argument, NULL, 'l'},
        {"recursive", no_argument, NULL, 'R'},
        {NULL, no_argument, NULL, 0}
    };
    int get = getopt_long(argc, argv, "alR", long_options, NULL);

    while(get != -1)
        {
        switch(get) {
            case('a'): {key.a = 1; break;}
            case('l'): {key.l = 1; break;}
            case('R'): {key.R = 1; break;}
            default  :  {break;}
        }

        get = getopt_long(argc, argv, "alR", long_options, NULL);
    }

    uid_t my_id = getuid();
    gid_t my_group = getgid();

    struct passwd* str_1 = getpwuid(my_id);
		struct group* str_2 = getgrgid(my_group);
    struct owner* person = (struct owner*)calloc(1, sizeof(*person));
    person->name = str_1->pw_name;
    person->group = str_2->gr_name;

    char* dirname = (char*)calloc(MAXNAME,sizeof(char));

    if(argv[optind] == NULL)
        strcpy(dirname,".");

    else
        strcpy(dirname,argv[optind]);

    print_dir(dirname, key, person);
    return 0;
}
