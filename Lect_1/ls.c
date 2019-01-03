#include <stdio.h>
#include <dirent.h>
#include <getopt.h>
#define MAXARGS 10

// output options
struct oution{
    unsigned int all : 1 ;
    unsigned int is1 : 1 ;
}oution;

int   pathc = 0;
char const *paths[MAXARGS-1] = {};

struct option opts[]={
      {"all", 0, 0, 'a'},
      {"almost-all", 0, 0, 'A'},
      {"version", 0, 0, 'v'}
};

void ls(char const* path){
    DIR *dir = opendir(path);
    struct dirent * dirp;
    while ((dirp = readdir(dir)) != NULL){
        if((dirp->d_name)[0] == '.' && oution.all == 0)
        continue;
        fprintf(stdout, "%s%c", dirp->d_name, oution.is1 ? '\n' : '\t');
    }
    if(oution.is1 == 0)
        fprintf(stdout, "\n");
}

int main(int argc, char const **argv)
{
    int ch;
    while((ch = getopt_long(argc,(char * const *)argv,"aAv1",opts,NULL)) != -1){
        switch(ch){
            case 'a': oution.all = 1; break;
            case 'A': oution.all = 0; break;
            case '1': oution.is1 = 1; break;
            case 'v': fprintf(stdout, "too early to get a version number\n"); return 0;
            default:  fprintf(stdout, "usage: ls [-Aav1][--OPTION] [path ...]\n");return 0;
        }
    }
    for(int i = optind; i < argc; ++i)
        paths[pathc++] = argv[i];
    
    if(pathc == 0)
        paths[pathc++] = ".";
    if(pathc == 1)
        ls(paths[0]);
    else{
        for(int i = 0; i < pathc-1; ++i){
            fprintf(stdout,"%s\n",paths[i]);
            ls(paths[i]);
            fprintf(stdout,"\n");
        }
        fprintf(stdout,"%s\n",paths[pathc-1]);
        ls(paths[pathc-1]);
    }
    return 0;
}
