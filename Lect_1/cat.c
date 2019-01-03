#include <stdio.h>
#include <unistd.h>
#include<fcntl.h>

#define MAXARGS 10

int simple_cat(int infd){
    char buf[ 4096* 128]={0};
    int size = sizeof(buf);
    ssize_t rd_stat = 0;

    while(1){
        rd_stat = read(infd, buf, sizeof(buf));
        if(rd_stat == -1){
            perror("fckup.\n");
            return -1;
        }
        else if(rd_stat == 0){
            return 0;
        }
        else{
            if(write(STDOUT_FILENO, buf, sizeof(buf)) == -1){
                perror("fckup.\n");
                return -1;
            }
        }
    }
}
int main(int argc, char const *argv[])
{
    char const *files[MAXARGS-1];
    int filenum = 0;

    for(int i = 1; i < argc; ++i){
        const char *arg = argv[i];

         switch(arg[0]){
            case '-':
                fprintf(stdout, "usage: cat [no option here][file ...]");
                return -1;
            default:
                files[filenum++] = argv[i];
        }
    }
    for(int i=0; i<filenum; ++i){
        int fd = open(files[i],O_RDONLY);
        simple_cat(fd);
    }
    return 0;
}
