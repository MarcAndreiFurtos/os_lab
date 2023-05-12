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

int directory(DIR *dirp) {
  int count = 0;
  struct dirent *dirRes = malloc(sizeof(struct dirent));
  if (!dirRes) {
    free(dirRes);
    return 0;
  }
  while ((dirRes = readdir(dirp)) != NULL) {
    const int len = strlen(dirRes->d_name);
    if (len > 0 && dirRes->d_name[len - 1] == 'c') {
      printf("%s\n", dirRes->d_name);
      count++;
    }
  }
  return count;
}

void printAccessRights(struct stat *file) {
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

void createSymlink(char filename[]) {
  puts("Enter linkname: ");
  char linkname[100];
  scanf("%s", linkname);
  symlink(filename, linkname);
}

void printLinkedFile(char filename[]) {
  char linkname[100];
  readlink(filename, linkname, 100);
  struct stat buff;
  lstat(linkname, &buff);
  printf("Size of target: %ld\n", buff.st_size);
}

int handleDirectory(char filename[]) {
  DIR *dir;
  dir = opendir(filename);
  int count = directory(dir);
  return count;
}

void handleMenu(char filename[], struct stat buff) {
  char input[10];
  if (S_ISREG(buff.st_mode)) {
    printf("Regular file: %s\nEnter options:\n-n (file name)\n-d(dim/size)\n-h "
           "(number of hard links)\n-m (time of last modif)\n-a (access "
           "rights)\n-l [filename] (create a symbolic link)\n",
           filename);
    fgets(input, 10, stdin);
    char options[10];
    sscanf(input, "-%10s", options);
    for (int i = 0; i < strlen(options); i++) {
      switch (options[i]) {
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
  } else if (S_ISLNK(buff.st_mode)) {
    printf("Symbolic link: %s\nEnter options:\n-n (link name)\n-l (delete "
           "link)\n-d (size of link)\n-z (size of target)\n-a (access rights "
           "for symbolic link)\n",
           filename);
    fgets(input, 10, stdin);
    char options[10];
    sscanf(input, "-%10s", options);
    for (int i = 0; i < strlen(options); i++) {
      switch (options[i]) {
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
  } else if (S_ISDIR(buff.st_mode)) {
    printf("Directory: %s\nEnter options:\n-n (name)\n-d(dim/size)\n-a (access "
           "rights)\n-c (total number of .c files)\n",
           filename);
    fgets(input, 10, stdin);
    char options[10];
    sscanf(input, "-%10s", options);
    for (int i = 0; i < strlen(options); i++) {
      switch (options[i]) {
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

bool isCFile(char *filename) {
  int len = strlen(filename);
  if (len > 0 && filename[len - 1] == 'c') {
    return true;
  }
  return false;
}

bool isFile(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) != 0) {
    // Unable to get file stats, return false
    return false;
  }
  return S_ISREG(path_stat.st_mode);
}

int countLines(const char *filename) {
  FILE *fp;
  int lines = 1;
  int c;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    // Unable to open file, return -1
    return -1;
  }

  // Read the file character by character and count the number of newlines
  while ((c = fgetc(fp)) != EOF) {
    if (c == '\n') {
      lines++;
    }
  }

  fclose(fp);

  return lines;
}

bool isSymbolicLink(const char *path) {
  struct stat path_stat;
  if (lstat(path, &path_stat) != 0) {
    // Unable to get file stats, return false
    return false;
  }
  return S_ISLNK(path_stat.st_mode);
}

int change_link_permissions(const char *path) {
  // Set the permissions for the symbolic link
  if (chmod(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP) == -1) {
    return -1; // Return -1 on error
  }

  return 0; // Return 0 on success
}

int parse_output(const char *output, int *num1, int *num2) {
  // Parse the first number
  *num1 = atoi(output);
  // Find the end of the first line
  const char *end_of_first_line = strchr(output, '\n');
  // If there is no second line, return an error
  if (!end_of_first_line || *(end_of_first_line + 1) == '\0') {
    return -1;
  }
  // Parse the second number
  *num2 = atoi(end_of_first_line + 1);
  return 0;
}

void compute_score(const char *file_name, int num_errors, int num_warnings) {
  int score;
  if (num_errors == 0 && num_warnings == 0) {
    score = 10;
  } else if (num_errors > 0) {
    score = 1;
  } else if (num_warnings > 10) {
    score = 2;
  } else {
    score = 2 + 8 * (10 - num_warnings) / 10;
  }

  FILE *fp = fopen("grades.txt", "a");
  if (fp == NULL) {
    printf("Error: failed to open grades.txt for writing.\n");
    return;
  }
  printf("%s: %d\n", file_name, score);
  fprintf(fp, "%s: %d\n", file_name, score);
  fclose(fp);
}

void create_file_in_dir(const char *dir_name) {
  char *file_name =
      malloc(strlen(dir_name) + 10); // Allocate memory for the file name
  sprintf(file_name, "%s_file.txt", dir_name); // Construct the file name
  char *file_path = malloc(strlen(dir_name) + strlen(file_name) +
                           2); // Allocate memory for the file path
  sprintf(file_path, "%s/%s", dir_name, file_name); // Construct the file path
  int fd = open(file_path, O_CREAT | O_WRONLY,
                0644); // Create the file with read/write permissions for user
                       // and group, and read permissions for others
  if (fd == -1) {
    perror("Error creating file"); // Print an error message if the file could
                                   // not be created
    exit(EXIT_FAILURE);
  }
  close(fd); // Close the file
  printf("File '%s' created in directory '%s'.\n", file_name,
         dir_name); // Print a success message
  free(file_name);  // Free the memory allocated for the file name
  free(file_path);  // Free the memory allocated for the file path
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s [file1 file2 ...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int a = 0;
  int num[2] = {0, 0};

  for (int i = 1; i < argc; i++) {
    // child process
    struct stat buff;
    int result =
        lstat(argv[i], &buff); // returns 0 on success and -1 on failure

    if (result == 0) {
      if (isCFile(argv[i])) {
        int pipa[2];
        pipe(pipa);
        pid_t pid = fork(); // create a child process

        if (pid == -1) {
          printf("Failed to create a child process.\n");
          exit(EXIT_FAILURE);
        } else if (pid == 0) {
          close(pipa[0]);
          char *args[] = {"./compileCfile.sh", argv[i], NULL};
          dup2(pipa[1], STDOUT_FILENO);
          execvp(args[0], args);
          exit(EXIT_SUCCESS);
        }
        close(pipa[1]);
        FILE *fp = fdopen(pipa[0], "r");
        char line[100];
        while (fgets(line, 100, fp) != NULL) {
          printf("%s", line);
          sscanf(line, "%*[^0-9]%d", &num[a]);
          a++;
        }
        compute_score(argv[i], num[0], num[1]);
      } else if (isFile(argv[i]) && isCFile(argv[i]) == false) {
        pid_t pid = fork(); // create a child process
        if (pid == -1) {
          printf("Failed to create a child process.\n");
          exit(EXIT_FAILURE);
        } else if (pid == 0) {
          printf("%d\n", countLines(argv[i]));
          handleMenu(argv[i], buff);
          exit(EXIT_SUCCESS);
        }
      } else if (isSymbolicLink(argv[i])) {
        pid_t pid = fork(); // create a child process
        if (pid == -1) {
          printf("Failed to create a child process.\n");
          exit(EXIT_FAILURE);
        } else if (pid == 0) {
          change_link_permissions(argv[i]);
          handleMenu(argv[i], buff);
          exit(EXIT_SUCCESS);
        }
      } else {
        pid_t pid = fork(); // create a child process
        if (pid == -1) {
          printf("Failed to create a child process.\n");
          exit(EXIT_FAILURE);
        } else if (pid == 0) {
          create_file_in_dir(argv[i]);
          handleMenu(argv[i], buff);
          exit(EXIT_SUCCESS);
        }
      }
    } else {
      printf("Could not read info about file.\n");
    }
  }

  // wait for all child processes to finish
  for (int i = 1; i < argc; i++) {
    wait(NULL);
  }

  return 0;
}
