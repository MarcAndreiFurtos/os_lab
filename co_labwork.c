 #include <stdio.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <string.h>

 void directory(DIR *dirp){
    struct dirent *dirRes = malloc(sizeof(struct dirent));
    if(!dirRes){
        free(dirRes);
        return;
    }
    while ((dirRes = readdir(dirp)) != NULL){
        const int len = strlen(dirRes->d_name);
        if ( len > 0 && dirRes->d_name[len-1] == 'c' ){
            printf("%s\n",dirRes->d_name);
        }
    }
 }




 int main(int argc,char *argv[]){
    if(argc<2){
        printf("Atleast 1 arguments is required by the program <file/dir/symlink path>\n");
        exit(EXIT_FAILURE);
    }

    int select;
    printf("please select the type which your path points to\n");
    printf("1 for directory\n");
    printf("2 for regular file\n");
    printf("3 for symlink\n");
    scanf("%d",&select);

    if(select == 1){
        printf("the path you have chosen is a directory\n");
        DIR *dirp = opendir(argv[1]);
        directory(dirp);
    }

 }
