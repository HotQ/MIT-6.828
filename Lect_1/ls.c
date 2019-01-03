#include <stdio.h>
#include <dirent.h>

#define MAXARGS 10
struct option{
    unsigned int isa : 1 ;
    unsigned int is1 : 1 ;
}option;

int main(int argc, char const **argv)
{
    char *path[MAXARGS-1];
    int pathnum;
    
    for(int i = 1; i < MAXARGS; ++i)
        if(argv[i] == 0) break;
        else {
            const char *arg = argv[i];
            switch(arg[0]){
                case '-': 
                switch(arg[1]){
                    case 'a': option.isa = 1; break;
                    case 'A': option.isa = 0; break;
                    case '1': option.is1 = 1; break;
                    default: fprintf(stdout, "usage: ls [-Aa1]\n");return 0;
                }
                break;
                default: fprintf(stdout, "usage: ls [-Aa1]\n");return 0;
            }     
        }

    DIR *dir = opendir(".");
    struct dirent * dirp;
    while ((dirp = readdir(dir)) != NULL){
        if((dirp->d_name)[0] == '.' && option.isa == 0)
        continue;
        fprintf(stdout, "%s%c", dirp->d_name, option.is1 ? '\n' : '\t');
    }

    if(option.is1 == 0)
        fprintf(stdout, "\n");

    return 0;
}
