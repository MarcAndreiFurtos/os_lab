#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int directory(DIR *dirp)
{
  int count = 0;
  struct dirent *dirRes = malloc(sizeof(struct dirent));
  if (!dirRes)
  {
    free(dirRes);
    return 0;
  }
  while ((dirRes = readdir(dirp)) != NULL)
  {
    const int len = strlen(dirRes->d_name);
    if (len > 0 && dirRes->d_name[len - 1] == 'c')
    {
      printf("%s\n", dirRes->d_name);
      count++;
    }
  }
  return count;
}

void printAccessRights(struct stat *file)
{
  printf("USER\n");
  printf("\tRead: %s\n", (file->st_mode & S_IRUSR) ? "Yes" : "No");
  printf("\tWrite: %s\n", (file->st_mode & S_IWUSR) ? "Yes" : "No");
  printf("\tExecute: %s\n", (file->st_mode & S_IXUSR) ? "Yes" : "No");
  printf("GROUP\n");
  printf("\tRead: %s\n", (file->st_mode & S_IRGRP) ? "Yes" : "No");
  printf("\tWrite: %s\n", (file->st_mode & S_IWGRP) ? "Yes" : "No");
  printf("\tExecute: %s\n", (file->st_mode & S_IXGRP) ? "Yes" : "No");
  printf("OTHER\n");
  printf("\tRead: %s\n", (file->st_mode & S_IROTH) ? "Yes" : "No");
  printf("\tWrite: %s\n", (file->st_mode & S_IWOTH) ? "Yes" : "No");
  printf("\tExecute: %s\n", (file->st_mode & S_IXOTH) ? "Yes" : "No");
}

void createSymlink(char filename[])
{
  puts("Enter linkname: ");
  char linkname[100];
  scanf("%s", linkname);
  symlink(filename, linkname);
}

void printLinkedFile(char filename[])
{
  char linkname[100];
  readlink(filename, linkname, 100);
  struct stat buff;
  lstat(linkname, &buff);
  printf("Size of target: %ld\n", buff.st_size);
}

int handleDirectory(char filename[])
{
  DIR *dir;
  dir = opendir(filename);
  int count = directory(dir);
  return count;
}

void handleMenu(char filename[], struct stat buff)
{
  char input[10];
  if (S_ISREG(buff.st_mode))
  {
    printf("Regular file: %s\nEnter options:\n-n (file name)\n-d(dim/size)\n-h "
           "(number of hard links)\n-m (time of last modif)\n-a (access "
           "rights)\n-l [filename] (create a symbolic link)\n",
           filename);
    fgets(input, 10, stdin);
    char options[10];
    sscanf(input, "-%10s", options);
    for (int i = 0; i < strlen(options); i++)
    {
      switch (options[i])
      {
      case 'n':
        printf("%s\n", filename);
        break;
      case 'd':
        printf("%ld\n", buff.st_size);
        break;
      case 'h':
        printf("%ld\n", buff.st_nlink);
        break;
      case 'm':
        printf("%s", ctime(&buff.st_mtime));
        break;
      case 'a':
        printAccessRights(&buff);
        break;
      case 'l':
        createSymlink(filename);
        break;
      default:
        break;
      }
    }
  }
  else if (S_ISLNK(buff.st_mode))
  {
    printf("Symbolic link: %s\nEnter options:\n-n (link name)\n-l (delete "
           "link)\n-d (size of link)\n-z (size of target)\n-a (access rights "
           "for symbolic link)\n",
           filename);
    fgets(input, 10, stdin);
    char options[10];
    sscanf(input, "-%10s", options);
    for (int i = 0; i < strlen(options); i++)
    {
      switch (options[i])
      {
      case 'n':
        printf("%s\n", filename);
        break;
      case 'l':
        unlink(filename);
        break;
      case 'd':
        printf("%ld\n", buff.st_size);
        break;
      case 'z':
        printLinkedFile(filename);
        break;
      case 'a':
        printAccessRights(&buff);
        break;
      default:
        break;
      }
    }
  }
  else if (S_ISDIR(buff.st_mode))
  {
    printf("Directory: %s\nEnter options:\n-n (name)\n-d(dim/size)\n-a (access "
           "rights)\n-c (total number of .c files)\n",
           filename);
    fgets(input, 10, stdin);
    char options[10];
    sscanf(input, "-%10s", options);
    for (int i = 0; i < strlen(options); i++)
    {
      switch (options[i])
      {
      case 'n':
        printf("%s\n", filename);
        break;
      case 'd':
        printf("%ld\n", buff.st_size);
        break;
      case 'a':
        printAccessRights(&buff);
        break;
      case 'c':
        printf("%d\n", handleDirectory(filename));
        break;
      default:
        break;
      }
    }
  }
}

bool isCFile(char *filename)
{
  int len = strlen(filename);
  if (len > 0 && filename[len - 1] == 'c')
  {
    return true;
  }
  return false;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s [file1 file2 ...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  for (int i = 1; i < argc; i++)
  {
    // child process
    struct stat buff;
    int result =
        lstat(argv[i], &buff); // returns 0 on success and -1 on failure

    if (result == 0)
    {
      if (isCFile(argv[i]))
      {
        pid_t pid = fork(); // create a child process
        if (pid == -1)
        {
          printf("Failed to create a child process.\n");
          exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
          // char command[100];
          // sprintf(command, "./compileCfile.sh %s", argv[i]);
          // system(command);
          char *args[] = {"./compileCfile.sh",argv[i],NULL};
          execvp(args[0],args);
          exit(EXIT_SUCCESS);
        }
      }
      else
      {
        pid_t pid = fork(); // create a child process
        if (pid == -1)
        {
          printf("Failed to create a child process.\n");
          exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
          handleMenu(argv[i], buff);
          exit(EXIT_SUCCESS);
        }
      }
    }
    else
    {
      printf("Could not read info about file.\n");
    }
  }

  // wait for all child processes to finish
  for (int i = 1; i < argc; i++)
  {
    wait(NULL);
  }

  return 0;
}
