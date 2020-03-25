//
// Created by werka on 3/16/20.
//
// hard link - stworzenie nowej nazwy na istniejący już plik, bez jego kopopwania
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

struct time_info{
    int undeclared;
    int sign;
    int value;
};

struct time_info * atime;
struct time_info * mtime;
int maxdepth = -2;
int top_catalog = 1;
int catalog_path_len;


int diff_day(time_t t) {
    return (int) difftime(time(NULL), t) / 86400;
}

void to_time_info(struct time_info * t, char * value){
    if(value[0]=='+'){
        t->sign = 1;
    }
    else if(value[0]=='-'){
        t->sign = -1;
    }
    else{
        t->sign = 0;
    }

    t->value = abs(atoi(value));
    t->undeclared = 0;
}

char * get_file_type(mode_t st_mode){
    switch(st_mode & S_IFMT){ //bitwise and
        case S_IFREG:
            return "file";
        case S_IFDIR:
            return "dir";
        case S_IFBLK:
            return "block dev";
        case S_IFCHR:
            return "char dev";
        case S_IFSOCK:
            return "sock";
        case S_IFLNK:
            return "slink";
        case S_IFIFO:
            return "fifo";
    }
    return "not known type";
}

//void file_information(const struct stat *file_stats, const char *file_path){
//    printf("Absolute path %s\n", file_path);
//    printf("Hard links %ld\n", file_stats->st_nlink);
//    printf("File type %s\n", get_file_type(file_stats->st_mode));
//    printf("File size %ld\n", file_stats->st_size);
//
//    char * buffer = (char*)calloc(43, sizeof(char));
//    struct tm* tm_info;
//
//    tm_info = localtime(&file_stats->st_atime);
//    strftime(buffer, 37, "Last access %Y-%m-%d %H:%M:%S", tm_info);
//    puts(buffer);
//
//    tm_info = localtime(&file_stats->st_mtime);
//    strftime(buffer, 43, "Last modification %Y-%m-%d %H:%M:%S", tm_info);
//    puts(buffer);
//
//}

//int check_time(const struct stat * file_stats){
//
//    long int diff = diff_day(file_stats->st_atime);
//
//    if(!(atime->undeclared==1 || (atime->sign==1 && diff>atime->value) || (atime->sign==-1 && diff<atime->value) || (atime->sign==0 && diff==atime->value)))
//        return 0;
//
//    diff = diff_day(file_stats->st_mtime);
//
//    if(!(mtime->undeclared==1 || (mtime->sign==1 && diff>mtime->value) || (mtime->sign==-1 && diff<mtime->value) || (mtime->sign==0 && diff==mtime->value)))
//        return 0;
//
//    return 1;
//}

void find(char * path){
    DIR * directory = opendir(path);
    if(directory == NULL){
        printf("There was a problem with loading the directory: %s\n", path);
        return;
    }

    struct dirent * dir_struct = readdir(directory);
    char * name;
    while(dir_struct!=NULL) {
        name = dir_struct->d_name;
        if(strcmp(name, ".")!=0 && strcmp(name, "..")!=0){

            char * file_path = (char *)calloc(strlen(path)+ strlen(name) + 2, sizeof(char));
            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, name);

            struct stat * buf = (struct stat *)calloc(1, sizeof(struct stat));
            lstat(file_path, buf);

//            if(check_time(buf)==1) {
//                file_information(buf, file_path);
//            }

            if(abs(maxdepth)>1 && strcmp(get_file_type(buf->st_mode), "dir")==0){
                pid_t child_pid = fork();
                if(child_pid == 0) {
                    printf("path: %s\n", &file_path[catalog_path_len+1]);
                    printf("process PID: %d\n", getpid());
                    execlp("ls", "ls", "-l", file_path, NULL);
                }
                else {
                    waitpid(child_pid, NULL, 0);
                }

                maxdepth--;
                find(file_path);
            }
            free(buf);
            free(file_path);
        }

        dir_struct = readdir(directory);
    }
    closedir(directory);

}

const int nftw_function(const char* file_path, const struct stat* file_stats, int type_flag, struct FTW* ftw){
    if(type_flag == FTW_D && (maxdepth<0 || ftw->level <= maxdepth) && top_catalog==0) {
        pid_t child_pid = fork();
        if(child_pid == 0) {
            printf("path: %s\n", &file_path[catalog_path_len+1]);
            printf("process PID: %d\n", getpid());
            execlp("ls", "ls", "-l", file_path, NULL);
        }
        else {
            waitpid(child_pid, NULL, 0);
        }
    }
    top_catalog=0;
    return 0;
}

int main(int argc, char ** argv){

    char * path = (char*)calloc(PATH_MAX+1, sizeof(char));
    char * ptr;
    if(argc<=1){
        ptr = realpath(".", path);
    }
    else{
        ptr = realpath(argv[1], path);
    }

    if(ptr==NULL){
        printf("The path is wrong!");
        free(path);
        return 1;
    }

    atime = (struct time_info *)calloc(1, sizeof(struct time_info));
    atime->undeclared = 1;
    mtime = (struct time_info *)calloc(1, sizeof(struct time_info));
    mtime->undeclared = 1;

    int nftw_flag = 0;

    for(int i = 2; i<argc; i++){
        if(strcmp(argv[i], "-atime")==0){
            if(argc<=i+1){
                printf("You haven't specified -atime argument");
                return 1;
            }
            to_time_info(atime, argv[i+1]);
            i += 1;
        }
        else if(strcmp(argv[i], "-mtime")==0) {
            if(argc<=i+1){
                printf("You haven't specified -mtime argument");
                return 1;
            }
            to_time_info(mtime, argv[i+1]);
            i += 1;

        }
        else if(strcmp(argv[i], "-maxdepth")==0) {
            if(argc<=i+1){
                printf("You haven't specified -maxdepth argument");
                return 1;
            }
            maxdepth = atoi(argv[i+1]);
            i += 1;
        }
        else if(strcmp(argv[i], "-nftw")==0){
            nftw_flag = 1;
        }
        else {
            printf("You entered not known argument %s\n", argv[i]);
        }
    }

    catalog_path_len = strlen(path);

    if(nftw_flag == 1){
        nftw(path, nftw_function, 16, FTW_PHYS); //after 16 opened files it will close a file before opening a new one, FTW_PHS - no symbolic links
    }
    else {

            DIR * directory = opendir(path);
            if(directory == NULL){
                printf("There was a problem with loading the directory: %s\n", path);
                return 1;
            }

        if(maxdepth!=0) {
            find(path);
        }
    }


    free(atime);
    free(mtime);
    free(path);

    return 0;
}
